#pragma once
#include <stdio.h>
#include <string.h>

#include "..\alarm-core.h"
#include "alarm-core-internal-defs.h"
#include "alarm-core-json-val-parsers.h" // temporary dependency; later split printer defs out

class alarmPrinter {
public:
    explicit alarmPrinter(Alarm& alarm) : m_alarm(alarm) {}

    inline void printAlarmPartitions(int startPt, int endPt) {
        lprintf("Partition(s)\n");
        printConfigHeader(partitionKeyValProcessors, PARTITION_KEYS_CNT);
        for (int j = startPt; j < endPt; j++) {
            if (!m_alarm.partitionDB[j].valid) continue;
            printConfigData(partitionKeyValProcessors, PARTITION_KEYS_CNT, (byte*)&m_alarm.partitionDB[j], PRTCLASS_ALL);
        }
        lprintf("\n");
    }

    inline void printAlarmZones(int startZn, int endZn) {
        lprintf("\nZone(s)\n");
        printConfigHeader(zoneCfgKeyValProcessors, ZONE_CFG_KEYS_CNT);
        for (int i = startZn; i < endZn; i++) {
            if (m_alarm.zonesDB[i].valid) {
                printConfigData(zoneCfgKeyValProcessors, ZONE_CFG_KEYS_CNT, (byte*)&m_alarm.zonesDB[i], PRTCLASS_ALL);
            }
        }
        lprintf("\n");
    }

    inline void printAlarmOptions(byte* optsPtr) {
        lprintf("\nGlobal options\n");
        printConfigHeader(gOptsKeyValProcessors, GOPTS_KEYS_CNT);
        printConfigData(gOptsKeyValProcessors, GOPTS_KEYS_CNT, optsPtr, PRTCLASS_ALL);
        lprintf("\n");
    }

    inline void printAlarmPgms() {
        lprintf("\nPGM(s)\n");
        printConfigHeader(pgmKeyValProcessors, PGM_KEYS_CNT);
        for (int i = 0; i < MAX_ALARM_PGM; i++) {
            if (!m_alarm.pgmsDB[i].valid) continue;
            printConfigData(pgmKeyValProcessors, PGM_KEYS_CNT, (byte*)&m_alarm.pgmsDB[i], PRTCLASS_ALL);
        }
        lprintf("\n");
    }

private:
    Alarm& m_alarm;

    inline void printConfigData(jsonKeyValProcessor targetKeys[], int numEntries, byte* targetPtr, int printClass) {
        const char* titlePtr = NULL;
        for (int i = 0; i < numEntries; i++) {
            if (printClass && (printClass != targetKeys[i].printClass)) continue;

            titlePtr = (const char*)targetKeys[i].unpatchCallBack(targetPtr, targetKeys[i].patchOffset, targetKeys[i].patchLen);
            if (!titlePtr) titlePtr = "";

            if (strlen(titlePtr) > targetKeys[i].keyStrLen) {
                for (int j = 0; j < (int)targetKeys[i].keyStrLen; j++) lprintf("%c", titlePtr[j]);
                lprintf(" ");
            } else {
                lprintf("%s ", titlePtr);
                for (size_t j = strlen(titlePtr); j < (int)targetKeys[i].keyStrLen; j++) lprintf(" ");
            }
        }
        lprintf("\n");
    }

    inline void printConfigHeader(jsonKeyValProcessor targetKeys[], int numEntries) {
        for (int i = 0; i < numEntries; i++) {
            lprintf("%s ", targetKeys[i].jsonKeyStr);
            for (size_t j = strlen(targetKeys[i].jsonKeyStr); j < (int)targetKeys[i].keyStrLen; j++) lprintf(" ");
        }
        lprintf("\n");
    }
};