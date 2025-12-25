#pragma once

#include <stdio.h>
#include <string.h>
#include "alarm-core.h"

// Forward-declare the global alarmJSON instance from alarm-core.cpp
extern alarmJSON parser;

// Wrapper function to call the member function from a C-style function pointer
static bool wrapProcessZoneJsonPayload(const char* jsonPayload, size_t length) {
    // Delegate the call to the processZoneJsonPayload method of the global 'parser' instance
    return parser.processZoneJsonPayload(jsonPayload, length);
}

// Structure to define a topic and its JSON handlers
struct JsonTopicHandler {
    const char* topic;              // MQTT topic to subscribe to
    const char* itemKey;            // JSON key that identifies the item (zone, partition, etc.)
    bool (*processor)(const char* jsonPayload, size_t length); // Function to process the entire JSON
    const char* description;        // Description of topic purpose
};

// Define the JSON topic handlers array as a static member of the  class
const JsonTopicHandler mqttTopicHandlers[] = {
    {MQTT_ZONES_CONTROL_TOPIC,      JSON_SECTION_ZONES,         &wrapProcessZoneJsonPayload,
     "Control zones (bypass, tamper, etc.)"},

    //{MQTT_PARTITIONS_CONTROL_TOPIC, JSON_SECTION_PARTITIONS,    &wrapProcessPartitionJsonPayload,
    // "Control partitions (arm, disarm, etc.)"},

    //{MQTT_OUTPUTS_CONTROL_TOPIC,    JSON_SECTION_PGMS,          &wrapProcessPgmJsonPayload,
    // "Control PGMs (on, off, pulse)"},

    //{MQTT_GLOBAL_OPT_CONTROL_TOPIC, JSON_SECTION_GLOBAL_OPTIONS, &wrapProcessGlobalOptionsJsonPayload,
    // "Set global alarm options"}
};

const int MQTT_TOPIC_HANDLER_COUNT = sizeof(mqttTopicHandlers) / sizeof(mqttTopicHandlers[0]);
class MqttProcessor {
public:
    /**
     * @brief Constructor that takes references to the Alarm instance and an existing JSON parser.
     * @param alarm The core Alarm object.
     * @param jsonParser An existing instance of the alarmJSON parser.
     */
    //MqttProcessor(Alarm& alarm, alarmJSON& jsonParser)
    //    // Store references to the objects provided by the caller.
    //    : m_alarm(alarm), m_jsonParser(jsonParser)
    MqttProcessor(alarmJSON& jsonParser)
        // Store references to the objects provided by the caller.
        : m_jsonParser(jsonParser)
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
        printf("MqttProcessor received message on topic: %s\n", topic);

        // This part remains the same, but it now uses the external parser
        // provided in the constructor.
        char* jsonBuffer = new char[length + 1];
        if (!jsonBuffer) {
            printf("Error: Failed to allocate memory for JSON buffer.\n");
            return false;
        }
        memcpy(jsonBuffer, payload, length);
        jsonBuffer[length] = '\0';

        if (strstr(topic, "/config")) {
            printf("Processing configuration payload...\n");
            int result = m_jsonParser.parseConfigJSON(jsonBuffer);
            if (result == 0) {
                printf("Configuration parsed successfully.\n");
                return true;
            }
            else {
                printf("Error parsing configuration.\n");
                return false;
            }
        }
        else {
            printf("Topic not relevant for configuration. Ignoring.\n");

        }

        delete[] jsonBuffer;
    }

    bool processMessage(const char* topic, const char* payload, size_t length) {
        for (int i = 0; i < MQTT_TOPIC_HANDLER_COUNT; ++i) {
            if (strcmp(topic, mqttTopicHandlers[i].topic) == 0) {
                // Found the handler, call its processor function
                return mqttTopicHandlers[i].processor(payload, length);
            }
        }
        // No handler found for this topic
        printf("No handler found for topic: %s\n", topic);
        return false;
    }

    //// Process a JSON message by finding the matching topic handler
    //bool processMqttMessage(const char* topic, const char* payload, size_t length) {
    //    // Find the handler for this topic
    //    for (int i = 0; i < MQTT_TOPIC_HANDLER_COUNT; i++) {
    //        if (strcmp(topic, mqttTopicHandlers[i].topic) == 0) {
    //            // Call the JSON processor for this topic
    //            if (mqttTopicHandlers[i].processor) {
    //                return mqttTopicHandlers[i].processor(m_jsonParser, payload, length);
    //            }
    //            else {
    //                printf("No processor defined for topic: %s\n", topic);
    //                //ErrWrite(LOG_ERR_WARNING, "No processor defined for topic: %s\n", topic);
    //                return false;
    //            }
    //        }
    //    }
    //    printf("No handler found for topic: %s\n", topic);
    //    //ErrWrite(LOG_ERR_WARNING, "No handler found for topic: %s\n", topic);
    //    return false;
    //}

private:
    //Alarm& m_alarm;
    alarmJSON& m_jsonParser; // This is now a reference, not an owned instance.
};

