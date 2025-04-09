//extern int	armPartition(byte partIxd, byte action);
extern void bypassZone(byte zoneIdx, int requestor);
extern void clearBypass(byte zoneIdx, int bypassType);

//
// actions DISARM, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM
//
void trigerArm(void* param1, void* param2, void* param3) {
	const int partID = *(int*)param1;
	const int action = *(int*)param2;
	if (!partitionDB[partID].valid)
		return;
	// param3 is not used
	// check first for valid params ranges and call
	if (!((action ==  DISARM) || (action == REGULAR_ARM) || (action == STAY_ARM) || (action == FORCE_ARM) || (action == INSTANT_ARM))) {
		ErrWrite(ERR_WARNING, "CMD parse: Invalid ARM action %d", action);
		return;
	}
	if (partID > MAX_PARTITION) {
		ErrWrite(ERR_WARNING, "CMD parse: Invalid PARTITION ID %d", partID);
		return;
	}
	//partitionRT[partID].newCmd = true;
	partitionRT[partID].changed |= CHG_NEW_CMD;				// mark that action is needed
	partitionRT[partID].targetArmStatus = action;			// set new target arm state
	//if(action != DISARM_ALL)								// set new target arm state
	//	partitionRT[partID].targetArmStatus = action;		// armed successfully
	//else {													// DISARM ALL command
	//	for (int i = 0; i < MAX_PARTITION; i++) {
	//		partitionRT[partID].targetArmStatus = action;	
	//	}
	//}
}

//
// fakeAnalogSetZn - set analog value to be readfrom ADC for partiqular zone
// action = ZONE_ANAL_SET_CMD
//
void fakeAnalogSetZn(void* param1, void* param2, void* param3) {
	int brd, brdZnNo;
	const int zn = *(int*)param1;										// index in zonesD
	//const int action = *(int*)param2;									// command - defined in enum ZONE_CMDS_t
	const char* payldPtr = (const char*)param3;							// pointer to the parameter in payload after "="
	if (*payldPtr != '=') {
		ErrWrite(ERR_CRITICAL, "fakeAnalogSetZn: payload format error: '=' not found! Payload: %s", payldPtr);
		return;
	}
	payldPtr++;
	//
	if (*payldPtr)
		lprintf("fakeAnalogSetZn: fake analog value for zone %s", payldPtr);
	else {
		ErrWrite(ERR_CRITICAL, "fakeAnalogSetZn: no analog value supplied with command");
		return;
	}
	//
	int res = atoi(payldPtr);
	//lprintf("fakeAnalogSetZn: Set analog input for zone %d to %d\n", zn, res);

	if (!zonesDB[zn].zoneType)											// zoneType == 0 means disabled
		return;
	if (zn >= MAX_ALARM_ZONES) {
		ErrWrite(ERR_CRITICAL, "fakeAnalogSetZn: Zone No %d out of range!\n", zn);
		return;
	}
	brd = IDX_2_ZN_BRD(zn);
	brdZnNo = IDX_2_ZN_NO(zn);
	lprintf("fakeAnalogSetZn: Board: %d   Zone: %d analog value will be set to %d\n", brd, brdZnNo, res);
	if (brd != MASTER_ADDRESS) {
		ErrWrite(ERR_CRITICAL, "fakeAnalogSetZn: Only master zones are supported: zone: %d, board %d\n", brdZnNo, brd);
		return;
	}
	// analog value will be set in struct ZONE MzoneDB
	int idxMzoneDB	= zn / 2;											// one entry in MzoneDB represent two zones in zonesDB
	//int znABshift	= (zn % 2? ZONE_B_SHIFT: ZONE_A_SHIFT);				// odd zones result is shifted in zoneABstat
	MzoneDB[idxMzoneDB].mvValue = (float)res;
	zoneVal2Code(MzoneDB, sizeof(MzoneDB) / sizeof(MzoneDB[0]));
}


//
// action = ZONE_OPEN_CMD, ZONE_AMASK_CMD, ZONE_CLOSE_CMD, ZONE_TAMPER_CMD, ZONE_BYPASS_CMD
//
void modifyZn(void* param1, void* param2, void* param3) {
	//byte newStat;
	const int zn			= *(int*)param1;								// index in zonesD
	const int action		= *(int*)param2;								// command - defined in enum ZONE_CMDS_t
	const char * payldPtr	= (const char*)param3;
	if (payldPtr)
		lprintf("modifyZn: WARNING - garbage found after modify zone command %s", payldPtr);
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
void modifyPgm(void* param1, void* param2, void* param3) {
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

