
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"

#include <windows.h>
unsigned long millis() {
	SYSTEMTIME st;
	GetSystemTime(&st);
	//printf("The system time is: %02d:%02d:%02d\n", st.wHour, st.wMinute,st.wSecond);
	unsigned long res = (st.wHour * 60 * 60 * 1000 + st.wMinute * 60 * 1000 + st.wSecond * 1000 + st.wMilliseconds);
	//printf("Time in mS: %lu\n", res);
	return res;
}

char		prnBuf[1024];
char 		token[256];


#include "AlarmClass.h" 
Alarm alarm;

void debugPrinter(const char* message, size_t length) {
    printf("[DEBUG] %.*s\n", (int)length, message);
}




//#include <unordered_map>
#include "parserClassHelpers.h"

// Function to extract zone fields from JSON using the zoneTags structure
int extractZoneFields(const nlohmann::json& zoneJson, ALARM_ZONE& zone) {
    // Initialize zone with zeros
    std::memset(&zone, 0, sizeof(ALARM_ZONE));
    zone.valid = true; // Set valid to true by default

    // Go through each tag in zoneTags array
    for (size_t i = 0; i < ZONE_TAGS_CNT; i++) {
        const char* tagName = zoneTags[i].keyStr;
        byte* targetAddress = reinterpret_cast<byte*>(&zone) + zoneTags[i].patchOffset;
        int length = zoneTags[i].patchLen;

        // Check if the tag exists in JSON
        if (zoneJson.contains(tagName)) {
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
                    std::cerr << "Error: Failed to patch field " << tagName << " with value " << valueStr << std::endl;
                    zone.valid = false; // Mark zone as invalid
                    return 0; // Indicate failure
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error extracting field " << tagName << ": " << e.what() << std::endl;
                zone.valid = false; // Mark zone as invalid
                return 0; // Indicate failure
            }
        }
    }
    Alarm::printConfigData(zoneTags, ZONE_TAGS_CNT, (byte*)&zone, PRTCLASS_ALL);
    return 1; // Indicate success
}

int extractGlobalOptionsFields(const nlohmann::json& optionsJson, ALARM_GLOBAL_OPTS_t& globalOptions) {
    // Initialize global options with zeros
    std::memset(&globalOptions, 0, sizeof(ALARM_GLOBAL_OPTS_t));

    // Go through each tag in globalOptionsTags array
    for (size_t i = 0; i < GLOBAL_OPTIONS_TAGS_CNT; i++) {
        const char* tagName = gOptsTags[i].keyStr;
        byte* targetAddress = reinterpret_cast<byte*>(&gOptsTags) + gOptsTags[i].patchOffset;
        int length = gOptsTags[i].patchLen;

        // Check if the tag exists in JSON
        if (optionsJson.contains(tagName)) {
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
                    std::cerr << "Error: Failed to patch field " << tagName << " with value " << valueStr << std::endl;
                    return 0; // Indicate failure
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error extracting field " << tagName << ": " << e.what() << std::endl;
                return 0; // Indicate failure
            }
        }
    }

    return 1; // Indicate success
}

int extractPartitionFields(const nlohmann::json& partitionJson, ALARM_PARTITION_t& partition) {
    // Initialize partition with zeros
    std::memset(&partition, 0, sizeof(ALARM_PARTITION_t));
    partition.valid = true; // Set valid to true by default

    // Go through each tag in partitionTags array
    for (size_t i = 0; i < PARTITION_TAGS_CNT; i++) {
        const char* tagName = partitionTags[i].keyStr;
        byte* targetAddress = reinterpret_cast<byte*>(&partition) + partitionTags[i].patchOffset;
        int length = partitionTags[i].patchLen;

        // Check if the tag exists in JSON
        if (partitionJson.contains(tagName)) {
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
                    std::cerr << "Error: Failed to patch field " << tagName << " with value " << valueStr << std::endl;
                    partition.valid = false; // Mark partition as invalid
                    return 0; // Indicate failure
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error extracting field " << tagName << ": " << e.what() << std::endl;
                partition.valid = false; // Mark partition as invalid
                return 0; // Indicate failure
            }
        }
    }

    return 1; // Indicate success
}

int main() {
    // Print a welcome message
    std::cout << "Windows Console Application: ALARM JSON Example\n";

    alarm.setDebugCallback(debugPrinter);

    // Open the JSON file
    std::ifstream inputFile("alarm-config.json");
    if (!inputFile.is_open()) {
        std::cerr << "Error: Failed to open alarm.json\n";
        return 1;
    }

    // Parse the JSON file
    nlohmann::json jsonData;
    try {
        inputFile >> jsonData;
    }
    catch (const nlohmann::json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << "\n";
        return 1;
    }

    // Deserialize JSON into Alarm object

    try {
        //alarm = jsonData.get<Alarm>();
        if (jsonData.contains("zones") && jsonData["zones"].is_array()) {
            for (const auto& zoneJson : jsonData["zones"]) {
                ALARM_ZONE zone = {};

                // Extract all zone fields using the zoneTags array
                extractZoneFields(zoneJson, zone);

                // Now you can use the zone object
                if (zone.valid) {
                    std::cout << "Extracted zone: " << zone.zoneName
                        << " (ID: " << static_cast<int>(zone.zoneID) << ")" << std::endl;

                    // Add zone to alarm system or do other processing
                    // alarm.addZone(zone);
                }
            }
        }

        if (jsonData.contains("partitions") && jsonData["partitions"].is_array()) {
            for (const auto& partitionJson : jsonData["partitions"]) {
                ALARM_PARTITION_t partition = {};

                // Extract all partition fields using the partitionTags array
                if (extractPartitionFields(partitionJson, partition)) {
                    std::cout << "Extracted partition: " << partition.partitionName
                        << " (Index: " << static_cast<int>(partition.partIdx) << ")" << std::endl;

                    // Add partition to alarm system or do other processing
                    // alarm.addPartition(partition);
                }
            }
        }

        if (jsonData.contains("globalOptions") && jsonData["globalOptions"].is_object()) {
            ALARM_GLOBAL_OPTS_t globalOptions = {};

            // Extract all global options fields using the globalOptionsTags array
            if (extractGlobalOptionsFields(jsonData["globalOptions"], globalOptions)) {
                std::cout << "Extracted global options successfully." << std::endl;

                // Apply global options to alarm system or do other processing
                // alarm.setGlobalOptions(globalOptions);
            }
        }

    }
    catch (const nlohmann::json::exception& e) {
        std::cerr << "Error deserializing JSON to Alarm: " << e.what() << "\n";
        return 1;
    }

    // Print the loaded data
    std::cout << "Alarm Data Loaded from JSON:\n";
    //std::cout << "  Zones: " << alarm.getZoneCount() << "\n";
    //std::cout << "  Partitions: " << alarm.getPartitionCount() << "\n";

    // Serialize back to JSON
    //nlohmann::json outputJson = alarm;
    //std::ofstream outputFile("output.json");
    //if (outputFile.is_open()) {
    //    outputFile << outputJson.dump(4); // Pretty print with 4-space indentation
    //    outputFile.close();
    //}

    return 0;
}



//// alarm-core.cpp : This file contains the 'main' function. Program execution begins and ends there.
////
//#define _CRT_SECURE_NO_WARNINGS
//#include <iostream>
//#include <stdio.h>
//#include <string.h>
//#include <conio.h>
//#include <ctype.h>
//	/* vsprintf example */
//#include <stdarg.h>
//#include <windows.h>
//
//#include <windows.h>
//unsigned long millis() {
//	SYSTEMTIME st;
//	GetSystemTime(&st);
//	//printf("The system time is: %02d:%02d:%02d\n", st.wHour, st.wMinute,st.wSecond);
//	unsigned long res = (st.wHour * 60 * 60 * 1000 + st.wMinute * 60 * 1000 + st.wSecond * 1000 + st.wMilliseconds);
//	//printf("Time in mS: %lu\n", res);
//	return res;
//}
//
//char		prnBuf[1024];
//char 		token[256];
//
//#include "AlarmClass.h"
//
//int ErrWrite(int err_code, const char* what, ...)           // callback to dump info to serial console from inside RS485 library
//{
//    va_list args;
//    va_start(args, what);
//    // vprintf(what, args);
//    // va_end(args);
//    // int vprintf(const char* format, va_list arg);
//
//    int index = 0;
//    //lprintf ("error code %d received in errors handling callback\n------------------------", err_code);
//    // update errors struct here
//    vsnprintf(prnBuf, sizeof(prnBuf) - 1, what, args);
//    switch (err_code)
//    {
//    case ERR_OK:
//        printf(prnBuf);
//        break;
//    case ERR_DEBUG:
//        printf(prnBuf);
//        break;
//    case ERR_INFO:
//        printf(prnBuf);
//        break;
//    case ERR_WARNING:
//        printf(prnBuf);
//        break;
//    case ERR_CRITICAL:
//        printf(prnBuf);
//        break;
//    default:
//        if(what)
//            printf(prnBuf);
//        break;
//    }
//    va_end(args);
//    return err_code;
//}
//
//Alarm myAlarm;
//
//int main(int argc, char* argv[])
//{
//    printf("Alarm test program\n");
//    printf("Press any key to exit\n");
//
//    // Create test data for a new zone
//    ALARM_ZONE newZone = {};
//    strncpy(newZone.zoneName, "Test Zone 1", NAME_LEN - 1); // Set zone name
//    newZone.boardID = 1;                                   // Example board ID
//    newZone.zoneID = 1;                                    // Example zone ID
//    newZone.zoneType = INSTANT;                            // Set zone type
//    newZone.zonePartition = PARTITION1;                   // Assign to partition 1
//    newZone.zoneAlarmType = STEADY_ALARM;                  // Set alarm type
//    newZone.valid = 1;                                     // Mark as unused initially
//
//    // Add the zone to the alarm system
//    int index = myAlarm.addZone(newZone);
//    if (index != -1) {
//        printf("Zone added at index %d\n", index);
//    }
//    else {
//        printf("Failed to add zone. No unused entries available.\n");
//    }
//
//    myAlarm.getZoneCount();
//    myAlarm.alarm_loop();
//
//    while (true) {
//        if (_kbhit()) {
//            break;
//        }
//    }
//    return 0;
//}
//
//
//// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
//// Debug program: F5 or Debug > Start Debugging menu
//
//// Tips for Getting Started: 
////   1. Use the Solution Explorer window to add/manage files
////   2. Use the Team Explorer window to connect to source control
////   3. Use the Output window to see build output and other messages
////   4. Use the Error List window to view errors
////   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
////   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
