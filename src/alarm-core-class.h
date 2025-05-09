#ifndef ALARM_H
#define ALARM_H

#include <functional>

typedef unsigned char byte;
#define lprintf			printf
#define ErrWrite (debugCallback ? (debugCallback) : defaultDebugOut)

#include "alarm-core-defs.h"
#include "alarm-core-timers.h"

//typedef void (*DebugCallbackFunc)(const char* message, size_t length);
typedef int (*DebugCallbackFunc)(int level, const char* format, ...);

template <typename T>
class FilteredArrayIterator {
public:
    // Constructor
    FilteredArrayIterator(T* ptr, T* end)
        : current(ptr), end(end) {
        // Skip invalid elements at the start
        advanceToValid();
    }

    // Dereference operator
    T& operator*() const {
        return *current;
    }

    // Arrow operator
    T* operator->() const {
        return current;
    }

    // Prefix increment
    FilteredArrayIterator& operator++() {
        ++current;
        advanceToValid();
        return *this;
    }

    // Postfix increment
    FilteredArrayIterator operator++(int) {
        FilteredArrayIterator temp = *this;
        ++(*this);
        return temp;
    }

    // Equality comparison
    bool operator==(const FilteredArrayIterator& other) const {
        return current == other.current;
    }

    // Inequality comparison
    bool operator!=(const FilteredArrayIterator& other) const {
        return current != other.current;
    }

private:
    T* current; // Pointer to the current element
    T* end;     // Pointer to one past the last element

    // Advance to the next valid element
    void advanceToValid() {
        while (current != end && !isValid(*current)) {
            ++current;
        }
    }

    // Internal validity check
    static bool isValid(const T& element) {
        return element.valid != 0; // Example: Check if the 'valid' field is non-zero
    }
};



class Alarm {
public:
    // Constructor and destructor
    Alarm();
    ~Alarm();

    // Public methods to interact with alarm system
    // These methods don't provide direct access to underlying arrays
    //void alarm_loop(void);
    void setDebugCallback(DebugCallbackFunc callback);
    // Zone-related methods
    int addZone(const ALARM_ZONE& newZone);
    int getZoneCount() const;
    bool isZoneOpen(int zoneIndex) const;
    bool isZoneBypassed(int zoneIndex) const;
    bool isZoneInAlarm(int zoneIndex) const;
    const char* getZoneName(int zoneIndex) const;
    void modifyZn(void* param1, void* param2, void* param3);
    
    // Partition-related methods
    int addPartition(const ALARM_PARTITION_t& newPartition);
    int getPartitionCount() const;
    bool isPartitionArmed(int partitionIndex) const;
    int getArmStatus(int partitionIndex) const;
    bool hasPartitionChanged(int partitionIndex) const;
    const char* getPartitionName(int partitionIndex) const;
	int isPartitionValid(const int idx) const;
	bool setPartitionTarget(int partitionIndex, ARM_METHODS_t targetArmStatus);
    int armPartition(byte prt, ARM_METHODS_t action);
    
    // PGM-related methods
	int addPgm(const ALARM_PGM& newPgm); 
    int getPgmCount() const;
    int getPgmValue(int pgmIndex) const;
    const char* getPgmName(int pgmIndex) const;
	void modifyPgm(void* param1, void* param2, void* param3);


    int zoneIndex(const char* name) const;
    int partitionIndex(const char* name) const;
    int pgmIndex(const char* name) const;

    // Global options methods
    void setGlobalOptions(const ALARM_GLOBAL_OPTS_t& globalOptions);
    bool isRestrictionActive(int restrictionType) const;
    
    // System methods
    void updateAlarmState();
    void processTimers();
    // 
    // printing methods - defined in alarmClassHelpers.h
	//
    static int defaultDebugOut(int err_code, const char* what, ...);
    static void printConfigData(struct tagAccess targetKeys[], int numEntries, byte* targetPtr, int printClass);
    //const char* zoneState2Str(struct zoneStates_t states[], int statesCnt, int action);
    void    printConfigHeader(struct tagAccess targetKeys[], int numEntries);
    void    printAlarmPartCfg(void);
    void    printAlarmZones(int startZn, int endZn);
    void    printAlarmOpts(byte* optsPtr);
    void    printAlarmPgms(void);
    void    printAlarmKeysw(byte* keyswArrPtr, int maxKeysw);
    void    printAlarmPartitionRT(int idx);
    void    printPartHeaderRT(void);
    void    printAlarmPartRT(void);
    void    reportZonesNamesBasedOnStatus(int prt, int stat);
    void    reportZonesNamesBasedOnFlag(int prt, int offset, byte bitmask);
    void    reportPartitionNamesBasedOnFlag(int offset);
    void    printZonesSummary(int prt);
    void    printParttionsSummary(void);
    const char* titleByAction(struct zoneStates_t Cmds[], int CmdsCnt, int stateCode);

    // Command methods
    //bool armPartition(int partitionIndex, int armMethod);
    bool disarmPartition(int partitionIndex);
    bool bypassZone(int zoneIndex);
    bool clearBypassZone(int zoneIndex);
    bool setPgm(int pgmIndex, int value);
	// debug printing callback function, shall be set from Alarm clas client. Defaults to defaultDebugOut
    DebugCallbackFunc debugCallback;

    // Iterators
public:
    auto beginValidZones() {
        return FilteredArrayIterator<ALARM_ZONE>(zonesDB, zonesDB + MAX_ALARM_ZONES);
    }
    auto endValidZones() {
        return FilteredArrayIterator<ALARM_ZONE>(zonesDB + MAX_ALARM_ZONES, zonesDB + MAX_ALARM_ZONES);
    }

    // Iterators for valid partitions
    auto beginValidPartitions() {
        return FilteredArrayIterator<ALARM_PARTITION_t>(partitionDB, partitionDB + MAX_PARTITION);
    }
    auto endValidPartitions() {
        return FilteredArrayIterator<ALARM_PARTITION_t>(partitionDB + MAX_PARTITION, partitionDB + MAX_PARTITION);
    }

    // Iterators for valid PGMs
    auto beginValidPgms() {
        return FilteredArrayIterator<ALARM_PGM>(pgmsDB, pgmsDB + MAX_ALARM_PGM);
    }
    auto endValidPgms() {
        return FilteredArrayIterator<ALARM_PGM>(pgmsDB + MAX_ALARM_PGM, pgmsDB + MAX_ALARM_PGM);
    }

private:
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
    // Global callback pointer (initialized to NULL)
    // Timer management
    void resetAllPartitionTimers(int partitionIndex);
    void initRTdata(void);
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

    // Command handling

    // Arm management
    int checkArmRestrctions(byte partIdx, int action);


    // Alarm processing
    void processAlarm(int alarm);
    void processLineErrors(int zn, int opts);
    void processTampers(int zn);
    void processAmasks(int zn);
    int processEntryDelayZones(int zn);
    int processOpenZone(int zn);
    void alarm_loop();


    // Private helper methods
    void initializeZones();
    void initializePgms();
    void initializePartitions();
    //void initRTdata(void);
	//void resetAllPartitionTimers(int partitionIndex);
    void synchPGMstates(void);
    bool validateZoneIndex(int zoneIndex) const;
    bool validatePartitionIndex(int partitionIndex) const;
    bool validatePgmIndex(int pgmIndex) const;
    
    // Core alarm functionality (to be implemented)
    void processZoneChange(int zoneIndex);
    void processPgmChange(int pgmIndex);
    void processPartitionChange(int partitionIndex);
    void updatePartitionStats(int partitionIndex);
    void clearPartitionBypass(int partitionIndex);
    
    // Timer-related methods
    void checkEntryDelayTimers(int partitionIndex);
    void checkExitDelayTimer(int partitionIndex);
    void resetPartitionTimers(int partitionIndex);
};  // end of class Alarm

//
#include "alarm-core-logic.h"
#include "alarm-core-parser-helpers.h"
#include "alarm-core-helpers.h"


int Alarm::defaultDebugOut(int err_code, const char* what, ...)           // callback to dump info to serial console from inside RS485 library
{
    char prnBuf[512];
    va_list args;
    va_start(args, what);
    int index = 0;
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
        if (what)
            printf(prnBuf);
        break;
    }
    va_end(args);
    return err_code;
}


// Function to set the debug callback
void Alarm::setDebugCallback(DebugCallbackFunc callback) {
    debugCallback = callback;
	//(*debugCallback)(ERR_WARNING, "Debug callback set\n");
    ErrWrite(ERR_WARNING, "Debug callback set\n");
}
// 
// Constructor
Alarm::Alarm() {
    //ErrWrite(ERR_WARNING, "Init alarm from file\n");
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
    return -1; // Return -1 if no unused entry is found
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
    return -1; // Return -1 if no unused entry is found
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
    return -1; // Return -1 if no unused entry is found
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
        ErrWrite(ERR_INFO, "Supervision loss restriction enabled.\n");
    }
    if (alarmGlobalOpts.restrOnTamper) {
        ErrWrite(ERR_INFO, "Tamper restriction enabled.\n");
    }
    if (alarmGlobalOpts.restrOnACfail) {
        ErrWrite(ERR_INFO, "AC failure restriction enabled.\n");
    }

    // Log the updated global options
    ErrWrite(ERR_INFO, "Global options updated successfully.\n");
    printAlarmOpts(reinterpret_cast<byte*>(&alarmGlobalOpts));
}

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

int Alarm::zoneIndex(const char* name) const {
    for (int i = 0; i < MAX_ALARM_ZONES; ++i) {
        if (zonesDB[i].valid == 0) { // Skip invalid entries
            continue;
        }
        if (strncmp(name, zonesDB[i].zoneName, NAME_LEN) == 0) { // Compare names
            return i; // Found the zone, return its index
        }
    }
    return -1; // Zone not found
}

int Alarm::partitionIndex(const char* name) const {
    for (int i = 0; i < MAX_PARTITION; ++i) {
        if (partitionDB[i].valid == 0) { // Skip invalid entries
            continue;
        }
        if (strncmp(name, partitionDB[i].partitionName, NAME_LEN) == 0) { // Compare names
            return i; // Found the partition, return its index
        }
    }
    return -1; // Partition not found
}

int Alarm::pgmIndex(const char* name) const {
    for (int i = 0; i < MAX_ALARM_PGM; ++i) {
        if (pgmsDB[i].valid == 0) { // Skip invalid entries
            continue;
        }
        if (strncmp(name, pgmsDB[i].pgmName, NAME_LEN) == 0) { // Compare names
            return i; // Found the PGM, return its index
        }
    }
    return -1; // PGM not found
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

#endif // ALARM_H

