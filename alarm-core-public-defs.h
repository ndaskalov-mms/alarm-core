#pragma once
enum ALARM_DOMAINS_t {
	RESERVED = -1,						// offsets the IDs for the folloing items in order to match sbProps[] indexes TODO ???
	ZONES = 0,
	PARTITIONS = 1,
	PGMS = 2,
	KEYSW = 3,
	GLOBAL_OPT = 4,
};
//
//
enum PGM_CMDS_t {
	PGM_OFF = 1,
	PGM_ON = 2,
	PGM_PULSE = 3,
};
//
// zone commands definitions
//
enum ZONE_CMDS_t {
	ZONE_BYPASS_CMD = 1,    // keep BYPASS and UNBYPASS cmds as power of 2 as they can be or-ed in 
	ZONE_UNBYPASS_CMD = 2,    // in zoneNewCmd TODO - check if this is correct
	ZONE_CLOSE_CMD = 3,
	ZONE_OPEN_CMD = 4,
	ZONE_AMASK_CMD = 5,
	ZONE_TAMPER_CMD = 6,
	ZONE_ANAL_SET_CMD = 7,
	ZONE_DIGITAL_SET_CMD = 8,
};
//
//
enum  ARM_METHODS_t {
	DISARM = 0,
	REGULAR_ARM = 0x1,
	FORCE_ARM = 0x2,
	INSTANT_ARM = 0x4,
	STAY_ARM = 0x8,
};