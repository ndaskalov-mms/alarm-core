#pragma once

#include "alarm-core-public-defs.h"
#include <string.h>
#include <stdio.h>

class Alarm;

// Structure to define a JSON key handler
struct JsonKeyHandler {
    const char* key;                // JSON key name
    bool (*handler)(Alarm& alarm, const char* value, int itemIndex, void* context); // Function to handle this key's value
    const char* description;        // Description of what this key does
};

// Structure to define a topic and its JSON handlers
struct JsonTopicHandler {
    const char* topic;              // MQTT topic to subscribe to
    const char* itemKey;            // JSON key that identifies the item (zone, partition, etc.)
    bool (*processor)(Alarm& alarm, const char* jsonPayload, size_t length); // Function to process the entire JSON
    const char* description;        // Description of topic purpose
};

// Function prototypes for JSON value handlers
bool handleZoneAction(Alarm& alarm, const char* value, int zoneIndex, void* context);
bool handlePartitionAction(Alarm& alarm, const char* value, int partitionIndex, void* context);
bool handlePgmAction(Alarm& alarm, const char* value, int pgmIndex, void* context);
bool handleGlobalOptionValue(Alarm& alarm, const char* value, int optionIndex, void* context);

// Function prototypes for JSON payload processors
bool processZoneJson(Alarm& alarm, const char* jsonPayload, size_t length);
bool processPartitionJson(Alarm& alarm, const char* jsonPayload, size_t length);
bool processPgmJson(Alarm& alarm, const char* jsonPayload, size_t length);
bool processGlobalOptionsJson(Alarm& alarm, const char* jsonPayload, size_t length);

// Define the key handlers for each topic type
JsonKeyHandler zoneKeyHandlers[] = {
    {"action", handleZoneAction, "Action to perform on zone (bypass, clear_bypass, tamper, open, close, anti-mask)"},
    {"value", nullptr, "Reserved for future use"}
};

JsonKeyHandler partitionKeyHandlers[] = {
    {"action", handlePartitionAction, "Action to perform on partition (arm, disarm, etc.)"},
    {"mode", nullptr, "Arm mode (regular, force, stay, instant)"}
};

JsonKeyHandler pgmKeyHandlers[] = {
    {"action", handlePgmAction, "Action to perform on PGM (on, off, pulse)"},
    {"duration", nullptr, "Duration for pulse action in milliseconds"}
};

JsonKeyHandler globalOptionKeyHandlers[] = {
    {"value", handleGlobalOptionValue, "Value to set for the global option"}
};

// Define the array of topics and their handlers
JsonTopicHandler jsonTopicHandlers[] = {
    {"/alarm/zones/control", "zone", processZoneJson, 
     "Control zones (bypass, tamper, etc.)"},
    
    {"/alarm/partitions/control", "partition", processPartitionJson, 
     "Control partitions (arm, disarm, etc.)"},
    
    {"/alarm/pgms/control", "pgm", processPgmJson, 
     "Control PGMs (on, off, pulse)"},
    
    {"/alarm/global/options", "option", processGlobalOptionsJson, 
     "Set global alarm options"}
};

// Number of topic handlers
const int JSON_TOPIC_HANDLER_COUNT = sizeof(jsonTopicHandlers)/sizeof(jsonTopicHandlers[0]);

// Function to subscribe to all topics
bool subscribeToJsonTopics(Alarm& alarm) {
    bool success = true;
    
    for (int i = 0; i < JSON_TOPIC_HANDLER_COUNT; i++) {
        printf("Subscribing to: %s\n", jsonTopicHandlers[i].topic);
        
        // Replace this with your actual MQTT subscribe mechanism
        bool subscribed = true; // Your subscription function here
        
        if (!subscribed) {
            printf("Failed to subscribe to topic: %s\n", jsonTopicHandlers[i].topic);
            success = false;
        }
    }
    
    return success;
}

// MQTT callback function to process JSON payloads
void mqttJsonCallback(Alarm& alarm, char* topic, byte* payload, unsigned int length) {
    printf("Received message on topic: %s\n", topic);
    printf("Payload: %.*s\n", length, payload);
    
    // Find the handler for this topic
    for (int i = 0; i < JSON_TOPIC_HANDLER_COUNT; i++) {
        if (strcmp(topic, jsonTopicHandlers[i].topic) == 0) {
            // Call the JSON processor for this topic
            if (jsonTopicHandlers[i].processor) {
                bool result = jsonTopicHandlers[i].processor(alarm, (const char*)payload, length);
                if (!result) {
                    printf("Error processing JSON for topic %s\n", topic);
                }
            } else {
                printf("No processor defined for topic: %s\n", topic);
            }
            return;
        }
    }
    
    printf("No handler found for topic: %s\n", topic);
}

// Implementation of JSON processors
bool processZoneJson(Alarm& alarm, const char* jsonPayload, size_t length) {
    // In a real implementation, use a proper JSON parser
    // For this example, we'll use a simple approach to extract values
    
    // Extract zone name and action from JSON
    char zoneName[32] = {0};
    char action[32] = {0};
    
    // Simple JSON parsing (replace with proper parser in production)
    if (sscanf(jsonPayload, "{\"zone\":\"%31[^\"]\",\"action\":\"%31[^\"]\"}", zoneName, action) == 2) {
        int zoneIndex = alarm.getZoneIndex(zoneName);
        if (zoneIndex < 0) {
            printf("Zone not found: %s\n", zoneName);
            return false;
        }
        
        // Process the action
        return handleZoneAction(alarm, action, zoneIndex, nullptr);
    }
    
    printf("Invalid JSON format for zone control\n");
    return false;
}

bool processPartitionJson(Alarm& alarm, const char* jsonPayload, size_t length) {
    // Extract partition name and action from JSON
    char partitionName[32] = {0};
    char action[32] = {0};
    
    // Simple JSON parsing (replace with proper parser in production)
    if (sscanf(jsonPayload, "{\"partition\":\"%31[^\"]\",\"action\":\"%31[^\"]\"}", partitionName, action) == 2) {
        int partitionIndex = alarm.getPartitionIndex(partitionName);
        if (partitionIndex < 0) {
            printf("Partition not found: %s\n", partitionName);
            return false;
        }
        
        // Process the action
        return handlePartitionAction(alarm, action, partitionIndex, nullptr);
    }
    
    printf("Invalid JSON format for partition control\n");
    return false;
}

bool processPgmJson(Alarm& alarm, const char* jsonPayload, size_t length) {
    // Extract PGM name and action from JSON
    char pgmName[32] = {0};
    char action[32] = {0};
    
    // Simple JSON parsing (replace with proper parser in production)
    if (sscanf(jsonPayload, "{\"pgm\":\"%31[^\"]\",\"action\":\"%31[^\"]\"}", pgmName, action) == 2) {
        int pgmIndex = alarm.getPgmIndex(pgmName);
        if (pgmIndex < 0) {
            printf("PGM not found: %s\n", pgmName);
            return false;
        }
        
        // Process the action
        return handlePgmAction(alarm, action, pgmIndex, nullptr);
    }
    
    printf("Invalid JSON format for PGM control\n");
    return false;
}

bool processGlobalOptionsJson(Alarm& alarm, const char* jsonPayload, size_t length) {
    // Extract option name and value from JSON
    char optionName[32] = {0};
    char value[32] = {0};
    
    // Simple JSON parsing (replace with proper parser in production)
    if (sscanf(jsonPayload, "{\"option\":\"%31[^\"]\",\"value\":\"%31[^\"]\"}", optionName, value) == 2) {
        // Process global option
        return alarm.setGlobalOptions(optionName, value);
    }
    
    printf("Invalid JSON format for global options\n");
    return false;
}

// Implementation of handlers for specific JSON keys
bool handleZoneAction(Alarm& alarm, const char* value, int zoneIndex, void* context) {
    printf("Processing zone action: %s for zone index %d\n", value, zoneIndex);
    
    unsigned int action = 0;
    if (strcmp(value, "bypass") == 0) action = ZONE_BYPASS_CMD;
    else if (strcmp(value, "clear_bypass") == 0) action = ZONE_UNBYPASS_CMD;
    else if (strcmp(value, "tamper") == 0) action = ZONE_TAMPER_CMD;
    else if (strcmp(value, "close") == 0) action = ZONE_CLOSE_CMD;
    else if (strcmp(value, "open") == 0) action = ZONE_OPEN_CMD;
    else if (strcmp(value, "anti-mask") == 0) action = ZONE_AMASK_CMD;
    else {
        printf("Unknown zone action: %s\n", value);
        return false;
    }
    
    // Call the zone modification function with the appropriate action
    alarm.modifyZn(&zoneIndex, &action, nullptr);
    return true;
}

bool handlePartitionAction(Alarm& alarm, const char* value, int partitionIndex, void* context) {
    printf("Processing partition action: %s for partition index %d\n", value, partitionIndex);
    
    ARM_METHODS_t action;
    if (strcmp(value, "disarm") == 0) action = DISARM;
    else if (strcmp(value, "arm") == 0) action = REGULAR_ARM;
    else if (strcmp(value, "arm_force") == 0) action = FORCE_ARM;
    else if (strcmp(value, "arm_stay") == 0) action = STAY_ARM;
    else if (strcmp(value, "arm_instant") == 0) action = INSTANT_ARM;
    else {
        printf("Unknown partition action: %s\n", value);
        return false;
    }
    
    // Set the partition target arm status
    return alarm.setPartitionTarget(partitionIndex, action);
}

bool handlePgmAction(Alarm& alarm, const char* value, int pgmIndex, void* context) {
    printf("Processing PGM action: %s for PGM index %d\n", value, pgmIndex);
    
    unsigned int action = 0;
    if (strcmp(value, "on") == 0) action = PGM_ON;
    else if (strcmp(value, "off") == 0) action = PGM_OFF;
    else if (strcmp(value, "pulse") == 0) action = PGM_PULSE;
    else {
        printf("Unknown PGM action: %s\n", value);
        return false;
    }
    
    // Call the PGM modification function with the appropriate action
    alarm.modifyPgm(&pgmIndex, &action, nullptr);
    return true;
}

bool handleGlobalOptionValue(Alarm& alarm, const char* value, int optionIndex, void* context) {
    // The option name should be provided in the context
    const char* optionName = (const char*)context;
    if (!optionName) {
        printf("Option name not provided in context\n");
        return false;
    }
    
    printf("Setting global option %s to %s\n", optionName, value);
    return alarm.setGlobalOptions(optionName, value);
}