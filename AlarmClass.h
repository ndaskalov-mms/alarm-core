#ifndef ALARM_H
#define ALARM_H

#include <functional>
extern int ErrWrite(int err_code, const char* what, ...);

typedef unsigned char byte;
#define lprintf			printf

#include "alarmClass-defs.h"
#include "timers.h"



class Alarm {
public:
    // Constructor and destructor
    Alarm();
    ~Alarm();

    // Public methods to interact with alarm system
    // These methods don't provide direct access to underlying arrays
    //void alarm_loop(void);
    // Zone-related methods
    int addZone(const ALARM_ZONE& newZone);
    int getZoneCount() const;
    bool isZoneOpen(int zoneIndex) const;
    bool isZoneBypassed(int zoneIndex) const;
    bool isZoneInAlarm(int zoneIndex) const;
    const char* getZoneName(int zoneIndex) const;
    
    // Partition-related methods
    int getPartitionCount() const;
    bool isPartitionArmed(int partitionIndex) const;
    int getArmStatus(int partitionIndex) const;
    bool hasPartitionChanged(int partitionIndex) const;
    const char* getPartitionName(int partitionIndex) const;
    
    // PGM-related methods
    int getPgmCount() const;
    int getPgmValue(int pgmIndex) const;
    const char* getPgmName(int pgmIndex) const;
    
    // Global options methods
    bool isRestrictionActive(int restrictionType) const;
    
    // System methods
    void updateAlarmState();
    void processTimers();

 //   // defined in parserClassHelpers.h
 //   int     pokeString(byte* basePtr, int offset, int len, const char* token);
 //   byte*   peekString(byte* basePtr, int offset, int len);
 //   int     pokeBool(byte* basePtr, int offset, int len, const char* token);
 //   byte*   peekBool(byte* basePtr, int offset, int len);
 //   int     pokeByte(byte* basePtr, int offset, int len, const char* token);
 //   byte*   peekByte(byte* basePtr, int offset, int len);
 //   byte*   peekPrtnNo(byte* basePtr, int offset, int len);
 //   int     pokePrtNo(byte* basePtr, int offset, int len, const char* token);
 //   byte*   peekLineErr(byte* basePtr, int offset, int len);
 //   int     pokeLineErr(byte* basePtr, int offset, int len, const char* token);
 //   byte*   peekZoneType(byte* basePtr, int offset, int len);
 //   int     pokeZoneType(byte* basePtr, int offset, int len, const char* token);
 //   int     pokeAlarmType(byte* basePtr, int offset, int len, const char* token);
	//byte*   peekAlarmType(byte* basePtr, int offset, int len);
    
    // defined in alarmClassHelpers.h
    //const char* zoneState2Str(struct zoneStates_t states[], int statesCnt, int action);
    void    printConfigData(struct tagAccess targetKeys[], int numEntries, byte* targetPtr, int printClass);
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


private:
    // Private static arrays for alarm data
    // zoneDB - database with all zones CONFIG info. 
    // zonesRT = all run time zones data. Info from slaves are fetched via pul command over RS485
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
   
    // Private helper methods
#include "alarmClass_logic.h"

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
#include "parserClassHelpers.h"
#include "alarmClassHelpers.h"
// 
// Constructor
Alarm::Alarm() {
    ErrWrite(ERR_WARNING, "Init alarm from file\n");
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
 void Alarm::synchPGMstates() {
     ErrWrite(ERR_WARNING, "synchPGMstates() - NOT IMPLEMENTED\n");
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

#endif // ALARM_H

