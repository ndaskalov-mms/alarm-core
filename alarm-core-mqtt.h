#pragma once

#include <stdio.h>
#include <string.h>
#include "alarm-core.h"

// Forward-declare the global alarmJSON instance from my_alarm-core.cpp
extern alarmJSON parser;

// Wrapper function to call the member function from a C-style function pointer
static bool wrapProcessJsonPayload(const char* jsonPayload, size_t length, ALARM_DOMAINS_t domain) {
    // Delegate the call to the processZoneJsonPayload method of the global 'parser' instance
    return parser.processJsonPayload(jsonPayload, length, domain);
}

// Structure to define a topic and its JSON handlers
struct JsonTopicHandler {
    const char* topic;              // MQTT topic to subscribe to
    ALARM_DOMAINS_t domain;         // JSON key that identifies the item (zone, partition, etc.)
    bool (*processor)(const char* jsonPayload, size_t length, ALARM_DOMAINS_t domain); // Function to process the entire JSON
    const char* description;        // Description of topic purpose
};

// Define the JSON topic handlers array as a static member of the  class
const JsonTopicHandler mqttTopicHandlers[] = {
    {MQTT_ZONES_CONTROL_TOPIC,          ZONES_CMD,      &wrapProcessJsonPayload, "Control zones (bypass, tamper, etc.)"},
    {MQTT_PARTITIONS_CONTROL_TOPIC, 	PARTITIONS_CMD, &wrapProcessJsonPayload, "Control partitions (arm, disarm, etc.)"},

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
     *      In your class, m_jsonParser is a reference member : alarmJSON & m_jsonParser;
     *      Reference members must be initialized in the initializer list(cannot be assigned later in constructor body).
     *      So this binds MqttProcessor to an external parser instance without copying it.
     */
    MqttProcessor(alarmJSON& jsonParser) : m_jsonParser(jsonParser)
    {
        printf("MqttProcessor initialized with an existing parser.\n");
    }

    /**
     * @brief Processes an incoming MQTT message.
     * @param topic The MQTT topic on which the message was received.
     * @param payload The message content.
     * @param length The length of the payload.
     */
    bool processConfigMessage(const char* topic, const char* payload, unsigned int length) {
        LOG_DEBUG("MqttProcessor received message on topic: %s\n", topic);

        // This part remains the same, but it now uses the external parser
        // provided in the constructor.

        if (strstr(topic, "/config")) {
            LOG_INFO("Processing configuration payload...\n");
            int result = m_jsonParser.parseConfigJSON(payload, length);
            if (result == 0) {
                LOG_INFO("Configuration parsed successfully.\n");
                return true;
            }
            else {
                LOG_ERROR("Error parsing configuration.\n");
                return false;
            }
        }
        else {
            LOG_WARNING("Topic not relevant for configuration. Ignoring.\n");

        }
    }
    
    bool processMessage(const char* topic, const char* payload, size_t length) {
        for (int i = 0; i < MQTT_TOPIC_HANDLER_COUNT; ++i) {
            if (strcmp(topic, mqttTopicHandlers[i].topic) == 0) {
                // Found the handler, call its processor function
                return mqttTopicHandlers[i].processor(payload, length, mqttTopicHandlers[i].domain);
            }
        }
        // No handler found for this topic
        LOG_CRITICAL("No handler found for topic: %s\n", topic);
        return false;
    }

private:
    alarmJSON& m_jsonParser; // This is now a reference, not an owned instance.
};

