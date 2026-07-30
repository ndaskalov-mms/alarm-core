#pragma once
#include "alarm-core-json-val-parsers.h"

// Printer-only metadata
struct printKeyValProcessor {
    const jsonKeyValProcessor* parserKey;
    byte* (*unpatchCallBack)(byte* basePtr, int offset, int len);
    size_t keyStrLen;
    byte printClass;
};

// zone config print map
static printKeyValProcessor zoneCfgPrintProcessors[] = {
    { &zoneCfgKeyValProcessors[0],  &peekString,   16,                        PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[1],  &peekByte,     strlen(ZN_ID_KEY_STR),     PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[2],  &peekZoneType, 12,                        PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[3],  &peekPrtnNo,   strlen(ZN_PRT_KEY_STR),    PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[4],  &peekAlarmType,8,                         PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[5],  &peekBool,     strlen(ZN_SHD_EN_KEY_STR), PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[6],  &peekBool,     strlen(ZN_BYP_EN_KEY_STR), PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[7],  &peekBool,     strlen(ZN_STAY_KEY_STR),   PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[8],  &peekBool,     strlen(ZN_FORCE_EN_KEY_STR),PRTCLASS_GENERAL },
    { &zoneCfgKeyValProcessors[9],  &peekBool,     strlen(ZN_INTEL_KEY_STR),  PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[10], &peekBool,     strlen(ZN_DLYTRM_KEY_STR), PRTCLASS_GENERAL  },
    { &zoneCfgKeyValProcessors[11], &peekBool,     strlen(ZN_TMPR_GLB_KEY_STR),PRTCLASS_LINE_ERR},
    { &zoneCfgKeyValProcessors[12], &peekLineErr,  strlen(ZN_TMPR_OPT_KEY_STR),PRTCLASS_LINE_ERR},
    { &zoneCfgKeyValProcessors[13], &peekBool,     strlen(ZN_AMSK_GLB_KEY_STR),PRTCLASS_LINE_ERR},
    { &zoneCfgKeyValProcessors[14], &peekLineErr,  strlen(ZN_AMSK_OPT_KEY_STR),PRTCLASS_LINE_ERR},
};
#define ZONE_CFG_PRINT_KEYS_CNT (sizeof(zoneCfgPrintProcessors)/sizeof(zoneCfgPrintProcessors[0]))

// partition config print map
static printKeyValProcessor partitionPrintProcessors[] = {
    { &partitionKeyValProcessors[0],  &peekString, 16,                                 PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[1],  &peekPrtnNo, strlen(PT_IDX_KEY_STR),             PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[2],  &peekBool,   strlen(PT_FORCE_ON_REG_ARM_KEY_STR),PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[3],  &peekBool,   strlen(PT_FORCE_ON_STAY_ARM_KEY_STR),PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[4],  &peekBool,   strlen(PT_ED2_FOLLOW_KEY_STR),      PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[5],  &peekBool,   strlen(PT_ALRM_OUT_EN_KEY_STR),     PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[6],  &peekByte,   strlen(PT_ALRM_LENGHT_KEY_STR),     PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[7],  &peekBool,   strlen(PT_NO_CUT_ON_FIRE_KEY_STR),  PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[8],  &peekByte,   strlen(PT_ALARM_RECYC_TIME_KEY_STR),PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[9],  &peekByte,   strlen(PT_ED1_INTERVAL_KEY_STR),    PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[10], &peekByte,   strlen(PT_ED2_INTERVAL_KEY_STR),    PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[11], &peekByte,   strlen(PT_EXIT_DLY_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[12], &peekBool,   strlen(PT_VALID_KEY_STR),           PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[13], &peekFollow, strlen(PT_FOLLOW_1_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[14], &peekFollow, strlen(PT_FOLLOW_2_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[15], &peekFollow, strlen(PT_FOLLOW_3_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[16], &peekFollow, strlen(PT_FOLLOW_4_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[17], &peekFollow, strlen(PT_FOLLOW_5_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[18], &peekFollow, strlen(PT_FOLLOW_6_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[19], &peekFollow, strlen(PT_FOLLOW_7_KEY_STR),        PRTCLASS_GENERAL },
    { &partitionKeyValProcessors[20], &peekFollow, strlen(PT_FOLLOW_8_KEY_STR),        PRTCLASS_GENERAL },
};
#define PARTITION_PRINT_KEYS_CNT (sizeof(partitionPrintProcessors)/sizeof(partitionPrintProcessors[0]))

// global options print map
static printKeyValProcessor gOptsPrintProcessors[] = {
    { &gOptsKeyValProcessors[0],  &peekByte,    strlen(GO_MAX_SLAVES_KEY_STR),       PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[1],  &peekBool,    strlen(GO_RESTR_SPRVS_LOSS_KEY_STR), PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[2],  &peekBool,    strlen(GO_RESTR_TAMPER_KEY_STR),     PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[3],  &peekBool,    strlen(GO_RESTR_AC_FAIL_KEY_STR),    PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[4],  &peekBool,    strlen(GO_RESTR_BAT_FAIL_KEY_STR),   PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[5],  &peekBool,    strlen(GO_RESTR_BELL_KEY_STR),       PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[6],  &peekBool,    strlen(GO_RESTR_BOARD_FAIL_KEY_STR), PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[7],  &peekBool,    strlen(GO_RESTR_AMASK_KEY_STR),      PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[8],  &peekBool,    strlen(GO_TROUBLE_LATCH_KEY_STR),    PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[9],  &peekBool,    strlen(GO_TAMPER_BPS_OPT_KEY_STR),   PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[10], &peekLineErr, strlen(GO_TAMPER_OPTS_KEY_STR),      PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[11], &peekLineErr, strlen(GO_AMASK_OPTS_KEY_STR),       PRTCLASS_GENERAL },
    { &gOptsKeyValProcessors[12], &peekLineErr, strlen(GO_RF_SPRVS_OPT_KEY_STR),     PRTCLASS_GENERAL },
};
#define GOPTS_PRINT_KEYS_CNT (sizeof(gOptsPrintProcessors)/sizeof(gOptsPrintProcessors[0]))

// pgm print map
static printKeyValProcessor pgmPrintProcessors[] = {
    { &pgmKeyValProcessors[0], &peekString, 16,                            PRTCLASS_GENERAL },
    { &pgmKeyValProcessors[1], &peekByte,   strlen(PGM_BRD_ID_KEY_STR),   PRTCLASS_GENERAL },
    { &pgmKeyValProcessors[2], &peekByte,   strlen(PGM_ID_KEY_STR),       PRTCLASS_GENERAL },
    { &pgmKeyValProcessors[3], &peekByte,   strlen(PGM_PULSE_LEN_KEY_STR),PRTCLASS_GENERAL },
    { &pgmKeyValProcessors[4], &peekByte,   strlen(PGM_VALID_KEY_STR),    PRTCLASS_GENERAL },
};
#define PGM_PRINT_KEYS_CNT (sizeof(pgmPrintProcessors)/sizeof(pgmPrintProcessors[0]))
