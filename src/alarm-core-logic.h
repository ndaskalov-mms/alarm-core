#pragma once

#ifndef ALARM_H
#error "This file should only be included inside the Alarm class definition"
#endif

#include "alarm-core-timers.h"
using namespace WinTimers;  // neede to avoid conflict between ArduinoJSON.h and windows.h

void Alarm::resetAllPartitionTimers(int prt) {
	int j;
	partitionRT[prt].partitionTimers[EXIT_DELAY_TIMER].timerDelay = partitionDB[prt].exitDelay;
	partitionRT[prt].partitionTimers[EXIT_DELAY_TIMER].bypassWhat = EXIT_DELAY_ZONES;
	partitionRT[prt].partitionTimers[EXIT_DELAY_TIMER].bypassMask = ZONE_EX_D_BYPASSED;
	//
	partitionRT[prt].partitionTimers[ENTRY_DELAY1_TIMER].timerDelay = partitionDB[prt].entryDelay1Intvl;
	partitionRT[prt].partitionTimers[ENTRY_DELAY1_TIMER].bypassWhat = ENTRY_DELAY1_ZONES;
	partitionRT[prt].partitionTimers[ENTRY_DELAY1_TIMER].bypassMask = ZONE_EDx_BYPASSED;
	//
	partitionRT[prt].partitionTimers[ENTRY_DELAY2_TIMER].timerDelay = partitionDB[prt].entryDelay2Intvl;
	partitionRT[prt].partitionTimers[ENTRY_DELAY2_TIMER].bypassWhat = ENTRY_DELAY2_ZONES;
	partitionRT[prt].partitionTimers[ENTRY_DELAY2_TIMER].bypassMask = ZONE_EDx_BYPASSED;
	//
	for (j = 0; j < MAX_PARTITION_TIMERS; j++)
		partitionRT[prt].partitionTimers[j].timerFSM = NOT_STARTED;
	//
	partitionRT[prt].changed |= (CHG_EXIT_DELAY_TIMER | CHG_ENTRY_DELAY1_TIMER | CHG_ENTRY_DELAY2_TIMER);	// TODO ??? tova trqbwa da e vyv funkciata force MQTT report
	return;
}
//-----------------------------------------------------------------------------------
// Implementation of selected methods
//-----------------------------------------------------------------------------------
//void Alarm::initRTdata(void) {
//	int i;
//	// reset zone's tun-time data
//	for (int j = 0; j < MAX_ALARM_ZONES; j++) {
//		memset((byte*)&zonesRT[j], 0x0, sizeof(ALARM_ZONE_RT));
//		zonesRT[j].zoneStat = ZONE_CLOSE;
//		zonesRT[j].changed = 0;
//	}
//	for (i = 0; i < MAX_PARTITION; i++) {
//		// reset all partition statistics
//		memset((byte*)&partitionSTATS[i], 0x0, sizeof(ALARM_PARTITION_STATS_t));
//		// init rut-time partition timers with configured delays in seconds
//		resetAllPartitionTimers(i);
//		// reset ARM state
//		partitionRT[i].armStatus = partitionRT[i].targetArmStatus = DISARM;
//		partitionRT[i].newCmd = false; partitionRT[i].changed = 0;
//	}
//	// clear all HW errors
//	alarmGlobalOpts.SprvsLoss = 0; alarmGlobalOpts.ACfail = alarmGlobalOpts.BatFail = alarmGlobalOpts.BellFail = alarmGlobalOpts.BrdFail = 0;
//}

//
// clearBypass- un- baypasses zone
// params:  int zn			- zone index 0 .. MAX_ALARM_ZONES
//			int	bypassType	- which bit to unset
// returns: none
//
void Alarm::clearBypass(byte zn, int bypassType) {
	if (!(zonesRT[zn].bypassed & bypassType))										// check if cleared already
		return;																		// zone is already bypassed/unbypassed
	zonesRT[zn].bypassed &= ~bypassType;											// bypass
	zonesRT[zn].changed |= ZONE_BYPASS_CHANGED;										// mark zone as changed
	partitionRT[zonesDB[zn].zonePartition].changed |= CHG_ZONE_CHANGED;				// and partition too
}
//
// clearBypasssAllZones- un- baypasses all alarm zones assigned to partition
//
void Alarm::clearBypassAllZones(int partIxd) {
	for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {
		if (zonesDB[zn].zonePartition == partIxd) {
			if (zonesRT[zn].bypassed == ZONE_NO_BYPASS)								// check if cleared already
				continue;															// zone is already bypassed/unbypassed
			zonesRT[zn].bypassed = 0;												// clears all bypasses 
			zonesRT[zn].changed |= ZONE_BYPASS_CHANGED;								// mark zone as changed
			partitionRT[zonesDB[zn].zonePartition].changed |= CHG_ZONE_CHANGED;		// and partition too
			
		}
	}
}
//
// bypassZone- bypasses zone if allowed. 
// params:  byte zn			- zone index 0 .. MAX_ALARM_ZONES 
//			int bypassType	- (USER) BYAPASS, ZONE_FORCED, ZONE_STAY_BYPASSED..., .....
// returns: none
//
inline void Alarm::bypassZone(byte zn, int bypassType) {
	if (!((zonesRT[zn].bypassed & bypassType ) ^ bypassType))						// check if the same bypasses are already in place
		return;																		// zone is already bypassed/unbypassed
	zonesRT[zn].bypassed |= bypassType;												// reflect the change
	zonesRT[zn].changed |= ZONE_BYPASS_CHANGED;										// mark zone as changed
	partitionRT[zonesDB[zn].zonePartition].changed |= CHG_ZONE_CHANGED;				// and partition too
	return;
}
//
// bypassAllForceZones - bypasses all alarm forceable  zones assigned to partition
//
void Alarm::bypassAllForceZones(int prtId) {
	for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {
		if (zonesDB[zn].zonePartition == prtId) {
			if (!zonesRT[zn].bypassed) {											// if zone is not bypassed already
				if (zonesRT[zn].zoneStat & ZONE_OPEN) {								// and is open
					if (zonesDB[zn].zoneForceEn) {									// and can be forced
						bypassZone(zn, ZONE_FORCED);
					}
				}
			}
		}
	}
}
//
// bypassAllStayZones - baypasses all stay  zones assigned to partition
//
void Alarm::bypassAllStayZones(int prtId) {
	for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {
		if (zonesDB[zn].zonePartition == prtId) {
			if (zonesDB[zn].zoneStayZone) {
				bypassZone(zn, ZONE_STAY_BYPASSED);
			}
		}
	}
}
void Alarm::bulkBypassZones(int prtId, int znType, int bypassBits, int invert) {
	for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {
		if (zonesDB[zn].zonePartition == prtId)										// skip zones belonging to other partitions
			if (zonesDB[zn].zoneType & znType) {									// if zoneType is what is wanted (zone types are bitmask) 
				if (invert)															// invert==true means clearBypass
					clearBypass(zn, bypassBits);
				else
					bypassZone(zn, bypassBits);
			}
	}
}
//
//
// set or get partition EXIT DELAY, ENTRY_DELAY1 or ENTRY_DELAY2 timer
// params: timer - int ENTRY_DELAY1 or ENTRY_DELAY2 or EXIT_DELAY_TIMER
//		   oper  - int SET, GET or FORCE
//		   partition - int index in partitionDB
// returns: ERR_OK for SET
//			> 0 if timeout set by corresponding delay in partition definition expired, 0 if not
// 
int Alarm::partitionTimer(int tmr, int oper, int prt) {
//
	if(tmr >= MAX_PARTITION_TIMERS)
		ErrWrite(ERR_CRITICAL, "partitionTimer: Invalid timer for partition %d\n", prt);
	if (oper == SET) {													// record the current time in milliseconds
		partitionRT[prt].partitionTimers[tmr].timerStart_ms = millis();
		partitionRT[prt].partitionTimers[tmr].timerFSM = RUNNING;		// and mark it as running
		partitionRT[prt].changed |= partitionRT[prt].partitionTimers[tmr].changedMask;// will be used to mark which timer is changed for MQTT publish for partition
	}
	else if (oper == GET) 									
		return ((unsigned long)(millis() - partitionRT[prt].partitionTimers[tmr].timerStart_ms) > ((unsigned long)partitionRT[prt].partitionTimers[tmr].timerDelay) * 1000);
	return ERR_OK;
}
//
void Alarm::processPartitionTimers(int prt) {
	int tmr;
	if (!partitionDB[prt].valid)																// not a valid partition
		return;
	for (tmr = 0; tmr < MAX_PARTITION_TIMERS; tmr++) {
		if (partitionRT[prt].partitionTimers[tmr].timerFSM == RUNNING) {
			if (partitionTimer(tmr, GET, prt)) {
				ErrWrite(ERR_DEBUG, "Partition EXIT/ENTRY delay timer expired\n");
				partitionRT[prt].partitionTimers[tmr].timerFSM = DONE;							// TIMER expired
				partitionRT[prt].changed |= partitionRT[prt].partitionTimers[tmr].changedMask;	// force MQTT report
				// unbypass all zones bypassed at ENTRY DELAY start
				bulkBypassZones(prt, partitionRT[prt].partitionTimers[tmr].bypassWhat, partitionRT[prt].partitionTimers[tmr].bypassMask, CLEAR_BYPASS);
			}
		}
	}
}
//
int Alarm::countNotBypassedEntryDelayZones(int prt) {
int res=0;
	for(int zn=0; zn < MAX_ALARM_ZONES; zn++) {  
		if(zonesDB[zn].zonePartition == prt) {
			if(zonesDB[zn].zoneType) {											// 0 = zone disabled
				if((zonesDB[zn].zoneType == ENTRY_DELAY1) || (zonesDB[zn].zoneType == ENTRY_DELAY2)) {
					if(!zonesRT[zn].bypassed) 
						res++;
				}
			}
		}
	}
	return res;
}	
//
// return all zones which will prevent arm - open zone which ae not bypassed
//
int Alarm::check4openUnbypassedZones(int prt) {
	int res = 0;
	for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {
		if (zonesDB[zn].zonePartition == prt) {
			if (zonesRT[zn].bypassed) 								
				continue;
			if (zonesDB[zn].zoneType && (zonesRT[zn].zoneStat != ZONE_CLOSE)) // 0 = zone disabled
					res++;
		}
	}
	return res;
}

//
// alarm-cmds.h

//extern void bypassZone(byte zoneIdx, int requestor);
//extern void clearBypass(byte zoneIdx, int bypassType);

//
// actions DISARM, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM
//
//void Alarm::trigerArm(void* param1, void* param2, void* param3) {
//	const int partID = *(int*)param1;
//	const int action = *(int*)param2;
//	// param3 is not used
//
//	if (!partitionDB[partID].valid)
//		return;
//	if (partID < 0 || partID >= MAX_PARTITION) {
//		ErrWrite(ERR_WARNING, "CMD parse: Invalid PARTITION ID %d", partID);
//		return;
//	}
//	// check first for valid params ranges and call
//	if (!((action == DISARM) || (action == REGULAR_ARM) || (action == STAY_ARM) || (action == FORCE_ARM) || (action == INSTANT_ARM))) {
//		ErrWrite(ERR_WARNING, "CMD parse: Invalid ARM action %d", action);
//		return;
//	}
//	//partitionRT[partID].newCmd = true;
//	partitionRT[partID].changed |= CHG_NEW_CMD;				// mark that action is needed
//	partitionRT[partID].targetArmStatus = action;			// set new target arm state
//	//if(action != DISARM_ALL)								// set new target arm state
//	//	partitionRT[partID].targetArmStatus = action;		// armed successfully
//	//else {													// DISARM ALL command
//	//	for (int i = 0; i < MAX_PARTITION; i++) {
//	//		partitionRT[partID].targetArmStatus = action;	
//	//	}
//	//}
//}
//
// action = ZONE_OPEN_CMD, ZONE_AMASK_CMD, ZONE_CLOSE_CMD, ZONE_TAMPER_CMD, ZONE_BYPASS_CMD
//
void Alarm::modifyZn(void* param1, void* param2, void* param3) {
	//byte newStat;
	const int zn = *(int*)param1;								// index in zonesD
	const int action = *(int*)param2;								// command - defined in enum ZONE_CMDS_t
	const char* payldPtr = (const char*)param3;
	if (payldPtr)
		ErrWrite(ERR_WARNING, "modifyZn: WARNING - garbage found after modify zone command %s\n", payldPtr);
	//lprintf("modifyZn: ZONE action %d\n", action);
	//
	if (!zonesDB[zn].zoneType)												// zoneType == 0 means disabled
		return;
	if (zn >= MAX_ALARM_ZONES) {
		ErrWrite(ERR_WARNING, "modifyZn: Zone No %d out of range!\n", zn);
		return;
	}
	// handle zone status change commands
	if (action == ZONE_OPEN_CMD || action == ZONE_CLOSE_CMD || action == ZONE_TAMPER_CMD || action == ZONE_AMASK_CMD) {
		// set result according to action selected 
		switch (action) {
		case ZONE_OPEN_CMD:
			zonesRT[zn].zoneStat = ZONE_OPEN;
			break;
		case ZONE_CLOSE_CMD:
			clearBypass(zn, ZONE_FORCED);										// if closed now it shall start acting as if it was not forced
			zonesRT[zn].in_alarm = NO_ALARM;									// when zone close, alarm/trouble goes off clear zone alarm flag
			zonesRT[zn].in_trouble = NO_TROUBLE;								// clear zone alarm/trouble flag
			zonesRT[zn].ignorredTamper = false;									// clear zone ignorred tamper flag
			zonesRT[zn].ignorredAmask = false;									// clear zone ignorred anti mask flag
			zonesRT[zn].openEDSD1zone = zonesRT[zn].openEDSD2zone = false;		// clear open EDx/SDx zone in EDx interval
			zonesRT[zn].zoneStat = ZONE_CLOSE;
			break;
		case ZONE_TAMPER_CMD:
			zonesRT[zn].zoneStat = ZONE_TAMPER;
			break;
		case ZONE_AMASK_CMD:
			zonesRT[zn].zoneStat = ZONE_AMASK;
			break;
		}
		zonesRT[zn].changed |= ZONE_STATE_CHANGED;								// mark zone as changed
		partitionRT[zonesDB[zn].zonePartition].changed |= CHG_ZONE_CHANGED;		// and partition too
	}
	else if (action == ZONE_BYPASS_CMD || action == ZONE_UNBYPASS_CMD) {
		switch (action) {
		case ZONE_BYPASS_CMD:
			// Do some checks if zone bypass is allowed and return if not
			if (zonesDB[zn].zoneType & SPECIAL_ZONES) {
				ErrWrite(ERR_WARNING, "Attempt to bypass special zone  (like 24H FIRE)  %s\n", zonesDB[zn].zoneName);
				break;															// fire zones cannot be bypassed, forced, ...
			}
			if (!(zonesDB[zn].zoneBypassEn)) {									// allowed to bypass on user request?
				ErrWrite(ERR_DEBUG, "Zone %s,  bypass disabled\n", zonesDB[zn].zoneName);
				break;															// no, return. (publishZoneStatusChanges will deect that command is not executed)
			}
			bypassZone(zn, ZONE_BYPASSED);
			//newZonesDataAvailable |= NEW_DATA_BIT;							// force alarm loop to execute		
			break;
		case ZONE_UNBYPASS_CMD:
			clearBypass(zn, ZONE_BYPASSED);
			break;
		}
		zonesRT[zn].changed |= ZONE_USR_BYPASS_CHANGED;							// mark that zone changed on user request
		partitionRT[zonesDB[zn].zonePartition].changed |= CHG_ZONE_CHANGED;		// and partition too
	}
	else {
		ErrWrite(ERR_WARNING, "modifyZn: Invalid ZONE action %d\n", action);
		return;
	}
}
//
//
//
void Alarm::modifyPgm(void* param1, void* param2, void* param3) {
	const int pgmIdx = *(int*)param1;
	const int action = *(int*)param2;
	lprintf("modifyPGM: PGM action %d\n", action);
	if (!pgmsDB[pgmIdx].valid)
		return;
	if (!(action == PGM_ON || action == PGM_OFF || action == PGM_PULSE)) {
		ErrWrite(ERR_WARNING, "modifyPGM: Invalid PGM action %d", action);
		return;
	}
	// set result according to action selected 
	//if (pgmsDB[brd][pgm].cValue == action)
	//	return;														// nothing to do
	ErrWrite(ERR_DEBUG, "modifyPGM: Setting PGM %s to %d\n", pgmsDB[pgmIdx].pgmName, action);
	pgmsDB[pgmIdx].tValue = action;									// reflect the change
	pgmsDB[pgmIdx].pgmFSM = TO_IMPLEMENT;							// mark there is new value to set
	//pgmsDB[pgmIdx].sValue = PGM_UNDEFINED;						// mark that transaction is on-going
	//pgmsDB[pgmIdx].cValue = PGM_UNDEFINED;						// mark that transaction is on-going
	//newPGMsDataAvailable |= NEW_DATA_BIT << brd;					// note it for later	
}
//


//
// check arm restrictions
// params: byte partIxd - partition number (ID) to be used as index in partitionDB
//			   int 	action	- bitmask, DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM, see enum  ARM_METHODS_t
//	returns: true if no restrictions found, otherwise false
//
int Alarm::checkArmRestrctions(byte partIdx, int action) {
	ErrWrite(ERR_DEBUG, "Checking arm restictions for part %d - ", partIdx);
	// TODO - add read from ADC for voltages and support for board, bell and RF link 
	if (alarmGlobalOpts.restrOnSprvsLoss & alarmGlobalOpts.SprvsLoss) {
		ErrWrite(ERR_INFO, "ARM blocked due to SUPERVISOR LOSS restiction\n");
		return false;
	}
	if (alarmGlobalOpts.restrOnACfail & alarmGlobalOpts.ACfail) {
		ErrWrite(ERR_INFO, "ARM blocked due to AC fail restiction\n");
		return false;
	}
	if (alarmGlobalOpts.restrOnBatFail & alarmGlobalOpts.BatFail) {
		ErrWrite(ERR_INFO, "ARM blocked due to BAT fail restiction\n");
		return false;
	}
	if (alarmGlobalOpts.restrOnBellFail & alarmGlobalOpts.BellFail) {
		ErrWrite(ERR_INFO, "ARM blocked due to BELL fail restiction\n");
		return false;
	}
	if (alarmGlobalOpts.restrOnBrdFail & alarmGlobalOpts.BrdFail) {
		ErrWrite(ERR_INFO, "ARM blocked due to BRD fail restiction\n");
		return false;
	}
	if (alarmGlobalOpts.restrOnTamper & partitionSTATS[partIdx].tamperZonesCnt) {
		if (partitionSTATS[partIdx].tamperZonesCnt != partitionSTATS[partIdx].ignorredTamperZonesCnt) {
			ErrWrite(ERR_INFO, "ARM blocked due to TAMPER fail restiction\n");
			return false;
		}
	}
	if (alarmGlobalOpts.restrOnAntimask & partitionSTATS[partIdx].amaskZonesCnt) {
		ErrWrite(ERR_INFO, "ARM blocked due to ANTI-MASK restiction\n");
		return false;
	}
	ErrWrite(ERR_DEBUG, "none\n"); 
	return true;
}
//
// arm partition 
// params: byte partIxd - partition number (ID) to be used as index in partitionDB
//		   byte	action	- bitmask, DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM, see enum  ARM_METHODS_t
//		   int follows	- 
// returns: 0 if successfull, !0 if restriction or error
//
int Alarm::armPartition(byte prt, ARM_METHODS_t action) {
	int followID; int i;
	if (!partitionDB[prt].valid) {
		ErrWrite(ERR_WARNING, "Request to arm/disarm invalid partition %d\n", prt);
		return true;
	}
	if (!(prt < MAX_PARTITION)) {
		ErrWrite(ERR_WARNING, "Request to arm/disarm out-of-range partition %d\n", prt);
		return true;
	}
	//
	// fan-out the command to follow partitions first
	for (i = 0; i < MAX_PARTITION; i++) {
		followID = partitionDB[prt].follows[i];
		if (!followID)																// follow partitions are starting from 1, 0 means invalid
			continue;
		followID--;																	// convert to index in partitionsDB
		if (!partitionDB[followID].valid)											// skip invalid partitions
			continue;
		else
			//trigerArm(&followID, &action, NULL);									// set the same action for follow partitions 
			setPartitionTarget(followID, action);
	}
	// now proceed to action on current partition
	if (action == DISARM) {
		ErrWrite(ERR_DEBUG, "Disarming partition %d\n", prt);
		clearBypassAllZones(prt);
		partitionSTATS[prt].alarmZonesCnt = 0;
		partitionRT[prt].armTime = millis();
		resetAllPartitionTimers(prt);
		return false;
	}
	//
	if (partitionRT[prt].armStatus == action) {
		ErrWrite(ERR_DEBUG, "Partition %d already %s armed\n", prt, action ? " " : "dis");
		return false;
	}
	if (action != DISARM) {															// this is always the case, we checked for DISARM earlier
		if (!checkArmRestrctions(prt, action)) {
			ErrWrite(ERR_INFO, "Partition %d cannot armed due to restrictions\n", prt);
			return true;
		}
	}
	ErrWrite(ERR_DEBUG, "Arming partition %d\n", prt);
	//
	// if STAY ARM or partition force enable, set FORCE bypass on all open zones
	if ((action == STAY_ARM) || (action == INSTANT_ARM)) {							// TODO - what is the diff btw STAY and INSTANT?
		bypassAllStayZones(prt);
	}
	//
	// if FORCE ARM or partition force enable, set FORCE bypass on all open zones
	if ((action == FORCE_ARM) ||
		((action == REGULAR_ARM) && partitionDB[prt].forceOnRegularArm) ||
		(((action == STAY_ARM) || (action == INSTANT_ARM)) && partitionDB[prt].forceOnStayArm)) {
		bypassAllForceZones(prt);
	}
	// check for open zones preventing arm
	if (check4openUnbypassedZones(prt)) {
		// revert all bypasses so far, EXIT_DELAY_ZONES includes all bypassable zones
		ErrWrite(ERR_DEBUG, "Partition %d cannot be armed due to open zone(s)\n", prt);
		bulkBypassZones(prt, EXIT_DELAY_ZONES, ZONE_STAY_BYPASSED | ZONE_FORCED, CLEAR_BYPASS);
		return true;
	}
	// set EXIT DELAY BYPASS ON ALL ZONES  first
	bulkBypassZones(prt, EXIT_DELAY_ZONES, ZONE_EX_D_BYPASSED, SET_BYPASS);
	//
	// partition is properly armed, do some houskeeping
	//
	partitionRT[prt].armStatus = action;							// update arm status
	partitionTimer(EXIT_DELAY_TIMER, SET, prt);						// start exit delay and publish accordingly
	ErrWrite(ERR_DEBUG, "Partition EXIT delay timer started\n");
	//
	ErrWrite(ERR_DEBUG, "Partition %d armed\n", prt);
	// 
	return false;
}
//
//  process alarm as specified by partition options:
// 		alarmOutputEn - enable to triger bell or siren once alarm condition is detected in partition
//		alarmCutOffTime -cut alarm output after 1-255 seconds
//		noCutOffOnFire - disable cut-off for fire alarms
//		alarmRecycleTime -re-enable after this time if alarm condition not fixed
//		MQTT:			alarm, audible_alarm, silent_alarm, strobe_alarm,
// TODO - implement me
//
void Alarm::processAlarm(int alarm) {
	//printf("Implemen siren control here\n");
}
//
void Alarm::processLineErrors(int zn, int opts) {
	switch (opts) {
	case LINE_ERR_OPT_DISABLED:									// do nothing
		return;
	case LINE_ERR_OPT_TROUBLE_ONLY:
		//reportTrouble(zone);									// only report the tamper, no alarms
		zonesRT[zn].in_trouble = TROUBLE;
		break;
	case LINE_ERR_OPT_ALARM_WHEN_ARMED:
		if (partitionRT[zonesDB[zn].zonePartition].armStatus)
			//trigerAlarm(zone.zonePartition);					// when armed - generete alarm
			zonesRT[zn].in_alarm = zonesDB[zn].zoneAlarmType;
 		else
 			//reportTrouble(zone);								// when disarmed - only report the tamper, no alarms
			zonesRT[zn].in_trouble = TROUBLE;
		break;
	case LINE_ERR_OPT_ALARM:
		//trigerAlarm(zone.zonePartition);
		zonesRT[zn].in_alarm = zonesDB[zn].zoneAlarmType;
		break;
	}
}
//
// process zone error, generates trouble or alarm
// if alarmGlobalOpts.tamperBpsOpt == true - if zone is bypassed ignore tamper;
//									  false - follow global or local tamper settings
// zone global tamper options:	alarmGlobalOpts.tamperOpts, bitmask, same as local - see #define ZONE_TAMPER_OPT_XXXXXX
// Tamper Bypass Options
// (default = enabled) When enabled in section[3034] [Tamper Recognition Options] , the control panel will
// ignore the zone’s bypass definition and will follow the option set in section (page 25) (enum ZONE_TAMPER_OPT_t) if a tamper or wire
// fault occurs on a bypassed zone.
// When disabled, Tamper Recognition follows the zone’s bypass definition.This means that the control panel will not perform 
// any action if a tamper or wire fault occurs on a bypassed zone.
// My understanding is: if tamper occurs in bypassed zone and tamper recognition is OFF, we have to ignore if the zone is bypassed.
// The reaction if tamper recognition is ON is specified by local or global tamper options, selection is done by followPanel option
//
void Alarm::processTampers(int zn) {
	int opt;
	if (zonesRT[zn].bypassed & ZONE_BYPASSED) {					// can we ignore tamper on USER bypassed zone
		if (alarmGlobalOpts.tamperBpsOpt) {						// allowed to ignore tamper in USER bypassed zone ONLY!
			ErrWrite(ERR_DEBUG, "Tamper in bypassed zone %s  with Tamper tamper bypass enabled in global options\n", zonesDB[zn].zoneName);
			zonesRT[zn].ignorredTamper = true;					// set zone ignorred tamper flag
			zonesRT[zn].in_trouble = NO_TROUBLE;				// reset in_trouble flag flag
			zonesRT[zn].in_alarm = NO_ALARM;					// reset in_alarm flag flag
			return;
		}
	}
	zonesRT[zn].ignorredTamper = false;							// reset zone ignorred tamper flag
	// process tamper according to global or local options, depends on Zone tamper follow panel flag in zone's definition
	if (zonesDB[zn].zoneTamperFpanel)							// find out global or local options to follow
		opt = alarmGlobalOpts.tamperOpts;						// follow the global tamper settings
	else
		opt = zonesDB[zn].zoneTamperOpts;								// follow the local tamper settings// process tamper according to local or gloobal tamper options
	processLineErrors(zn, opt);
	return;
}
//
// process anti-mask error (line short), generates trouble or alarm
// global a-mask options:	alarmGlobalOpts.antiMaskOpts, bitmask, same as local - see #define LINE_ERR_OPT_XXXXXX
//
void Alarm::processAmasks(int zn) {
	int opt;
	if (zonesDB[zn].zoneAmaskFpanel)							// find out global or local options to follow
		opt = alarmGlobalOpts.antiMaskOpt;						// follow the global tamper settings
	else
		opt = zonesDB[zn].zoneAmaskOpts;						// follow the global tamper settings
	processLineErrors(zn, opt);
	return;
}
//
//	processEntryDelayZones(struct ALARM_ZONE zone)
//	return:	TRUE if alarm shall be issued, otherwise FALSE
//
int Alarm::processEntryDelayZones(int zn) {
	byte  prt = zonesDB[zn].zonePartition;
	byte  timer;
	// select correct timer & collaterals based on zone's ENTRY DELAY type 
	switch (zonesDB[zn].zoneType) {
	case ENTRY_DELAY1:
		zonesRT[zn].openEDSD1zone = true;						// mark it for statistics
		timer = ENTRY_DELAY1_TIMER;
		break;
	case STAY_DELAY1:
		zonesRT[zn].openEDSD1zone = true;						// mark it for statistics
		timer = ENTRY_DELAY1_TIMER;
		break;
	case ENTRY_DELAY2:
		zonesRT[zn].openEDSD2zone = true;						// mark it for statistics
		timer = ENTRY_DELAY2_TIMER;
		break;
	case FOLLOW:
		if (!partitionDB[prt].followEntryDelay2)
			return zonesDB[zn].zoneAlarmType;					// note - if condition fails, the execution falls thru 
	case STAY_DELAY2_ANTI_MASK:
		zonesRT[zn].openEDSD2zone = true;						// mark it for statistics
		timer = ENTRY_DELAY2_TIMER;
		break;
	default:
		ErrWrite(ERR_CRITICAL, "processEntryDelayZones: zone %s is not of ENTRY/STAY?FOLLOW type\n", zonesDB[zn].zoneName);	
		break;
	}
	//
	// handle ENTRY DELAY FSM here										
	if (partitionRT[prt].partitionTimers[timer].timerFSM == NOT_STARTED) {	// timer is not started, do it now
		partitionTimer(timer, SET, prt);
		ErrWrite(ERR_DEBUG, "Partition ENTRY delay timer started\n");
		// bypass all zones of this type and FOLLOW type zones, when timer is done, they will be unbypassed
		bulkBypassZones(prt, (zonesDB[zn].zoneType | FOLLOW), ZONE_EDx_BYPASSED, SET_BYPASS);
		return NO_ALARM;										// no alarm needed YET
	}
	else if (partitionRT[prt].partitionTimers[timer].timerFSM == RUNNING) {					// shall never happen as zone shall be bypassed when entry delay is active	
		ErrWrite(ERR_DEBUG, "Zone %s open but ENTRY DELAY stil RUNNING\n", zonesDB[zn].zoneName);
		return NO_ALARM;										// no alarm needed
	}
	else 														// *prtEntryDelayFSM == DONE - ED timer exired, request alarm
		return zonesDB[zn].zoneAlarmType;						// request alarm because entry delay expired
}
//
// process open zones , generates trouble or alarm
//
int Alarm::processOpenZone(int zn) {
	int alarm = NO_ALARM;									// raise alarm?
	byte prt = zonesDB[zn].zonePartition;
	//
	if (zonesRT[zn].bypassed || (zonesDB[zn].zoneType == ZONE_DISABLED))
		return NO_ALARM;									// ignore disabled and bypassed zones
	// check for 24H and special zones, they  generate unconditionally alarm when opened
	if (zonesDB[zn].zoneType & (SPECIAL_ZONES | H24_BUZZER | H24_BURGLAR)) {	
		alarm |= zonesDB[zn].zoneAlarmType;					// issue alarm
	}
	// if partition is NOT armed - no alarm
	if (partitionRT[prt].armStatus == DISARM) 					
		return NO_ALARM;										
	// now process open zone
	switch (zonesDB[zn].zoneType) {
	case ZONE_DISABLED:
		break;												// nothing to do
	case INSTANT:
		alarm |= zonesDB[zn].zoneAlarmType;					// issue alarm
		break;
	case ENTRY_DELAY1:										// ENTRY delay zones are treated as such only in 
	case ENTRY_DELAY2:										// REGULAR and FORCE arm, otherwise they are treated as instant zones
		if ((partitionRT[prt].armStatus == REGULAR_ARM) || (partitionRT[prt].armStatus == FORCE_ARM)) 
			alarm |= processEntryDelayZones(zn);
		else
			alarm |= zonesDB[zn].zoneAlarmType;
		break;
	case STAY_DELAY1:										// STAY delay zones are treated as such only in 
	case STAY_DELAY2_ANTI_MASK:								// STAY and INSTANT arm, otherwise they are treated as instant zones
		if ((partitionRT[prt].armStatus == STAY_ARM) || (partitionRT[prt].armStatus == INSTANT_ARM)) 
			alarm |= processEntryDelayZones(zn);
		else
			alarm |= zonesDB[zn].zoneAlarmType;
		break;
	case FOLLOW:
		// if all ENTRY_DELAY_X zones are bypassed, FOLLOW zones shall kick-off the delay timer (2) if enable in partition options
		if (partitionSTATS[prt].notBypassedEntyDelayZones)	// check if all entry delay zones are bypassed
			// start of ENTRY DELAY X will bypass FOLLOW zones as well, means if we get here, the enrty delay is not started
			alarm |= zonesDB[zn].zoneAlarmType;				// generate alarm
		else												// follow zone when all entry delay zones are bypassed will end-up here
			alarm |= processEntryDelayZones(zn);			// handle it as entry delay 2 zone
		break;
	default:
		break;
	}
	return alarm;
}
//
// alarmLoop() - implement all alarm business
//
void Alarm::alarm_loop() {
	//
	int prt; int alarm = NO_ALARM; int trouble = NO_TROUBLE;
	// 
	// run alarm loop only if there is zones status change or on ALARM_LOOP_INTERVAL
	if (!timeoutOps(GET, ALARM_LOOP_TIMER))										// run the loop on spec intervals									
		return;																	// interval did not expired, do something else
	else
		timeoutOps(SET, ALARM_LOOP_TIMER);										// restart timer and execute the alarm logic
	//
	for (prt = 0; prt < MAX_PARTITION; prt++) {									// process each partition individually
		if (!partitionDB[prt].valid)											// not a valid partition
			continue;
		//
		processPartitionTimers(prt);											// ENTRY and EXIT DELAYS
		//
		if (!partitionRT[prt].changed)											// something changed in partition?
			continue;															// no, continue
		//
		memset((byte*)&partitionSTATS[prt], 0x0, sizeof(ALARM_PARTITION_STATS_t)); // clear partition stats
		//
		for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {							// for each board' zone
			if (zonesDB[zn].zonePartition != prt)								// zone belongs to different partiton
				continue;														// yes, try the next one
			if (!zonesDB[zn].zoneType)											// 0 = DISABLED
				continue;
			//printAlarmZone(zonesDB[brd][zn]);
			//
			if (zonesRT[zn].zoneStat == ZONE_CLOSE) 					
				;																// nothing to do
			//
			if (zonesRT[zn].zoneStat & ZONE_TAMPER) {							// tamper error in zone?
				processTampers(zn);
				partitionSTATS[zonesDB[zn].zonePartition].tamperZonesCnt++;		// update statistics
			}
			//
			if (zonesRT[zn].zoneStat & ZONE_AMASK) {							// antimask error in zone?
				processAmasks(zn);
				partitionSTATS[zonesDB[zn].zonePartition].amaskZonesCnt++;		// update statistics
			}
			//
			if (zonesRT[zn].zoneStat & ZONE_OPEN) {								// open zone?
				//lprintf("Open zone %d (%s) on board %d found\n", zn, zonesDB[brd][zn].zoneName, brd);
				//processOpenH24Zone(zonesDB[brd][zn]);							// call to process H24* zones
				zonesRT[zn].in_alarm = processOpenZone(zn);						// returns true if alarm requested
				partitionSTATS[zonesDB[zn].zonePartition].openZonesCnt++;		// update statistics
			}
			if (zonesRT[zn].bypassed & ZONE_BYPASSED)	 						// update statistics, only user bypassed zones
				partitionSTATS[prt].bypassedZonesCnt++;
			if (zonesRT[zn].ignorredTamper)										// update statistics
				partitionSTATS[prt].ignorredTamperZonesCnt++;
			if (zonesRT[zn].ignorredAmask)		 								// update statistics
				partitionSTATS[prt].ignorredAmaskZonesCnt++;
			if (zonesRT[zn].openEDSD1zone)										// count here as ED zones are counted only in REGULAR/FORCE arm
				partitionSTATS[prt].openZonesCntEDSD1++;						// update count of open ED zones
			if (zonesRT[zn].openEDSD2zone)										// SD zones are counted only in STAY/INSTANT arm
				partitionSTATS[prt].openZonesCntEDSD2++;						// update count of open SD zones
			if (zonesRT[zn].in_alarm) 
				partitionSTATS[prt].alarmZonesCnt++;
			// 	
			// aggregate all alarm/trouble statuses (0 means no alarm/no trouble will be generated if armed)
			alarm	|= zonesRT[zn].in_alarm;									// in_alarm contains bitmask of requested alarm type (STEADY, PULSING,..) of zone
			trouble |= zonesRT[zn].in_trouble;									// in_trouble contains bitmask of requested alarm type when troubel
		}
		// done with zones processing
		// do partition level processing staff here
		partitionSTATS[prt].notBypassedEntyDelayZones = countNotBypassedEntryDelayZones(prt);	// mark how many entry delay zone are not bypassed, must be here cause armPartition force bypasses
		//
		// if all ENTRY DELAY zones are closed, reset entry delay FSM 
		if ((partitionRT[prt].partitionTimers[ENTRY_DELAY1_TIMER].timerFSM == DONE) && !partitionSTATS[prt].openZonesCntEDSD1)	
			partitionRT[prt].partitionTimers[ENTRY_DELAY1_TIMER].timerFSM = NOT_STARTED;
		if ((partitionRT[prt].partitionTimers[ENTRY_DELAY2_TIMER].timerFSM == DONE) && !partitionSTATS[prt].openZonesCntEDSD2)
			partitionRT[prt].partitionTimers[ENTRY_DELAY2_TIMER].timerFSM = NOT_STARTED;
		//
		// check for ARM partition commands
		if (partitionRT[prt].changed & CHG_NEW_CMD) {
			//partitionRT[prt].changed &= ~CHG_NEW_CMD;
			if (!armPartition(prt, partitionRT[prt].targetArmStatus)) 					// if successfull arm (0 = success)
				partitionRT[prt].armStatus = partitionRT[prt].targetArmStatus;			// armed successfully
			else
				partitionRT[prt].targetArmStatus = partitionRT[prt].armStatus;			// no change in arm status
			//publishArmStatus(prt);
		}
		//
		// publish and print results
		//printZonesSummary(prt);
		//doPublishing(prt);
		// mark done with this partition
		partitionRT[prt].changed = 0;													// mark as processed
	}
	// siren control
	processAlarm(alarm);
	printParttionsSummary();
}
