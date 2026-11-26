#pragma once
#pragma once
#include <stdio.h>
#include <string.h>

typedef unsigned char byte;

// global storage definitions
char prnBuf[1024];
char token[256];
char jsonBuffer[32768];
char tempJsonBuf[8192];         // used for publishing MQTT messages

#include "alarm-core-config.h"
#include "src/alarm-core-debug.h"
#include "alarm-FS-wrapper.h"
#include "alarm-core.h"
#include "src/alarm-core-JSON.h"
#include "src/alarm-core-printer.h"// Include the new header-only parser
#include "alarm-core-mqtt.h"

inline void debugPrinter(const char* message, size_t length) {
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
// full definition to translation units that don't need it (e.g. alarm-core-tests.h)
bool dispatchMqttTestMessage(const char* topic, const char* payload, size_t length) {
    return myMqttProcessor.processIncomingMQTTmsg(topic, payload, length);
}

int alarm_setup() {
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
    m_printer.printAlarmOptions();
    m_printer.printAlarmPartitions(0, MAX_PARTITION);
    m_printer.printAlarmZones(0, MAX_ALARM_ZONES);
    m_printer.printAlarmPgms();

    char zoneStatusJson[256];
    if (parser.buildZoneStatusJson(0, zoneStatusJson, sizeof(zoneStatusJson))) {
        printf("Zone status JSON: %s\n", zoneStatusJson);
    }
    // MQTT

    MQTT_INIT(g_mqttCtx, myMqttProcessor, mqttServer, mqttClientId, mqttUser, mqttPassword);
    //MQTT_INIT(g_mqttCtx, myMqttProcessor, "tcp://192.168.33.201:1883", "alarm-core-client", mqttUser, mqttPassword);
    //my_alarm.setPublisher(mqttPublishWrapper, &g_mqttCtx);

    return 0;
}

