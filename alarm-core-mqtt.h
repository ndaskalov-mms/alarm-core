#pragma once

#include <stdio.h>
#include <string.h>
#include <cstdarg>
#include "alarm-core.h"

struct mqttTopicHandler {
    const char* topic;
    ALARM_DOMAINS_t domain;
    const char* description;
};

const mqttTopicHandler mqttTopicHandlers[] = {
    { MQTT_ZONES_CONTROL_TOPIC,      ZONES_CMD,      "Control zones (bypass, tamper, etc.)" },
    { MQTT_PARTITIONS_CONTROL_TOPIC, PARTITIONS_CMD, "Control partitions (arm, disarm, etc.)" },
};
const int MQTT_TOPIC_HANDLER_COUNT = sizeof(mqttTopicHandlers) / sizeof(mqttTopicHandlers[0]);

class MqttProcessor {
public:
    MqttProcessor(alarmJSON& jsonParser, Alarm& alarm)
        : m_jsonParser(jsonParser), m_alarm(alarm) {
        printf("MqttProcessor initialized with parser + alarm.\n");
    }

    bool processByDomain(ALARM_DOMAINS_t domain, const char* payload, size_t length) {
        return m_jsonParser.processControlJsonPld(payload, length, domain);
    }

    bool processControlMessage(const char* topic, const char* payload, size_t length) {
        for (int i = 0; i < MQTT_TOPIC_HANDLER_COUNT; ++i) {
            if (strcmp(topic, mqttTopicHandlers[i].topic) == 0) {
                return processByDomain(mqttTopicHandlers[i].domain, payload, length);
            }
        }
        LOG_CRITICAL("No handler found for topic: %s\n", topic);
        return false;
    }

    bool processConfigMessage(const char* topic, const char* payload, unsigned int length) {
        if (!strstr(topic, "/config")) return false;
        LOG_INFO("Processing configuration payload...\n");
        return (m_jsonParser.processConfigJsonPld(payload, length) == 0);
    }

    bool processIncomingMQTTmsg(const char* topic, const char* payload, size_t length) {
        return strstr(topic, "/config")
            ? processConfigMessage(topic, payload, (unsigned int)length)
            : processControlMessage(topic, payload, length);
    }

    // moved from free functions:
    void publish2broker(const char* payload, const char* topic, ...);
    void publishArmStatus(int prt);
    void publishAlarm(byte prtIdx);
    void publishTroubleZone(int zone);
    void publishAlarmZone(int zone);
    void publishAlarmAndTroubleZones();
    void publishPartitionStatus(int prt);
    void publishZonesStatusChanges(int prt);
    void publishPGMStatusChanges();
    void publishAll(int prt);
    bool buildZoneRtJson(int zone, char* outJson, size_t outJsonSize) const;

private:
    alarmJSON& m_jsonParser;
    Alarm& m_alarm;
    char m_topicBuf[8192]{};
};

inline void MqttProcessor::publish2broker(const char* payload, const char* topic, ...) {
    va_list args;
    va_start(args, topic);
    vsnprintf(m_topicBuf, sizeof(m_topicBuf), topic, args);
    va_end(args);

	// call the MQTT publish callback if set
    printf("Topic: %s, Payload: %s\n", m_topicBuf, payload);
}

inline void MqttProcessor::publishArmStatus(int prt) { (void)prt; }
inline void MqttProcessor::publishAlarm(byte prtIdx) { (void)prtIdx; }
inline void MqttProcessor::publishTroubleZone(int zone) { (void)zone; }
inline void MqttProcessor::publishAlarmZone(int zone) { (void)zone; }
inline void MqttProcessor::publishAlarmAndTroubleZones() {}
inline void MqttProcessor::publishPartitionStatus(int prt) { (void)prt; }
inline void MqttProcessor::publishZonesStatusChanges(int prt) {
    int cnt = 0;
    for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {          // for each board' zone
        if (m_alarm.zonesDB[zn].zonePartition != prt)
            continue;                                       // zone belongs to other partition
        if (!m_alarm.zonesDB[zn].zoneType)
            continue;                                       // zone not valid
        if (!m_alarm.zonesRT[zn].changed)
            continue;                                       // no changes in zone to publish
        cnt++;                                              // we need to publish only if cnt != 0 (at least one zone changed)
        if (m_alarm.zonesRT[zn].changed & ZONE_STATE_CHANGED) {     // zone state changed (OPEN/CLOSE/TAMPER/AMASK
            
        }
        if (m_alarm.zonesRT[zn].changed & ZONE_USR_BYPASS_CHANGED) {// if zone is BYPASSED/UNBYPASSED on user request
            if (m_alarm.zonesRT[zn].bypassed & ZONE_BYPASSED)       // report it
                publish2broker(TRUE_PAYLOAD, ZONES_STATES_TOPIC, m_alarm.zonesDB[zn].zoneName, BYPASS_PROPERTY);
            else
                publish2broker(FALSE_PAYLOAD, ZONES_STATES_TOPIC, m_alarm.zonesDB[zn].zoneName, BYPASS_PROPERTY);
        }
        m_alarm.zonesRT[zn].changed = 0;
    }
    if (cnt)
        timeoutOps(FORCE, MQTT_PUBLISH_TIMER);              // force partition status publish
}

inline const char* zoneStatToText(byte zoneStat) {
    if (zoneStat & ZONE_TAMPER) return "TAMPER";
    if (zoneStat & ZONE_AMASK)  return "AMASK";
    if (zoneStat & ZONE_OPEN)   return "OPEN";
    return "CLOSE";
}

inline bool MqttProcessor::buildZoneRtJson(int zone, char* outJson, size_t outJsonSize) const {
    if (!outJson || outJsonSize == 0) {
        return false;
    }
    if (zone < 0 || zone >= MAX_ALARM_ZONES) {
        outJson[0] = '\0';
        return false;
    }

    const ALARM_ZONE_RT& z = m_alarm.zonesRT[zone];

    // Keys are based on ALARM_ZONE_RT field names.
    // zoneStat: OPEN/CLOSE/AMASK/TAMPER
    // changed: hex string
    // bypassed: bitmask as hex string
    // others: true/false
    int n = snprintf(
        outJson,
        outJsonSize,
        "{"
        "\"zoneStat\":\"%s\","
        "\"bypassed\":\"0x%02X\","
        "\"changed\":\"0x%02X\","
        "\"in_alarm\":%s,"
        "\"in_trouble\":%s,"
        "\"ignorredTamper\":%s,"
        "\"ignorredAmask\":%s,"
        "\"openEDSD1zone\":%s,"
        "\"openEDSD2zone\":%s"
        "}",
        zoneStatToText(z.zoneStat),
        (unsigned int)z.bypassed,
        (unsigned int)z.changed,
        z.in_alarm ? "true" : "false",
        z.in_trouble ? "true" : "false",
        z.ignorredTamper ? "true" : "false",
        z.ignorredAmask ? "true" : "false",
        z.openEDSD1zone ? "true" : "false",
        z.openEDSD2zone ? "true" : "false"
    );

    return (n > 0) && ((size_t)n < outJsonSize);
}

inline void MqttProcessor::publishPGMStatusChanges() {}
//
inline void MqttProcessor::publishAll(int prt) {
    publishZonesStatusChanges(prt);
    publishPartitionStatus(prt);
    publishAlarmAndTroubleZones();
}


/*

//
// reportArmStatus - reports partition arming on coresponding topic
// parmas: int prt - partiton ID
// TODO - what will do with DELAYED type alarms like H24_FIRE_DELAYED?
//
void publishArmStatus(int prt) {
    const char* regArmPld = FALSE_PAYLOAD;
    const char* stayArmPld = FALSE_PAYLOAD;
    const char* forceArmPld = FALSE_PAYLOAD;
    const char* currState;
    //
    switch (partitionRT[prt].armStatus) { // in disarm case, we need to send false to actual arm property
    case STAY_ARM:
    case INSTANT_ARM:
        stayArmPld = TRUE_PAYLOAD;
        currState = CUR_STATE_ARMED_HOME;
        break;
    case FORCE_ARM:
        forceArmPld = TRUE_PAYLOAD;
        currState = CUR_STATE_ARMED_FORCE;
        break;
    case REGULAR_ARM:
        regArmPld = TRUE_PAYLOAD;
        currState = CUR_STATE_ARMED_AWAY;
        break;
    case DISARM:
        currState = CUR_STATE_DISARMED;
        //publish2broker(TRUE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_PROPERTY);
        //publish2broker(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_FINISHED_PROPERTY);
        break;
    default:
        ErrWrite(ERR_WARNING, "reportArm invalid partition armStatus %d \n", partitionRT[prt].armStatus);
        return;
    }
    publish2broker(stayArmPld, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, STAY_ARM_PROPERTY);
    publish2broker(forceArmPld,PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, FORCE_ARM_PROPERTY);
    publish2broker(regArmPld,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ARM_PROPERTY);
    //
    // publish on  ....states/partitions/PartitionName/current_state  payload  current state  -armed_home, armed_away, arming, disarmed,...
    if (partitionRT[prt].partitionTimers[EXIT_DELAY_TIMER].timerFSM == RUNNING)
        currState = CUR_STATE_ARMING;
    publish2broker(currState,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, CURRENT_STATE_PROPERTY);
}
//
//
void publishAlarm(byte prtIdx) {
    publish2broker(TRUE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prtIdx].partitionName, ALARM_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prtIdx].alarmZonesCnt, tmpBuf, 10), PARTITIONS_STATES_TOPIC, partitionDB[prtIdx].partitionName, ALARM_ZONES_PROPERTY);    // send over MQTT
}
//
// publishTroubleZone - reports to MQTT, panel, etc that tamper occured in partition/zone
// TODO - implement me
//
void publishTroubleZone(int zone) {
    //lprintf("Report Trouble - Implement me\n");
    publish2broker(TRUE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zone].zoneName, TROUBLE_PROPERTY); // send over MQTT
}
//
// publishAlarmZone - reports to MQTT, panel, etc that tamper occured in partition/zone
// TODO - implement me
//
void publishAlarmZone(int zone) {
    //lprintf("Report Trouble - Implement me\n");
    publish2broker(TRUE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zone].zoneName, ALARM_PROPERTY); // send over MQTT
}
//
void publishAlarmAndTroubleZones() {
    for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {
        if (!zonesDB[zn].zoneType)									                // 0 = DISABLED
            continue;
        if (zonesRT[zn].in_alarm)
            publishAlarmZone(zn);
        if (zonesRT[zn].in_trouble)
            publishTroubleZone(zn);
    }
}
//
void publishPartitionStatus(int prt) {
    int time4publish = false;
    if (timeoutOps(GET, MQTT_PUBLISH_TIMER)) {           					    // publish zones statistics on spec intervals
        time4publish = true;
    }
    timeoutOps(SET, MQTT_PUBLISH_TIMER);                                        // re-set timer to avoid imediate publish after forced publish
    if (!partitionDB[prt].valid)				 							    // not a valid partition
        return;
    if (!partitionRT[prt].changed && !time4publish)                             // need to publish
        return;                                                                 // nothing changed and tiner is still ticking
    publishArmStatus(prt);                                                      // publish arm status
    //("\n--------------Publishing changed partition statistics ----------------------\n");
    publish2broker(_itoa(partitionSTATS[prt].tamperZonesCnt,tmpBuf,10),           PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, TAMPER_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].openZonesCnt,tmpBuf,10),             PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, OPEN_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].amaskZonesCnt,tmpBuf,10),            PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, ANTIMASK_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].bypassedZonesCnt,tmpBuf,10),         PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, BYPASSED_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].openZonesCntEDSD1,tmpBuf,10),        PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, OPEN_ED_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].openZonesCntEDSD2,tmpBuf,10),        PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, OPEN_SD_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].ignorredTamperZonesCnt,tmpBuf,10),   PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, IGNORRED_TAMPER_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].ignorredAmaskZonesCnt,tmpBuf,10),    PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, IGNORRED_AMASK_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].alarmZonesCnt,tmpBuf,10),            PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, ALARM_ZONES_PROPERTY);
    publish2broker(_itoa(partitionSTATS[prt].notBypassedEntyDelayZones,tmpBuf,10),PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, NOT_BYPASSED_ED_ZONES_PROPERTY);
    if (partitionSTATS[prt].alarmZonesCnt)
        printf("In alarm now\n");
    publish2broker((partitionSTATS[prt].alarmZonesCnt ? "true" : "false"), PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, ALARM_PROPERTY);    // send over MQTT
    // report EXIT DELAY
    if (partitionRT[prt].changed & CHG_EXIT_DELAY_TIMER) {
        if (partitionRT[prt].partitionTimers[EXIT_DELAY_TIMER].timerFSM == RUNNING) {
            publish2broker(TRUE_PAYLOAD,    PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_PROPERTY);
            publish2broker(FALSE_PAYLOAD,   PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_FINISHED_PROPERTY);
        }
        else {
            publish2broker(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_PROPERTY);
            publish2broker(TRUE_PAYLOAD,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_FINISHED_PROPERTY);
        }
    }
    if (partitionRT[prt].changed & (CHG_ENTRY_DELAY1_TIMER | CHG_ENTRY_DELAY2_TIMER)) {
        if ((partitionRT[prt].partitionTimers[ENTRY_DELAY1_TIMER].timerFSM == RUNNING) ||
            (partitionRT[prt].partitionTimers[ENTRY_DELAY2_TIMER].timerFSM == RUNNING)) {
            publish2broker(TRUE_PAYLOAD,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_PROPERTY);
            publish2broker(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_FINISHED_PROPERTY);
        }
        else {                                                                                  // ENTRY_DELAY_X timer done or not started
            publish2broker(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_PROPERTY);
            publish2broker(TRUE_PAYLOAD,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_FINISHED_PROPERTY);
        }
    }
    lprintf("\n-------------------------------------------------------------------------------\n");
}
//
//
//
void publishZonesStatusChanges(int prt) {
    int cnt = 0;
    for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {          // for each board' zone
        if (zonesDB[zn].zonePartition != prt)
            continue;                                       // zone belongs to other partition
        if (!zonesDB[zn].zoneType)
            continue;                                       // zone not valid
        if (!zonesRT[zn].changed)
            continue;                                       // no changes in zone to publish
        cnt++;                                              // we need to publish only if cnt != 0 (at least one zone changed)
        if (zonesRT[zn].changed & ZONE_STATE_CHANGED) {     // zone state changed (OPEN/CLOSE/TAMPER/AMASK
            if (zonesRT[zn].zoneStat & ZONE_TAMPER) {       // tamper in zone
                publish2broker(TRUE_PAYLOAD,   ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY);  // TODO - is this really needed
            }
            else if (zonesRT[zn].zoneStat & ZONE_AMASK) {        // amask in zone
                publish2broker(TRUE_PAYLOAD,   ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY);  // TODO - is this really needed
            }
            else if (zonesRT[zn].zoneStat & ZONE_OPEN) {         // open zone
                publish2broker(TRUE_PAYLOAD,   ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
            }
            else {                                          // closed zone
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                publish2broker(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
            }
        }
        if (zonesRT[zn].changed & ZONE_USR_BYPASS_CHANGED) {// if zone is BYPASSED/UNBYPASSED on user request
            if (zonesRT[zn].bypassed & ZONE_BYPASSED)       // report it
                publish2broker(TRUE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zn].zoneName, BYPASS_PROPERTY);
            else
                publish2broker(FALSE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zn].zoneName, BYPASS_PROPERTY);
        }
        zonesRT[zn].changed = 0;
    }
    if(cnt)
        timeoutOps(FORCE, MQTT_PUBLISH_TIMER);              // force partition status publish
}
//
//
//
void publishPGMStatusChanges() {
    for (int pgm = 0; pgm < MAX_ALARM_PGM; pgm++) {          // for each pgm
        if (!pgmsDB[pgm].valid)
            continue;                                       // zone not valid
        if (pgmsDB[pgm].pgmFSM != TO_REPORT)
            continue;                                       // no changes in zone to publis
        publish2broker(pgmsDB[pgm].cValue ? PGM_ON_PROPERTY : PGM_OFF_PROPERTY, MQTT_OUTPUTS_STATES, pgmsDB[pgm].pgmName);
        pgmsDB[pgm].pgmFSM = ALL_DONE;
    }
}
//
//
//
void doPublishing(int prt) {
    lprintf("-------------- MQTT publishing -----------------------------------\n");
    publishZonesStatusChanges(prt);
    publishPartitionStatus(prt);
    if (!timeoutOps(GET, ALARM_PUBLISH_TIMER))            				// publish alarm(s)  on spec intervals
        return;
    if (partitionSTATS[prt].alarmZonesCnt)
        publishAlarm(prt);
    publishAlarmAndTroubleZones();
    lprintf("-------------- MQTT publishing done -------------------------------\n");
}

*/
