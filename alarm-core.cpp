#pragma once
#include <stdio.h>
#include <string.h>
#include <string>
#include <cctype>

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

void runJsonMQTTTests(Alarm& alarm, const char* fileName);

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
       if(parser.parseConfigJSON(jsonBuffer)) {
        printf("Failed to parse config JSON\n");
        return -1;
	}

    my_alarm.printAlarmPartition(0, MAX_PARTITION);
    my_alarm.printAlarmZones(0, MAX_ALARM_ZONES);
    my_alarm.printAlarmPgms();

    runJsonMQTTTests(my_alarm, jsonMqttTestVectorsFname);
    return 0;
}
//
// ------------------------  JSON MQTT TEST vectors ------------------------
//
struct JsonMQTTTestVector {
    std::string topic;       // The MQTT topic to test
    std::string payload;     // The JSON payload to send
    std::string description; // Description of the test case
};

static std::string trimCopy(const std::string& input) {
    size_t start = 0;
    while (start < input.size() && std::isspace((unsigned char)input[start])) {
        start++;
    }

    size_t end = input.size();
    while (end > start && std::isspace((unsigned char)input[end - 1])) {
        end--;
    }

    return input.substr(start, end - start);
}

static bool isWhitespaceOnly(const std::string& input) {
    for (size_t i = 0; i < input.size(); i++) {
        if (!std::isspace((unsigned char)input[i])) {
            return false;
        }
    }
    return true;
}

static bool extractValue(const std::string& line, const char* key, std::string& outValue) {
    size_t keyLen = strlen(key);
    if (line.size() < keyLen) {
        return false;
    }

    for (size_t i = 0; i < keyLen; i++) {
        if (std::tolower((unsigned char)line[i]) != std::tolower((unsigned char)key[i])) {
            return false;
        }
    }

    outValue = trimCopy(line.substr(keyLen));
    return true;
}

static bool parseTestVectorBlock(const std::string& block, int blockIndex, JsonMQTTTestVector& outTest) {
    outTest.description.clear();
    outTest.topic.clear();
    outTest.payload.clear();

    bool payloadStarted = false;
    size_t pos = 0;
    while (pos <= block.size()) {
        size_t end = block.find('\n', pos);
        if (end == std::string::npos) {
            end = block.size();
        }

        std::string line = block.substr(pos, end - pos);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::string value;
        if (extractValue(line, "Description:", value)) {
            outTest.description = value;
            payloadStarted = false;
        } else if (extractValue(line, "Topic:", value)) {
            outTest.topic = value;
            payloadStarted = false;
        } else if (extractValue(line, "Payload:", value)) {
            outTest.payload = value;
            payloadStarted = true;
        } else if (payloadStarted) {
            if (!outTest.payload.empty()) {
                outTest.payload += "\n";
            }
            outTest.payload += line;
        }

        if (end == block.size()) {
            break;
        }
        pos = end + 1;
    }

    if (outTest.description.empty() || outTest.topic.empty() || outTest.payload.empty()) {
        printf("Invalid MQTT test vector block #%d\n", blockIndex);
        return false;
    }

    return true;
}

static bool alarmFileReadLine(IOptr fileHandle, std::string& outLine) {
    outLine.clear();
    byte ch = 0;

    while (true) {
        size_t readLen = alarmFileRead(&ch, 1, fileHandle);
        if (readLen != 1) {
            return !outLine.empty();
        }
        if (ch == '\n') {
            return true;
        }
        if (ch != '\r') {
            outLine.push_back((char)ch);
        }
    }
}

static void executeJsonMqttTestBlock(
    Alarm& alarm,
    const std::string& block,
    int blockIndex,
    int& totalTests,
    int& passCount,
    int& parseFailCount) {
    (void)alarm;

    totalTests++;
    JsonMQTTTestVector test;
    if (!parseTestVectorBlock(block, blockIndex, test)) {
        parseFailCount++;
        return;
    }

    printf("\n--------------------- Test #%d ----------------------\n", totalTests);
    printf("Description: %s\n", test.description.c_str());
    printf("Topic: %s\n", test.topic.c_str());
    printf("Payload: %s\n", test.payload.c_str());

    bool result = myMqttProcessor.processMessage(
        test.topic.c_str(),
        test.payload.c_str(),
        strlen(test.payload.c_str())
    );

    printf("Result: %s\n", result ? "SUCCESS" : "FAILED");
    if (result) {
        passCount++;
    }

    printf("------------------------------------------------------------\n");
}

/**
 * @brief Run the JSON MQTT test vectors
 *
 * This function iterates through all the JSON test vectors and calls
 * the processJsonMessage method of the Alarm class to test the JSON
 * processing functionality.
 *
 * @param my_alarm Reference to the Alarm class instance
 */
 void runJsonMQTTTests(Alarm& alarm, const char* fileName) {
     printf("\n=================== STARTING JSON MQTT TESTS ===================\n");

     IOptr testsFile = alarmFileOpen(fileName, "rb");
     if (!testsFile) {
         printf("Unable to open MQTT test vectors file: %s\n", fileName);
         return;
     }

     int passCount = 0;
     int totalTests = 0;
     int parseFailCount = 0;
     int blockIndex = 0;
     std::string line;
     std::string currentBlock;

     while (alarmFileReadLine(testsFile, line)) {
         if (isWhitespaceOnly(line)) {
             if (!isWhitespaceOnly(currentBlock)) {
                 blockIndex++;
                 executeJsonMqttTestBlock(alarm, currentBlock, blockIndex, totalTests, passCount, parseFailCount);
                 currentBlock.clear();
             }
             continue;
         }

         if (!currentBlock.empty()) {
             currentBlock += "\n";
         }
         currentBlock += line;
     }

     if (!isWhitespaceOnly(currentBlock)) {
         blockIndex++;
         executeJsonMqttTestBlock(alarm, currentBlock, blockIndex, totalTests, passCount, parseFailCount);
     }

     alarmFileClose(testsFile);

     printf("\n=================== JSON MQTT TEST SUMMARY ===================\n");
     printf("Tests passed: %d / %d (%.1f%%)\n",
         passCount, totalTests, totalTests ? ((float)passCount / totalTests * 100.0f) : 0.0f);
     printf("Parse failures: %d\n", parseFailCount);
     printf("============================================================\n\n");
 }
