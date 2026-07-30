#pragma once
#include <stdio.h>
#include <string.h>

#include "..\alarm-core.h"
#include "alarm-core-internal-defs.h"
#include "alarm-core-printer-defs.h"
//#include "alarm-core-json-val-parsers.h" // temporary dependency; later split printer defs out

class alarmPrinter {
public:
    explicit alarmPrinter(Alarm& alarm) : m_alarm(alarm) {}

    inline void printAlarmPartitions(int startPt, int endPt) {
        lprintf("Partition(s)\n");
        printConfigHeader(partitionPrintProcessors, PARTITION_PRINT_KEYS_CNT);
        for (int j = startPt; j < endPt; j++) {
            if (!m_alarm.partitionDB[j].valid) continue;
            printConfigData(partitionPrintProcessors, PARTITION_PRINT_KEYS_CNT, (byte*)&m_alarm.partitionDB[j], PRTCLASS_ALL);
        }
        lprintf("\n");
    }

    inline void printAlarmZones(int startZn, int endZn) {
        lprintf("\nZone(s)\n");
        printConfigHeader(zoneCfgPrintProcessors, ZONE_CFG_PRINT_KEYS_CNT);
        for (int i = startZn; i < endZn; i++) {
            if (m_alarm.zonesDB[i].valid) {
                printConfigData(zoneCfgPrintProcessors, ZONE_CFG_PRINT_KEYS_CNT, (byte*)&m_alarm.zonesDB[i], PRTCLASS_ALL);
            }
        }
        lprintf("\n");
    }

    inline void printAlarmOptions(byte* optsPtr) {
        lprintf("\nGlobal options\n");
        printConfigHeader(gOptsPrintProcessors, GOPTS_PRINT_KEYS_CNT);
        printConfigData(gOptsPrintProcessors, GOPTS_PRINT_KEYS_CNT, optsPtr, PRTCLASS_ALL);
        lprintf("\n");
    }

    inline void printAlarmPgms() {
        lprintf("\nPGM(s)\n");
        printConfigHeader(pgmPrintProcessors, PGM_PRINT_KEYS_CNT);
        for (int i = 0; i < MAX_ALARM_PGM; i++) {
            if (!m_alarm.pgmsDB[i].valid) continue;
            printConfigData(pgmPrintProcessors, PGM_PRINT_KEYS_CNT, (byte*)&m_alarm.pgmsDB[i], PRTCLASS_ALL);
        }
        lprintf("\n");
    }

private:
    Alarm& m_alarm;

    inline void printConfigData(printKeyValProcessor targetKeys[], int numEntries, byte* targetPtr, int printClass) {
        const char* titlePtr = NULL;
        for (int i = 0; i < numEntries; i++) {
            if (printClass && (printClass != targetKeys[i].printClass)) continue;
            const jsonKeyValProcessor* p = targetKeys[i].parserKey;
            titlePtr = (const char*)targetKeys[i].unpatchCallBack(targetPtr, p->patchOffset, p->patchLen);
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

    inline void printConfigHeader(printKeyValProcessor targetKeys[], int numEntries) {
        for (int i = 0; i < numEntries; i++) {
            const char* key = targetKeys[i].parserKey->jsonKeyStr;
            lprintf("%s ", key);
            for (size_t j = strlen(key); j < (int)targetKeys[i].keyStrLen; j++) lprintf(" ");
        }
        lprintf("\n");
    }
};