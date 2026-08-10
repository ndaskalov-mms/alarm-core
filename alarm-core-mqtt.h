#pragma once

#include <stdio.h>
#include <string.h>
#include <cstdarg>
#include "alarm-core.h"

class MqttProcessor; // forward

#ifndef ARDUINO                 // Windows/desktop test path

// Dummy MQTT client object
//struct DummyMqttClient {};
//DummyMqttClient mqttClient;

// MQTT publish wrapper function
//static void mqttPublishWrapper(void* context, const char* topic, const char* payload) {
//    // 
//    // Example implementation: just print the topic and payload
//    printf("[MQTT] Topic: %s, Payload: %s\n", topic, payload);
//}

#else

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


inline void MqttProcessor::publish2broker(const char* payload, const char* topic, ...) {
    va_list args;
    va_start(args, topic);
    va_end(args);
}
