//
#include <cstdarg>
#include "alarm-core-mqtt_defs.h"
//
extern int WIFIsetup(void);
//
void OTAconfigHandler(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);
void globalOptionsHandler(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);
void zonesPartEtcControlHandler(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);
int  zonePartPgmSubscribe(int what, int idx);
int  subscribeOTAconfig(int what, int idx);
int  subscribeGlobalOptions(int what, int idx);

//
struct inpPlds_t {
    const char consoleCmd[NAME_LEN];        // barely used
    const char MQTTpld[NAME_LEN];
    unsigned int action;
    void (*cBack)(void* param1, void* param2, void* param3);
};
//
// Partition control supported payloads (commands)
struct  inpPlds_t partPlds[] = {
    // TODO: some PAI command are not implemented - arm_sleep, arm_stay_stayd, arm_sleep_stay 
    {"DP", "disarm",        DISARM,         &trigerArm},
    {"RA", "arm",           REGULAR_ARM,    &trigerArm}, {"FA", "arm_force",    FORCE_ARM,  &trigerArm},
    {"SA", "arm_stay",      STAY_ARM,       &trigerArm}, {"IA", "arm_instant",  INSTANT_ARM, &trigerArm},
};
//
// Zone control supported payloads (commands)
struct  inpPlds_t zonePlds[] = {
    {"AS", "analogSet",     ZONE_ANAL_SET_CMD,  &fakeAnalogSetZn}, /*{"DS", "digitalSet",  ZONE_DIGITAL_SET_CMD,  &digitalSetZn},*/
    {"ZB", "bypass",        ZONE_BYPASS_CMD,    &modifyZn}, {"ZU", "clear_bypass",  ZONE_UNBYPASS_CMD,  &modifyZn},
    {"ZT", "tamper",        ZONE_TAMPER_CMD,    &modifyZn}, {"ZC", "close",         ZONE_CLOSE_CMD,     &modifyZn},
    {"ZO", "open",          ZONE_OPEN_CMD,      &modifyZn}, {"ZM", "anti-mask",     ZONE_AMASK_CMD,     &modifyZn},
};
//
// pgm (output) control supported payloads (commands). TODO - implement pulse command
struct  inpPlds_t pgmPlds[] = {
    {"PON", "on",   PGM_ON, &modifyPgm}, {"POF", "off",  PGM_OFF,  &modifyPgm}, {"PULSE", "pulse", PGM_PULSE, &modifyPgm},
};
//
// format of the entries of the array handling MQTT topics
struct MQTT_cmd_t {
    const char	topic[MAX_MQTT_TOPIC];                                  // MQTT control topic, consist of topic/name, wher name is zone/partition/pgm name
    int         what;                                                   // what domein of alarm element (zones/partitions/pgms/...) is concerned
    void        (*cBack)(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);  // topic handler
    struct inpPlds_t* pldsArrPtr;                                       // pointer to valid payloads array
    int         maxPlds;                                                // num of elements in payloads array
    int        (*subcBack)(int what, int idx);                          // subscribe handler
};
//
// Array of MQTT topics handlers
// ADD HERE NEW TOPICS AND CORRESPONDING HANDLERS
//
struct MQTT_cmd_t MQTT_subscriptions[] = {
    {MQTT_ZONES_CONTROL,         ZONES,         &zonesPartEtcControlHandler, zonePlds, (sizeof(zonePlds)/sizeof(zonePlds[0])), &zonePartPgmSubscribe},
    {MQTT_PARTITIONS_CONTROL,    PARTITIONS,    &zonesPartEtcControlHandler, partPlds, (sizeof(partPlds)/sizeof(partPlds[0])), &zonePartPgmSubscribe},
    {MQTT_OUTPUTS_CONTROL,       PGMS,          &zonesPartEtcControlHandler, pgmPlds,  (sizeof(pgmPlds) /sizeof(pgmPlds[0])),  &zonePartPgmSubscribe},
    {MQTT_METRICS_EXCHANGE,      -1,            &OTAconfigHandler,           NULL,                   0,                        &subscribeOTAconfig},
    {MQTT_GLOBAL_OPT_CONTROL,    GLOBAL_OPT,    &globalOptionsHandler,       NULL,                   0,                        &subscribeGlobalOptions },
};
#define MQTT_SUBS_CNT   (sizeof(MQTT_subscriptions)/sizeof(MQTT_subscriptions[0]))
//
// void processMQTTpayload(struct  inpPlds_t Cmds[], int CmdsCnt, int idx, const byte payld[], int len)
// tries to match the string in payload[] to command (bypass, clear bypass, arm, siarm...) in Cmds[] and if yes, calls the corresponding callback
// params:  struct  inpPlds_t Cmds[]    - array of supported payloads (commands) for zones, partitions, etc
//          int CmdsCnt                 - number of entries in the array of supported payloads
//          int idx                     - index in zones/partitions/pgms/etc database matching the name 
//                                        extracted from received MQTT topic. -1 MEANS SET ALL
//          const byte payld[]          - payload
//          int len                     - payload len
// returns: none
//
void processMQTTpayload(struct  inpPlds_t Cmds[], int CmdsCnt, int idx, const byte payld[], int len) {
    int i; const byte * secStr = NULL;
    for (i = 0; i < CmdsCnt; i++) {
        // match payload to what allowed by command
        if (!_strnicmp((const char*)payld, Cmds[i].MQTTpld, strlen(Cmds[i].MQTTpld))) { // note: payld is not NULL terminated
            lprintf("Found CMD: MQTT cmd %s, action = %d at index %d\n", Cmds[i].MQTTpld, Cmds[i].action, i);
            secStr = &payld[strlen(Cmds[i].MQTTpld)];               // secStr is pointer to rest (if any) of the payload after the command, to be parsed by the callback;
            break;
        }
        else
            continue;                                               // no match, continue the search
    }
    if (i < CmdsCnt) {                                             
        Cmds[i].cBack(&idx, &(Cmds[i].action), (void *) secStr);    // process the payload, idx contain index in particular database (zonesDB, partitonsDB,...)
    }
    else {                                                          // payload doesn't match any of the entries in allowed payloads array
        lprintf("Unrecognized MQTT command payload:  %.*s\n", len, payld);
        return;
    }
}
//
// void MQTTcallback(char* rcvTopic, byte* payload, unsigned int pldLen)
// This callback is called from MQTT stack when something is received on subscribed topic
//  params: char* rcvTopic          - pointer to topic which got a message (payload)
//          byte* payload           - pointer to payload received (not null terminated) 
//          unsigned int pldLen     - len of payload
//  returns: none
// 
// Note - publish from callback has bad behawior. ErrWrite internally publishes error if MQTT_LOG is true
//
void MQTTcallback(char* rcvTopic, byte* payload, unsigned int pldLen) {
    int i, j, z; int idx; int nArg;
    const char*     topicPtr; 
    //unsigned int    topicLen;
    int found = 0;
    //
    payload[pldLen] = 0;                                                 // payload is not NULL terminated, but we are expecting only strings, so shall be safe to terminate it
    //lprintf("MQTT recv:%s ", rcvTopic);
    //lprintf("Payload: %.*s Len:%d\n", pldLen, payload, pldLen);
    //
    for (idx = 0; idx < MQTT_SUBS_CNT; idx++) {                             // iterate over all supported MQTT topics
        topicPtr = MQTT_subscriptions[idx].topic;
        //topicLen = strlen(MQTT_subscriptions[idx].topic);
        nArg = 0;  
        //
        // counts num of argument (%s) extracted to be transferred to hanfler
        // in rcvTopic zone/parmeters/pgms/optons etc names  are variables (arguments), e.g. /pdox/control/zones/ZONE_NAME
        // in MQTT_subscriptions array the topic can contain %s in place where such variable is allowed e.g. /pdox/control/zones/%s
        // we have find if rcvTopic matches any of MQTT_subscriptions array topics, where %s can be matched to any string up to topic separator '/'
        // variables shall be extracted in line[], in order of parsing and separated by NULLs
        // once done, callback to handle the topic is called with line[] containing all variables (later called arguments)
        //
        for (i = 0, j = 0, z = 0; i < strlen(rcvTopic); i++, j++) {         // try to match the received topic to subscribed ones
            if (rcvTopic[i] == topicPtr[j])                                 // compare char by char
                continue;                                                   // so far equal, check the next character
            if (!((topicPtr[j++] == '%') && (topicPtr[j++] == 's')))        // check for %s in local topic, in recTopic it will be matching something like zone name
                break;                                                      // not the same topic
            // we have to extract from rcvTopic what matches %s in local topic to tmp storage organized in line[] buff
            int k = 0;                                                      // keeps track of the len of part of the received topic which matches %s 
            while (rcvTopic[i] && (rcvTopic[i] != '/')) {                   // copy everything until '/' or end-of-string, but not more than NAME_LEN
                if (z >= sizeof(line)) {                                    // check for buffer (line) overflow
                    ErrWrite(ERR_CRITICAL, "MQTT topic %s parameters exceed buffer size (1024)", rcvTopic);
                    break;
                }
                if (++k >= NAME_LEN) {                                      // %s expansions are limited to NAME_LEN
                    ErrWrite(ERR_CRITICAL, "MQTT topic %s parameters len exceed NAME_LEN (32)", rcvTopic);
                    break;
                }
                line[z++] = rcvTopic[i++];                                  // copy to to line[], which is viewed as sequence of NULL erminated strings
            }                           
            line[z++] = 0; nArg++;                                          // keep track of the arguments (%s) extracted
        }
        if (rcvTopic[i] || topicPtr[j])                                     // rcvTopic and topicPtr matched completely (till the end-off-string)?                                     
            continue;                                                       // continue with the next subscribed topic
        else {
            found = 1;                                                      // found, terminate the search and process the payload
            break;
        }
    }
    if (!found) {
        ErrWrite(ERR_WARNING, "MQTT topic: %s dosn't match any subscribed topic\n", rcvTopic);  // this shallnot happened
        return;
    }
    ErrWrite(ERR_WARNING, "MQTT topic: %s found\n", rcvTopic);
    //
    // call the topic processing callback now
    //MQTT_subscriptions[idx].cBack(MQTT_subscriptions[idx].what, nArg, line, payload, pldLen);
    MQTT_subscriptions[idx].cBack(ALARM_DOMAINS_t (idx), nArg, line, payload, pldLen);
}
//
// int str2idx(const char arrayOfStructs[], int numEnt, int stride, int nameOffset, const char* str, int* res)
//          try to match string str (e.g. extracted csv file or from rcvTopic string corresponding to %s in subscribed topic (in MQTT_subscriptions[]))
//          to string name in all the structs in arrayOfStructs[] (e.g. zonesDB[], partitionsDB[], etc)
// params:  const char arrayOfStructs[]  - pointer to array (database) where we have to look to find the name received as part of the received topic 
//          int numEnt          - count of the entries in the database to look in. All DBs (zonesDB, partitionsDB, etc) are organized as array of structs
//          int stride          - len of ene element (used for indexing)
//          int nameOffset      - offset of the name string in each entry
//          const char* str     - subtopic extracted from rcvTopic corresponding to %s spec
//          int* res            - placeholder to store the result
// return:  index in database at which the corresponding name is found.
//
int str2idx(const char arrayOfStructs[], int numEnt, int stride, int nameOffset, const char* str, int* res) {
    int i = 0;
    for (i = 0; i < numEnt; i++) {
        if (!strncmp(str, &arrayOfStructs[i*stride+nameOffset], NAME_LEN))  // compare the subTopic with all subtopic candidates like zone/partitions/pgms etc names
            break;                                                          // found
    }
    if (i == numEnt) {
        ErrWrite(ERR_CRITICAL, "String %s  not found\n", str);              // shall never end here
        return 0;
    }
    *res = i;                                                               // return index in the DB for particular item 
    return 1;
}
//
// below we have family of MQTT topic handlers
// Example: topic to subscribe to is specified by "/pdox/settings/global_options/%s", where %s is the name of the global option like maxSlaves or 
// "/pdox/control/zones/%s", where %s is the name of on of the zones like HALL_PIR
// during subscription process, %s is replaced by the corresponding option name, e.g. maxSlaves or HALL_PIR
// callback received will be on a "/pdox/settings/global_options/maxSlaves" or "/pdox/control/zones/HALL_PIR"
// in MQTTcallback we extract the corresponding subtopic (maxSlave or HALL_PIR in this examples) and store it in buffer pointed by parList 
// in globalOptions case, it is option name (maxSlave), and shall be the first and only string in the buffer pointed by  parList
// in control zones case, it is zone name (HALL_PIR), and shall be also the first and only string in the buffer pointed by  parList
// 
// void zonesControlHandler(ALARM_DOMAINS_t topicIdx, int argCnt, const char* parList, byte* pldPtr, int pldLen)
// process payload received on topic related to zones/partitions/pgms
// like "/pdox/control/zones/%s"; "/pdox/control/outputs/%s"; "/pdox/control/partitions/%s"
// %s shall be valid (zone/part/pgm) name
// params:      ALARM_DOMAINS_t topicIdx - index in MQTT_subscriptions[], pointing to current topic 
//              int argCnt              - how many param (zone/partition/pgm) names extracted when prcing received topic according to
//                                        topic specs (with %s) - specified in MQTT_subscriptions[]. In this case it shall be 1
//              const char* parList     - NULL separated list of arguments (shall be only one)
//              byte* pldPtr            - pointer to received on this topic payload
//              int pldLen              - length of received on this topic payload
//
void zonesPartEtcControlHandler(ALARM_DOMAINS_t topicIdx, int argCnt, const char* parList, byte* pldPtr, int pldLen) {
    //lprintf("MQTT callback - control/zones/partitions/pgms cmd received, subtopic: %s payload: %s\n", parList, pldPtr);
    int idx; int minIdx = 0, maxIdx = 0;                    // to store the index of the zone with zone name in parList in zonesDB
    int dbPropsIdx;
    dbPropsIdx = MQTT_subscriptions[topicIdx].what;         // use as index in dbPtrArr[].From dbPtrArr fetch the db geometry
    //
    if (!_stricmp(parList, SUBTOPIC_WILDCARD)) {            // check for 'all' first, means it is requested to set all entries in the database to same payload
        maxIdx = dbPtrArr[dbPropsIdx].elmCnt;                // set all, loop will run from 0 to elements count 
    }
    else {
        if (!str2idx(dbPtrArr[dbPropsIdx].dbBaseAddr, dbPtrArr[dbPropsIdx].elmCnt, dbPtrArr[dbPropsIdx].elmLen, dbPtrArr[dbPropsIdx].nameOffs, (const char*)parList, &idx))
            return;                                         // subtopic not found
        // got it, idx will hold the zone/partition/pgm/... index in zonesDB/partitionDB/pgmsDB/....
        minIdx = idx; maxIdx = minIdx+1;                    // minIdx is entry index, set maxIdx to minIdx+1  to run the loop only once                                
    }
    // we have zone/part/pgm/... index now and can process the payload
    for(int i=minIdx; i< maxIdx; i++)
        processMQTTpayload(MQTT_subscriptions[topicIdx].pldsArrPtr, MQTT_subscriptions[topicIdx].maxPlds, i, pldPtr, pldLen);
}
// 
// void globalOptionsHandler(int topicIdx, int argCnt, const char* parList, byte* pldPtr, int pldLen)
// process payload received on global options set topic ("/pdox/settings/global_options/%s") 
// %s shall be valid option like maxSlaves
// params:      ALARM_DOMAINS_t topicIdx- type of the item (GLOBAL_OPT)
//              int argCnt              - how many param (options) extracted when parcing received topic according to
//                                        topic specs (with %s), specified in MQTT_subscriptions[]. In this case it shall be 1
//              const char* parList     - NULL separated list of arguments (shall be only one)
//              byte* pldPtr            - pointer to received on this topic payload
//              int pldLen              - length of received on this topic payload
//
void globalOptionsHandler(ALARM_DOMAINS_t topicIdx, int argCnt, const char* parList, byte* pldPtr, int pldLen) {
    //lprintf("MQTT callback - globalOptions cmd received, subtopic: %s payload: %s\n", parList, pldPtr);
    int idx;                                                    // to store the index of the option in gOptsTags array
    int res;
    //
    if (!str2idx((const char*)gOptsTags, GLOBAL_OPTIONS_TAGS_CNT, sizeof(gOptsTags[0]), offsetof(struct tagAccess, keyStr), (const char*)parList, &idx)) {
        ErrWrite(ERR_CRITICAL, "Alarm Global option %s  not found. Payload = %s fail\n", parList, (const char*)pldPtr);
        return;                                                 // option name not found
    }
    // got it, idx will hold the option index in gOptsTags
    if(!gOptsTags[idx].patchCallBack((byte*)&alarmGlobalOpts, gOptsTags[idx].patchOffset, gOptsTags[idx].patchLen, (const char*)pldPtr))            
        ErrWrite(ERR_CRITICAL, "Setting Alarm Global option %s  with Payload = %s fail\n", parList,  (const char*)pldPtr);
}
// 
// void OTAconfigHandler(int topicIdx, int argCnt, const char* parList, byte* pldPtr, int pldLen)
// process payload received on "/pdox/metrics/exchange" topic 
// params:      int topicIdx            - NA
//              int argCnt              - how many param (options) extracted when parcing received topic according to
//                                        topic specs (with %s), specified in MQTT_subscriptions[]. In this case it shall be 0
//              const char* parList     - NA
//              byte* pldPtr            - pointer to received on this topic payload
//              int pldLen              - length of received on this topic payload
//
void OTAconfigHandler(ALARM_DOMAINS_t topicIdx, int argCnt, const char* parList, byte* pldPtr, int pldLen) {
        //lprintf("MQTT callback - metrics/exchange received:\n");
        //lprintf("%s\n", payload);
#ifndef ARDUINO
        // on Windows, we don't have MQTT support, so read MQTT payload from file
        if (!storageSetup()) {						            // mount file system
            ErrWrite(ERR_CRITICAL, "Mount storage fail\n");
            return;
        }
        pldPtr = tempMQTTpayload;                               // pldPtr must point to buffer with config CSV
        pldLen = sizeof(tempMQTTpayload);                       // here is the size of the buffer, after read will contain the bytes read
        pldLen = readCsvConfig((const char*)pldPtr, tempMQTTpayload, sizeof(tempMQTTpayload)); //read config file  into tempMQTTpayload
        storageClose();
        if (!pldLen) {
            ErrWrite(ERR_WARNING, "Wrong or missing CSV config file\n");
            return;
        }
#endif
        if (parseConfigFile((char*)pldPtr, pldLen, false)) {
            ErrWrite(ERR_CRITICAL, "METRICS/EXCHANGE parse fail\n");
            return;
        }
        //printAlarmConfig();
        lprintf("Alarm config received from MQTT will stored in %s\n", csvConfigFname);
        // Mount Storage and write it to csv config file
        if (!storageSetup()) {						// mount file system
            ErrWrite(ERR_CRITICAL, "Mount storage fail\n");
            return;
        }
        if (!renameFile(csvConfigFname, csvBackupFname)) {
            ErrWrite(ERR_CRITICAL, "Renaming MQTT config fail\n");
            storageClose();
            return;
        }
        if (!saveCsvConfig(csvConfigFname, pldPtr, pldLen)) {
            ErrWrite(ERR_CRITICAL, "Writing MQTT config fail\n");
            storageClose();
            return;
        }
        storageClose();
        newConfigAvailable = true;
        ErrWrite(ERR_CRITICAL, "MQTT received config successfully stored\n");
 }
//
// 
// int subsscriptTopic( const char mqttTopic[], const char mqttName[])
// subsribe to MQTT topic, in case of issue, disconnects MQTT client and set reconnect timer
// params:      const char mqttTopic[]  - topic to subscribe to
//              const char mqttName[]   - parameter to substitute $s in the topic
// returns:     true if success, otherwise false      
// TODO: modify to use multiple parameters (%s) by using varable args                               
//
int subsscriptTopic(const char mqttTopic[], const char mqttName[]) {
    snprintf(tempMQTTbuf, sizeof(tempMQTTbuf), mqttTopic, mqttName);
    //ErrWrite(ERR_DEBUG, "MQTTsetup: subscribing to %s\n", tempMQTTbuf);
    if (!MQTTclient.subscribe(tempMQTTbuf)) {                       // subscribe failure, cancel and
        MQTTclient.disconnect();                                    // try again later
        timeoutOps(SET, MQTT_CONNECT_TIMER);						// restart timer
        ErrWrite(ERR_DEBUG, "MQTTsetup: subscribe to %s failure, MQTT disconnecting...\n", tempMQTTbuf);
        return false;
    }
    ErrWrite(ERR_DEBUG, "Subscribed to %s\n", tempMQTTbuf);
    return true;
}
//
// Family of functions to subscribe to different MQTT topics depends on the alarm domain
//
// void zonePartPgmSubscribe(int what, int idx)
//  subscribe for control topic for /zones/partitions/outputs/etc
//  params:  int what   - index in dbPtrArr[] database to select proper DB' geometry (base address, offset of name, etc)
//           int idx    - index in MQTT_subscriptions[] to get topic to subscribe to
//  return:  true   - if all subscriptions aresuccessfull
//           false  - failure occured
//
int zonePartPgmSubscribe(int what, int idx) {
    const char* dbPtr = dbPtrArr[what].dbBaseAddr;
    int entriesCnt = dbPtrArr[what].elmCnt;
    int offset = dbPtrArr[what].nameOffs;
    int stride = dbPtrArr[what].elmLen;
    int validOff = dbPtrArr[what].validOffs;
    int atLeastOneValid = 0;                                        // subscribe only for valid zones/part/pgms/...


    for (int i = 0; i < entriesCnt; i++) {                          // check if valid/enabled. For zones zoneType = 0 means disabled
        if (!*((uint16_t*) &(dbPtr[i*stride + validOff]) ) )          // zoneType and partition and pgm .valid are UINT16 types
            continue;                                               // not valid, go to next
        if (!subsscriptTopic(MQTT_subscriptions[idx].topic, &(dbPtr[i * stride + offset])))
            return false;                                           // upper level will disconnect MQTT stack and will try to reconnect
        atLeastOneValid++;
    }
    if (atLeastOneValid)                                             // if at least one is valid, subscribe to /pdox/topic/all
        if (!subsscriptTopic(MQTT_subscriptions[idx].topic, SUBTOPIC_WILDCARD))
            return false;
    return true;

}
//
int subscribeOTAconfig(int what, int idx) {
    if(!subsscriptTopic(MQTT_subscriptions[idx].topic, ""))
        return false;
    return true;
}
//
int subscribeGlobalOptions(int what, int idx) {
    for (int i = 0; i < GLOBAL_OPTIONS_TAGS_CNT; i++) {                          // for zones zoneType = 0 means disabled
        if (!subsscriptTopic(MQTT_subscriptions[idx].topic, gOptsTags[i].keyStr))
            return false;
    }
    return true;
}
//
// 
// 
#ifdef HASS_INTEGRATION
int MQTT_HASS_integration() {
    int cnt;
    for (int zn = 0; zn < MAX_ALARM_ZONES; zn++) {          // for each zone
        if (!zonesDB[zn].zoneType)
            continue;
        // prepare  HASS integration JSON for zone in global line[]
        cnt = 0;
        cnt += sprintf(&line[cnt], "\n{\n\t" HASS_AVAILABILITY, HASS_AVAILABILITY_TOPIC);
        cnt += sprintf(&line[cnt], "\t" HASS_DEVICE, HASS_ID_MANUFACTURER, HASS_ID_MODEL, HASS_IDENTITY, HASS_ID_MANUFACTURER, HASS_ID_MODEL, HASS_ID_NAME, HASS_ID_SW_VERS);
        cnt += sprintf(&line[cnt], "\n\t" HAZZ_ZONE_DETAILS "\n}\n", HASS_CLASS_ZONE, zonesDB[zn].zoneName, FALSE_PAYLOAD, TRUE_PAYLOAD, zonesDB[zn].zoneName, OPEN_PROPERTY, HASS_IDENTITY, zonesDB[zn].zoneName, OPEN_PROPERTY);
        PublishMQTT(line, HASS_BIN_SENSOR, HASS_IDENTITY, zonesDB[zn].zoneName);
    }
    //
    for (int prt = 0; prt < MAX_PARTITION; prt++) {          // for each partition
        if (!partitionDB[prt].valid)
            continue;
        cnt = 0;
        cnt += sprintf(&line[cnt], "\n{\n\t" HASS_AVAILABILITY, HASS_AVAILABILITY_TOPIC);
        cnt += sprintf(&line[cnt], "\t" HASS_COMMAND_TOPIC ",\n", partitionDB[prt].partitionName);
        cnt += sprintf(&line[cnt], "\t" HASS_DEVICE, HASS_ID_MANUFACTURER, HASS_ID_MODEL, HASS_IDENTITY, HASS_ID_MANUFACTURER, HASS_ID_MODEL, HASS_ID_NAME, HASS_ID_SW_VERS);
        cnt += sprintf(&line[cnt], "\n\t" HASS_ALARM_PANEL_DETAILS "\n}\n", partitionDB[prt].partitionName, "arm", "arm_stay", "arm_force", "disarm", partitionDB[prt].partitionName, HASS_IDENTITY, partitionDB[prt].partitionName);
        PublishMQTT(line, HASS_ALARM_PANEL, HASS_IDENTITY, partitionDB[prt].partitionName);
    }
    // subscribe for /pdox/control/partitions/all as well
    cnt = 0;
    cnt += sprintf(&line[cnt], "\n{\n\t" HASS_AVAILABILITY, HASS_AVAILABILITY_TOPIC);
    cnt += sprintf(&line[cnt], "\t" HASS_COMMAND_TOPIC ",\n", SUBTOPIC_WILDCARD);
    cnt += sprintf(&line[cnt], "\t" HASS_DEVICE, HASS_ID_MANUFACTURER, HASS_ID_MODEL, HASS_IDENTITY, HASS_ID_MANUFACTURER, HASS_ID_MODEL, HASS_ID_NAME, HASS_ID_SW_VERS);
    cnt += sprintf(&line[cnt], "\n\t" HASS_ALARM_PANEL_DETAILS "\n}\n", SUBTOPIC_WILDCARD, "arm", "arm_stay", "arm_force", "disarm", SUBTOPIC_WILDCARD, HASS_IDENTITY, SUBTOPIC_WILDCARD);
    PublishMQTT(line, HASS_ALARM_PANEL, HASS_IDENTITY, SUBTOPIC_WILDCARD);
    //
    return true;
}
#endif
//
// void mqttSetup(void)
// tries at MQTT_CONNECT_TIMER interval to setup MQTT connection and subscribe to needed topics
// if not successful, disconnect (in case of subscription failure) and restarts the interval
//
void mqttSetup(void) {
//
    int i; int res = 0;
    //lprintf("Time for MQTT connection retry?\n");
    if (!timeoutOps(GET, MQTT_CONNECT_TIMER)) {									// run the loop on spec intervals									
        return;																	// nothing to do
    }
    lprintf("Connecting to MQTT...\n");
    MQTTclient.setCallback(MQTTcallback);
    if (MQTTclient.setBufferSize(MAX_MQTT_PAYLOAD)) {
        if(MQTTclient.getBufferSize() == MAX_MQTT_PAYLOAD)
            lprintf("Success setting MQTT buffer size\n");
        else
            ErrWrite(ERR_CRITICAL, "Failures setting MQTT buffer size\n");
    }
    else
        lprintf("Failures setting MQTT buffer size\n");
    //
    if (MQTTclient.connect(MQTT_CLIENT_NAME, mqttUser, mqttPassword, willTopic, willQoS, willRetain, willMessage, true)) {
        lprintf("MQTT client connected as %s\n", MQTT_CLIENT_NAME);
        // subscribe to endpoints 
        for (i = 0; i < MQTT_SUBS_CNT; i++) {
            if (!MQTT_subscriptions[i].subcBack(MQTT_subscriptions[i].what, i)) // in case of unsuccessfull subscription the callback 
                return;                                                         // will disconnect MQTT client, no point to continue
        }    
#ifdef HASS_INTEGRATION
        // publish HASS integration staff
        res = MQTT_HASS_integration();
#endif
        if(res && (i==MQTT_SUBS_CNT))                                             
            lprintf("Subscribing finished\n");
    }
    else {
        timeoutOps(SET, MQTT_CONNECT_TIMER);					// cannot connect, restart timer
        ErrWrite(ERR_WARNING, "MQTT connect failed with state %d\n", MQTTclient.state());
    }
}
//
//
void MQTT_loop() {
    if (WIFIsetup()) {                                      // WLAN connected
        if (!MQTTclient.connected())
            mqttSetup();									// MQTT - connect if not connected
        else
            MQTTclient.loop();                              // run MQTT code
    }// run MQTT stack
    //else
        //lprintf("!");                                     // MQTT client not connected\n
}
//
//
// PublishMQTT(const char* payload, const char* what, ...)  - called to publish something to MQTT brocker
// payload is constant string, while topic consist of format string and parameters (printf like)
// params:          const char* payload -  payload string
//                  const char* topic    - topic includding format specs (%s, %d, ...)
//                  ...                 -   va list parameters for topic
//
void  PublishMQTT(const char* payload, const char* topic, ...) {
    if (!MQTTclient.connected()) {
        //lprintf("MQTT client not connected\n");
        return;
    }
    va_list args;
	va_start(args, topic);
	vsnprintf((char *) tempMQTTbuf, sizeof(tempMQTTbuf)-1, topic, args);
    //("before publish\n");
    MQTTclient.publish(tempMQTTbuf, payload);
	va_end(args);
    //lprintf("Published %s %s\n", tempMQTTbuf, payload);
}
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
        //PublishMQTT(TRUE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_PROPERTY);
        //PublishMQTT(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_FINISHED_PROPERTY);
        break;
    default:
        ErrWrite(ERR_WARNING, "reportArm invalid partition armStatus %d \n", partitionRT[prt].armStatus);
        return;
    }
    PublishMQTT(stayArmPld, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, STAY_ARM_PROPERTY);
    PublishMQTT(forceArmPld,PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, FORCE_ARM_PROPERTY);
    PublishMQTT(regArmPld,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ARM_PROPERTY);
    //
    // publish on  ....states/partitions/PartitionName/current_state  payload  current state  -armed_home, armed_away, arming, disarmed,...
    if (partitionRT[prt].partitionTimers[EXIT_DELAY_TIMER].timerFSM == RUNNING)
        currState = CUR_STATE_ARMING;
    PublishMQTT(currState,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, CURRENT_STATE_PROPERTY);
}
//
//
void publishAlarm(byte prtIdx) {
    PublishMQTT(TRUE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prtIdx].partitionName, ALARM_PROPERTY);
    PublishMQTT(_itoa(partitionSTATS[prtIdx].alarmZonesCnt, tmpBuf, 10), PARTITIONS_STATES_TOPIC, partitionDB[prtIdx].partitionName, ALARM_ZONES_PROPERTY);    // send over MQTT
}
//
// publishTroubleZone - reports to MQTT, panel, etc that tamper occured in partition/zone
// TODO - implement me
//
void publishTroubleZone(int zone) {
    //lprintf("Report Trouble - Implement me\n");
    PublishMQTT(TRUE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zone].zoneName, TROUBLE_PROPERTY); // send over MQTT
}
//
// publishAlarmZone - reports to MQTT, panel, etc that tamper occured in partition/zone
// TODO - implement me
//
void publishAlarmZone(int zone) {
    //lprintf("Report Trouble - Implement me\n");
    PublishMQTT(TRUE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zone].zoneName, ALARM_PROPERTY); // send over MQTT
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
    PublishMQTT(_itoa(partitionSTATS[prt].tamperZonesCnt,tmpBuf,10),           PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, TAMPER_ZONES_PROPERTY);	
    PublishMQTT(_itoa(partitionSTATS[prt].openZonesCnt,tmpBuf,10),             PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, OPEN_ZONES_PROPERTY);	
    PublishMQTT(_itoa(partitionSTATS[prt].amaskZonesCnt,tmpBuf,10),            PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, ANTIMASK_ZONES_PROPERTY);
    PublishMQTT(_itoa(partitionSTATS[prt].bypassedZonesCnt,tmpBuf,10),         PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, BYPASSED_ZONES_PROPERTY);  
    PublishMQTT(_itoa(partitionSTATS[prt].openZonesCntEDSD1,tmpBuf,10),        PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, OPEN_ED_ZONES_PROPERTY);   
    PublishMQTT(_itoa(partitionSTATS[prt].openZonesCntEDSD2,tmpBuf,10),        PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, OPEN_SD_ZONES_PROPERTY);   
    PublishMQTT(_itoa(partitionSTATS[prt].ignorredTamperZonesCnt,tmpBuf,10),   PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, IGNORRED_TAMPER_ZONES_PROPERTY);  
    PublishMQTT(_itoa(partitionSTATS[prt].ignorredAmaskZonesCnt,tmpBuf,10),    PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, IGNORRED_AMASK_ZONES_PROPERTY);   
    PublishMQTT(_itoa(partitionSTATS[prt].alarmZonesCnt,tmpBuf,10),            PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, ALARM_ZONES_PROPERTY);
    PublishMQTT(_itoa(partitionSTATS[prt].notBypassedEntyDelayZones,tmpBuf,10),PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, NOT_BYPASSED_ED_ZONES_PROPERTY);  
    if (partitionSTATS[prt].alarmZonesCnt)
        printf("In alarm now\n");
    PublishMQTT((partitionSTATS[prt].alarmZonesCnt ? "true" : "false"), PARTITIONS_STATES_TOPIC, partitionDB[prt].partitionName, ALARM_PROPERTY);    // send over MQTT
    // report EXIT DELAY
    if (partitionRT[prt].changed & CHG_EXIT_DELAY_TIMER) {
        if (partitionRT[prt].partitionTimers[EXIT_DELAY_TIMER].timerFSM == RUNNING) {
            PublishMQTT(TRUE_PAYLOAD,    PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_PROPERTY);
            PublishMQTT(FALSE_PAYLOAD,   PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_FINISHED_PROPERTY);
        }
        else {
            PublishMQTT(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_PROPERTY);
            PublishMQTT(TRUE_PAYLOAD,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, EXIT_DELAY_FINISHED_PROPERTY);
        }
    }
    if (partitionRT[prt].changed & (CHG_ENTRY_DELAY1_TIMER | CHG_ENTRY_DELAY2_TIMER)) {
        if ((partitionRT[prt].partitionTimers[ENTRY_DELAY1_TIMER].timerFSM == RUNNING) ||
            (partitionRT[prt].partitionTimers[ENTRY_DELAY2_TIMER].timerFSM == RUNNING)) {
            PublishMQTT(TRUE_PAYLOAD,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_PROPERTY);
            PublishMQTT(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_FINISHED_PROPERTY);
        }
        else {                                                                                  // ENTRY_DELAY_X timer done or not started
            PublishMQTT(FALSE_PAYLOAD, PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_PROPERTY);
            PublishMQTT(TRUE_PAYLOAD,  PARTITIONS_STATES_TOPIC, (const char*)&partitionDB[prt].partitionName, ENTRY_DELAY_FINISHED_PROPERTY);
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
                PublishMQTT(TRUE_PAYLOAD,   ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY);  // TODO - is this really needed
            }
            else if (zonesRT[zn].zoneStat & ZONE_AMASK) {        // amask in zone
                PublishMQTT(TRUE_PAYLOAD,   ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY);  // TODO - is this really needed
            }
            else if (zonesRT[zn].zoneStat & ZONE_OPEN) {         // open zone
                PublishMQTT(TRUE_PAYLOAD,   ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY); 
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
            }
            else {                                          // closed zone
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, OPEN_PROPERTY);
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, ANTIMASK_PROPERTY);
                PublishMQTT(FALSE_PAYLOAD,  ZONES_STATES_TOPIC, zonesDB[zn].zoneName, TAMPER_PROPERTY);
            }
        }
        if (zonesRT[zn].changed & ZONE_USR_BYPASS_CHANGED) {// if zone is BYPASSED/UNBYPASSED on user request
            if (zonesRT[zn].bypassed & ZONE_BYPASSED)       // report it
                PublishMQTT(TRUE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zn].zoneName, BYPASS_PROPERTY);
            else
                PublishMQTT(FALSE_PAYLOAD, ZONES_STATES_TOPIC, zonesDB[zn].zoneName, BYPASS_PROPERTY);
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
        PublishMQTT(pgmsDB[pgm].cValue ? PGM_ON_PROPERTY : PGM_OFF_PROPERTY, MQTT_OUTPUTS_STATES, pgmsDB[pgm].pgmName);
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
