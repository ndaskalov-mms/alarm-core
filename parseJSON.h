// parseJSON.h
#pragma once

#include <string>
#include <iostream>
#include "json.hpp"
#include "AlarmClass.h"
#include "parserClassHelpers.h"
#include "debug.h"

// Function to extract zone fields from JSON using the zoneTags structure
int extractZoneFields(const nlohmann::json& zoneJson, ALARM_ZONE& zone) {
    // Initialize zone with zeros
    std::memset(&zone, 0, sizeof(ALARM_ZONE));
    zone.valid = true; // Set valid to true by default

    // Use a byte array to track which tags were found in the JSON
    byte tagFound[ZONE_TAGS_CNT] = { 0 };
    int missingTagCount = 0;

    // Go through each tag in zoneTags array
    for (size_t i = 0; i < ZONE_TAGS_CNT; i++) {
        const char* tagName = zoneTags[i].keyStr;
        byte* targetAddress = reinterpret_cast<byte*>(&zone) + zoneTags[i].patchOffset;
        int length = zoneTags[i].patchLen;

        // Check if the tag exists in JSON
        if (zoneJson.contains(tagName)) {
            tagFound[i] = 1;  // Mark this tag as found

            // Get the patch callback function for this tag
            auto patchCallback = zoneTags[i].patchCallBack;

            try {
                // Define valueStr outside the if/else chain
                std::string valueStr;

                // Handle different types of JSON values
                if (zoneJson[tagName].is_string()) {
                    valueStr = zoneJson[tagName];
                }
                else if (zoneJson[tagName].is_number()) {
                    valueStr = std::to_string(zoneJson[tagName].get<int>());
                }
                else if (zoneJson[tagName].is_boolean()) {
                    valueStr = zoneJson[tagName].get<bool>() ? "true" : "false";
                }
                else {
                    // Unsupported type, skip this field
                    continue;
                }

                // Invoke the patchCallback and check its return value
                if (!patchCallback(targetAddress, 0, length, valueStr.c_str())) {
                    GlobalDebugLogger(LOG_ERR_CRITICAL, "Error: Failed to patch field %s with value %s\n",
                        tagName, valueStr.c_str());
                    zone.valid = false; // Mark zone as invalid
                    return 0; // Indicate failure
                }
            }
            catch (const std::exception& e) {
                GlobalDebugLogger(LOG_ERR_CRITICAL, "Error extracting field %s: %s\n",
                    tagName, e.what());
                zone.valid = false; // Mark zone as invalid
                return 0; // Indicate failure
            }
        }
        else {
            // Tag not found in JSON
            missingTagCount++;
        }
    }

    // Issue warnings for missing tags
    if (missingTagCount > 0) {
        GlobalDebugLogger(LOG_ERR_WARNING, "Zone '%s' (ID: %d) is missing %d tag(s):\n",
            zone.zoneName, static_cast<int>(zone.zoneID), missingTagCount);

        for (size_t i = 0; i < ZONE_TAGS_CNT; i++) {
            if (tagFound[i] == 0) {
                GlobalDebugLogger(LOG_ERR_WARNING, "   - Missing tag: '%s'\n", zoneTags[i].keyStr);
            }
        }
    }

    return 1; // Indicate success
}


// Function to extract global options fields from JSON
int extractGlobalOptionsFields(const nlohmann::json& optionsJson, ALARM_GLOBAL_OPTS_t& globalOptions) {
    // Initialize global options with zeros
    std::memset(&globalOptions, 0, sizeof(ALARM_GLOBAL_OPTS_t));

    // Use a byte array to track which tags were found in the JSON
    byte tagFound[GLOBAL_OPTIONS_TAGS_CNT] = { 0 };
    int missingTagCount = 0;

    // Go through each tag in globalOptionsTags array
    for (size_t i = 0; i < GLOBAL_OPTIONS_TAGS_CNT; i++) {
        const char* tagName = gOptsTags[i].keyStr;
        byte* targetAddress = reinterpret_cast<byte*>(&globalOptions) + gOptsTags[i].patchOffset;
        int length = gOptsTags[i].patchLen;

        // Check if the tag exists in JSON
        if (optionsJson.contains(tagName)) {
            tagFound[i] = 1;  // Mark this tag as found

            // Get the patch callback function for this tag
            auto patchCallback = gOptsTags[i].patchCallBack;

            try {
                // Define valueStr outside the if/else chain
                std::string valueStr;

                // Handle different types of JSON values
                if (optionsJson[tagName].is_string()) {
                    valueStr = optionsJson[tagName];
                }
                else if (optionsJson[tagName].is_number()) {
                    valueStr = std::to_string(optionsJson[tagName].get<int>());
                }
                else if (optionsJson[tagName].is_boolean()) {
                    valueStr = optionsJson[tagName].get<bool>() ? "true" : "false";
                }
                else {
                    // Unsupported type, skip this field
                    continue;
                }

                // Invoke the patchCallback and check its return value
                if (!patchCallback(targetAddress, 0, length, valueStr.c_str())) {
                    GlobalDebugLogger(LOG_ERR_CRITICAL, "Error: Failed to patch field %s with value %s\n",
                        tagName, valueStr.c_str());
                    return 0; // Indicate failure
                }
            }
            catch (const std::exception& e) {
                GlobalDebugLogger(LOG_ERR_CRITICAL, "Error extracting field %s: %s\n",
                    tagName, e.what());
                return 0; // Indicate failure
            }
        }
        else {
            // Tag not found in JSON
            missingTagCount++;
        }
    }

    // Issue warnings for missing tags
    if (missingTagCount > 0) {
        GlobalDebugLogger(LOG_ERR_WARNING, "Global options is missing %d tag(s):\n", missingTagCount);

        for (size_t i = 0; i < GLOBAL_OPTIONS_TAGS_CNT; i++) {
            if (tagFound[i] == 0) {
                GlobalDebugLogger(LOG_ERR_WARNING, "   - Missing tag: '%s'\n", gOptsTags[i].keyStr);
            }
        }
    }
    return 1; // Indicate success
}


// Function to extract partition fields from JSON
int extractPartitionFields(const nlohmann::json& partitionJson, ALARM_PARTITION_t& partition) {
    // Initialize partition with zeros
    std::memset(&partition, 0, sizeof(ALARM_PARTITION_t));
    partition.valid = true; // Set valid to true by default

    // Use a byte array to track which tags were found in the JSON
    byte tagFound[PARTITION_TAGS_CNT] = { 0 };
    int missingTagCount = 0;

    // Go through each tag in partitionTags array
    for (size_t i = 0; i < PARTITION_TAGS_CNT; i++) {
        const char* tagName = partitionTags[i].keyStr;
        byte* targetAddress = reinterpret_cast<byte*>(&partition) + partitionTags[i].patchOffset;
        int length = partitionTags[i].patchLen;

        // Check if the tag exists in JSON
        if (partitionJson.contains(tagName)) {
            tagFound[i] = 1;  // Mark this tag as found

            // Get the patch callback function for this tag
            auto patchCallback = partitionTags[i].patchCallBack;

            try {
                // Define valueStr outside the if/else chain
                std::string valueStr;

                // Handle different types of JSON values
                if (partitionJson[tagName].is_string()) {
                    valueStr = partitionJson[tagName];
                }
                else if (partitionJson[tagName].is_number()) {
                    valueStr = std::to_string(partitionJson[tagName].get<int>());
                }
                else if (partitionJson[tagName].is_boolean()) {
                    valueStr = partitionJson[tagName].get<bool>() ? "true" : "false";
                }
                else {
                    // Unsupported type, skip this field
                    continue;
                }

                // Invoke the patchCallback and check its return value
                if (!patchCallback(targetAddress, 0, length, valueStr.c_str())) {
                    GlobalDebugLogger(LOG_ERR_CRITICAL, "Error: Failed to patch field %s with value %s\n",
                        tagName, valueStr.c_str());
                    partition.valid = false; // Mark partition as invalid
                    return 0; // Indicate failure
                }
            }
            catch (const std::exception& e) {
                GlobalDebugLogger(LOG_ERR_CRITICAL, "Error extracting field %s: %s\n",
                    tagName, e.what());
                partition.valid = false; // Mark partition as invalid
                return 0; // Indicate failure
            }
        }
        else {
            // Tag not found in JSON
            missingTagCount++;
        }
    }

    // Print the partition configuration
    //Alarm::printConfigData(partitionTags, PARTITION_TAGS_CNT, (byte*)&partition, PRTCLASS_ALL);

    // Issue warnings for missing tags
    if (missingTagCount > 0) {
        GlobalDebugLogger(LOG_ERR_WARNING, "Partition '%s' (ID: %d) is missing %d tag(s):\n",
            partition.partitionName, static_cast<int>(partition.partIdx), missingTagCount);

        for (size_t i = 0; i < PARTITION_TAGS_CNT; i++) {
            if (tagFound[i] == 0) {
                GlobalDebugLogger(LOG_ERR_WARNING, "   - Missing tag: '%s'\n", partitionTags[i].keyStr);
            }
        }
    }

    return 1; // Indicate success
}


// Function to parse JSON file and extract data into alarm system structures
bool parseJsonConfig(const std::string& filename, Alarm& alarm) {
    // Open the JSON file
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        GlobalDebugLogger(LOG_ERR_CRITICAL, "Error: Failed to open %s\n", filename.c_str());
        return false;
    }

    // Parse the JSON file
    nlohmann::json jsonData;
    try {
        inputFile >> jsonData;
    }
    catch (const nlohmann::json::exception& e) {
        GlobalDebugLogger(LOG_ERR_CRITICAL, "Error parsing JSON: %s\n", e.what());
        return false;
    }

    // Process zones
    if (jsonData.contains("zones") && jsonData["zones"].is_array()) {
        for (const auto& zoneJson : jsonData["zones"]) {
            ALARM_ZONE zone = {};

            // Extract all zone fields using the zoneTags array
            if (extractZoneFields(zoneJson, zone) && zone.valid) {
                GlobalDebugLogger(LOG_ERR_DEBUG, "Extracted zone: %s (ID: %d)\n", zone.zoneName, static_cast<int>(zone.zoneID));
                // Add zone to alarm system
                alarm.addZone(zone);
            }
        }
    }

    // Process partitions
    if (jsonData.contains("partitions") && jsonData["partitions"].is_array()) {
        for (const auto& partitionJson : jsonData["partitions"]) {
            ALARM_PARTITION_t partition = {};

            // Extract all partition fields using the partitionTags array
            if (extractPartitionFields(partitionJson, partition) && partition.valid) {
                GlobalDebugLogger(LOG_ERR_DEBUG, "Extracted partition: %s (Index: %d)\n", partition.partitionName, static_cast<int>(partition.partIdx));
                // Add partition to alarm system if needed
                alarm.addPartition(partition);
            }
        }
    }

    // Process global options
    if (jsonData.contains("globalOptions") && jsonData["globalOptions"].is_object()) {
        ALARM_GLOBAL_OPTS_t globalOptions = {};

        // Extract all global options fields using the globalOptionsTags array
        if (extractGlobalOptionsFields(jsonData["globalOptions"], globalOptions)) {
            GlobalDebugLogger(LOG_ERR_DEBUG, "Extracted global options: \n");
            // Apply global options to alarm system if needed
            alarm.setGlobalOptions(globalOptions);
        }
    }

    return true;
}
