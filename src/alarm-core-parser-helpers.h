// file: parserClassHelpers.h
#pragma once
#define _CRT_SECURE_NO_WARNINGS
extern char token[256];

//#define CSV_COMMENT_CHAR '#'

//
#ifdef ARDUINO
#define _itoa itoa
#define _stricmp strcmp  // TODO - not case sensitive
#endif

/**
 * @brief   Describes how to access specific parameter for zone, partition, pgm, etc
*/
struct tagAccess {
    const   char keyStr[NAME_LEN];                                      // param name e.g. zone name, partition name zone type, etc
    int     pos;                                                        // param position in header line (column) in CSV file
    byte    patchOffset;                                                // offset in corresponding DB (zoneDB, partionDB, etc) where we have to put the data read from CSV
    byte    patchLen;                                                   // offset is relativ to beginning of the struct which forms correspondin entr
    int     (*patchCallBack)  (byte* basePtr, int  offset, int len, const char* token);   // callback function to patch the value read from condig file
    byte*   (*unpatchCallBack)(byte* basePtr, int  offset, int len);                      // callback function to get string representation of the value read from config struct
    size_t  keyStrLen;                                                  // used for pretty printing only
    byte    printClass;                                                 // used to select what to print 
};
//
// ??????Str2val arrays contain list of all possible values which can be present in the config file cells different digit or true/false bool
//  the token read from parser when parsing the config file is string compared to match and corresponding integer value is retrieved
//
struct str2val {
    char    keyStr[NAME_LEN];
    int     val;
};
/**
 * @brief   Arrays containing key:value list of all possible content of the zone DATA lines which can be present in the config file cells different digit or true/false bool
*/
static struct str2val zoneTypeStr2val[] = {
{ZONE_DISABLED_TTL			, ZONE_DISABLED},
{INSTANT_TTL				, INSTANT},
{ENTRY_DELAY1_TTL			, ENTRY_DELAY1},
{ENTRY_DELAY2_TTL			, ENTRY_DELAY2},
{FOLLOW_TTL					, FOLLOW},
{STAY_DELAY1_TTL			, STAY_DELAY1},
{STAY_DELAY2_ANTI_MASK_TTL	, STAY_DELAY2_ANTI_MASK},
{H24_BUZZER_TTL				, H24_BUZZER},
{H24_BURGLAR_TTL			, H24_BURGLAR},
{H24_HOLDUP_TTL				, H24_HOLDUP},
{H24_GAS_TTL				, H24_GAS},
{H24_HEAT_TTL				, H24_HEAT},
{H24_WATER_TTL				, H24_WATER},
{H24_FREEZE_TTL				, H24_FREEZE},
{H24_FIRE_DELAYED_TTL		, H24_FIRE_DELAYED},
{H24_FIRE_STANDARD_TTL		, H24_FIRE_STANDARD} };
//
#define ZONE_TYPES_CNT (sizeof(zoneTypeStr2val)/sizeof(struct str2val))
//
static struct str2val alarmTypeStr2Val[] = { 
{STEADY_ALARM_TTL	        , STEADY_ALARM},
{SILENT_ALARM_TTL	        , SILENT_ALARM},
{PULSED_ALARM_TTL	        , PULSED_ALARM},
{REPORT_ALARM_TTL	        , REPORT_ALARM} };
//
#define ALARMS_TITLE_CNT (sizeof(alarmTypeStr2Val)/sizeof(struct str2val))
//
static struct str2val lineErrStr2Val[] = { 
{LINE_ERR_OPT_DISABLED_TTL			   , LINE_ERR_OPT_DISABLED },
{LINE_ERR_OPT_TROUBLE_ONLY_TTL		   , LINE_ERR_OPT_TROUBLE_ONLY},
{LINE_ERR_OPT_ALARM_WHEN_ARMED_TTL	   , LINE_ERR_OPT_ALARM_WHEN_ARMED},
{LINE_ERR_OPT_ALARM_TTL				   , LINE_ERR_OPT_ALARM} };
//
#define ERRORS_TITLE_CNT (sizeof(lineErrStr2Val)/sizeof(struct str2val))
//
// Family of peek/poke functions to store values read from config file to provided offset
// the offset points into corresponding struct field of the corresponding database (pgmDB[board][pgm], zonesDB[board][zone]...
// 

/**
 * @brief           Function to write string to memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Len of bytes to be written
 * @param token     String to be written
 * @return          Always true
*/
static int pokeString(byte* basePtr, int offset, int len, const char* token) {
    //strncpy((&((char*)basePtr)[offset]), token, len);
    snprintf((char*)basePtr + offset, len, "%s", token);
    return true;
}

/**
 * @brief           Function to calc pointer used to read string from memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @return          pointer to string
*/
static byte* peekString(byte* basePtr, int offset, int len) {
    byte* ret;
    ret = &basePtr[offset];
    return ret;
}

/**
 * @brief           Function to write boolean val to memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @param token     shall be "true" or "false"
 * @return          Always true
*/
static int pokeBool(byte* basePtr, int offset, int len, const char* token) {
    (basePtr[offset] = !_stricmp(token, "true"));
    return true;
}

/**
 * @brief           Function to read bool value from memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @return          pointer to "true" or "false" string
*/
static byte* peekBool(byte* basePtr, int offset, int len) {
    return (byte *)(basePtr[offset]? TRUE_t : FALSE_t);
}

/**
 * @brief           Function to write byte val to memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @param token     ASCII string to be converted to unsigned byte before write
 * @return          Always true
*/
static int pokeByte(byte* basePtr, int offset, int len, const char* token) {
    unsigned res  = (unsigned int)atoi(token);
    if (res > 255) {
        // TODO (ERR_WARNING, "Byte value larger than 255. Truncated");
        res = 255;
    }
    basePtr[offset] = res;
    return true;
}

/**
 * @brief           Function to read byte value from memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @return          Pointer to byte value converted to ASCII string in global var token[512]
*/
static byte* peekByte(byte* basePtr, int offset, int len) {
    snprintf(token, sizeof(token), "%d", basePtr[offset]);
    //_itoa(basePtr[offset], token, 10);
    return ((byte*)&token);
}
//
#define patchFollow     pokeByte
#define unPatchFollow   peekByte

/**
 * @brief           Function to read partition number from memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @return          Pointer to partition number converted to ASCII string in global var token[512]
*/
static byte* peekPrtnNo(byte* basePtr, int offset, int len) {
    //_itoa(basePtr[offset]+1, token, 10);                            // input partitions range starts at 1, but internally starts from 0
    snprintf(token, sizeof(token), "%d", basePtr[offset] + 1);
    return ((byte*)&token);
}

/**
 * @brief           Function to write partition number val to memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @param token     ASCII string to be converted to partition number before write
 * @return          Always true
*/
static int pokePrtNo(byte* basePtr, int offset, int len, const char* token) {
    int tmp;
    tmp = (unsigned int)atoi(token);
    tmp--;                                                          // input partitions range starts at 1, but internally starts from 0
    if (tmp < 0 || tmp >= MAX_PARTITION) {
        LOG_CRITICAL("Follows: Partition number %d out of range\n", tmp);
        return false;
    }
    basePtr[offset] = tmp;
    return true;
}

/**
 * @brief           Function to read LINE ERROR from memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @return          Pointer to LINE ERROR converted to ASCII string in global var token[512]
*/
static byte* peekLineErr(byte* basePtr, int offset, int len) {
    int i; char* ret = NULL;
    for (i = 0; i < ERRORS_TITLE_CNT; i++) {
        if (basePtr[offset] == lineErrStr2Val[i].val) {
            ret = (lineErrStr2Val[i].keyStr);
            //lprintf("Line error read to %s\n", ret);
            break;
        }
    }
    if (i == ERRORS_TITLE_CNT) {
        lprintf(" Invalid Line error type: %s\n", token);
        return NULL;
    }
    return (byte*) ret;
}

/**
 * @brief           Function to read ZONE TYPE from memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @return          Pointer to ZONE TYPE converted to ASCII string in global var token[512]
*/
static byte* peekZoneType(byte* basePtr, int offset, int len) {
    int i; char* ret = NULL;
    for (i = 0; i < ZONE_TYPES_CNT; i++) {
        if (basePtr[offset] == zoneTypeStr2val[i].val) {
            ret = (zoneTypeStr2val[i].keyStr);
            //lprintf("Zone type read to %s\n", ret);
            break;
        }
    }
    if (i == ZONE_TYPES_CNT) {
        lprintf(" Invalid zone type: %s\n", token);
        return NULL;
    }
    return (byte*)ret;
}
//
// TODO - combine all patch LineError, ZoneType and AlarmType in one function
// 

/**
 * @brief           Function to write LINE ERROR val to memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @param token     ASCII string to be converted to partition number before write
 * @return          Always true
*/
static int pokeLineErr(byte* basePtr, int offset, int len, const char* token) {
    int i;
    for (i = 0; i < ERRORS_TITLE_CNT; i++) {
        if (!_stricmp(token, lineErrStr2Val[i].keyStr)) {
            basePtr[offset] = lineErrStr2Val[i].val;
            //lprintf("Line err set to %s\n", errKeys[i].title);
            break;
        }
    }
    if (i == ERRORS_TITLE_CNT) {
        lprintf(" Invalid Line error opts: %s\n", token);
        return false;
    }
    return true;
}

/**
 * @brief           Function to write ZONE TYPE val to memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @param token     ASCII string to be converted to partition number before write
 * @return          Always true
*/
static int pokeZoneType(byte* basePtr, int offset, int len, const char* token) {
    int i;
    for (i = 0; i < ZONE_TYPES_CNT; i++) {
        if (!_stricmp(token, zoneTypeStr2val[i].keyStr)) {
            basePtr[offset] = zoneTypeStr2val[i].val;
            //lprintf("Zone type set to %s\n", zoneCfgKeys[i].title);
            break;
        }
    }
    if (i == ZONE_TYPES_CNT) {
        lprintf(" Invalid Zone type: %s\n", token);
        return false;
    }
    return true;
}

/**
 * @brief           Function to write ALARM TYPE val to memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @param token     ASCII string to be converted to partition number before write
 * @return          Always true
*/
static int pokeAlarmType(byte* basePtr, int offset, int len, const char* token) {
    int i;
    for (i = 0; i < ALARMS_TITLE_CNT; i++) {
        if (!_stricmp(token, alarmTypeStr2Val[i].keyStr)) {
            basePtr[offset] = alarmTypeStr2Val[i].val;
            //lprintf("Alarm type set to %s\n", alarmTitles[i].title);
            break;
        }
    }
    if (i == ALARMS_TITLE_CNT) {
        lprintf(" Invalid Alarm type: %s\n", token);
        return false;
    }
    return true;
}
/**
 * @brief           Function to read ALARM TYPE from memory
 * @param basePtr   Memory start
 * @param offset    Memory offset
 * @param len       Not used
 * @return          Pointer to ALARM TYPE converted to ASCII string in global var token[512]
*/
static byte* peekAlarmType(byte* basePtr, int offset, int len) {
    int i; char* ret = NULL;
    for (i = 0; i < ALARMS_TITLE_CNT; i++) {
        if (basePtr[offset] == alarmTypeStr2Val[i].val) {
            ret = (alarmTypeStr2Val[i].keyStr);
            //lprintf("Alarm type read to %s\n", ret);
            break;
        }
    }
    if (i == ALARMS_TITLE_CNT) {
        lprintf(" Invalid Alarm type: %s\n", token);
        return NULL;
    }
    return (byte*)ret;
}

/**
 * @brief Zone tags.  Contains tag as a string and corresponding offset from beginnig of struct ALARM_ZONE. Used to modify tag values direct in memory.
*/
struct tagAccess zoneTags[] = {
{ZN_NAME_TTL,           -1, offsetof(struct ALARM_ZONE, zoneName),         sizeof(((struct ALARM_ZONE*)0)->zoneName),        &pokeString   , &peekString    , 16                       , PRTCLASS_GENERAL},
{ZN_ID_TTL,             -1, offsetof(struct ALARM_ZONE, zoneID),           sizeof(((struct ALARM_ZONE*)0)->zoneID),          &pokeByte     , &peekByte      , strlen(ZN_ID_TTL)        , PRTCLASS_GENERAL},
{ZN_TYPE_TTL,           -1, offsetof(struct ALARM_ZONE, zoneType),         sizeof(((struct ALARM_ZONE*)0)->zoneType),        &pokeZoneType , &peekZoneType  , 12                       , PRTCLASS_GENERAL},
{ZN_PRT_TTL,            -1, offsetof(struct ALARM_ZONE, zonePartition),    sizeof(((struct ALARM_ZONE*)0)->zonePartition),   &pokePrtNo    , &peekPrtnNo    , strlen(ZN_PRT_TTL)       , PRTCLASS_GENERAL},
{ZN_ALARM_TTL,          -1, offsetof(struct ALARM_ZONE, zoneAlarmType),    sizeof(((struct ALARM_ZONE*)0)->zoneAlarmType),   &pokeAlarmType, &peekAlarmType , 8                        , PRTCLASS_GENERAL},
{ZN_SHD_EN_TTL,         -1, offsetof(struct ALARM_ZONE, zoneShdwnEn),      sizeof(((struct ALARM_ZONE*)0)->zoneShdwnEn),     &pokeBool     , &peekBool      , strlen(ZN_SHD_EN_TTL)    , PRTCLASS_GENERAL},
{ZN_BPS_EN_TTL,         -1, offsetof(struct ALARM_ZONE, zoneBypassEn),     sizeof(((struct ALARM_ZONE*)0)->zoneBypassEn),    &pokeBool     , &peekBool      , strlen(ZN_BPS_EN_TTL)    , PRTCLASS_GENERAL},
{ZN_STAY_TTL,           -1, offsetof(struct ALARM_ZONE, zoneStayZone),     sizeof(((struct ALARM_ZONE*)0)->zoneStayZone),    &pokeBool     , &peekBool      , strlen(ZN_STAY_TTL)      , PRTCLASS_GENERAL},
{ZN_FORCE_TTL,          -1, offsetof(struct ALARM_ZONE, zoneForceEn),      sizeof(((struct ALARM_ZONE*)0)->zoneForceEn),     &pokeBool     , &peekBool      , strlen(ZN_FORCE_TTL)     , PRTCLASS_GENERAL},
{ZN_INTEL_TTL,          -1, offsetof(struct ALARM_ZONE, zoneIntelizone),   sizeof(((struct ALARM_ZONE*)0)->zoneIntelizone),  &pokeBool     , &peekBool      , strlen(ZN_INTEL_TTL)     , PRTCLASS_GENERAL},
{ZN_DLYTRM_TTL,         -1, offsetof(struct ALARM_ZONE, delayTrm),         sizeof(((struct ALARM_ZONE*)0)->delayTrm),        &pokeBool     , &peekBool      , strlen(ZN_DLYTRM_TTL)    , PRTCLASS_GENERAL},
{ZN_TMPR_GLB_TTL,       -1, offsetof(struct ALARM_ZONE, zoneTamperFpanel), sizeof(((struct ALARM_ZONE*)0)->zoneTamperFpanel),&pokeBool     , &peekBool      , strlen(ZN_TMPR_GLB_TTL)  , PRTCLASS_LINE_ERR},
{ZN_TMPR_OPT_TTL,       -1, offsetof(struct ALARM_ZONE, zoneTamperOpts),   sizeof(((struct ALARM_ZONE*)0)->zoneTamperOpts),  &pokeLineErr  , &peekLineErr   , strlen(ZN_TMPR_OPT_TTL)  , PRTCLASS_LINE_ERR},
{ZN_AMSK_GLB_TTL,       -1, offsetof(struct ALARM_ZONE, zoneAmaskFpanel),  sizeof(((struct ALARM_ZONE*)0)->zoneAmaskFpanel), &pokeBool     , &peekBool      , strlen(ZN_AMSK_GLB_TTL)  , PRTCLASS_LINE_ERR},
{ZN_AMSK_OPT_TTL,       -1, offsetof(struct ALARM_ZONE, zoneAmaskOpts),    sizeof(((struct ALARM_ZONE*)0)->zoneAmaskOpts),   &pokeLineErr  , &peekLineErr  , strlen(ZN_AMSK_OPT_TTL)   , PRTCLASS_LINE_ERR},
}; 
#define ZONE_TAGS_CNT (sizeof(zoneTags)/sizeof(struct tagAccess))

/**
 * @brief Partition tags.  Contains tag as a string and corresponding offset from beginnig of struct ALARM_PARTITION_t. Used to modify tag values direct in memory.
*/
struct tagAccess partitionTags[] = {
{PT_NAME_TTL			,-1, offsetof(struct ALARM_PARTITION_t, partitionName),     sizeof(((struct ALARM_PARTITION_t*)0)->partitionName),      &pokeString, &peekString  , 16                           , PRTCLASS_GENERAL },
{PT_IDX					,-1, offsetof(struct ALARM_PARTITION_t, partIdx),           sizeof(((struct ALARM_PARTITION_t*)0)->partIdx),            &pokePrtNo,  &peekPrtnNo  , strlen(PT_IDX)               , PRTCLASS_GENERAL },
{PT_FORCE_ON_REG		,-1, offsetof(struct ALARM_PARTITION_t, forceOnRegularArm), sizeof(((struct ALARM_PARTITION_t*)0)->forceOnRegularArm),  &pokeBool,   &peekBool    , strlen(PT_FORCE_ON_REG)      , PRTCLASS_GENERAL },
{PT_FORCE_ON_STAY		,-1, offsetof(struct ALARM_PARTITION_t, forceOnStayArm),    sizeof(((struct ALARM_PARTITION_t*)0)->forceOnStayArm),     &pokeBool,   &peekBool    , strlen(PT_FORCE_ON_STAY)     , PRTCLASS_GENERAL },
{PT_ED2_FOLLOW			,-1, offsetof(struct ALARM_PARTITION_t, followEntryDelay2), sizeof(((struct ALARM_PARTITION_t*)0)->followEntryDelay2),  &pokeBool,   &peekBool    , strlen(PT_ED2_FOLLOW)        , PRTCLASS_GENERAL },
{PT_ALRM_OUT_EN			,-1, offsetof(struct ALARM_PARTITION_t, alarmOutputEn),     sizeof(((struct ALARM_PARTITION_t*)0)->alarmOutputEn),      &pokeBool,   &peekBool    , strlen(PT_ALRM_OUT_EN)       , PRTCLASS_GENERAL },
{PT_ALRM_LENGHT			,-1, offsetof(struct ALARM_PARTITION_t, alarmCutOffTime),   sizeof(((struct ALARM_PARTITION_t*)0)->alarmCutOffTime),    &pokeByte,   &peekByte    , strlen(PT_ALRM_LENGHT)       , PRTCLASS_GENERAL },
{PT_NO_CUT_ON_FIRE		,-1, offsetof(struct ALARM_PARTITION_t, noCutOffOnFire),    sizeof(((struct ALARM_PARTITION_t*)0)->noCutOffOnFire),     &pokeBool,   &peekBool    , strlen(PT_NO_CUT_ON_FIRE)    , PRTCLASS_GENERAL },
{PT_ALARM_RECYCLE_TIME	,-1, offsetof(struct ALARM_PARTITION_t, alarmRecycleTime),  sizeof(((struct ALARM_PARTITION_t*)0)->alarmRecycleTime),   &pokeByte,   &peekByte    , strlen(PT_ALARM_RECYCLE_TIME), PRTCLASS_GENERAL },
{PT_ED1_INTERVAL		,-1, offsetof(struct ALARM_PARTITION_t, entryDelay1Intvl),  sizeof(((struct ALARM_PARTITION_t*)0)->entryDelay1Intvl),   &pokeByte,   &peekByte    , strlen(PT_ED1_INTERVAL)      , PRTCLASS_GENERAL },
{PT_ED2_INTERVAL		,-1, offsetof(struct ALARM_PARTITION_t, entryDelay2Intvl),  sizeof(((struct ALARM_PARTITION_t*)0)->entryDelay2Intvl),   &pokeByte,   &peekByte    , strlen(PT_ED2_INTERVAL)      , PRTCLASS_GENERAL },
{PT_EXIT_DELAY			,-1, offsetof(struct ALARM_PARTITION_t, exitDelay),         sizeof(((struct ALARM_PARTITION_t*)0)->exitDelay),          &pokeByte,   &peekByte    , strlen(PT_EXIT_DELAY)        , PRTCLASS_GENERAL },
{PT_VALID				,-1, offsetof(struct ALARM_PARTITION_t, valid),             sizeof(((struct ALARM_PARTITION_t*)0)->valid),              &pokeBool,   &peekBool    , strlen(PT_VALID	)          , PRTCLASS_GENERAL },
{PT_FOLLOW_1			,-1, offsetof(struct ALARM_PARTITION_t, follows),           sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_1)          , PRTCLASS_GENERAL },
{PT_FOLLOW_2			,-1, offsetof(struct ALARM_PARTITION_t, follows) + 1,       sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_2)          , PRTCLASS_GENERAL },
{PT_FOLLOW_3			,-1, offsetof(struct ALARM_PARTITION_t, follows) + 2,       sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_3)          , PRTCLASS_GENERAL },
{PT_FOLLOW_4			,-1, offsetof(struct ALARM_PARTITION_t, follows) + 3,       sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_4)          , PRTCLASS_GENERAL },
{PT_FOLLOW_5			,-1, offsetof(struct ALARM_PARTITION_t, follows) + 4,       sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_5)          , PRTCLASS_GENERAL },
{PT_FOLLOW_6			,-1, offsetof(struct ALARM_PARTITION_t, follows) + 5,       sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_6)          , PRTCLASS_GENERAL },
{PT_FOLLOW_7			,-1, offsetof(struct ALARM_PARTITION_t, follows) + 6,       sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_7)          , PRTCLASS_GENERAL },
{PT_FOLLOW_8			,-1, offsetof(struct ALARM_PARTITION_t, follows) + 7,       sizeof(byte),                                               &patchFollow, &unPatchFollow  , strlen(PT_FOLLOW_8)          , PRTCLASS_GENERAL },
};
#define PARTITION_TAGS_CNT (sizeof(partitionTags)/sizeof(partitionTags[0]))

/**
 * @brief Global options tags Contains tag as a string and corresponding offset from beginnig of struct ALARM_GLOBAL_OPTS_t. Used to modify tag values direct in memory.
*/
struct tagAccess gOptsTags[] = {
{GO_MAX_SLAVES,		 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, maxSlaveBrds),      sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->maxSlaveBrds),     &pokeByte,    &peekByte   , strlen(GO_MAX_SLAVES),		PRTCLASS_GENERAL },
{GO_RESTR_SPRVS_LOSS,-1, offsetof(struct ALARM_GLOBAL_OPTS_t, restrOnSprvsLoss),  sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->restrOnSprvsLoss), &pokeBool,    &peekBool   , strlen(GO_RESTR_SPRVS_LOSS),  PRTCLASS_GENERAL },
{GO_RESTR_TAMPER,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, restrOnTamper),     sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->restrOnTamper),    &pokeBool,    &peekBool   , strlen(GO_RESTR_TAMPER),	    PRTCLASS_GENERAL },
{GO_RESTR_AC_FAIL,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, restrOnACfail),     sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->restrOnACfail),    &pokeBool,    &peekBool   , strlen(GO_RESTR_AC_FAIL),	    PRTCLASS_GENERAL },
{GO_RESTR_BAT_FAIL,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, restrOnBatFail),    sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->restrOnBatFail),   &pokeBool,    &peekBool   , strlen(GO_RESTR_BAT_FAIL),	PRTCLASS_GENERAL },
{GO_RESTR_BELL,		 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, restrOnBellFail),   sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->restrOnBellFail),  &pokeBool,    &peekBool   , strlen(GO_RESTR_BELL),		PRTCLASS_GENERAL },
{GO_RESTR_ON_BRDFAIL,-1, offsetof(struct ALARM_GLOBAL_OPTS_t, restrOnBrdFail),    sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->restrOnBrdFail),   &pokeBool,    &peekBool   , strlen(GO_RESTR_ON_BRDFAIL),  PRTCLASS_GENERAL },
{GO_RESTR_AMASK,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, restrOnAntimask),   sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->restrOnAntimask),  &pokeBool,    &peekBool   , strlen(GO_RESTR_AMASK),	    PRTCLASS_GENERAL },
{GO_TROUBLE_LATCH,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, troubleLatch),      sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->troubleLatch),     &pokeBool,    &peekBool   , strlen(GO_TROUBLE_LATCH),	    PRTCLASS_GENERAL },
{GO_TAMPER_BPS_OPT,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, tamperBpsOpt),      sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->tamperBpsOpt),     &pokeBool,    &peekBool   , strlen(GO_TAMPER_BPS_OPT),	PRTCLASS_GENERAL },
{GO_TAMPER_OPTS,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, tamperOpts),        sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->tamperOpts),       &pokeLineErr, &peekLineErr, strlen(GO_TAMPER_OPTS),	    PRTCLASS_GENERAL },
{GO_AMASK_OPTS,		 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, antiMaskOpt),       sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->antiMaskOpt),      &pokeLineErr, &peekLineErr, strlen(GO_AMASK_OPTS),		PRTCLASS_GENERAL },
{GO_RF_SPRVS_OPTS,	 -1, offsetof(struct ALARM_GLOBAL_OPTS_t, rfSprvsOpt),        sizeof(((struct ALARM_GLOBAL_OPTS_t*)0)->rfSprvsOpt),       &pokeLineErr, &peekLineErr, strlen(GO_RF_SPRVS_OPTS),	    PRTCLASS_GENERAL },
};
#define GLOBAL_OPTIONS_TAGS_CNT (sizeof(gOptsTags)/sizeof(struct tagAccess))

/**
 * @brief PGM tags. Contains tag as a string and corresponding offset from beginnig of struct ALARM_PGM. Used to modify tag values direct in memory.
*/
struct tagAccess pgmTags[] = {
  {PGM_NAME,            -1, offsetof(struct ALARM_PGM, pgmName),                 sizeof(((struct ALARM_PGM*)0)->pgmName),                   &pokeString,  &peekString,  16,                     PRTCLASS_GENERAL },
  {PGM_BRD,             -1, offsetof(struct ALARM_PGM, boardID),                 sizeof(((struct ALARM_PGM*)0)->boardID),                   &pokeByte,    &peekByte,    strlen(PGM_BRD),        PRTCLASS_GENERAL },
  {PGM_ID,              -1, offsetof(struct ALARM_PGM, pgmID),                   sizeof(((struct ALARM_PGM*)0)->pgmID),                     &pokeByte,    &peekByte,    strlen(PGM_ID),         PRTCLASS_GENERAL },
  {PGM_PULSE_LEN,       -1, offsetof(struct ALARM_PGM, pulseLen),                sizeof(((struct ALARM_PGM*)0)->pulseLen),                  &pokeByte,    &peekByte,    strlen(PGM_PULSE_LEN),  PRTCLASS_GENERAL },
  {PGM_VALID,           -1, offsetof(struct ALARM_PGM, valid),                   sizeof(((struct ALARM_PGM*)0)->valid),                     &pokeByte,    &peekByte,    strlen(PGM_VALID),      PRTCLASS_GENERAL },
};
#define PGM_TAGS_CNT (sizeof(pgmTags)/sizeof(struct tagAccess))


/**
 * @brief Flags used to mark what was imported so far from CSV file. Must be power of 2.
*/
enum parseFlags_t {
    CLEAR_FLAGS     = 0,
    HAVE_GOPT_HDR   = 1,
    HAVE_ZN_HDR     = 2,
    HAVE_PGM_HDR    = 4,
    HAVE_PRT_HDR    = 8,
    HAVE_G_OPT      = 16,
};

/**
 * @brief Contains all data needed to process line header
*/
//struct csvHeader_t {
//    const           char* hdrTTL;                                   //!< header title - first cell in each line to identify the line content (e.g. zone, partition, zHeader, pgmHeader, etc.)
//    int			    domain_idx;						                //!< index in dbProps to find the database for the corresponding entry
//    int             (*handlerCB)  (int dataTyp, char* strbuf);      //!< pointer to function to handle the rest of the line
//    struct          tagAccess * hdrParamArrPtr;                     //!< array with all possible parameters in the header
//    int             paramCnt;                                       //!< size of the above array
//    unsigned int    setFlags;                                       //!< we need to have certain data in order to know how to interpreate current one - e.g. we need to get global options before any data, or to get header before the data line
//    unsigned int    checkFlags;                                     //!< we need to have certain data in order to know how to interpreate current one - e.g. we need to get global options before any data, or to get header before the data line
//    byte *          dataPtr;                                        //!< pointer to temp storage to store the imported data
//    int             (*dataStoreCB) (int dataTyp, maxTmp_t* src, int saveFl);    //!< fuct callback to postproces and copy line data from tmp storage to corrsponding database
//};


/**
 * @brief Contains all line headers allowed in CSV file
*/
//struct csvHeader_t csvHeaders[] = {
//        {CSV_ZONE_HDR_TTL,	    RESERVED,   &getHeaderLine, zoneTags,       ZONE_TAGS_CNT,            HAVE_ZN_HDR,            CLEAR_FLAGS,                                NULL,           NULL},
//        {CSV_PRT_HDR_TTL,	    RESERVED,   &getHeaderLine, partitionTags,  PARTITION_TAGS_CNT,       HAVE_PRT_HDR,           CLEAR_FLAGS,                                NULL,           NULL},
//        {CSV_PGM_HDR_TTL,	    RESERVED,   &getHeaderLine, pgmTags ,       PGM_TAGS_CNT,             HAVE_PGM_HDR,           CLEAR_FLAGS,                                NULL,           NULL},
//        {CSV_GOPT_HDR_TTL,      RESERVED,   &getHeaderLine, gOptsTags,      GLOBAL_OPTIONS_TAGS_CNT,  HAVE_GOPT_HDR,          CLEAR_FLAGS,                                NULL,           NULL},
//        {CSV_ZONE_TTL,		    ZONES,      &getDataLine,   zoneTags,       ZONE_TAGS_CNT,            CLEAR_FLAGS,            HAVE_GOPT_HDR|HAVE_ZN_HDR |HAVE_G_OPT,      (byte*)&maxTmp, &postprocesStoreImportedCSVData},
//        {CSV_PRT_TTL,	        PARTITIONS, &getDataLine,   partitionTags,  PARTITION_TAGS_CNT,       CLEAR_FLAGS,            HAVE_GOPT_HDR|HAVE_PRT_HDR|HAVE_G_OPT,      (byte*)&maxTmp, &postprocesStoreImportedCSVData},
//        {CSV_PGM_TTL,	        PGMS,       &getDataLine,   pgmTags ,       PGM_TAGS_CNT,             CLEAR_FLAGS,            HAVE_GOPT_HDR|HAVE_PGM_HDR|HAVE_G_OPT,      (byte*)&maxTmp, &postprocesStoreImportedCSVData},
//        {CSV_GOPT_TTL,          GLOBAL_OPT, &getDataLine,   gOptsTags,      GLOBAL_OPTIONS_TAGS_CNT,  HAVE_G_OPT,             HAVE_GOPT_HDR,                              (byte*)&maxTmp, &postprocesStoreImportedCSVData},
//        //{CSV_KEYSW_HDR_TTL,   KEYSW,      &getHeaderLine,   KEYSW_HDR   },
//};
//#define CSV_HEADERS_CNT         (sizeof(csvHeaders)/sizeof(csvHeaders[0]))
//
//TODO - must be obsolte soon
//struct str2val tamperKeys[] = { {"DISABLE", ZONE_TAMPER_OPT_DISABLED }, {"TROUBLE_ONLY", ZONE_TAMPER_OPT_TROUBLE_ONLY}, {"ALARM_WHEN_ARMED", ZONE_TAMPER_OPT_ALARM_WHEN_ARMED},  {"ALARM", ZONE_TAMPER_OPT_ALARM} };
//#define TAMPERS_TITLE_CNT (sizeof(tamperKeys)/sizeof(struct str2val))
//
//struct str2val antiMaskKeys[] = { {"DISABLE", ZONE_ANTI_MASK_OPT_DISABLED }, {"TROUBLE_ONLY", ZONE_ANTI_MASK_OPT_TROUBLE_ONLY}, {"ALARM_WHEN_ARMED",ZONE_ANTI_MASK_OPT_ALARM_WHEN_ARMED},  {"ALARM", ZONE_ANTI_MASK_OPT_ALARM} };
//#define ANTIMASKS_TITLE_CNT (sizeof(antiMaskKeys)/sizeof(struct str2val))

