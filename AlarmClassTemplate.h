#ifndef ALARM_H
#define ALARM_H

#include <functional>
#include "alarm-defs.h"

class Alarm {
public:
    // Constructor and destructor
    Alarm();
    ~Alarm();

    // Public methods to interact with alarm system
    // These methods don't provide direct access to underlying arrays
    
    // Zone-related methods
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
    
    // Command methods
    bool armPartition(int partitionIndex, int armMethod);
    bool disarmPartition(int partitionIndex);
    bool bypassZone(int zoneIndex);
    bool clearBypassZone(int zoneIndex);
    bool setPgm(int pgmIndex, int value);

private:
    // Private static arrays for alarm data
    ALARM_ZONE zonesDB[MAX_ALARM_ZONES];
    ALARM_ZONE_RT zonesRT[MAX_ALARM_ZONES];
    ALARM_PGM pgmsDB[MAX_ALARM_PGM];
    ALARM_GLOBAL_OPTS_t alarmGlobalOpts;
    ALARM_PARTITION_t partitionDB[MAX_PARTITION];
    ALARM_PARTITION_RUN_TIME_t partitionRT[MAX_PARTITION];
    ALARM_PARTITION_STATS_t partitionSTATS[MAX_PARTITION];
    
    // Private helper methods
    void initializeZones();
    void initializePgms();
    void initializePartitions();
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
};

//-----------------------------------------------------------------------------------
// Implementation of selected methods
//-----------------------------------------------------------------------------------

// Constructor
Alarm::Alarm() {
    initializeZones();
    initializePgms();
    initializePartitions();
    alarmGlobalOpts = {};
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
    }
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

bool Alarm::armPartition(int partitionIndex, int armMethod) {
    if (!validatePartitionIndex(partitionIndex) || !partitionDB[partitionIndex].valid) 
        return false;
        
    // Set partition for arming
    partitionRT[partitionIndex].targetArmStatus = armMethod;
    partitionRT[partitionIndex].changed |= CHG_NEW_CMD;
    return true;
}

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

