#pragma once

#include "alarm-core-public-defs.h"  // move to the cpp file???
#include "alarm-core-mqtt-defs.h"
#include <string.h>
#include <stdio.h>

#ifdef ARDUINO
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#endif

typedef unsigned char byte;

// declarations
void trigerArm(void* param1, void* param2, void* param3);
extern              void zoneCmd(void* param1, void* param2, void* param3); 
extern              void pgmCmd(void* param1, void* param2, void* param3);
extern int          getZoneCount();
extern int          getZoneIndex(const char* name);
const char*         getZoneName(int zoneIdx);
//extern bool         isZoneValid(int zoneIdx);
//
extern int          getPartitionCount();
extern int          getPartitionIndex(const char* name);
extern const char*  getPartitionName(int partIdx);
extern bool         isPartitionValid(int partIdx);
extern void         setPartitionTarget(int partIdx, int action);
//
extern int          getPgmCount();
//extern bool         isPgmValid(int pgmIdx);
extern int          getPgmIndex(const char* name);
extern const char*  getPgmName(int pgmIdx);
//
extern bool         setGlobalOptions(const char* opt_name, const char* opt_val);
extern int          getGlobalOptionsCnt();
extern const char*  getGlobalOptionKeyStr(int);


// subscription handlers
bool subscribeZonesToMQTT(int what);
bool subscribePartitionsToMQTT(int what);
bool subscribePgmsToMQTT(int what);
bool subscribeGlobalOptionsToMQTT(int what);
//int  subscribeOTAconfig(int what, int idx);
// 
// received payload handlers
//void OTAconfigHandler(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);
void globalOptionsHandler(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);
void zonesPartEtcControlHandler(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);

//extern Alarm alarm; // Global instance of the Alarm class


byte 		tempMQTTpayload[MAX_MQTT_PAYLOAD + 10];	// add some padding for \0
char 		line[1024];                             // used in MQTT callback to store the received payload
//
struct inpPlds_t {
    const char MQTTpld[MAX_MQTT_TOKEN_LEN];
    unsigned int action;
    void (*cBack)(void* param1, void* param2, void* param3);
};
//
// Partition control supported payloads (commands)
struct  inpPlds_t partPlds[] = {
    // TODO: some PAI command are not implemented - arm_sleep, arm_stay_stayd, arm_sleep_stay 
    {"disarm",          DISARM,        &trigerArm},
    {"arm",             REGULAR_ARM,   &trigerArm},
    {"arm_force",       FORCE_ARM,     &trigerArm},
    {"arm_stay",        STAY_ARM,      &trigerArm},
	{"arm_instant",     INSTANT_ARM,   &trigerArm},
  //{"arm_sleep",       ARM_SLEEP,     &trigerArm},
  //{"arm_stay_stayd",  ARM_STAY_STAYD,&trigerArm},
  //{"arm_sleep_stay",  ARM_SLEEP_STAY,&trigerArm},
};
//
// Zone control supported payloads (commands)
struct  inpPlds_t zonePlds[] = {
    {"bypass",        ZONE_BYPASS_CMD,    &zoneCmd},
    {"clear_bypass",  ZONE_UNBYPASS_CMD,  &zoneCmd},
    {"tamper",        ZONE_TAMPER_CMD,    &zoneCmd},
    {"close",         ZONE_CLOSE_CMD,     &zoneCmd},
    {"open",          ZONE_OPEN_CMD,      &zoneCmd},
    {"anti-mask",     ZONE_AMASK_CMD,     &zoneCmd},
  //{"analogSet",     ZONE_ANAL_SET_CMD,  &fakeAnalogSetZn},
  //{"digitalSet",  ZONE_DIGITAL_SET_CMD, &digitalSetZn},
};
//
// pgm (output) control supported payloads (commands). TODO - implement pulse command
struct  inpPlds_t pgmPlds[] = {
    {"on",      PGM_ON,     &pgmCmd},
    {"off",     PGM_OFF,    &pgmCmd},
    {"pulse",   PGM_PULSE,  &pgmCmd},
};
//
// format of the entries of the array handling MQTT topics
struct MQTT_cmd_t {
	const char	topic[MAX_MQTT_TOPIC];                                  // MQTT control topic, consist of topic/name, wher name is zone/partition/pgm name
	ALARM_DOMAINS_t what;                                               // what domain of alarm element (zones/partitions/pgms/...) is concerned
	void        (*cBack)(ALARM_DOMAINS_t what, int argCnt, const char* parList, byte* pldPtr, int pldLen);  // topic handler
	struct inpPlds_t* pldsArrPtr;                                       // pointer to valid payloads array
	int         maxPlds;                                                // num of elements in payloads array
	bool         (*subcBack)(int what);                                 // subscribe handler
};
//
//
// Array of MQTT topics handlers
// ADD HERE NEW TOPICS AND CORRESPONDING HANDLERS
//
struct MQTT_cmd_t MQTT_subscriptions[] = {
	{MQTT_ZONES_CONTROL,        ZONES,       &zonesPartEtcControlHandler, zonePlds, (sizeof(zonePlds) / sizeof(zonePlds[0])),	&subscribeZonesToMQTT},
	{MQTT_PARTITIONS_CONTROL,   PARTITIONS,  &zonesPartEtcControlHandler, partPlds, (sizeof(partPlds) / sizeof(partPlds[0])),	&subscribePartitionsToMQTT},
	{MQTT_OUTPUTS_CONTROL,      PGMS,        &zonesPartEtcControlHandler, pgmPlds,  (sizeof(pgmPlds) / sizeof(pgmPlds[0])),		&subscribePgmsToMQTT},
	//{MQTT_METRICS_EXCHANGE,     -1,          &OTAconfigHandler,           NULL,                   0,								&subscribeOTAconfig},
	{MQTT_GLOBAL_OPT_CONTROL,   GLOBAL_OPT,  &globalOptionsHandler,       NULL,                      0,	                        &subscribeGlobalOptionsToMQTT },
};
#define MQTT_SUBS_CNT   (sizeof(MQTT_subscriptions)/sizeof(MQTT_subscriptions[0]))
//
// ----------------------- Subscribe to MQTT topics -----------------------
// int subsscriptTopic( const char mqttTopic[], const char mqttName[])
// subsribe to MQTT topic, in case of issue, disconnects MQTT client and set reconnect timer
// params:      const char mqttTopic[]  - topic to subscribe to
//              const char mqttName[]   - parameter to substitute $s in the topic
// returns:     true if success, otherwise false      
// TODO: modify to use multiple parameters (%s) by using varable args                               
//
int subsscriptTopic(const char mqttTopic[], const char mqttName[]) {
    snprintf((char *)tempMQTTpayload, sizeof(tempMQTTpayload), mqttTopic, mqttName);  // use tempMQTTpayload as temp buf
    //printf("MQTTsetup: subscribing to %s\n", tempMQTTbuf);
#ifdef ARDUINO
    //if (!MQTTclient.subscribe(tempMQTTbuf)) {                       // subscribe failure, cancel and
    //    MQTTclient.disconnect();                                    // try again later
    //    timeoutOps(SET, MQTT_CONNECT_TIMER);						// restart timer
    //    ErrWrite(ERR_DEBUG, "MQTTsetup: subscribe to %s failure, MQTT disconnecting...\n", tempMQTTbuf);
    //    return false;
    //}
#endif
    printf("Subscribed to %s\n", tempMQTTpayload);
    return true;
}
//
// Family of functions to subscribe to different MQTT topics depends on the alarm domain
//
// bool zonePartPgmSubscribe(int what)
//  subscribe for control topic for /zones/partitions/outputs/etc
//  params:  int what    - index in MQTT_subscriptions[] to get topic to subscribe to
//  return:  true   - if all subscriptions aresuccessfull
//           false  - failure occured
//
//
bool subscribeZonesToMQTT(int what) {
    bool success = true;

    // Get the maximum number of zones from the Alarm class
    int maxZones = getZoneCount();
    // Use the topic from MQTT_subscriptions for ZONES
    const char* topicTemplate = MQTT_subscriptions[what].topic;

    for (int i = 0; i < maxZones; ++i) {
        const char* zoneName = getZoneName(i);
        if (zoneName[0] == '\0') { // Check for empty string
            continue; // Skip invalid zones
        }
        if (!subsscriptTopic(topicTemplate, zoneName)) {
            printf("Failed to subscribe zone: %s\n", zoneName);
            success = false;
        }
    }
    return success;
}
//
bool subscribePartitionsToMQTT(int what) {
    bool success = true;

    // Get the maximum number of partitions from the Alarm class
    int maxPartitions = getPartitionCount();
    // Use the topic from MQTT_subscriptions for PARTITIONS
    const char* topicTemplate = MQTT_subscriptions[what].topic;

    for (int i = 0; i < maxPartitions; ++i) {
        const char* partitionName = getPartitionName(i);
        if (partitionName[0] == '\0') { // Check for empty string
            continue; // Skip invalid partitions
        }
        if (!subsscriptTopic(topicTemplate, partitionName)) {
            printf("Failed to subscribe partition: %s\n", partitionName);
            success = false;
        }
    }
    return success;
}
//
bool subscribePgmsToMQTT(int what) {
    bool success = true;

    // Get the maximum number of PGMs from the Alarm class
    int maxPgms = getPgmCount();
    // Use the topic from MQTT_subscriptions for PGMS
    const char* topicTemplate = MQTT_subscriptions[what].topic;

    for (int i = 0; i < maxPgms; ++i) {
        const char* pgmName = getPgmName(i);
        if (pgmName[0] == '\0') { // Check for empty string
            continue; // Skip invalid PGMs
        }
        if (!subsscriptTopic(topicTemplate, pgmName)) {
            printf("Failed to subscribe PGM: %s\n", pgmName);
            success = false;
        }
    }
    return success;
}

//
//bool subscribeOTAconfigToMQTT(const ALARM_DOMAINS_t what) {
//    if (!subsscriptTopic(MQTT_subscriptions[what].topic, ""))
//        return false;
//    return true;
//}
//
bool subscribeGlobalOptionsToMQTT(int what) {
    // Use the topic from MQTT_subscriptions for Global options
    const char* topicTemplate = MQTT_subscriptions[what].topic;
	auto maxOptions = getGlobalOptionsCnt();
    for (int i = 0; i < maxOptions; i++) {                          // for zones zoneType = 0 means disabled
        if (!subsscriptTopic(topicTemplate, getGlobalOptionKeyStr(i)))
            return false;
    }
    return true;
}
//
// subscribe to endpoints 
bool subscribeMQTTtopics(void) {
    int i;
    for (i = 0; i < MQTT_SUBS_CNT; i++) {
		if (!MQTT_subscriptions[i].subcBack(i))                // subscribe to the topic
            return FALSE;
    }
    printf("Subscribing finished\n");
	return TRUE;
}
//
// ------------------------- Receive MQTT messages -------------------------
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
    const char* topicPtr;
    //unsigned int    topicLen;
    int found = 0;
    //
    //payload[pldLen] = 0;                                                 // payload is not NULL terminated, but we are expecting only strings, so shall be safe to terminate it
    printf("MQTT recv:%s ", rcvTopic);
    printf("Payload: %.*s Len:%d\n", pldLen, payload, pldLen);
    
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
            while (rcvTopic[i] && (rcvTopic[i] != '/')) {                   // copy everything until '/' or end-of-string, but not more than MAX_MQTT_TOKEN_LEN
                if (z >= sizeof(line)) {                                    // check for buffer (line) overflow
                    printf("MQTT topic %s parameters exceed buffer size (1024)", rcvTopic);
                    break;
                }
                if (++k >= MAX_MQTT_TOKEN_LEN) {                                      // %s expansions are limited to MQTT_TOKEN_LEN
                    printf("MQTT topic %s parameters len exceed MAX_MQTT_TOKEN_LEN (32)", rcvTopic);
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
        printf("MQTT topic: %s dosn't match any subscribed topic\n", rcvTopic);  // this shallnot happened
        return;
    }
    printf( "MQTT topic: %s found\n", rcvTopic);
    //
    // call the topic processing callback now
    MQTT_subscriptions[idx].cBack(ALARM_DOMAINS_t(idx), nArg, line, payload, pldLen);
}
//
// void processMQTTpayload(struct  inpPlds_t Cmds[], int CmdsCnt, int idx, const byte payld[], int len)
// tries to match the string in payload[] to command (bypass, clear bypass, arm, siarm...) in Cmds[] and if yes, calls the corresponding callback
// params:  struct  inpPlds_t Cmds[]    - array of supported payloads (commands) for zones, partitions, etc
//          int CmdsCnt                 - number of entries in the array of supported payloads
//          int itemIdx                 - index of zone/partition/pgm/etc in Alarm private data
//                                        TODO  - extracted from received MQTT topic. -1 MEANS SET ALL
//          const byte payld[]          - payload
//          int len                     - payload len
// returns: none
//
void processMQTTpayload(struct  inpPlds_t Cmds[], int CmdsCnt, int itemIdx, const byte payld[], int len) {
    int i; const byte* secStr = NULL;
    for (i = 0; i < CmdsCnt; i++) {
        // match payload to what allowed by command
		if (strlen(Cmds[i].MQTTpld) != len)
			continue;                                             // length of the payload doesn't match the command, continue the search
        if (!_strnicmp((const char*)payld, Cmds[i].MQTTpld, strlen(Cmds[i].MQTTpld))) { // note: payld is not NULL terminated
            printf("Found CMD: MQTT cmd %s, action = %d at index %d\n", Cmds[i].MQTTpld, Cmds[i].action, i);
            secStr = &payld[strlen(Cmds[i].MQTTpld)];             // secStr is pointer to rest (if any) of the payload after the command, to be parsed by the callback;
            break;
        }
        else
            continue;                                             // no match, continue the search
    }
    if (i < CmdsCnt) {
        Cmds[i].cBack(&itemIdx, &(Cmds[i].action), (void*)secStr);    // process the payload, idx contain index in particular database (zonesDB, partitonsDB,...)
    }                                                             // for exaple with zonesPartEtcControlHandler(...)
    else {                                                        // payload doesn't match any of the entries in allowed payloads array
        printf("Unrecognized MQTT command payload:  %.*s\n", len, payld);
        return;
    }
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
	int itemIdx = 0;                                // temporary buffer to hold the name of the zone/partition/pgm/...
    int minIdx = 0, maxIdx = 0;                 // to store the index of the zone with zone name in parList in zonesDB
    //
    printf("MQTT callback - control/zones/partitions/pgms cmd received, subtopic: %s payload: %s\n", parList, pldPtr);
    //
    if (!_stricmp(parList, SUBTOPIC_WILDCARD)) {            // check for 'all' first, means it is requested to set all entries in the database to same payload
        //maxIdx = dbPtrArr[dbPropsIdx].elmCnt;             // set all, loop will run from 0 to elements count 
		printf("SUBTOPIC_WILDCARD not supported yet :-(\n");
        return;
    }
    else {
        //if (!str2idx(dbPtrArr[dbPropsIdx].dbBaseAddr, dbPtrArr[dbPropsIdx].elmCnt, dbPtrArr[dbPropsIdx].elmLen, dbPtrArr[dbPropsIdx].nameOffs, (const char*)parList, &idx))
        //    return;   
		// tries to find name (%s) from subtopic. Returns 0 if subtopic not found, else index in corresponding database
        switch (topicIdx) {
        case ZONES: {
            itemIdx = getZoneIndex(parList);
            break;
        }
        case PARTITIONS: {
			itemIdx = getPartitionIndex(parList);
            break;
        }
		case PGMS: {
			itemIdx = getPgmIndex(parList);
			break;
		}
        default:
			printf("Unknown topic %d\n", topicIdx);
            break;
        }   
		// TODO - handle SUBTOPIC_WILDCARD
        // got it, idx will hold the zone/partition/pgm/... index in zonesDB/partitionDB/pgmsDB/....
        //minIdx = idx; maxIdx = minIdx + 1;                    // minIdx is entry index, set maxIdx to minIdx+1  to run the loop only once                                
    }
    // we have zone/part/pgm/... index now and can process the payload
	//for (int i = minIdx; i < maxIdx; i++)                     // TODO - handle SUBTOPIC_WILDCARD
	if (itemIdx < 0) {
		printf("MQTT sub-topic: %s not found\n", parList);
		return;
	}
    processMQTTpayload(MQTT_subscriptions[topicIdx].pldsArrPtr, MQTT_subscriptions[topicIdx].maxPlds, itemIdx, pldPtr, pldLen);
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
    printf("MQTT callback - globalOptions cmd received, subtopic: %s payload: %s\n", parList, pldPtr);
	printf("Setting Alarm Global option %s  with Payload = %s\n", parList, (const char*)pldPtr);
	if(setGlobalOptions(parList, (const char*)pldPtr))	// set global options
        printf("Global option '%s' set to '%s' successfully.\n", parList, (const char*)pldPtr);
    else 
        printf("Failed to set global option '%s' with value '%s'.\n", parList, (const char*)pldPtr);
}

//
// ------------------ Alarm class methods wrappers, called upon specific MQTT command payload ------------------
//
void trigerArm(void* param1, void* param2, void* param3) {
    int partIdx = *(int*)param1;
    const ARM_METHODS_t action = *(ARM_METHODS_t*)param2;
    // param3 is not used

    if (!isPartitionValid(partIdx))                   // check if partition is valid, not really needed
        return;

    // check first for valid params ranges and call
    if (!((action == DISARM) || (action == REGULAR_ARM) || (action == STAY_ARM) || (action == FORCE_ARM) || (action == INSTANT_ARM))) {
        printf("CMD parse: Invalid ARM action %d", action);
        return;
    }
    setPartitionTarget(partIdx, action);			    // set new target arm state
    //if(action != DISARM_ALL)								// set new target arm state
    //	partitionRT[partID].targetArmStatus = action;		// armed successfully
    //else {												// TODO DISARM ALL command
    //	for (int i = 0; i < MAX_PARTITION; i++) {
    //		partitionRT[partID].targetArmStatus = action;	
    //	}
    //}
}
//
#ifdef ARDUINO
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
        if (MQTTclient.getBufferSize() == MAX_MQTT_PAYLOAD)
            lprintf("Success setting MQTT buffer size\n");
        else
            LOG_CRITICAL("Failures setting MQTT buffer size\n");
    }
    else
        LOG_CRITICAL("Failures setting MQTT buffer size\n");
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
        if (res && (i == MQTT_SUBS_CNT))
            lprintf("Subscribing finished\n");
    }
    else {
        timeoutOps(SET, MQTT_CONNECT_TIMER);					// cannot connect, restart timer
        LOG_WARNING("MQTT connect failed with state %d\n", MQTTclient.state());
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
#endif // ARDUINO
//



//
// ------------------------  TEST vectors ------------------------
//
struct MQTTTestVector {
    const char* topic;       // The MQTT topic to test
    const char* payload;     // The payload to send
    const char* description; // Description of the test case
};
// This array contains the test vectors for the MQTT commands
MQTTTestVector testVectors[] = {

    // Valid global options commands
    {"/pdox/settings/global_options/MaxSlaves", "5", "Set MaxSlaves to 5"},
    {"/pdox/settings/global_options/RestrSprvsL", "true", "Enable restriction on supervision loss"},
    {"/pdox/settings/global_options/RestrTamper", "false", "Disable restriction on tamper"},
    {"/pdox/settings/global_options/RestrACfail", "true", "Enable restriction on AC failure"},
    {"/pdox/settings/global_options/RestrBatFail", "false", "Disable restriction on battery failure"},
    {"/pdox/settings/global_options/RestrOnBell", "true", "Enable restriction on bell failure"},
    {"/pdox/settings/global_options/RestrOnBrdFail", "false", "Disable restriction on board failure"},
    {"/pdox/settings/global_options/RestrOnAmask", "true", "Enable restriction on antimask"},
    {"/pdox/settings/global_options/TroubleLatch", "false", "Disable trouble latch"},
    {"/pdox/settings/global_options/TamperBpsOpt", "true", "Enable tamper bypass option"},
    {"/pdox/settings/global_options/TamperOpts", "1", "Set tamper options to 1"},
    {"/pdox/settings/global_options/AntiMaskOpt", "2", "Set antimask options to 2"},
    {"/pdox/settings/global_options/RfSprvsOpt", "3", "Set RF supervision options to 3"},

    // Invalid global options commands
    {"/pdox/settings/global_options/InvalidOption", "true", "Test invalid global option"},
    {"/pdox/settings/global_options/MaxSlaves", "invalid", "Set MaxSlaves to an invalid value"},
    {"/pdox/settings/global_options/RestrSprvsL", "123", "Set RestrSprvsL to an invalid value"},
    {"/pdox/settings/global_options/RestrTamper", "", "Set RestrTamper with an empty payload"},
    {"/pdox/settings/global_options/RestrACfail", "invalid", "Set RestrACfail to an invalid value"},

    // Edge cases
    {"/pdox/settings/global_options/", "true", "Test with an empty global option name"},
    {"/pdox/settings/global_options/MaxSlaves", "", "Set MaxSlaves with an empty payload"},
    {"/pdox/settings/global_options/RestrSprvsL", " ", "Set RestrSprvsL with a whitespace payload"},
    {"/pdox/settings/global_options/RestrTamper", "false", "Set RestrTamper to false (valid edge case)"},

    // Valid zone commands
    {"/pdox/control/zones/Front Door",                  "bypass",       "Test bypass command for Zone1"},
    {"/pdox/control/zones/Back Door",                   "clear_bypass", "Test clear_bypass command for Zone2"},
    {"/pdox/control/zones/Motion Sensor Living Room",   "tamper",       "Test tamper command for Zone3"},
    {"/pdox/control/zones/Window Sensor Bedroom",       "open",         "Test open command for Zone4"},
    {"/pdox/control/zones/Smoke Detector",              "close",        "Test close command for Zone5"},

    // Valid partition commands
    {"/pdox/control/partitions/Main Floor",     "disarm",   "Test disarm command for Partition1"},
    {"/pdox/control/partitions/Upper Floor",    "arm",      "Test arm command for Partition2"},
    {"/pdox/control/partitions/Main Floor",     "arm_force","Test arm_force command for Partition3"},
    {"/pdox/control/partitions/Upper Floor",    "arm_stay", "Test arm_stay command for Partition4"},
    {"/pdox/control/partitions/Main Floor",     "arm_instant","Test arm_instant command for Partition5"},

    // Valid PGM commands
    {"/pdox/control/outputs/External Siren",    "on",   "Test on command for PGM1"},
    {"/pdox/control/outputs/Garage Door",       "off",  "Test off command for PGM2"},
    {"/pdox/control/outputs/Entrance Light",    "pulse","Test pulse command for PGM3"},

    // Invalid cases
    {"/pdox/control/zones/InvalidZone", "bypass", "Test bypass command for an invalid zone"},
    {"/pdox/control/partitions/InvalidPartition", "arm", "Test arm command for an invalid partition"},
    {"/pdox/control/outputs/InvalidPGM", "on", "Test on command for an invalid PGM"},
    {"/pdox/control/zones/Back Door", "invalid_command", "Test invalid command for Zone1"},
    {"/pdox/control/partitions/Upper Floor", "invalid_command", "Test invalid command for Partition1"},
    {"/pdox/control/outputs/Entrance Light", "invalid_command", "Test invalid command for PGM1"},

    // Edge cases
    {"/pdox/control/zones/", "bypass", "Test bypass command with an empty zone name"},
    {"/pdox/control/partitions/", "arm", "Test arm command with an empty partition name"},
    {"/pdox/control/outputs/", "on", "Test on command with an empty PGM name"},
    {"/pdox/control/zones/Back Door", "", "Test empty payload for Zone1"},
    {"/pdox/control/partitions/Main Floor", "", "Test empty payload for Partition1"},
    {"/pdox/control/outputs/Garage Door", "", "Test empty payload for PGM1"},
};
// This function runs the test vectors and prints the results
void runMQTTTests() {
	for (const auto& test : testVectors) {
        printf("\n--------------------- Description: %s ----------------------\n", test.description);
		printf("Testing topic: %s with payload: %s len %zu\n", test.topic, test.payload, strlen(test.payload));
		// Simulate the MQTT callback
		MQTTcallback((char*)test.topic, (byte*)test.payload, strlen(test.payload));
		printf("------------------------------------------------------------\n");
		//alarm.alarm_loop(); // Call the alarm loop to process the MQTT callback
	}
}

