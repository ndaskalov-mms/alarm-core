#pragma once
#include <stdio.h>
#include <string.h>

typedef unsigned char byte;

#include "alarm-core-config.h"
#include "src\alarm-core-debug.h"
#include "alarm-FS-wrapper.h"
#include "alarm-core.h"
#include "src\alarm-core-JSON.h" // Include the new header-only parser
#include "alarm-core-mqtt.h"


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
Alarm my_alarm;
// instance of the JSON parser class
alarmJSON parser(my_alarm);

// Create the MqttProcessor, "injecting" the dependencies (myAlarm and myJsonParser).
//MqttProcessor myMqttProcessor(my_alarm, parser);
MqttProcessor myMqttProcessor(parser);

#ifndef ARDUINO
// MQTT publish wrapper function
// Dummy MQTT client object
struct DummyMqttClient {};
DummyMqttClient mqttClient;
// Add dummy definitions for mqttPublishWrapper and mqttClient if not already defined
static void mqttPublishWrapper(void* context, const char* topic, const char* payload) {
    // Example implementation: just print the topic and payload
    printf("[MQTT] Topic: %s, Payload: %s\n", topic, payload);
}
#endif

int main() {
	LOG_DEBUG("Starting Alarm Core JSON MQTT Tests...\n");
	// debug callback setup
    my_alarm.setDebugCallback(GlobalDebugLogger);
    my_alarm.debugCallback(LOG_ERR_OK, "test\n");

	// MQTT publisher setup
    // args passed to the Alarm class: static wrapper function (mqttPublishWrapper) and pointer of your client object (&mqttClient)
    my_alarm.setPublisher(mqttPublishWrapper, &mqttClient);


    if (!loadConfig(jsonConfigFname, (byte *)jsonBuffer, sizeof(jsonBuffer))) {
        printf("Failed to load config file\n");
        return -1;
    }
       if(!parser.parseConfigJSON(jsonBuffer)) {
        printf("Failed to parse config JSON\n");
        return -1;
	}

    my_alarm.printAlarmPartition(0, MAX_PARTITION);
    my_alarm.printAlarmZones(0, MAX_ALARM_ZONES);
    my_alarm.printAlarmPgms();

    runJsonMQTTTests(my_alarm);
    return 0;
}
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
 * @param my_alarm Reference to the Alarm class instance
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
         //my_alarm.alarm_loop();
     }
 
     printf("\n=================== JSON MQTT TEST SUMMARY ===================\n");
     printf("Tests passed: %d / %d (%.1f%%)\n",
         passCount, totalTests, (float)passCount / totalTests * 100);
     printf("============================================================\n\n");
 }
