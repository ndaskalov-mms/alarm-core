#pragma once

#include <stdio.h>
#include <string.h>
#include <cstdarg>
#include <stdint.h>
#include "alarm-core.h"

class MqttProcessor; // forward

#define MQTT_RECONNECT_mS 3000

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

#ifndef ARDUINO                                         // Windows/desktop path (Paho C)
#include <MQTTClient.h>

// -------------------- PAHO WRAPPER CONTEXT --------------------
struct PahoMqttContext {
    MQTTClient client = nullptr;
    MqttProcessor* processor = nullptr;

    const char* brokerAddress = nullptr; // e.g. "tcp://192.168.33.201:1883"
    const char* clientId = nullptr;
    const char* user = nullptr;
    const char* pass = nullptr;

    uint32_t lastReconnectAttemptMs = 0;
    bool subscribed = false;
};

// -------------------- PAHO WRAPPER FUNCTIONS --------------------
static bool pahoInit(
    PahoMqttContext& ctx,
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
    return true;
}

static bool pahoConnectAndSubscribe(PahoMqttContext& ctx) {
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
static void pahoService(PahoMqttContext& ctx, uint32_t reconnectIntervalMs)
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

static void pahoDestroy(PahoMqttContext& ctx) {
    if (!ctx.client) return;
    if (MQTTClient_isConnected(ctx.client)) {
        MQTTClient_disconnect(ctx.client, 1000);
    }
    MQTTClient_destroy(&ctx.client);
    ctx.subscribed = false;
}

// Alarm publisher callback-compatible wrapper
static void mqttPublishWrapper(void* context, const char* topic, const char* payload) {
    PahoMqttContext* ctx = static_cast<PahoMqttContext*>(context);
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

#else
// Arduino branch unchanged for now

// Arduino MQTT context placeholder (keeps main() call unchanged: &mqttClient)
struct ArduinoMqttClientContext {};
ArduinoMqttClientContext mqttClient;


// Wrapper for MQTT incoming processing (desktop/test path)
static bool mqttProcessIncomingMsgWrapper(const char* topic, const char* payload, size_t length) {
    return myMqttProcessor.processIncomingMQTTmsg(topic, payload, length);
}

// MQTT publish wrapper function (Arduino path)
static void mqttPublishWrapper(void* context, const char* topic, const char* payload) {
    //
    (void)context; // Replace with real MQTT client cast/publish when integrated
    //
    if (!MQTTclient.connected()) {                    // TODO: enable connection check
        lprintf("MQTT client not connected\n");
        return;
    }
    Serial.print(F("[MQTT] Topic: "));
    Serial.print(topic);
    Serial.print(F(", Payload: "));
    Serial.println(payload);
}
#endif

inline void MqttProcessor::publish2broker(const char* payload, const char* topic, ...) {
    va_list args;
    va_start(args, topic);
    va_end(args);
}
