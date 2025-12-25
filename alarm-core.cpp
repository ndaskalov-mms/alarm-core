// this is heeded for windows JSON library
//#include "json.hpp"
#include <stdio.h>
typedef unsigned char byte;
#include "alarm-core-config.h"
#include "src\alarm-core-debug.h"
//#include "src\alarm-core-timers.h"
#include "alarm-FS-wrapper.h"
#include "alarm-core.h"
#include "src\alarm-core-JSON.h" // Include the new header-only parser
//#include "alarm-core-mqtt.h" // Include the new header
//#include "alarm-core-CSV-parser.h" // Include the new header
#include <string.h>
//#include "..\esp-json-parser\include\json_parser-code.h"

// Sample JSON configuration for testing
// #define alarm_config_json "{\"zones\":[{\"zName\":\"Front\"},{\"zName\":\"Back\"}]}"

// global storage definitions
char prnBuf[1024];
char token[256];
char jsonBuffer[32768];
//TimerManager alarmTimerManager;

void runJsonMQTTTests(Alarm& alarm);


void debugPrinter(const char* message, size_t length) {
    printf("[DEBUG] %.*s\n", (int)length, message);
}

// instance of the Alarm class
Alarm alarm;
// instance of the JSON parser class
alarmJSON parser(alarm);
// Create the MqttProcessor, "injecting" the dependencies (myAlarm and myJsonParser).
// 
#include "alarm-core-mqtt.h"
//MqttProcessor myMqttProcessor(alarm, parser);
MqttProcessor myMqttProcessor(parser);

// MQTT publish wrapper function
// Dummy MQTT client object
struct DummyMqttClient {};
DummyMqttClient mqttClient;
// Add dummy definitions for mqttPublishWrapper and mqttClient if not already defined
static void mqttPublishWrapper(void* context, const char* topic, const char* payload) {
    // Example implementation: just print the topic and payload
    printf("[MQTT] Topic: %s, Payload: %s\n", topic, payload);
}



int main() {

	// debug callback setup
    alarm.setDebugCallback(GlobalDebugLogger);
    alarm.debugCallback(LOG_ERR_OK, "test\n");

	// MQTT publisher setup
    // args passed to the Alarm class: static wrapper function (mqttPublishWrapper) and pointer of your client object (&mqttClient)
    alarm.setPublisher(mqttPublishWrapper, &mqttClient);

    storageSetup();
            
    IOptr configFile = alarmFileOpen(jsonConfigFname, "rb");
    if (!configFile) {
        GlobalDebugLogger(LOG_ERR_CRITICAL, "Failed to open config file: %s", jsonConfigFname);
        storageClose();
		return -1;
    }

    int fileSize = alarmFileSize(configFile);

    if (!jsonBuffer) {
        GlobalDebugLogger(LOG_ERR_CRITICAL, "Failed to allocate memory for config file.");
        storageClose();
		return -1;
    }
    size_t bytesRead = alarmFileRead((byte*)jsonBuffer, fileSize, configFile);
    if (bytesRead != fileSize) {
        GlobalDebugLogger(LOG_ERR_CRITICAL, "Failed to read config file: %s", jsonConfigFname);
        storageClose();
        free(jsonBuffer);
		return -1;
    }
    alarmFileClose(configFile);
    storageClose();

    jsonBuffer[fileSize] = '\0'; // Null-terminate the buffer

    parser.parseConfigJSON(jsonBuffer);

    runJsonMQTTTests(alarm);
    return 0;
}
//
// wrapers for the Alarm class methods
//
int getZoneCount() {
    return alarm.getZoneCount();
}
//
int getZoneIndex(const char* name) {
    return alarm.getZoneIndex(name);
}
//
//const char* getZoneName(int zoneIdx) {
//    return alarm.getZoneName(zoneIdx);
//}
//
//bool isZoneValid(int zoneIdx) {
//    return alarm.isZoneValid(zoneIdx);
//}
//
//int getPartitionCount() {
//    return alarm.getPartitionCount();
//}
////
//int getPartitionIndex(const char* name) {
//    return alarm.getPartitionIndex(name);
//}
////
//const char* getPartitionName(int partIdx) {
//    return alarm.getPartitionName(partIdx);
//}
////
//bool isPartitionValid(int partIdx) {
//    return alarm.isPartitionValid(partIdx);
//}
////
//void setPartitionTarget(int partIdx, int action) {
//    alarm.setPartitionTarget(partIdx, static_cast<ARM_METHODS_t>(action));
////}
////
//int getPgmCount() {
//    return alarm.getPgmCount();
//}
//
//bool isPgmValid(int pgmIdx) {
//    return alarm.isPgmValid(pgmIdx);
//}
//
//int getPgmIndex(const char* name) {
//    return alarm.getPgmIndex(name);
//}
////
//const char* getPgmName(int pgmIdx) {
//    return alarm.getPgmName(pgmIdx);
//}
////
//bool setGlobalOptions(const char* opt_name, const char* opt_val) {
//    return alarm.setGlobalOptions(opt_name, opt_val);
//}
////
//int getGlobalOptionsCnt() {
//    return alarm.getGlobalOptionsCnt();
//}
////
//const char* getGlobalOptionKeyStr(int idx) {
//    return alarm.getGlobalOptionKeyStr(idx);
//}
//
//void zoneCmd(Alarm& alarm_instance, void* param1, void* param2, void* param3) {
//	return alarm.modifyZn(param1, param2, param3);
//}
////
//void pgmCmd(Alarm& alarm_instance, void* param1, void* param2, void* param3) {
//	return alarm_instance.modifyPgm(param1, param2, param3);
//}

//
// ------------------------  JSON MQTT TEST vectors ------------------------
//
struct JsonMQTTTestVector {
    const char* topic;       // The MQTT topic to test
    const char* payload;     // The JSON payload to send
    const char* description; // Description of the test case
};

#include "jsonTestVectors.h"

/**
 * @brief Run the JSON MQTT test vectors
 *
 * This function iterates through all the JSON test vectors and calls
 * the processJsonMessage method of the Alarm class to test the JSON
 * processing functionality.
 *
 * @param alarm Reference to the Alarm class instance
 */
 void runJsonMQTTTests(Alarm& alarm) {
     printf("\n=================== STARTING JSON MQTT TESTS ===================\n");
 
     int passCount = 0;
     int totalTests = sizeof(jsonTestVectors) / sizeof(jsonTestVectors[0]);
 
     for (int i = 0; i < totalTests; i++) {
         const auto& test = jsonTestVectors[i];
         printf("\n--------------------- Test #%d ----------------------\n", i + 1);
         printf("Description: %s\n", test.description);
         printf("Topic: %s\n", test.topic);
         printf("Payload: %s\n", test.payload);
 
         // Process the JSON message
         //bool result = myMqttProcessor.processConfigMessage(
         //    test.topic,
         //    (const char *)test.payload,
         //    strlen(test.payload)
         //);
         bool result = myMqttProcessor.processMessage(
             test.topic,
             (const char*)test.payload,
             strlen(test.payload)
         );
 
         printf("Result: %s\n", result ? "SUCCESS" : "FAILED");
         if (result) passCount++;
 
         printf("------------------------------------------------------------\n");
 
         // Optionally call alarm_loop to process any changes
         //alarm.alarm_loop();
     }
 
     printf("\n=================== JSON MQTT TEST SUMMARY ===================\n");
     printf("Tests passed: %d / %d (%.1f%%)\n",
         passCount, totalTests, (float)passCount / totalTests * 100);
     printf("============================================================\n\n");
 }
