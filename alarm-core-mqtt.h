#pragma once

#include <stdio.h>
#include <string.h>
#include "alarm-core.h"

// Forward-declare the global alarmJSON instance from my_alarm-core.cpp
extern alarmJSON parser;

// Structure to define a topic and its JSON handlers
struct mqttTopicHandler {
    const char* topic;              // MQTT topic to subscribe to
    ALARM_DOMAINS_t domain;         // JSON key that identifies the item (zone, partition, etc.)
    const char* description;        // Description of topic purpose
};

// Define the JSON topic handlers array as a static member of the  class
const mqttTopicHandler mqttTopicHandlers[] = {
    {MQTT_ZONES_CONTROL_TOPIC,          ZONES_CMD,      "Control zones (bypass, tamper, etc.)"},
    {MQTT_PARTITIONS_CONTROL_TOPIC, 	PARTITIONS_CMD, "Control partitions (arm, disarm, etc.)"},
    //{MQTT_OUTPUTS_CONTROL_TOPIC,    JSON_SECTION_PGMS,          &wrapProcessPgmJsonPayload,
    // "Control PGMs (on, off, pulse)"},
    //{MQTT_GLOBAL_OPT_CONTROL_TOPIC, JSON_SECTION_GLOBAL_OPTIONS, &wrapProcessGlobalOptionsJsonPayload,
    // "Set global my_alarm options"}
};
const int MQTT_TOPIC_HANDLER_COUNT = sizeof(mqttTopicHandlers) / sizeof(mqttTopicHandlers[0]);

class MqttProcessor {
public:
    /**
     * @brief Constructor that takes references to an existing JSON parser.
     * @param jsonParser    An existing instance of the alarmJSON parser.
     *      Store references to the objects provided by the caller.
     *      :m_jsonParser(jsonParser)   Initializes member m_jsonParser from that reference.
     *      In your class, m_jsonParser is a reference member.
     *      Reference members must be initialized in the initializer list(cannot be assigned later in constructor body).
     *      So this binds MqttProcessor to an external parser instance without copying it.
     */
    MqttProcessor(alarmJSON& jsonParser) : m_jsonParser(jsonParser)
    {
        printf("MqttProcessor initialized with an existing parser.\n");
    }

    bool processByDomain(ALARM_DOMAINS_t domain, const char* payload, size_t length) {
        return m_jsonParser.processControlJsonPld(payload, length, domain);
    }

    /**
     * @brief Processes an incoming MQTT message.
     * @param topic The MQTT topic on which the message was received.
     * @param payload The message content.
     * @param length The length of the payload.
     */
    bool processControlMessage(const char* topic, const char* payload, size_t length) {
        for (int i = 0; i < MQTT_TOPIC_HANDLER_COUNT; ++i) {
            if (strcmp(topic, mqttTopicHandlers[i].topic) == 0) {
                // Found the handler, call its processor function
                return processByDomain(mqttTopicHandlers[i].domain, payload, length);
            }
        }
        // No handler found for this topic
        LOG_CRITICAL("No handler found for topic: %s\n", topic);
        return false;
    }

  bool processConfigMessage(const char* topic, const char* payload, unsigned int length) {
        if (strstr(topic, "/config")) {
            LOG_INFO("Processing configuration payload...\n");
            int result = m_jsonParser.processConfigJsonPld(payload, length);
            if (result == 0) {
                LOG_INFO("Configuration parsed successfully.\n");
                return true;
            }
            else {
                LOG_ERROR("Error parsing configuration.\n");
                return false;
            }
        }
        return false;
    }

    bool processIncomingMQTTmsg(const char* topic, const char* payload, size_t length) {
        return strstr(topic, "/config")
            ? processConfigMessage(topic, payload, (unsigned int)length)
            : processControlMessage(topic, payload, length);
    }

private:
    alarmJSON& m_jsonParser; // This is now a reference, not an owned instance.
};

