//
// alarmClassHelpers.h - init defaults, print and other helper functions 
//

//  print config data  data
//  params: (pointer)to ALARM_ZONE  
//          printClass - 0 prints all, !0 prints only what is selected
//
void Alarm::printConfigData(struct tagAccess targetKeys[], int numEntries, byte *targetPtr, int printClass) {
    const char * titlePtr = NULL;
    for (int i = 0; i < numEntries; i++) {
        if (printClass && (printClass != targetKeys[i].printClass))           // printClass == 0 (PRTCLS_ALL) means print all
            continue;
        titlePtr = (const char *)targetKeys[i].unpatchCallBack(targetPtr, targetKeys[i].patchOffset, targetKeys[i].patchLen);
        if (titlePtr == NULL)
            titlePtr = "";
        if (strlen(titlePtr) > targetKeys[i].keyStrLen) {                                   // can be NULL
            for (int j = 0; j < (int)targetKeys[i].keyStrLen; j++)
                lprintf("%c", titlePtr[j]);
            lprintf(" ");
        }
        else {
            lprintf("%s ", titlePtr);
            for (size_t j = strlen(titlePtr); j < (int)targetKeys[i].keyStrLen; j++)
                lprintf(" ");
        }
    }
    lprintf("\n");
}
//
void Alarm::printConfigHeader(struct tagAccess targetKeys[], int numEntries) {
    for (int i = 0; i < numEntries; i++) {
        lprintf("%s ", targetKeys[i].keyStr);
        for (int j = strlen(targetKeys[i].keyStr); j < (int)targetKeys[i].keyStrLen; j++)
            lprintf("%c", ' ');
    }
    lprintf("\n");
}
//
// print config partition data
//
void Alarm::printAlarmPartCfg(void) {
    lprintf("Partition(s)\n");
    printConfigHeader(partitionTags, PARTITION_TAGS_CNT);
    for (int j = 0; j < MAX_PARTITION; j++) {
        if (!partitionDB[j].valid)
            continue;
        printConfigData(partitionTags, PARTITION_TAGS_CNT, (byte*)&partitionDB[j], PRTCLASS_ALL);
    }
    lprintf("\n");
}
//
//  print alarm zones data
//  parms: (byte pointer) to array of ALARM_ZONE  containing the zones to be printed
//
void Alarm::printAlarmZones(int startZn, int endZn) {
    lprintf("\nZone(s)\n");
    printConfigHeader(zoneTags, ZONE_TAGS_CNT);
    for (int i = startZn; i < endZn; i++) {             // for each board' zone
        if (zonesDB[i].zoneType)                        // 0 means disable zone
            printConfigData(zoneTags, ZONE_TAGS_CNT, (byte*)&zonesDB[i], PRTCLASS_ALL);
    }
    lprintf("\n");
}
//
// print Alarm Global options
//
void Alarm::printAlarmOpts(byte* optsPtr) {
    lprintf("\nGlobal options\n");
    printConfigHeader(gOptsTags, GLOBAL_OPTIONS_TAGS_CNT);
    printConfigData(gOptsTags, GLOBAL_OPTIONS_TAGS_CNT, optsPtr, PRTCLASS_ALL);
    lprintf("\n");
}
//
//
//
//void printAlarmPgms(alarmPgmArr_t* pgmArrPtr, int startBoard, int endBoard) {
void Alarm::printAlarmPgms(void) {
    //alarmPgmArr_t *pgmArr = (alarmPgmArr_t *)pgmArrPtr;
    //lprintf("       boardID pgmID tval cval pulseLen pgmName\n");
    lprintf("\nPGM(s)\n");
    printConfigHeader(pgmTags, PGM_TAGS_CNT);
    for (int i = 0; i < MAX_ALARM_PGM; i++) {                        // iterate
        if (!pgmsDB[i].valid)
            continue;
        printConfigData(pgmTags, PGM_TAGS_CNT,(byte*)&pgmsDB[i], PRTCLASS_ALL);
        //printOneAlarmPgm((struct ALARM_PGM*) &(*pgmArrPtr)[i]);
    }
    lprintf("\n");
}
//
//  print alarm keysw data
//  parms: (byte pointer) to array of ALARM_KEYSW  containing the keysw to be printed
//
void Alarm::printAlarmKeysw(byte* keyswArrPtr, int maxKeysw) {
    lprintf("------------------------ Not IMPLEMENTED ---------------------------------\n");
//    alarmKeyswArr_t *pgmArr = (alarmKeyswArr_t *)keyswArrPtr;
//	lprintf("      partition; type; action; boardID;	zoneID;  keyswName[16];\n");
//	for (int i = 0; i < maxKeysw; i++) {                        // iterate
//	   lprintf ("KeySwitch data: %2d\t%2d\t%2d\t%2d\t%2d%16s\n",(*pgmArr)[i].partition,(*pgmArr)[i].type, (*pgmArr)[i].action, (*pgmArr)[i].boardID,
//														   (*pgmArr)[i].zoneID, (*pgmArr)[i].keyswName);
//	}
}
//
//
//
void Alarm::printAlarmPartitionRT(int idx) {
    lprintf("%d\t%d\t%.8ld\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
        partitionRT[idx].armStatus, partitionSTATS[idx].notBypassedEntyDelayZones, partitionRT[idx].armTime , 
        partitionRT[idx].partitionTimers[ENTRY_DELAY1_TIMER].timerFSM, partitionRT[idx].partitionTimers[ENTRY_DELAY2_TIMER].timerFSM,
        partitionSTATS[idx].openZonesCnt, partitionSTATS[idx].openZonesCntEDSD1, partitionSTATS[idx].openZonesCntEDSD2,
        partitionSTATS[idx].bypassedZonesCnt, partitionSTATS[idx].tamperZonesCnt, partitionSTATS[idx].ignorredTamperZonesCnt,
        partitionSTATS[idx].amaskZonesCnt, partitionSTATS[idx].ignorredAmaskZonesCnt);
    lprintf("\n");
}
//
void Alarm::printPartHeaderRT() {
    lprintf("Partition(s) rut-time data\n");
    lprintf("armStat\tNbEdZnC\tarmTime\t\tentDly1\t\tentDly2\t\tentD1FS\tentD2FS\topnZcnt\toZEDc\toZSDc\tbpsZcnt\ttamZcnt\tignTmZC\tFollows:\n");
}
//
// print run-time partition data
//
void Alarm::printAlarmPartRT(void) {
    printPartHeaderRT();
	for (int j = 0; j < MAX_PARTITION; j++) {
        printAlarmPartitionRT(j);
    }
}
//
// report (print) zone names with particular status OPEN/CLOSE/TAMPER/AMASK
//
void Alarm::reportZonesNamesBasedOnStatus(int prt, int stat) {
    int res = 0;
    if (!partitionDB[prt].valid)			                            // zone belongs to valid partiton
        return;													        // yes, try the next one
    for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {                      // for each board' zone
        if (zonesDB[zn].zonePartition != prt)			                // zone belongs to different partiton
            continue;													// yes, try the next one
        if (!zonesDB[zn].zoneType)									    // 0 = DISABLED
            continue;
        //printAlarmZone(zn);
        if (zonesRT[zn].zoneStat == stat) {
            ErrWrite(LOG_ERR_DEBUG, "%s ", zonesDB[zn].zoneName);
            res++;
        }
    }
    if (!res)
        ErrWrite(LOG_ERR_DEBUG, "%s", "None");
    ErrWrite(LOG_ERR_DEBUG, "%s", "\n");
}
//
// report (print) zone names with particular flag set - bypassed, ignorred tamper,ignorredAmask, etc
//
void Alarm::reportZonesNamesBasedOnFlag(int prt, int offset, byte bitmask) {
    int res = 0; byte* basePtr;
    if (!partitionDB[prt].valid)			                            // zone belongs to valid partiton
        return;													        // yes, try the next one
    for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {                      // for each board' zone
        if (zonesDB[zn].zonePartition != prt)			                // zone belongs to different partiton
            continue;
        if (!zonesDB[zn].zoneType)		    							// 0 = DISABLED
            continue;
        basePtr = (byte*)&zonesRT[zn];
        if (basePtr[offset] & bitmask) {
            ErrWrite(LOG_ERR_DEBUG, "%s ", zonesDB[zn].zoneName);
            res++;
        }
    }
    if (!res)
        ErrWrite(LOG_ERR_DEBUG,"%s", "None");
    ErrWrite(LOG_ERR_DEBUG, "%s", "\n");
}
//
// report (print) zone names with particular flag set - bypassed, ignorred tamper,ignorredAmask, etc
//
void Alarm::reportPartitionNamesBasedOnFlag(int offset) {
    int prt; int res = 0; byte* basePtr;
    for (prt = 0; prt < MAX_PARTITION; prt++) {
        if (!partitionDB[prt].valid)
            continue;
        basePtr = (byte*)&partitionSTATS[prt];
        if (basePtr[offset]) {
            ErrWrite(LOG_ERR_DEBUG, "%s ", partitionDB[prt].partitionName);
            res++;
        }
    }
    if (!res)
        ErrWrite(LOG_ERR_DEBUG, "%s", "None");
    ErrWrite(LOG_ERR_DEBUG, "%s", "\n");
}
// 
//
void Alarm::printZonesSummary(int prt) {
    //if (partitionRT[prt].changed)                                         // TODO shall be requested by Alarm client
    //    timeoutOps(FORCE, STATUS_REPORT_TIMER);
    //if (timeoutOps(GET, STATUS_REPORT_TIMER)) {					        // publish zones statistics on spec intervals									
    //    timeoutOps(SET, STATUS_REPORT_TIMER);
        //lprintf("-------------- Periodic status report --------------\n");
        //ErrWrite(ERR_DEBUG, "-------------- Recent zones changes: ");     // first report all changed zones
        //reportChangedZones();
        lprintf("-------------- Zones summary for partition %s --------------\n", partitionDB[prt].partitionName);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones w TAMPER:\t\t");
        reportZonesNamesBasedOnStatus(prt, ZONE_TAMPER);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones w AMASK:\t\t");
        reportZonesNamesBasedOnStatus(prt, ZONE_AMASK);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones OPEN:\t\t");
        reportZonesNamesBasedOnStatus(prt, ZONE_OPEN);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones USER BYPASSED:\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, bypassed), ZONE_BYPASSED);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones FORCE BYPASSED:\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, bypassed), ZONE_FORCED);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones STAY BYPASSED:\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, bypassed), ZONE_STAY_BYPASSED);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones ENTRY-D BYPASSED:\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, bypassed), ZONE_EDx_BYPASSED);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones EXYT-D BYPASSED:\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, bypassed), ZONE_EX_D_BYPASSED);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones w IGNORRED TAMPER:");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, ignorredTamper), 0xFF);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones w IGNORRED AMASK:\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, ignorredAmask), 0xFF);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Open ED1&2 Zones:\t\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, openEDSD1zone), 0xFF);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Open SD1&2 Zones:\t\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, openEDSD2zone), 0xFF);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones in ALARM:\t\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, in_alarm), 0xFF);
        ErrWrite(LOG_ERR_DEBUG, "%s", "Zones in TROUBLE:\t");
        reportZonesNamesBasedOnFlag(prt, offsetof(struct ALARM_ZONE_RT, in_trouble), 0xFF);
    //}
}
void Alarm::printParttionsSummary() {
    //if (timeoutOps(GET, STATUS_REPORT_TIMER)) {					        // publish zones statistics on spec intervals									
    //    timeoutOps(SET, STATUS_REPORT_TIMER);                             // TODO shall be requested by Alarm client
        //
        // print now partitons 
        //
        lprintf("-------------- Partitions summary --------------\n");
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w OPEN zones:\t");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, openZonesCnt));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w ALARM zones:\t");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, alarmZonesCnt));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w BYPASSED zones:\t");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, bypassedZonesCnt));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w TAMPER zones:\t");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, tamperZonesCnt));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w OPEN ED1 zones:\t");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, openZonesCntEDSD1));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w OPEN ED2 zones: ");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, openZonesCntEDSD2));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w IGNORRED TAMPERS: ");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, ignorredTamperZonesCnt));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w AMASK zones:\t");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, amaskZonesCnt));
        ErrWrite(LOG_ERR_DEBUG, "%s", "Partitions w IGNORRED AMASK:\t");
        reportPartitionNamesBasedOnFlag(offsetof(struct ALARM_PARTITION_STATS_t, ignorredAmaskZonesCnt));
        lprintf("-------------- End of report --------------\n");
    //}
}


/**
 * @brief Tries to load complete alarm configuration data from storage or memory.
 *
 * This function attempts to initialize the alarm loop by loading the complete alarm
 * configuration data. It may involve retrieving data from storage or memory, depending
 * on the implementation.
 *
 * @return 0 on success, non-zero on errors.
 *
 */
 //int initAlarmLoop() {
 //    ErrWrite(LOG_ERR_WARNING, "Init alarm from file\n");
 //    if (!storageSetup()) {					                                    // mount file system
 //        ErrWrite(LOG_ERR_CRITICAL, "Error initializing storage while processing the new config\n");
 //        storageClose();
 //        return false;
 //    }
 //    //  Read config file into tempMQTTpayload buffer
 //    int rlen = loadConfig(jsonConfigFname, tempMQTTpayload, sizeof(tempMQTTpayload));
 //    if (!rlen) {  
 //        ErrWrite(LOG_ERR_WARNING, "Wrong or missing CSV config file\n");
 //        storageClose();
 //        return false;
 //    }
 //    //
 //    // clear all storage arrays (DBs) for all alarm domains - zonesDB[], partitonsDB[], pgmsDB[], ...  
 //    //clearAllDBs();
 //    //
 //    // now parse the config data and store it in corresponding arrays (DBs) - zonesDB[], partitonsDB[], pgmsDB[], ... 
 //    // config data are loaded in  tempMQTTpayload   from CSV config file                   
 //    if (parseConfigFile((char*)&tempMQTTpayload, rlen, true)) {
 //        ErrWrite(LOG_ERR_CRITICAL, "Config file parse fail\n");
 //        return 0;
 //    }
 //    else
 //        ErrWrite(LOG_ERR_CRITICAL, "Config file successfully parsed\n");
 //    //
 //    //initBoardsData();
 //    //initRTdata();
 //    //synchPGMstates();                                                       // copy mPGM cValue (default init value) to pgmsDB
 //    //printAlarmOpts((byte*)&alarmGlobalOpts);
 //    //printAlarmPartCfg();
 //    //printAlarmZones(0, MAX_ALARM_ZONES);
 //    //printAlarmPgms((alarmPgmArr_t*)&alarmConfig.pgmConfig, MASTER_ADDRESS, maxSlaves);
 //    //printAlarmPgms();
 //    //printAlarmKeysw((byte*) &keyswDB, MAX_KEYSW_CNT); 
 //    //  write BIN config
 //    //if(ENABLE_CONFIG_CREATE) { 					// do not create config, we have to wait for data from MQTT 	
 //    //    saveBinConfig(configFileName);          // create the file, maybe this is the first ride TODO - make it to check only once
 //    //}
 //#ifndef ARDUINO
 //    //lprintf("Saving JSON config file\n");
 //    //saveJsonConfig(jsonConfigFname);
 //#endif
 //    storageClose();
 //    //
 //    return true;
 //}

//
//
// ------------------- old staff TODO - remove ---------------------
//
// report (print) zone names recently changed
//
//void reportChangedZones(void) {
//    int res = 0;
//    for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {                      // for each board' zone
//        if (!partitionDB[zonesDB[zn].zonePartition].valid)			    // zone belongs to valid partiton
//            continue;													// yes, try the next one
//        if (!zonesDB[zn].zoneType)									    // 0 = DISABLED
//            continue;
//        if (zoneStatusChanged(zn))                                      // zone status changed
//            res++;
//        //printAlarmZone(zn);
//        if (zoneStatChanged(zn)) {                                      // zoneStat changed only
//            int zoneStateBak = zoneStatBackup(zn);
//            lprintf("\nZone %s status changed from %s to %s", zonesDB[zn].zoneName, zoneState2Str(zoneStatesTitles, ZONE_STATES_TITLE_CNT, zoneStateBak), zoneState2Str(zoneStatesTitles, ZONE_STATES_TITLE_CNT, zonesRT[zn].zoneStat));
//        }
//        else if (zoneBypassChanged(zn))
//            lprintf("\nZone %s bypass changed from %s to %s", zonesDB[zn].zoneName, (zonesRT[zn].zoneBypassBak & ZONE_BYPASSED) ? "BYPASSED" : "NOT BYPASSED", (zonesRT[zn].bypassed & ZONE_BYPASSED) ? "BYPASSED" : "NOT BYPASSED");
//        else if (zoneForcedChanged(zn))
//            lprintf("\nZone %s forced status changed from %s to %s", zonesDB[zn].zoneName, (zonesRT[zn].bypassed & ZONE_FORCED) ? "FORCED" : "NOT NORCED", (zonesRT[zn].bypassed & ZONE_FORCED)  ? "FORCED" : "NOT FORCED");
//    }
//    if (!res)
//        lprintf(" None\n");
//    else
//        lprintf("\n");
//}
// 
//
//const char* Alarm::titleByAction(struct zoneStates_t Cmds[], int CmdsCnt, int stateCode) {
//    int i;
//    for (i = 0; i < CmdsCnt; i++) {
//        if (Cmds[i].state == stateCode) {
//            return (Cmds[i].stateTitle);
//        }
//    }
//    lprintf("Cannot find zone state by stateCode %d\n", stateCode);
//    return &UNKNOWN_TTL[0];
//}
/**
 * @brief Init board(s) level data 
 */
//void initBoardsData() {
//    maxSlaves = alarmGlobalOpts.maxSlaveBrds;                                       
//    for (int i = 0; i < MAX_SLAVES; i++) {                                 // for each board 
//        brdsDB[i].valid = (i <= maxSlaves) ? 1 : 0;
//        brdsDB[i].brdFail = 0;  brdsDB[i].totalErrors = 0;
//    }
//}
/**
 * @brief Clear all storage for all alarm domains - zones, partitons, pgms, global options, etc
 */
    //void clearAllDBs() {
    //    int i;
    //    //lprintf("Size of sbPtrArr = %d\n", sizeof(dbPtrArr));
    //    for(i=0; i<DB_PTR_ARR_CNT; i++)
    //        memset((void*)dbPtrArr[i].dbBaseAddr, 0, dbPtrArr[i].elmCnt * dbPtrArr[i].elmLen);
    //}
///
//// clean-up run time data - staistics and zone statuses TODO - update here when new data added
////
//void initRTdata(void) {
//    int i;
//    // reset zone's tun-time data
//    for (int j = 0; j < MAX_ALARM_ZONES; j++) {    
//        memset((byte*)&zonesRT[j], 0x0, sizeof(ALARM_ZONE_RT));
//        zonesRT[j].zoneStat = ZONE_CLOSE;
//        zonesRT[j].changed = 0;
//    }
//    for (i = 0; i < MAX_PARTITION; i++) {
//        // reset all partition statistics
//        memset((byte*)&partitionSTATS[i], 0x0, sizeof(ALARM_PARTITION_STATS_t));
//        // init rut-time partition timers with configured delays in seconds
//        resetAllPartitionTimers(i);
//        // reset ARM state
//        partitionRT[i].armStatus = partitionRT[i].targetArmStatus = DISARM;
//        partitionRT[i].newCmd = false; partitionRT[i].changed = 0;
//    }
//    // clear all HW errors
//    alarmGlobalOpts.SprvsLoss = 0; alarmGlobalOpts.ACfail = alarmGlobalOpts.BatFail = alarmGlobalOpts.BellFail = alarmGlobalOpts.BrdFail = 0;
//}

// -------------  timers ------------------------------------
    //#define ALARM_LOOP_INTERVAL		1000
    //#define STATUS_REPORT_INTERVAL	30000
    //#define	ALARM_PUBLISH_INTERVAL	500
    //
    //enum timeoutOper {
    //    SET = 1,
    //    GET = 2,
    //    FORCE = 3,
    //};
    ////
    //enum TIMERS {
    //    ALARM_LOOP_TIMER = 1,
    //    STATUS_REPORT_TIMER,
    //};
    ////
    //// command records structure for cmdDB
    //struct TIMER {
    //    int timerID;
    //    unsigned long interval;
    //    unsigned long setAt;
    //};
    ////
    //// timerss database to look-up timer params TODO - move all timers staff to helpers or separate file. 
    //// 
    //struct TIMER timerDB[] = {  {ALARM_LOOP_TIMER, ALARM_LOOP_INTERVAL, 0},
    //                            {STATUS_REPORT_TIMER, STATUS_REPORT_INTERVAL, 0},
    //};
    ////
    //int findTimer(byte timer) {
    //    //ErrWrite(ERR_DEBUG, "Looking for record for timer ID   %d \n", timer);
    //    for (int i = 0; i < sizeof(timerDB) / sizeof(struct TIMER); i++) {
    //        //printf("Looking at index  %d out of  %d:\n", i, sizeof(cmdDB)/sizeof(struct COMMAND)-1);
    //        if (timerDB[i].timerID == timer) {
    //            //ErrWrite(ERR_DEBUG,"Found timer at  index %d\n", i);
    //            return  i;
    //        }
    //    }
    //    ErrWrite(ERR_DEBUG, "Timer not found!!!!!!!\n");
    //    return ERR_DB_INDEX_NOT_FND;
    //}
    //// 
    //// set timeout / check if timeout expired
    //// TODO - organize all timeouts as separate database similar to commands
    //// 
    //bool timeoutOps(int oper, int whichOne) {
    //    int index;
    //    // find timer index first
    //    if ((index = findTimer(whichOne)) < 0) {        // timer not found
    //        ErrWrite(ERR_CRITICAL, "Timer %d NOT FOUND\n", whichOne);
    //        return false;						        // TODO - report error
    //    }
    //    if (oper == SET) {                              // record the current time in milliseconds
    //        timerDB[index].setAt = millis();
    //        return 0;
    //    }
    //    if (oper == FORCE) {                            // force timer to report as expired on next GET op
    //        timerDB[index].setAt = 0;
    //        return 0;
    //    }
    //    else {
    //        unsigned long res = (unsigned long)(millis());  // GET
    //        return ((res - timerDB[index].setAt) > (unsigned long)timerDB[index].interval);
    //    }
    //}
    ////
    //// set timer interval
    //// 
    //bool timerSetInterval(int whichOne, unsigned long Interval) {
    //    int index;
    //    // find timer index first
    //    if ((index = findTimer(whichOne)) < 0)	    // timer not found
    //        return false;						    // TODO - report error
    //    timerDB[index].interval = Interval;
    //    return true;
    //}
    ////
    //// -------------- end timers -----------------------------
// 
//extern int      countNotBypassedEntryDelayZones(int partIdx);
//extern int      alrmConfig2Json(struct CONFIG_t* alarmCfg, IOptr stream);
//extern void     synchPGMstates();
//
// 
//const char* zoneState2Str(struct zoneStates_t states[], int statesCnt, int action) {
//    int i;
//    for (i = 0; i < statesCnt; i++) {
//        if (states[i].state == action) {
//            //lprintf("Found CMD: MQTT cmd %s, action = %d\n", Cmds[i].MQTTcmd, Cmds[i].action);
//            return (states[i].stateTitle);
//        }
//    }
//    ErrWrite(ERR_CRITICAL, "zoneState2Str: invalid zoneState input: %d\n", action);
//    return NULL;
//}