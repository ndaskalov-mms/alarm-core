#pragma once
#ifndef ARDUINO
#include <stdio.h>
#include <string>
#include <cctype>
#include <string.h>
//
typedef unsigned char byte;
//
#include "alarm-core-config.h"
#include "src\alarm-core-debug.h"
#include "alarm-FS-wrapper.h"
#include "alarm-core.h"
//#include "alarm-core-mqtt.h"   // <-- add this: provides full MqttProcessor definition
//
extern Alarm my_alarm;
bool dispatchMqttTestMessage(const char* topic, const char* payload, size_t length); // forward decl
extern const char jsonMqttTestVectorsFname[];

struct JsonMQTTTestVector {
    std::string topic;
    std::string payload;
    std::string description;
};

static std::string trimCopy(const std::string& input) {
    size_t start = 0;
    while (start < input.size() && std::isspace((unsigned char)input[start])) start++;
    size_t end = input.size();
    while (end > start && std::isspace((unsigned char)input[end - 1])) end--;
    return input.substr(start, end - start);
}

static bool isWhitespaceOnly(const std::string& input) {
    for (size_t i = 0; i < input.size(); i++)
        if (!std::isspace((unsigned char)input[i])) return false;
    return true;
}

static bool extractValue(const std::string& line, const char* key, std::string& outValue) {
    size_t keyLen = strlen(key);
    if (line.size() < keyLen) return false;
    for (size_t i = 0; i < keyLen; i++)
        if (std::tolower((unsigned char)line[i]) != std::tolower((unsigned char)key[i])) return false;
    outValue = trimCopy(line.substr(keyLen));
    return true;
}
//
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
        }
        else if (extractValue(line, "Topic:", value)) {
            outTest.topic = value;
            payloadStarted = false;
        }
        else if (extractValue(line, "Payload:", value)) {
            outTest.payload = value;
            payloadStarted = true;
        }
        else if (payloadStarted) {
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

int alarmWriteLine(const char* line, IOptr stream) {
    size_t len = strlen(line);
    if (!alarmFileWrite((const byte*)line, (int)len, stream)) return false;
    return alarmFileWrite((const byte*)"\n", 1, stream);
}

static void executeJsonMqttTestBlock(
    Alarm& alarm,
    const std::string& block,
    int blockIndex,
    int& totalTests,
    int& passCount,
    int& parseFailCount,
    IOptr outStream) {          // <-- added
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

    bool result = dispatchMqttTestMessage(
        test.topic.c_str(),
        test.payload.c_str(),
        strlen(test.payload.c_str())
    );

    printf("Result: %s\n", result ? "SUCCESS" : "FAILED");
    if (result) passCount++;

    if (outStream) {
        char resultLine[512];
        snprintf(resultLine, sizeof(resultLine), "%s:%s:%s",
            test.topic.c_str(), result ? "OK" : "ERROR", test.description.c_str());
        alarmWriteLine(resultLine, outStream);   // reuse alarmWriteLine from alarm-FS-wrapper.h
    }

    printf("------------------------------------------------------------\n");
}

void runJsonMQTTTests(Alarm& alarm, const char* fileName, const char* outFileName) {
    printf("\n=================== STARTING JSON MQTT TESTS ===================\n");

    IOptr testsFile = alarmFileOpen(fileName, "rb");
    if (!testsFile) {
        printf("Unable to open MQTT test vectors file: %s\n", fileName);
        return;
    }

    IOptr outStream = nullptr;
    if (outFileName) {
        outStream = alarmFileOpen(outFileName, "w");
        if (!outStream) {
            printf("Unable to open results output file: %s\n", outFileName);
        }
    }

    int passCount = 0, totalTests = 0, parseFailCount = 0, blockIndex = 0;
    std::string line, currentBlock;

    while (alarmFileReadLine(testsFile, line)) {
        if (isWhitespaceOnly(line)) {
            if (!isWhitespaceOnly(currentBlock)) {
                blockIndex++;
                executeJsonMqttTestBlock(alarm, currentBlock, blockIndex, totalTests, passCount, parseFailCount, outStream);
                currentBlock.clear();
            }
            continue;
        }
        if (!currentBlock.empty()) currentBlock += "\n";
        currentBlock += line;
    }

    if (!isWhitespaceOnly(currentBlock)) {
        blockIndex++;
        executeJsonMqttTestBlock(alarm, currentBlock, blockIndex, totalTests, passCount, parseFailCount, outStream);
    }

    alarmFileClose(testsFile);
    if (outStream) alarmFileClose(outStream);

    printf("\n=================== JSON MQTT TEST SUMMARY ===================\n");
    printf("Tests passed: %d / %d (%.1f%%)\n",
        passCount, totalTests, totalTests ? ((float)passCount / totalTests * 100.0f) : 0.0f);
    printf("Parse failures: %d\n", parseFailCount);
    printf("============================================================\n\n");
}


#endif // ARDUINO