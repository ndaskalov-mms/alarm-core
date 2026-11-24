#include <stdio.h>
#include <string.h>

typedef unsigned char byte;

// global storage definitions
char prnBuf[1024];
char token[256];
char jsonBuffer[32768];
char tempJsonBuf[8192];         // used for publishing MQTT messages

#include "alarm-core-config.h"
#include "src\alarm-core-debug.h"
#include "alarm-FS-wrapper.h"
#include "alarm-core.h"
#include "src\alarm-core-JSON.h"
#include "src\alarm-core-printer.h"
#include "alarm-core-mqtt.h"


void debugPrinter(const char* message, size_t length) {
    printf("[DEBUG] %.*s\n", (int)length, message);
}

// instance of the Alarm class
Alarm my_alarm;
alarmPrinter m_printer(my_alarm);
alarmJSON parser(my_alarm, m_printer);
MqttProcessor myMqttProcessor(parser, my_alarm);

// global/static
mqttContext g_mqttCtx;

// Wrapper to allow calling into myMqttProcessor without exposing MqttProcessor's
// full definition to translation units that don't need it (e.g. alarm-core-tests.cpp)
bool dispatchMqttTestMessage(const char* topic, const char* payload, size_t length) {
    return myMqttProcessor.processIncomingMQTTmsg(topic, payload, length);
}

int setup() {
    LOG_DEBUG("Starting Alarm Core setup.\n");
    my_alarm.setDebugCallback(GlobalDebugLogger);
    my_alarm.debugCallback(LOG_ERR_OK, "test\n");

    if (!loadConfig(jsonConfigFname, (byte*)jsonBuffer, sizeof(jsonBuffer))) {
        printf("Failed to load config file\n");
        return -1;
    }

    if (parser.processConfigJsonPld(jsonBuffer, strlen(jsonBuffer))) {
        printf("Failed to parse config JSON\n");
        return -1;
    }

    m_printer.printAlarmPartitions(0, MAX_PARTITION);
    m_printer.printAlarmZones(0, MAX_ALARM_ZONES);
    m_printer.printAlarmPgms();

    char zoneStatusJson[256];
    if (parser.buildZoneStatusJson(0, zoneStatusJson, sizeof(zoneStatusJson))) {
        printf("Zone status JSON: %s\n", zoneStatusJson);
    }

    MQTT_INIT(g_mqttCtx, myMqttProcessor, "tcp://192.168.33.201:1883", "alarm-core-client", mqttUser, mqttPassword);
    //my_alarm.setPublisher(mqttPublishWrapper, &g_mqttCtx);

    return 0;
}

void loop() {
    my_alarm.alarm_loop();
    MQTT_SERVICE(g_mqttCtx, MQTT_RECONNECT_mS);
}

#ifndef ARDUINO
void runJsonMQTTTests(Alarm& alarm, const char* fileName, const char* outFileName = nullptr);

int main() {
    LOG_DEBUG("Starting Alarm...\n");
    setup();

    for (;;) {
        loop();
    }

    MQTT_DESTROY(g_mqttCtx);
    runJsonMQTTTests(my_alarm, jsonMqttTestVectorsFname);
    return 0;
}
//
#include "alarm-core-tests.h"
//
#endif

//#pragma once
//#include <stdio.h>
//#include <string.h>
////#include <string>
////#include <cctype>
//
//typedef unsigned char byte;
//
//// global storage definitions
//char prnBuf[1024];
//char token[256];
//char jsonBuffer[32768];
//char tempJsonBuf[8192];         // used for publishing MQTT messages
//
//
//
//#include "alarm-core-config.h"
//#include "src\alarm-core-debug.h"
//#include "alarm-FS-wrapper.h"
//#include "alarm-core.h"
//#include "src\alarm-core-JSON.h" // Include the new header-only parser
//#include "src\alarm-core-printer.h" // Include the new header-only parser
//#include "alarm-core-mqtt.h"
//
//
////TimerManager alarmTimerManager;
//
//void runJsonMQTTTests(Alarm& alarm, const char* fileName, const char* outFileName = nullptr);
//
//void debugPrinter(const char* message, size_t length) {
//    printf("[DEBUG] %.*s\n", (int)length, message);
//}
//
//// instance of the Alarm class
//Alarm my_alarm;
//alarmPrinter m_printer(my_alarm);
//
//// inject global printer into parser
//alarmJSON parser(my_alarm, m_printer);
//
//// MqttProcessor class instance. Provides interface to MQTT client. Receives reference to
//// alarmJSON class instance and Alarm class instance. This way,  "injecting" the dependencies
//// (m_jsonParser, m_alarm) is implemented. What is received on particular MQTT topic is routed to 
//// processIncommingMQTTmsg() method, which then calls appropriate JSON processor (processConfigMessage() or
//// processControlMessage() -> processByDomain() ) based on the topic and payload. They call as part of processing 
//// alarmJSON object processConfigJsonPld(), processControlJsonPld(), etc
//
//MqttProcessor myMqttProcessor(parser, my_alarm);
//
//// global/static
//mqttContext g_mqttCtx;
//
//int setup() {
//    LOG_DEBUG("Starting Alarm Core setup.\n");
//    // debug callback setup
//    my_alarm.setDebugCallback(GlobalDebugLogger);
//    my_alarm.debugCallback(LOG_ERR_OK, "test\n");
//
//    // MQTT client initialization
//    // MQTT_setup(myMqttProcessor,  mqttUser, mqttPassword);
//
//    if (!loadConfig(jsonConfigFname, (byte*)jsonBuffer, sizeof(jsonBuffer))) {
//        printf("Failed to load config file\n");
//        return -1;
//    }
//
//    if (parser.processConfigJsonPld(jsonBuffer, strlen(jsonBuffer))) {
//        printf("Failed to parse config JSON\n");
//        return -1;
//    }
//
//    m_printer.printAlarmPartitions(0, MAX_PARTITION);
//    m_printer.printAlarmZones(0, MAX_ALARM_ZONES);
//    m_printer.printAlarmPgms();
//
//    // Example call from `alarm-core.cpp`
//    char zoneStatusJson[256];
//    if (parser.buildZoneStatusJson(0, zoneStatusJson, sizeof(zoneStatusJson))) {
//        printf("Zone status JSON: %s\n", zoneStatusJson);
//    }
//
//    MQTT_INIT(g_mqttCtx, myMqttProcessor, "tcp://192.168.33.201:1883", "alarm-core-client", mqttUser, mqttPassword);
//    // my_alarm.setPublisher(mqttPublishWrapper, &g_mqttCtx);
//
//    return 0;
//}
//
//void loop() {
//	// Main loop for processing alarms and MQTT messages
//	my_alarm.alarm_loop();
//	// Here you would typically check for incoming MQTT messages and process them
//	// For example, if using Paho MQTT, you might call a function to check for messages
//	// and then pass them to myMqttProcessor.processIncomingMQTTmsg(topic, payload, length);
//    pahoService(g_mqttCtx, MQTT_RECONNECT_mS);
//}
//
//int main() {
//	LOG_DEBUG("Starting Alarm...\n");
//	setup();
//
//
//    for (;;) {
//		loop();
//    }
//
//    MQTT_DESTROY(g_mqttCtx);
//
//    // Example call from `alarm-core.cpp`
//    char zoneStatusJson[256];
//    if (parser.buildZoneStatusJson(0, zoneStatusJson, sizeof(zoneStatusJson))) {
//        printf("Zone status JSON: %s\n", zoneStatusJson);
//    }
//
//    runJsonMQTTTests(my_alarm, jsonMqttTestVectorsFname);
//    return 0;
//}
//
////void publishAlarmAndTroubleZones() {
////    for (int zn = 0; zn < MAX_ALARM_ZONES; ++zn) {
////        if (!m_alarm.zonesDB[zn].zoneType) continue;
////        //if (m_alarm.zonesRT[zn].in_alarm)   publishAlarmZone(zn);
////        //if (m_alarm.zonesRT[zn].in_trouble) publishTroubleZone(zn);
////    }
////}
////
////void publishPartitionStatus(int prt) { (void)prt; }
////
////void publishZonesStatusChanges(int prt) {
////    for (int zn = 0; zn < MAX_ALARM_ZONES; ++zn) {
////        if (m_alarm.zonesDB[zn].zonePartition != prt) continue;
////        if (!m_alarm.zonesDB[zn].zoneType) continue;
////        if (!m_alarm.zonesRT[zn].changed) continue;
////
////
////
////        m_alarm.zonesRT[zn].changed = 0;
////    }
////}
////
////void publishPGMStatusChanges() {}
////
////void publishAll(int prt) {
////    publishZonesStatusChanges(prt);
////    publishPartitionStatus(prt);
////    publishAlarmAndTroubleZones();
////}
////

