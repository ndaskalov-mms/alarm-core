#ifndef ALARM_H
#define ALARM_H

typedef unsigned char byte;
#define lprintf			printf
#ifndef LOGLEVELS_ENUM_DEFINED
#define LOGLEVELS_ENUM_DEFINED
enum LogLevel_t {
    LOG_ERR_OK = 0,
    LOG_ERR_INFO = 1,
    LOG_ERR_DEBUG = 2,
    LOG_ERR_WARNING = -2,
    LOG_ERR_CRITICAL = -1,
};
#endif
//#define ErrWrite (debugCallback ? (debugCallback) : defaultDebugOut)


#include "..\alarm-core-public-defs.h"
#include "alarm-core-internal-defs.h"

//typedef void (*DebugCallbackFunc)(const char* message, size_t length);
typedef void (*DebugCallbackFunc)(LogLevel_t level, const char* format, ...);

class Alarm {

    // Grant alarmJSON access to private and protected members
    friend class alarmJSON;

public:
    // Constructor and destructor
    Alarm();
    ~Alarm();

    // debug prints
    void ErrWrite(LogLevel_t level, const char* format, ...);

    // Public methods to interact with alarm system
    // These methods don't provide direct access to underlying arrays
    void        alarm_loop(void);
    void        setDebugCallback(DebugCallbackFunc callback);

    // Zone-related methods
    int         getZoneIndex(const char* name) const; 
    int         addZone(const ALARM_ZONE& newZone);
    int         getZoneCount() const;
    bool        isZoneOpen(int zoneIndex) const;
    bool        isZoneBypassed(int zoneIndex) const;
    bool        isZoneInAlarm(int zoneIndex) const;
    const char* getZoneName(int zoneIndex) const;
    void        modifyZn(void* param1, void* param2, void* param3);
    
    // Partition-related methods
    int         getPartitionIndex(const char* name) const;
    int         addPartition(const ALARM_PARTITION_t& newPartition);
    int         getPartitionCount() const;
    bool        isPartitionArmed(int partitionIndex) const;
    int         getArmStatus(int partitionIndex) const;
    bool        hasPartitionChanged(int partitionIndex) const;
    const char* getPartitionName(int partitionIndex) const;
	int         isPartitionValid(const int idx) const;
	bool        setPartitionTarget(int partitionIndex, ARM_METHODS_t targetArmStatus);
    int         armPartition(byte prt, ARM_METHODS_t action);
    
    // PGM-related methods
    int         getPgmIndex(const char* name) const;
	int         addPgm(const ALARM_PGM& newPgm); 
    int         getPgmCount() const;
    int         getPgmValue(int pgmIndex) const;
    const char* getPgmName(int pgmIndex) const;
	void        modifyPgm(void* param1, void* param2, void* param3);

    // Global options methods
    void        setGlobalOptions(const ALARM_GLOBAL_OPTS_t& globalOptions);
    bool        setGlobalOptions(const char* opt_name, const parsedValue* opt_val);
    bool        isRestrictionActive(int restrictionType) const;
    int         getGlobalOptionsCnt() const;
    const char* getGlobalOptionKeyStr(int idx) const;
        
    // System methods
    void        updateAlarmState();
    void        processTimers();
    bool        disarmPartition(int partitionIndex);

	// Debugging and logging methods
    static void defaultDebugOut(LogLevel_t err_code, const char* what, ...);

    // printing methods - defined in alarm-core-helpers.h
	//
    static void printConfigData(struct jsonValProcessor targetKeys[], int numEntries, byte* targetPtr, int printClass);
    void        printConfigHeader(struct jsonValProcessor targetKeys[], int numEntries);
    void        printAlarmPartition(int startPt, int endPt);
    void        printAlarmZones(int startZn, int endZn);
    void        printAlarmOpts(byte* optsPtr);
    void        printAlarmPgms(void);
    void        printAlarmKeysw(byte* keyswArrPtr, int maxKeysw);
    void        printAlarmPartitionRT(int idx);
    void        printPartHeaderRT(void);
    void        printAlarmPartRT(void);
    void        reportZonesNamesBasedOnStatus(int prt, int stat);
    void        reportZonesNamesBasedOnFlag(int prt, int offset, byte bitmask);
    void        reportPartitionNamesBasedOnFlag(int offset);
    void        printZonesSummary(int prt);
    void        printParttionsSummary(void);
    //const char* titleByAction(struct zoneStates_t Cmds[], int CmdsCnt, int stateCode);

	// debug printing callback function, shall be set from Alarm clas client. Defaults to defaultDebugOut
    DebugCallbackFunc debugCallback;


#define INTERNAL_JSON_HANDLERS // comment out to disable internal JSON handlers
#ifdef INTERNAL_JSON_HANDLERS
    // Structure to define a JSON key handler
    struct JsonKeyHandler {
        const char* key;                // JSON key name
        bool (*handler)(Alarm& alarm, const char* value, int itemIndex, void* context); // Function to handle this key's value
        const char* description;        // Description of what this key does
    };

    // Structure to define a topic and its JSON handlers
    struct JsonTopicHandler {
        const char* topic;              // MQTT topic to subscribe to
        const char* itemKey;            // JSON key that identifies the item (zone, partition, etc.)
        bool (*processor)(Alarm& alarm, const char* jsonPayload, size_t length); // Function to process the entire JSON
        const char* description;        // Description of topic purpose
    };

    // Static JSON handler functions
    static bool handleZoneJsonPayload(Alarm& alarm, const char* value, int zoneIndex, void* context);
    static bool handlePartitionJsonPayload(Alarm& alarm, const char* value, int partitionIndex, void* context);
    static bool handlePgmJsonPayload(Alarm& alarm, const char* value, int pgmIndex, void* context);
    static bool handleGlobalOptionJsonPayload(Alarm& alarm, const char* value, int optionIndex, void* context);

    // Static JSON processor functions
    static bool processZoneJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length);
    static bool processPartitionJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length);
    static bool processPgmJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length);
    static bool processGlobalOptionsJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length);

    // Accessor methods for MQTT integration
    static const JsonTopicHandler* getJsonTopicHandlers() { return mqttTopicHandlers; }
    static int getJsonTopicHandlerCount() { return MQTT_TOPIC_HANDLER_COUNT; }
    
    // Process a JSON message received via MQTT
    bool processMqttMessage(const char* topic, const char* payload, size_t length);
#endif //INTERNAL_JSON_HANDLERS

private:

#ifdef INTERNAL_JSON_HANDLERS
    // Static array of MQTT topics and their handlers
    static const JsonTopicHandler mqttTopicHandlers[];
    static const int MQTT_TOPIC_HANDLER_COUNT;
#endif //INTERNAL_JSON_HANDLERS

    // Private static arrays for alarm data
    // zoneDB - database with all zones CONFIG info. 
    // zonesRT = all run time zone related data. 
    struct ALARM_ZONE zonesDB[MAX_ALARM_ZONES];
    struct ALARM_ZONE_RT zonesRT[MAX_ALARM_ZONES];
    // PGMs DB
    struct ALARM_PGM pgmsDB[MAX_ALARM_PGM];
    // alarm keysw records structure to hold all alarm pgms related info     
    struct ALARM_KEYSW keyswDB[MAX_KEYSW_CNT];
    // alarm global options storage
    struct ALARM_GLOBAL_OPTS_t  alarmGlobalOpts;
    // alarm partition options storage
    struct ALARM_PARTITION_t			partitionDB[MAX_PARTITION];		// Partitions CONFIG data
    struct ALARM_PARTITION_RUN_TIME_t	partitionRT[MAX_PARTITION];		// Partitions Entry Delay data
    struct ALARM_PARTITION_STATS_t		partitionSTATS[MAX_PARTITION];	// Partitons Run-Time statistics
    // SW version
    uint16_t	swVersion = SW_VERSION;							// software version

    // functions 
    // Timer management
    void resetAllPartitionTimers(int partitionIndex);
    int partitionTimer(int tmr, int oper, int prt);
    void processPartitionTimers(int prt);

    // Zone management
    void clearBypass(byte zn, int bypassType);
    void clearBypassAllZones(int partIxd);
    void bypassZone(byte zn, int bypassType);
    void bypassAllForceZones(int prtId);
    void bypassAllStayZones(int prtId);
    void bulkBypassZones(int prtId, int znType, int bypassBits, int invert);

    // Counting and checking functions
    int countNotBypassedEntryDelayZones(int prt);
    int check4openUnbypassedZones(int prt);

    // Arm management
    int checkArmRestrctions(byte partIdx, int action);

    // Alarm processing
    void processAlarm(int alarm);
    void processLineErrors(int zn, int opts);
    void processTampers(int zn);
    void processAmasks(int zn);
    int processEntryDelayZones(int zn);
    int processOpenZone(int zn);

    // Private helper methods
    void initializeZones();
    void initializePgms();
    void initializePartitions();
    void synchPGMstates(void);
    bool validateZoneIndex(int zoneIndex) const;
    bool validatePartitionIndex(int partitionIndex) const;
    bool validatePgmIndex(int pgmIndex) const;

    // Timer-related methods
    void checkEntryDelayTimers(int partitionIndex);
    void checkExitDelayTimer(int partitionIndex);
    void resetPartitionTimers(int partitionIndex);

};  // end of class Alarm

//
#include "alarm-core-logic.h"
#include "alarm-core-json-val-parsers.h"
#include "alarm-core-helpers.h"


void Alarm::defaultDebugOut(LogLevel_t err_code, const char* what, ...)
{
    char prnBuf[512];
    va_list args;
    va_start(args, what);
    vsnprintf(prnBuf, sizeof(prnBuf) - 1, what, args);
    switch (err_code)
    {
    case LOG_ERR_OK:
        printf("%s", prnBuf);  // Fixed: Use as string, not format
        break;
    case LOG_ERR_DEBUG:
        printf("%s", prnBuf);  // Fixed
        break;
    case LOG_ERR_INFO:
        printf("%s", prnBuf);  // Fixed
        break;
    case LOG_ERR_WARNING:
        printf("%s", prnBuf);  // Fixed
        break;
    case LOG_ERR_CRITICAL:
        printf("%s", prnBuf);  // Fixed
        break;
    default:
        if (what)
            printf("%s", prnBuf);  // Fixed
        break;
    }
    va_end(args);
}
//
// Function to set the debug callback
void Alarm::setDebugCallback(DebugCallbackFunc callback) {
    debugCallback = callback;
	//(*debugCallback)(ERR_WARNING, "Debug callback set\n");
    ErrWrite(LOG_ERR_WARNING, "Debug callback set\n");
}
// 
// Constructor
Alarm::Alarm() {
    //ErrWrite(LOG_ERR_WARNING, "Init alarm from file\n");
    initializeZones();
    initializePgms();
    initializePartitions();
    //initRTdata();
    alarmGlobalOpts = {};
    alarmGlobalOpts.SprvsLoss = 0; alarmGlobalOpts.ACfail = alarmGlobalOpts.BatFail = alarmGlobalOpts.BellFail = alarmGlobalOpts.BrdFail = 0;
    synchPGMstates();
}

// Destructor
Alarm::~Alarm() {
    // Clean up if needed
}

// Private initialization methods
void Alarm::initializeZones() {
    for (int i = 0; i < MAX_ALARM_ZONES; ++i) {
        zonesDB[i] = {};
        zonesRT[i] = {};
        zonesRT[i].zoneStat = ZONE_CLOSE;
        zonesRT[i].changed = 0;
    }
}

void Alarm::initializePgms() {
    for (int i = 0; i < MAX_ALARM_PGM; ++i) {
        pgmsDB[i] = {};

    }
}

void Alarm::initializePartitions() {
    for (int i = 0; i < MAX_PARTITION; ++i) {
        partitionDB[i] = {};
        partitionRT[i] = {};
        partitionSTATS[i] = {};
        resetAllPartitionTimers(i);
        // reset ARM state
        partitionRT[i].armStatus = partitionRT[i].targetArmStatus = DISARM;
        partitionRT[i].newCmd = false; partitionRT[i].changed = 0;
    }
}
//
int Alarm::addZone(const ALARM_ZONE& newZone) {
    for (int i = 0; i < MAX_ALARM_ZONES; ++i) {
        if (zonesDB[i].valid == 0) { // Check if the entry is unused
            zonesDB[i] = newZone;   // Copy the content of the parameter
            zonesDB[i].valid = 1;   // Mark the entry as used
            return i;               // Return the index of the entry
        }
    }
    return ERR_IDX_NOT_FND; // Return -1 if no unused entry is found
}
//
int Alarm::addPartition(const ALARM_PARTITION_t& newPartition) {
    for (int i = 0; i < MAX_PARTITION; ++i) {
        if (partitionDB[i].valid == 0) { // Check if the entry is unused
            partitionDB[i] = newPartition; // Copy the content of the parameter
            partitionDB[i].valid = 1; // Mark the entry as used
            resetPartitionTimers(i); // Reset timers for the new partition
            partitionRT[i] = {}; // Initialize runtime data
            partitionSTATS[i] = {}; // Initialize statistics
            return i; // Return the index of the entry
        }
    }
    return ERR_IDX_NOT_FND; // Return -1 if no unused entry is found
}
//
int Alarm::addPgm(const ALARM_PGM& newPgm) {
    for (int i = 0; i < MAX_ALARM_PGM; ++i) {
        if (pgmsDB[i].valid == 0) { // Check if the entry is unused
            pgmsDB[i] = newPgm;    // Copy the content of the parameter
            pgmsDB[i].valid = 1;   // Mark the entry as used
            return i;              // Return the index of the entry
        }
    }
    return ERR_IDX_NOT_FND; // Return -1 if no unused entry is found
}
//
int Alarm::isPartitionValid(int idx) const {
    return (validatePartitionIndex(idx));
}
//
void Alarm::setGlobalOptions(const ALARM_GLOBAL_OPTS_t& globalOptions) {

    // Update the global options
    alarmGlobalOpts = globalOptions;

   // Example: Reset system components based on new options
    if (alarmGlobalOpts.restrOnSprvsLoss) {
        ErrWrite(LOG_ERR_INFO, "Supervision loss restriction enabled.\n");
    }
    if (alarmGlobalOpts.restrOnTamper) {
        ErrWrite(LOG_ERR_INFO, "Tamper restriction enabled.\n");
    }
    if (alarmGlobalOpts.restrOnACfail) {
        ErrWrite(LOG_ERR_INFO, "AC failure restriction enabled.\n");
    }

    // Log the updated global options
    ErrWrite(LOG_ERR_INFO, "Global options updated successfully.\n");
    printAlarmOpts(reinterpret_cast<byte*>(&alarmGlobalOpts));
}
//
bool Alarm::setGlobalOptions(const char* opt_name, const parsedValue* opt_val) {
    // Loop through the gOptsTags array to find the matching option name
    for (size_t i = 0; i < GOPTS_KEYS_CNT; ++i) {
        if (strncmp(opt_name, gOptsValProcessors[i].jsonValStr, NAME_LEN) == 0) {
            // Found the matching option name
            const int offset = gOptsValProcessors[i].patchOffset;
            const int length = gOptsValProcessors[i].patchLen;
            auto patchCallback = gOptsValProcessors[i].patchCallBack;

            // Call the patchCallBack function to set the value
            if (patchCallback(reinterpret_cast<byte*>(&alarmGlobalOpts), offset, length, opt_val)) {
                ErrWrite(LOG_ERR_DEBUG, "Global option '%s' set to '%s' successfully.\n", opt_name, opt_val->s);
                return TRUE;
            }
            else {
                ErrWrite(LOG_ERR_DEBUG, "Failed to set global option '%s' with value '%s'.\n", opt_name, opt_val);
                return FALSE;
            }
            return FALSE; // Exit the function after processing the option
        }
    }

    // If the option name is not found in gOptsTags
    printf("Global option '%s' not found.\n", opt_name);
    return FALSE;           // option not found
}
//
int Alarm::getGlobalOptionsCnt() const {
    return GOPTS_KEYS_CNT;
}
//
const char* Alarm::getGlobalOptionKeyStr(int idx) const {
    if (idx < 0 || idx >= GOPTS_KEYS_CNT)
        return nullptr;
    return gOptsValProcessors[idx].jsonValStr;
}
//
 void Alarm::synchPGMstates() {
     printf("synchPGMstates() - NOT IMPLEMENTED\n"); //(ErrWrite(ERR_WARNING, "synchPGMstates() - NOT IMPLEMENTED\n");
}
// Validation methods
bool Alarm::validateZoneIndex(int zoneIndex) const {
    return (zoneIndex >= 0 && zoneIndex < MAX_ALARM_ZONES);
}

bool Alarm::validatePartitionIndex(int partitionIndex) const {
    return (partitionIndex >= 0 && partitionIndex < MAX_PARTITION);
}

bool Alarm::validatePgmIndex(int pgmIndex) const {
    return (pgmIndex >= 0 && pgmIndex < MAX_ALARM_PGM);
}

// Public accessor methods with bounds checking

//#include <cstring> // For strncmp

int Alarm::getZoneIndex(const char* name) const {
    for (int i = 0; i < MAX_ALARM_ZONES; ++i) {
        if (zonesDB[i].valid == 0) { // Skip invalid entries
            continue;
        }
        if (strncmp(name, zonesDB[i].zoneName, NAME_LEN) == 0) { // Compare names
            return i; // Found the zone, return its index
        }
    }
    return ERR_IDX_NOT_FND; // Zone not found
}

int Alarm::getPartitionIndex(const char* name) const {
    for (int i = 0; i < MAX_PARTITION; ++i) {
        if (partitionDB[i].valid == 0) { // Skip invalid entries
            continue;
        }
        if (strncmp(name, partitionDB[i].partitionName, NAME_LEN) == 0) { // Compare names
            return i; // Found the partition, return its index
        }
    }
    return 	ERR_IDX_NOT_FND        ; // Partition not found
}

int Alarm::getPgmIndex(const char* name) const {
    for (int i = 0; i < MAX_ALARM_PGM; ++i) {
        if (pgmsDB[i].valid == 0) { // Skip invalid entries
            continue;
        }
        if (strncmp(name, pgmsDB[i].pgmName, NAME_LEN) == 0) { // Compare names
            return i; // Found the PGM, return its index
        }
    }
    return ERR_IDX_NOT_FND; // PGM not found
}

int Alarm::getZoneCount() const {
    return MAX_ALARM_ZONES;
}

bool Alarm::isZoneOpen(int zoneIndex) const {
    if (!validateZoneIndex(zoneIndex)) return false;
    return (zonesRT[zoneIndex].zoneStat & ZONE_OPEN) != 0;
}

bool Alarm::isZoneBypassed(int zoneIndex) const {
    if (!validateZoneIndex(zoneIndex)) return false;
    return (zonesRT[zoneIndex].bypassed != 0);
}

bool Alarm::isZoneInAlarm(int zoneIndex) const {
    if (!validateZoneIndex(zoneIndex)) return false;
    return (zonesRT[zoneIndex].in_alarm != NO_ALARM);
}

const char* Alarm::getZoneName(int zoneIndex) const {
    if (!validateZoneIndex(zoneIndex)) return "";
    return zonesDB[zoneIndex].zoneName;
}

int Alarm::getPartitionCount() const {
    return MAX_PARTITION;
}

bool Alarm::isPartitionArmed(int partitionIndex) const {
    if (!validatePartitionIndex(partitionIndex)) return false;
    return (partitionRT[partitionIndex].armStatus != DISARM);
}

int Alarm::getArmStatus(int partitionIndex) const {
    if (!validatePartitionIndex(partitionIndex)) return DISARM;
    return partitionRT[partitionIndex].armStatus;
}

bool Alarm::hasPartitionChanged(int partitionIndex) const {
    if (!validatePartitionIndex(partitionIndex)) return false;
    return (partitionRT[partitionIndex].changed != CHG_NO_CHANGE);
}

const char* Alarm::getPartitionName(int partitionIndex) const {
    if (!validatePartitionIndex(partitionIndex)) return "";
    return partitionDB[partitionIndex].partitionName;
}
bool Alarm::setPartitionTarget(int partitionIndex, ARM_METHODS_t targetArmStatus) {
	if (!validatePartitionIndex(partitionIndex)) return false;
	partitionRT[partitionIndex].targetArmStatus = targetArmStatus;
	partitionRT[partitionIndex].changed |= CHG_NEW_CMD; return true;
}


int Alarm::getPgmCount() const {
    return MAX_ALARM_PGM;
}

int Alarm::getPgmValue(int pgmIndex) const {
    if (!validatePgmIndex(pgmIndex)) return 0;
    return pgmsDB[pgmIndex].cValue;
}

const char* Alarm::getPgmName(int pgmIndex) const {
    if (!validatePgmIndex(pgmIndex)) return "";
    return pgmsDB[pgmIndex].pgmName;
}

bool Alarm::isRestrictionActive(int restrictionType) const {
    // Example implementation - would need to be expanded based on specific restriction types
    switch (restrictionType) {
        case 0: return (alarmGlobalOpts.restrOnSprvsLoss != 0);
        case 1: return (alarmGlobalOpts.restrOnTamper != 0);
        case 2: return (alarmGlobalOpts.restrOnACfail != 0);
        default: return false;
    }
}

void Alarm::updateAlarmState() {
    // This would implement the main alarm loop logic
    // Process all zones, partitions, and timers
    
    // Example pseudocode:
    for (int i = 0; i < MAX_PARTITION; ++i) {
        if (!partitionDB[i].valid) continue;
        
        // Process partition timers
        // Check for zone changes
        // Update partition statistics
        // Handle arm/disarm commands
        
        partitionRT[i].changed = CHG_NO_CHANGE; // Reset changes once processed
    }
}

void Alarm::processTimers() {
    // Process all timer-related tasks
    for (int i = 0; i < MAX_PARTITION; ++i) {
        if (!partitionDB[i].valid) continue;
        
        checkEntryDelayTimers(i);
        checkExitDelayTimer(i);
    }
}

//bool Alarm::armPartition(int partitionIndex, int armMethod) {
//    if (!validatePartitionIndex(partitionIndex) || !partitionDB[partitionIndex].valid) 
//        return false;
//        
//    // Set partition for arming
//    partitionRT[partitionIndex].targetArmStatus = armMethod;
//    partitionRT[partitionIndex].changed |= CHG_NEW_CMD;
//    return true;
//}

bool Alarm::disarmPartition(int partitionIndex) {
    return armPartition(partitionIndex, DISARM);
}

// Example implementations of private helper methods

void Alarm::checkEntryDelayTimers(int partitionIndex) {
    // Check and handle entry delay timers for the partition
    // Implementation would depend on how timers are managed
}

void Alarm::checkExitDelayTimer(int partitionIndex) {
    // Check and handle exit delay timer for the partition
    // Implementation would depend on how timers are managed
}

void Alarm::resetPartitionTimers(int partitionIndex) {
    // Reset all timers for a partition
    // Implementation would depend on how timers are managed
}
//
// debug prints and log
void Alarm::ErrWrite(LogLevel_t level, const char* format, ...) {
    va_list args;
    va_start(args, format);

    // Create a buffer for the formatted message
    char prnBuf[512];
    vsnprintf(prnBuf, sizeof(prnBuf), format, args);

    // Now pass the formatted string to the appropriate output function
    if (debugCallback) {
        debugCallback(level, "%s", prnBuf);  // Formatted message as a string
    }
    else {
        defaultDebugOut(level, "%s", prnBuf);  // Same here
    }

    va_end(args);
}


#ifdef INTERNAL_JSON_HANDLERS
// Define the JSON processor functions (payload handlers)
bool Alarm::processGlobalOptionsJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length) {
    printf("processGlobalOptionsJson() - NOT IMPLEMENTED\n");
    return false;
}
bool Alarm::processPgmJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length) {
    printf("processPgmJson() - NOT IMPLEMENTED\n");
    return false;
}

bool Alarm::processPartitionJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length) {
    printf("processPartitionJson() - NOT IMPLEMENTED\n");
	return false;   
}
bool Alarm::processZoneJsonPayload(Alarm& alarm, const char* jsonPayload, size_t length) {

    printf("processZoneJson() - NOT IMPLEMENTED\n");    
    //printf("Processing zone action: %s for zone index %d\n", value, zoneIndex);

    unsigned int action = 0;
    //if (strcmp(value, "bypass") == 0) action = ZONE_BYPASS_CMD;
    //else if (strcmp(value, "clear_bypass") == 0) action = ZONE_UNBYPASS_CMD;
    //else if (strcmp(value, "tamper") == 0) action = ZONE_TAMPER_CMD;
    //else if (strcmp(value, "close") == 0) action = ZONE_CLOSE_CMD;
    //else if (strcmp(value, "open") == 0) action = ZONE_OPEN_CMD;
    //else if (strcmp(value, "anti-mask") == 0) action = ZONE_AMASK_CMD;
    //else {
    //    printf("Unknown zone action: %s\n", value);
    //    return false;
    //}

    // Call the zone modification function with the appropriate action
    //alarm.modifyZn(&zoneIndex, &action, nullptr);
    return false;// Define the JSON
}
 // Define the JSON topic handlers array as a static member of the Alarm class
const Alarm::JsonTopicHandler Alarm::mqttTopicHandlers[] = {
    {MQTT_ZONES_CONTROL_TOPIC,      JSON_SECTION_ZONES,         &Alarm::processZoneJsonPayload,
     "Control zones (bypass, tamper, etc.)"},
    
    {MQTT_PARTITIONS_CONTROL_TOPIC, JSON_SECTION_PARTITIONS,    &Alarm::processPartitionJsonPayload,
     "Control partitions (arm, disarm, etc.)"},
    
    {MQTT_OUTPUTS_CONTROL_TOPIC,    JSON_SECTION_PGMS,          &Alarm::processPgmJsonPayload,
     "Control PGMs (on, off, pulse)"},
    
    {MQTT_GLOBAL_OPT_CONTROL_TOPIC, JSON_SECTION_GLOBAL_OPTIONS, &Alarm::processGlobalOptionsJsonPayload,
     "Set global alarm options"}
};

const int Alarm::MQTT_TOPIC_HANDLER_COUNT = sizeof(Alarm::mqttTopicHandlers) / sizeof(Alarm::mqttTopicHandlers[0]);

// Process a JSON message by finding the matching topic handler
bool Alarm::processMqttMessage(const char* topic, const char* payload, size_t length) {
    // Find the handler for this topic
    for (int i = 0; i < MQTT_TOPIC_HANDLER_COUNT; i++) {
        if (strcmp(topic, mqttTopicHandlers[i].topic) == 0) {
            // Call the JSON processor for this topic
            if (mqttTopicHandlers[i].processor) {
                return mqttTopicHandlers[i].processor(*this, payload, length);
            } else {
                ErrWrite(LOG_ERR_WARNING, "No processor defined for topic: %s\n", topic);
                return false;
            }
        }
    }
    
    ErrWrite(LOG_ERR_WARNING, "No handler found for topic: %s\n", topic);
    return false;
}

#endif //INTERNAL_JSON_HANDLERS

#endif // ALARM_H

