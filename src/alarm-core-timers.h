#pragma once
//
enum timeoutOper {
	SET = 1,
	GET = 2,
	FORCE = 3,
};
//
//enum TIMERS {
//	ALARM_LOOP_TIMER = 1,
//	STATUS_REPORT_TIMER,
//	MQTT_CONNECT_TIMER = 6,
//	WIFI_CONNECT_TIMER = 7,
//};

namespace WinTimers {
#ifndef ARDUINO
#include <windows.h>
	unsigned long millis() {
		SYSTEMTIME st;
		GetSystemTime(&st);
		printf("The system time is: %02d:%02d:%02d\n", st.wHour, st.wMinute,st.wSecond);
		unsigned long res = (st.wHour * 60 * 60 * 1000 + st.wMinute * 60 * 1000 + st.wSecond * 1000 + st.wMilliseconds);
		printf("Time in mS: %lu\n", res);
		return res;
	} 
#endif
//	//
//	// command records structure for cmdDB
//	struct TIMER {
//		int timerID;
//		unsigned long interval;
//		unsigned long setAt;
//	};
//	//
//	// timerss database to look-up timer params TODO - move all timers staff to helpers or separate file. 
//	// 
//	struct TIMER timerDB[] = {	{ALARM_LOOP_TIMER, ALARM_LOOP_INTERVAL, 0},
//								{STATUS_REPORT_TIMER, STATUS_REPORT_INTERVAL, 0},
//								{MQTT_CONNECT_TIMER, MQTT_CONNECT_INTERVAL, 0},
//							 };
//
//	int findTimer(byte timer) {
//		//Alarm::ErrWrite(ERR_DEBUG, "Looking for record for timer ID   %d \n", timer);
//		for (int i = 0; i < sizeof(timerDB) / sizeof(struct TIMER); i++) {
//			//printf("Looking at index  %d out of  %d:\n", i, sizeof(cmdDB)/sizeof(struct COMMAND)-1);
//			if (timerDB[i].timerID == timer) {
//				//ErrWrite(ERR_DEBUG,"Found timer at  index %d\n", i);
//				return  i;
//			}
//		}
//		//ErrWrite(ERR_DEBUG, "Timer not found!!!!!!!\n");
//		return ERR_IDX_NOT_FND;
//	}
//	// 
//	// set timeout / check if timeout expired
//	// TODO - organize all timeouts as separate database similar to commands
//	// 
//	bool timeoutOps(int oper, int whichOne) {
//		int index;
//		// find timer index first
//		if ((index = findTimer(whichOne)) == ERR_IDX_NOT_FND) {        // timer not found
//			//Alarm::ErrWrite(ERR_CRITICAL, "Timer %d NOT FOUND\n", whichOne);
//			return false;						        // TODO - report error
//		}
//		if (oper == SET) {                              // record the current time in milliseconds
//			timerDB[index].setAt = millis();
//			return 0;
//		}
//		if (oper == FORCE) {                            // force timer to report as expired on next GET op
//			timerDB[index].setAt = 0;
//			return 0;
//		}
//		else {
//			unsigned long res = (unsigned long)(millis());  // GET
//			return ((res - timerDB[index].setAt) > (unsigned long)timerDB[index].interval);
//		}
//	}
//
//	// set timer interval
//	// 
//	bool timerSetInterval(int whichOne, unsigned long Interval) {
//		int index;
//		// find timer index first
//		if ((index = findTimer(whichOne)) == ERR_IDX_NOT_FND)	    // timer not found
//			return false;						    // TODO - report error
//		timerDB[index].interval = Interval;
//		return true;
//	}
//
}
//
// -------------- end timers -----------------------------

// OLD: using namespace WinTimers;
// OLD: bool timeoutOps(int oper, int whichOne) { ... }

// NEW: Replace with TimerManager methods
