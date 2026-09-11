#pragma once

#include <stdio.h>
#include <string.h>
#include <cstdarg>
#include <stdint.h>
#include "alarm-core.h"

class MqttProcessor; // forward

#define MQTT_RECONNECT_mS 3000

#ifndef ARDUINO
#include <MQTTClient.h>
using MqttClientHandle = MQTTClient;               // Paho C client handle (void*)
#define MQTT_CLIENT_HANDLE_INIT nullptr
#else
#include <PubSubClient.h>                          // or whichever Arduino MQTT lib you use
using MqttClientHandle = PubSubClient*;             // pointer to Arduino MQTT client object
#define MQTT_CLIENT_HANDLE_INIT nullptr
#endif


using RouteHandler = bool (*)(MqttProcessor&, const char* payload, size_t length, ALARM_DOMAINS_t domain);
struct mqttRoute {
    const char* topic;
    size_t topicLen;
    RouteHandler handler;
    ALARM_DOMAINS_t domain;
    const char* description;
};

//
// MqttProcessor class definition. Provides interface to MQTT client. Receives reference to
// alarmJSON class instance and Alarm class instance. This way,  "injecting" the dependencies
// (m_jsonParser, m_alarm) is implemented. What is received on particular MQTT topic is routed to 
// processIncommingMQTTmsg() method, which then calls appropriate JSON processor (processConfigMessage() or
// processControlMessage() -> processByDomain() or other) based on the topic and payload. They call as part of processing 
// alarmJSON object processConfigJsonPld(), processControlJsonPld(), etc
//
class MqttProcessor {
public:
    MqttProcessor(alarmJSON& jsonParser, Alarm& alarm) : m_jsonParser(jsonParser), m_alarm(alarm) {
        printf("MqttProcessor initialized with parser + alarm.\n");
    }

    bool processIncomingMQTTmsg(const char* topic, size_t topicLen, const char* payload, size_t length) {
        if (!topic || !payload) return false;

        for (size_t i = 0; i < MQTT_ROUTE_COUNT; ++i) {
            if (topicEquals(topic, topicLen, mqttRoutes[i].topic, mqttRoutes[i].topicLen)) {
                return mqttRoutes[i].handler(*this, payload, length, mqttRoutes[i].domain);
            }
        }

        LOG_CRITICAL("No handler found for topic (len=%zu)\n", topicLen);
        return false;
    }

    // Backward-compatible overload for null-terminated callers
    bool processIncomingMQTTmsg(const char* topic, const char* payload, size_t length) {
        if (!topic) return false;
        return processIncomingMQTTmsg(topic, strlen(topic), payload, length);
    }

    void publish2broker(const char* payload, const char* topic, ...);

private:
    // Route handlers
    static bool handleDomainConfig(MqttProcessor& self, const char* payload, size_t length, ALARM_DOMAINS_t domain) {
        return self.m_jsonParser.processControlJsonPld(payload, length, domain);
    }

    static bool handleFullConfig(MqttProcessor& self, const char* payload, size_t length, ALARM_DOMAINS_t unused) {
        return (self.m_jsonParser.processConfigJsonPld(payload, length) == 0);
    }

    static bool topicEquals(const char* a, size_t aLen, const char* b, size_t bLen) {
        return a && b && (aLen == bLen) && (memcmp(a, b, aLen) == 0);
    }

    // Routes

    static const mqttRoute mqttRoutes[];
    static const size_t MQTT_ROUTE_COUNT;

    alarmJSON& m_jsonParser;
    Alarm& m_alarm;
};

//
// Route table definition
//

// table of MQTT routes and their corresponding handlers
const mqttRoute MqttProcessor::mqttRoutes[] = {
    { MQTT_FULL_CONFIG_TOPIC,       sizeof(MQTT_FULL_CONFIG_TOPIC) - 1,     &MqttProcessor::handleFullConfig, RESERVED,        "Full alarm config" },
    { MQTT_ZONES_CONTROL_TOPIC,     sizeof(MQTT_ZONES_CONTROL_TOPIC) - 1,   &MqttProcessor::handleDomainConfig,     ZONES_CMD,       "Control zones" },
    { MQTT_PARTITIONS_CONTROL_TOPIC,sizeof(MQTT_PARTITIONS_CONTROL_TOPIC) - 1,&MqttProcessor::handleDomainConfig,   PARTITIONS_CMD,  "Control partitions" },
    { MQTT_GLOBAL_OPT_CONTROL_TOPIC,sizeof(MQTT_GLOBAL_OPT_CONTROL_TOPIC) - 1,&MqttProcessor::handleDomainConfig,   GLOBAL_OPT_CFG,  "Global options config patch" },
};
const size_t MqttProcessor::MQTT_ROUTE_COUNT = sizeof(MqttProcessor::mqttRoutes) / sizeof(MqttProcessor::mqttRoutes[0]);

// -------------------- MQTT CLIENT CONTEXT (shared across platforms) --------------------
struct mqttContext {
    MqttClientHandle client = MQTT_CLIENT_HANDLE_INIT;
    MqttProcessor* processor = nullptr;

    const char* brokerAddress = nullptr;
    const char* clientId = nullptr;
    const char* user = nullptr;
    const char* pass = nullptr;

    uint32_t lastReconnectAttemptMs = 0;
    bool subscribed = false;
};

#ifndef ARDUINO                                         // Windows/desktop path (Paho C)
#include <MQTTClient.h>

// -------------------- PAHO WRAPPER FUNCTIONS --------------------
static bool pahoInit(
    mqttContext& ctx,
    MqttProcessor& processor,
    const char* brokerAddress,
    const char* clientId,
    const char* user,
    const char* pass)
{
    ctx.processor = &processor;
    ctx.brokerAddress = brokerAddress;
    ctx.clientId = clientId;
    ctx.user = user;
    ctx.pass = pass;
    ctx.subscribed = false;
    ctx.lastReconnectAttemptMs = 0;

    const int rc = MQTTClient_create(
        &ctx.client,
        ctx.brokerAddress,
        ctx.clientId,
        MQTTCLIENT_PERSISTENCE_NONE,
        nullptr);

    if (rc != MQTTCLIENT_SUCCESS) {
        lprintf("MQTTClient_create failed rc=%d\n", rc);
        return false;
    }
    else  
        lprintf("MQTTClient_create succeeded \n");
    return true;
}

static bool pahoConnectAndSubscribe(mqttContext& ctx) {
    if (!ctx.client) return false;
    if (MQTTClient_isConnected(ctx.client)) return true;

    MQTTClient_connectOptions connOpts = MQTTClient_connectOptions_initializer;
    connOpts.keepAliveInterval = 20;
    connOpts.cleansession = 1;
    connOpts.username = ctx.user ? ctx.user : "";
    connOpts.password = ctx.pass ? ctx.pass : "";

    int rc = MQTTClient_connect(ctx.client, &connOpts);
    if (rc != MQTTCLIENT_SUCCESS) {
        lprintf("MQTT connect failed rc=%d\n", rc);
        return false;
    }

    bool ok = true;
    ok &= (MQTTClient_subscribe(ctx.client, MQTT_FULL_CONFIG_TOPIC, 1) == MQTTCLIENT_SUCCESS);
    ok &= (MQTTClient_subscribe(ctx.client, MQTT_ZONES_CONTROL_TOPIC, 1) == MQTTCLIENT_SUCCESS);
    ok &= (MQTTClient_subscribe(ctx.client, MQTT_PARTITIONS_CONTROL_TOPIC, 1) == MQTTCLIENT_SUCCESS);
    ok &= (MQTTClient_subscribe(ctx.client, MQTT_GLOBAL_OPT_CONTROL_TOPIC, 1) == MQTTCLIENT_SUCCESS);

    ctx.subscribed = ok;
    if (!ok) {
        lprintf("MQTT subscribe failed\n");
    }
    return ok;
}

// Call periodically (this is your Paho equivalent of loop processing)
static void pahoService(mqttContext& ctx, uint32_t reconnectIntervalMs)
{
    if (!ctx.client || !ctx.processor) return;

    if (!MQTTClient_isConnected(ctx.client)) {
        if ((millis() - ctx.lastReconnectAttemptMs) >= reconnectIntervalMs) {
            ctx.lastReconnectAttemptMs = millis();
            (void)pahoConnectAndSubscribe(ctx);
        }
        return;
    }

    char* topicName = nullptr;
    int topicLen = 0;
    MQTTClient_message* message = nullptr;

    const int rc = MQTTClient_receive(ctx.client, &topicName, &topicLen, &message, 10);

    if (rc == MQTTCLIENT_TOPICNAME_TRUNCATED) {
        lprintf("MQTT receive: topic name truncated, dropping message\n");
        if (message) MQTTClient_freeMessage(&message);
        if (topicName) MQTTClient_free(topicName);
        return;
    }

    if (rc != MQTTCLIENT_SUCCESS || !message) {
        if (message) MQTTClient_freeMessage(&message);
        if (topicName) MQTTClient_free(topicName);
        return;
    }

    const char* payload = static_cast<const char*>(message->payload);
    const size_t payloadLen = (message->payloadlen > 0) ? (size_t)message->payloadlen : 0;
    const size_t safeTopicLen = (topicLen > 0)
        ? (size_t)topicLen
        : ((topicName != nullptr) ? strlen(topicName) : 0);

    (void)ctx.processor->processIncomingMQTTmsg(topicName, safeTopicLen, payload, payloadLen);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
}

static void pahoDestroy(mqttContext& ctx) {
    if (!ctx.client) return;
    if (MQTTClient_isConnected(ctx.client)) {
        MQTTClient_disconnect(ctx.client, 1000);
    }
    MQTTClient_destroy(&ctx.client);
    ctx.subscribed = false;
}

// Alarm publisher callback-compatible wrapper
static void mqttPublishWrapper(void* context, const char* topic, const char* payload) {
    mqttContext* ctx = static_cast<mqttContext*>(context);
    if (!ctx || !ctx->client || !topic || !payload) return;
    if (!MQTTClient_isConnected(ctx->client)) {
        lprintf("MQTT publish skipped, not connected\n");
        return;
    }

    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = (void*)payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = 1;
    pubmsg.retained = 0;

    MQTTClient_deliveryToken token = 0;
    const int rc = MQTTClient_publishMessage(ctx->client, topic, &pubmsg, &token);
    if (rc == MQTTCLIENT_SUCCESS) {
        (void)MQTTClient_waitForCompletion(ctx->client, token, 1000);
    }
    else {
        lprintf("MQTT publish failed rc=%d\n", rc);
    }
}

#else  // ARDUINO path (ESP32/ESP8266/etc. using PubSubClient)

#include <WiFiClient.h>   // or <WiFiClientSecure.h> for TLS
// PubSubClient.h already included above via MqttClientHandle alias section

// Underlying network transport used by PubSubClient
static WiFiClient g_wifiNetClient;

// PubSubClient requires a free function callback (no captures / no member fn),
// so we route through a single global mqttContext pointer.
static mqttContext* g_activeMqttCtx = nullptr;

// -------------------- PUBSUBCLIENT MESSAGE CALLBACK --------------------
static void arduinoMqttCallback(char* topic, byte* payload, unsigned int length) {
    if (!g_activeMqttCtx || !g_activeMqttCtx->processor) return;

    // PubSubClient payload is NOT null-terminated - treat as raw bytes
    g_activeMqttCtx->processor->processIncomingMQTTmsg(
        topic,
        strlen(topic),
        reinterpret_cast<const char*>(payload),
        static_cast<size_t>(length)
    );
}

// -------------------- ARDUINO MQTT WRAPPER FUNCTIONS --------------------
static bool arduinoMqttSubscribeAll(mqttContext& ctx) {
    bool ok = true;
    ok &= ctx.client->subscribe(MQTT_FULL_CONFIG_TOPIC, 1);
    ok &= ctx.client->subscribe(MQTT_ZONES_CONTROL_TOPIC, 1);
    ok &= ctx.client->subscribe(MQTT_PARTITIONS_CONTROL_TOPIC, 1);
    ok &= ctx.client->subscribe(MQTT_GLOBAL_OPT_CONTROL_TOPIC, 1);

    ctx.subscribed = ok;
    if (!ok) {
        lprintf("MQTT subscribe failed\n");
    }
    return ok;
}

static bool arduinoMqttConnectAndSubscribe(mqttContext& ctx) {
    if (!ctx.client) return false;
    if (ctx.client->connected()) return true;

    const char* user = ctx.user ? ctx.user : nullptr;
    const char* pass = ctx.pass ? ctx.pass : nullptr;

    bool connected = (user && user[0])
        ? ctx.client->connect(ctx.clientId, user, pass)
        : ctx.client->connect(ctx.clientId);

    if (!connected) {
        lprintf("MQTT connect failed, state=%d\n", ctx.client->state());
        return false;
    }

    return arduinoMqttSubscribeAll(ctx);
}

// brokerAddress expected as plain host (e.g. "192.168.33.201"), brokerPort separately.
// If you keep a single "tcp://host:port" string like the Paho path, parse it before calling this.
static bool arduinoMqttInit(
    mqttContext& ctx,
    MqttProcessor& processor,
    const char* brokerHost,
    uint16_t brokerPort,
    const char* clientId,
    const char* user,
    const char* pass)
{
    static PubSubClient staticClient(g_wifiNetClient);  // one instance is enough for a single broker

    ctx.client = &staticClient;
    ctx.processor = &processor;
    ctx.brokerAddress = brokerHost;
    ctx.clientId = clientId;
    ctx.user = user;
    ctx.pass = pass;
    ctx.subscribed = false;
    ctx.lastReconnectAttemptMs = 0;

    g_activeMqttCtx = &ctx;  // needed by the free-function callback

    ctx.client->setServer(brokerHost, brokerPort);
    ctx.client->setCallback(arduinoMqttCallback);
    ctx.client->setBufferSize(1024);   // increase if your JSON payloads are large; default is 256

    return true;
}

// Call this frequently from loop()
static void arduinoMqttService(mqttContext& ctx, uint32_t reconnectIntervalMs) {
    if (!ctx.client) return;

    if (!ctx.client->connected()) {
        const uint32_t now = millis();
        if ((now - ctx.lastReconnectAttemptMs) >= reconnectIntervalMs) {
            ctx.lastReconnectAttemptMs = now;
            (void)arduinoMqttConnectAndSubscribe(ctx);
        }
        return;
    }

    ctx.client->loop();  // processes incoming messages, triggers arduinoMqttCallback()
}

static void arduinoMqttDestroy(mqttContext& ctx) {
    if (!ctx.client) return;
    ctx.client->disconnect();
    ctx.client = nullptr;
    if (g_activeMqttCtx == &ctx) {
        g_activeMqttCtx = nullptr;
    }
}

// Alarm publisher callback-compatible wrapper
static void mqttPublishWrapper(void* context, const char* topic, const char* payload) {
    mqttContext* ctx = static_cast<mqttContext*>(context);
    if (!ctx || !ctx->client || !topic || !payload) return;

    if (!ctx->client->connected()) {
        lprintf("MQTT publish skipped, not connected\n");
        return;
    }

    if (!ctx->client->publish(topic, payload)) {
        lprintf("MQTT publish failed\n");
    }
    Serial.print(F("[MQTT] Topic: "));
    Serial.print(topic);
    Serial.print(F(", Payload: "));
    Serial.println(payload);

}

#endif

// -------------------- PLATFORM-NEUTRAL ALIASES --------------------
// Allows alarm-core.cpp (and other callers) to use one set of names
// regardless of target platform.
#define MQTT_INIT      mqttInitFromUrl   // normalizes URL parsing internally

#ifndef ARDUINO
    #define MQTT_SERVICE   pahoService
    #define MQTT_DESTROY   pahoDestroy
#else
    #define MQTT_SERVICE   arduinoMqttService
    #define MQTT_DESTROY   arduinoMqttDestroy
#endif

inline void MqttProcessor::publish2broker(const char* payload, const char* topic, ...) {
    va_list args;
    va_start(args, topic);
    va_end(args);
}

// -------------------- BROKER URL PARSING (platform-neutral) --------------------
// Accepts URLs like "tcp://192.168.33.201:1883" or "192.168.33.201:1883" or
// just "192.168.33.201" (falls back to defaultPort).
struct MqttBrokerUrl {
    char     host[64];
    uint16_t port;
};

static bool parseMqttBrokerUrl(const char* url, MqttBrokerUrl& out, uint16_t defaultPort = 1883) {
    out.host[0] = '\0';
    out.port = defaultPort;
    if (!url || !*url) return false;

    // Skip scheme prefix ("tcp://", "ssl://", "mqtt://", etc.) if present
    const char* p = url;
    const char* schemeSep = strstr(p, "://");
    if (schemeSep) {
        p = schemeSep + 3;
    }

    const char* colon = strchr(p, ':');
    size_t hostLen = colon ? (size_t)(colon - p) : strlen(p);

    if (hostLen == 0 || hostLen >= sizeof(out.host)) {
        return false;
    }

    memcpy(out.host, p, hostLen);
    out.host[hostLen] = '\0';

    if (colon) {
        int parsedPort = atoi(colon + 1);
        if (parsedPort > 0 && parsedPort <= 65535) {
            out.port = static_cast<uint16_t>(parsedPort);
        }
    }

    return true;
}

// -------------------- UNIFORM MQTT_INIT WRAPPER --------------------
// Normalizes the signature difference between pahoInit (single URL string)
// and arduinoMqttInit (host + port). Both platforms are called the same way
// from alarm-core.cpp: MQTT_INIT(ctx, processor, brokerUrl, clientId, user, pass)
static bool mqttInitFromUrl(
    mqttContext& ctx,
    MqttProcessor& processor,
    const char* brokerUrl,
    const char* clientId,
    const char* user,
    const char* pass)
{
#ifndef ARDUINO
    return pahoInit(ctx, processor, brokerUrl, clientId, user, pass);
#else
    MqttBrokerUrl parsed;
    if (!parseMqttBrokerUrl(brokerUrl, parsed)) {
        lprintf("Invalid MQTT broker URL: %s\n", brokerUrl ? brokerUrl : "(null)");
        return false;
    }
    return arduinoMqttInit(ctx, processor, parsed.host, parsed.port, clientId, user, pass);
#endif
}
