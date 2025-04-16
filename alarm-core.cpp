// alarm-core.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
	/* vsprintf example */
#include <stdarg.h>
#include <windows.h>

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

int ErrWrite(int err_code, const char* what, ...)           // callback to dump info to serial console from inside RS485 library
{
    va_list args;
    va_start(args, what);
    // vprintf(what, args);
    // va_end(args);
    // int vprintf(const char* format, va_list arg);

    int index = 0;
    //lprintf ("error code %d received in errors handling callback\n------------------------", err_code);
    // update errors struct here
    vsnprintf(prnBuf, sizeof(prnBuf) - 1, what, args);
    switch (err_code)
    {
    case ERR_OK:
        printf(prnBuf);
        break;
    case ERR_DEBUG:
        printf(prnBuf);
        break;
    case ERR_INFO:
        printf(prnBuf);
        break;
    case ERR_WARNING:
        printf(prnBuf);
        break;
    case ERR_CRITICAL:
        printf(prnBuf);
        break;
    default:
        if(what)
            printf(prnBuf);
        break;
    }
    va_end(args);
    return err_code;
}

Alarm myAlarm;

int main(int argc, char* argv[])
{
    printf("Alarm test program\n");
    printf("Press any key to exit\n");

    // Create test data for a new zone
    ALARM_ZONE newZone = {};
    strncpy(newZone.zoneName, "Test Zone 1", NAME_LEN - 1); // Set zone name
    newZone.boardID = 1;                                   // Example board ID
    newZone.zoneID = 1;                                    // Example zone ID
    newZone.zoneType = INSTANT;                            // Set zone type
    newZone.zonePartition = PARTITION1;                   // Assign to partition 1
    newZone.zoneAlarmType = STEADY_ALARM;                  // Set alarm type
    newZone.valid = 1;                                     // Mark as unused initially

    // Add the zone to the alarm system
    int index = myAlarm.addZone(newZone);
    if (index != -1) {
        printf("Zone added at index %d\n", index);
    }
    else {
        printf("Failed to add zone. No unused entries available.\n");
    }

    myAlarm.getZoneCount();
    myAlarm.alarm_loop();

    while (true) {
        if (_kbhit()) {
            break;
        }
    }
    return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
