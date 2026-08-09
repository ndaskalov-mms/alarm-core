#pragma once
// MQTT topics defs
#define MQTT_FULL_CONFIG_TOPIC				"/alarm/config"
#define MQTT_PARTITIONS_CONTROL_TOPIC		"/alarm/partitions/control"
#define MQTT_PARTITIONS_STATES_TOPIC		"/alarm/partitions/states"
#define MQTT_ZONES_CONTROL_TOPIC	        "/alarm/zones/control"
#define MQTT_ZONES_STATES_TOPIC             "/alarm/zones/states"
#define MQTT_OUTPUTS_CONTROL_TOPIC			"/alarm/outputs/control"
#define MQTT_OUTPUTS_STATES_TOPIC			"/alarm/outputs/states"
#define MQTT_GLOBAL_OPT_CONTROL_TOPIC		"/alarm/global_options/control"

// config JSON sections
//example - {
//		"partitions": [ { prt 1 config JSON }, { prt 2 config JSON }] ,
//		"zones" : [ { zone 1 config JSON }, { zone 2 config JSON }] ,
//		"globalOptions" : { global options config JSON  },
// 		"pgms" : [{ pgm 1 config JSON }, { pgm 2 config JSON }] ,
//		"keyswitches" : [{ ksw 1 config JSON },	{ ksw 2 config JSON }]
//		}
#define JSON_SECTION_ZONES				"zones"
#define JSON_SECTION_PARTITIONS			"partitions"
#define JSON_SECTION_PGMS				"pgms"
#define JSON_SECTION_GLOBAL_OPTIONS		"global_options"

// ------------   zone  config JSON ------------------
// example - receive topic:	 "/alarm/zones/config",
//			 payload:  {"zName": "Back Door","zBRD":0,"zID":1,"zType":"ENTRY_DELAY2","zPartn":1,
//						"zAlarmT":"PULSED_ALARM","zShdnEn":false,"zBypEn":true,"zStayZ":true,"zFrceEn":true,
//						"zIntelZ":false,"zDlyTRM":false,"zTmprGlb":true,"zTmprOPT":"TROUBLE_ONLY","zAmskGlb":true,
//						"zAmskOpt":"DISABLE"	}
// 
// keys allowed in zone JSON config 
#define ZN_NAME_KEY_STR					"zName"				// val: str zone name max len NAME_LEN
#define ZN_BRD_ID_KEY_STR				"zBRD"				// val: int which board zone is attached to. 
#define ZN_ID_KEY_STR					"zID"				// val: int internal index of zone record in zones database Normally shal not be used
#define ZN_TYPE_KEY_STR					"zType"				// val: str zone type, allowed values are defined below as ZONE_XXXX_VAL_STR
#define ZN_PRT_KEY_STR					"zPartn"			// val: int partition number zone is assigned to. 1..MAX_PARTITION TODO: shall be modified to use part. name
#define ZN_ALARM_KEY_STR				"zAlarmT"			// val: str zone alarm type, allowed values are defined below as XXXX_ALARM_VAL_STR
#define ZN_SHD_EN_KEY_STR				"zShdnEn"			// val: bool zone shutdown enable/disable TODO: check usage, shall be removed
#define ZN_BYP_EN_KEY_STR				"zBypEn"			// val: bool zone bypass enable/disable
#define ZN_STAY_KEY_STR					"zStayZ"			// val: bool zone is defined as stay zone
#define ZN_FORCE_EN_KEY_STR				"zFrceEn"			// val: bool zone force bypass enable/disable
#define ZN_INTEL_KEY_STR				"zIntelZ"			// val: bool zone intelizone TODO: check usage, shall be removed
#define ZN_DLYTRM_KEY_STR				"zDlyTRM"			// val: bool allow delay timer for zone, if false, zone will be treated as instant zone
#define ZN_TMPR_GLB_KEY_STR				"zTmprGlb"			// val: bool on tamper use global settings for panel
#define ZN_TMPR_OPT_KEY_STR				"zTmprOPT"			// val: str zone tamper option, allowed values are defined below as LINE_ERR_OPT_XXXX_VAL_STR
#define ZN_AMSK_GLB_KEY_STR				"zAmskGlb"			// val: bool on anti-mask use global settings for panel
#define ZN_AMSK_OPT_KEY_STR				"zAmskOpt"			// val: str zone anti-mask option, allowed values are defined below as LINE_ERR_OPT_XXXX_VAL_STR
//#define ZN_CONTROL_KEY_STR			"zControl"
//#define ZN_STATUS_KEY_STR				"zStatus"

// value strings allowed for zone type ( ZN_TYPE_KEY_STR) in zone config JSON
#define	ZONE_DISABLED_VAL_STR			"DISABLED"             
#define	INSTANT_VAL_STR					"INSTANT"              		
#define	ENTRY_DELAY1_VAL_STR			"ENTRY_DELAY1"         	
#define	ENTRY_DELAY2_VAL_STR			"ENTRY_DELAY2"         	
#define	FOLLOW_VAL_STR					"FOLLOW"               			
#define	STAY_DLY1_VAL_STR				"STAY_DELAY1"          
#define	STAY_DLY2_ANTI_MASK_VAL_STR		"STAY_DELAY2_ANTI_MASK"	
#define	H24_BUZZER_VAL_STR				"H24_BUZZER"           
#define	H24_BURGLAR_VAL_STR				"H24_BURGLAR"          
#define	H24_HOLDUP_VAL_STR				"H24_HOLDUP"           
#define	H24_GAS_VAL_STR					"H24_GAS"              
#define	H24_HEAT_VAL_STR				"H24_HEAT"             
#define	H24_WATER_VAL_STR				"H24_WATER"            
#define	H24_FREEZE_VAL_STR				"H24_FREEZE"           
#define	H24_FIRE_DELAYED_VAL_STR		"H24_FIRE_DELAYED"     
#define	H24_FIRE_STANDARD_VAL_STR		"H24_FIRE_STANDARD"    


//
// value strings allowed for alarm type (ZN_ALARM_KEY_STR) in zone config JSON
#define	STEADY_ALARM_VAL_STR			"STEADY_ALARM"
#define	SILENT_ALARM_VAL_STR			"SILENT_ALARM"
#define	PULSED_ALARM_VAL_STR			"PULSED_ALARM"
#define	REPORT_ALARM_VAL_STR			"REPORT_ALARM"

//
// value strings allowed as values for ZN_AMSK_OPT_KEY_STR and ZN_TMPR_OPT_KEY_STR control in zone config JSON
#define LINE_ERR_OPT_DISABLED_VAL_STR			"DISABLE"
#define LINE_ERR_OPT_TROUBLE_ONLY_VAL_STR		"TROUBLE_ONLY"
#define LINE_ERR_OPT_ALARM_WHEN_ARMED_VAL_STR	"ALARM_WHEN_ARMED"
#define LINE_ERR_OPT_ALARM_ONLY_VAL_STR			"ALARM"

//
// Zone control (ZN_CONTROL_KEY_STR) supported JSON values (commands)
// example - receive topic:	"/alarm/zones/control",
//			 payload:		{"zName":"Front Door","bypass":true}
//
#define ZONE_BYPASS_VAL_STR				"bypass"      
#define ZONE_TAMPER_VAL_STR				"tamper"      
#define ZONE_OPEN_VAL_STR				"open"        
#define ZONE_ANTI_MSK_VAL_STR			"anti-mask" 

// ------------   ZONE RUNTIME STATUS JSON KEYS and VALS ------------------
// example zone status example JSON object:
//		send topic:	"/alarm/zones/status",
//		payload:
//		{
//			ZN_ZONE_STAT_KEY_STR:		ZN_OPEN_VAL,		// ZN_CLOSE_VAL,  ZN_ERROR_VAL
//			ZN_BYPASSED_KEY_STR:		ZN_NO_BYPASS_VAL,	// ZN_STAY_BYPASSED_VAL, ZN_EDx_BYPASSED_VAL, ZN_EX_D_BYPASSED_VAL, ZN_NO_BYPASS_VAL, ZN_FORCED_VAL	
//			ZN_ERROR_KEY_STR:			ZN_NO_ERROR_VAL,	// ZN_TAMPER_VAL, ZN_AMASK_VAL
//			ZN_IN_ALARM_KEY_STR: 		YES,				// NO
//			ZN_IN_TROUBLE_KEY_STR:		YES,				// NO
//			ZN_IGNORED_TAMPER_KEY_STR:	YES,				// NO
//			ZN_IGNORED_AMASK_KEY_STR:	YES,				// NO
//			ZN_OPEN_EDSD1_ZONE_KEY_STR:	YES,				// NO
//			ZN_OPEN_EDSD2_ZONE_KEY_STR:	YES,				// NO
//		}
// 

// Zone runtime status key and vals
#define ZN_ZONE_STAT_KEY_STR			"zoneStat"
#define ZN_OPEN_VAL						"OPEN"
#define ZN_ERROR_VAL					"ERROR"
#define ZN_CLOSE_VAL					"CLOSED"

// Zone bypass status key and vals
#define ZN_BYPASSED_KEY_STR				"bypassed"
#define ZN_NO_BYPASS_VAL				"NO_BYPASS"
#define ZN_STAY_BYPASSED_VAL			"STAY_BYP"
#define ZN_EDx_BYPASSED_VAL				"ENT_DLY_BYP"
#define ZN_EX_D_BYPASSED_VAL			"EXIT_DLY_BYP"
#define ZN_FORCED_VAL					"FORCED"

// Zone error key and vals
#define ZN_ERROR_KEY_STR				"ERROR"
#define ZN_TAMPER_VAL					"TAMPER"
#define ZN_AMASK_VAL					"AMASK"
#define ZN_NO_ERROR_VAL					"NO"

#define ZN_IN_ALARM_KEY_STR				"inAlarm"		// val: bool - YES if zone is in Alarm, NO if not in Alarm
#define ZN_IN_TROUBLE_KEY_STR			"inTrouble"		// val: bool - YES if zone is in Trouble, NO if not in Trouble
#define ZN_IGNORED_TAMPER_KEY_STR		"ignTamper"		// val: bool - YES if zone has Ignored Tamper, NO if not has Ignored Tamper
#define ZN_IGNORED_AMASK_KEY_STR		"ignAmask"		// val: bool - YES if zone has Ignored Anti-Mask, NO if not has Ignored Anti-Mask
#define ZN_OPEN_EDSD1_ZONE_KEY_STR		"openEDSD1zn"	// val: bool - YES if zone is in Open ED1/SD1 Zone, NO if not in Open ED1/SD1 Zone
#define ZN_OPEN_EDSD2_ZONE_KEY_STR		"openEDSD2zn"	// val: bool - YES if zone is in Open ED2/SD2 Zone, NO if not in Open ED2/SD2 Zone


//
// ------------   PARTITIONS_CFG ------------------
// example partition config JSON:
// receive topic:	"/alarm/partitions/config",
// payload:
//		{"pName":"Main Floor","pIdx":1,"pValid":true,"pFrceOnRegArm":true,"pFrceOnStayArm":true,"pED2znFollow":true,
//		"pAlrmOutEn":true,"pAlrmTime":120,"pNoCutOnFire":true,"pAlrmRecTime":30,"pED1Intvl":30,"pED2Intvl":45,"pExitDly":60,
//		"pFollow1":1,"pFollow2":0,"pFollow3":0,"pFollow4":0,"pFollow5":0,"pFollow6":0,"pFollow7":0,"pFollow8":0 }
//
// keys allowed in partition JSON config  
#define	PT_NAME_KEY_STR					"pName"						//	val: string, partition name
#define	PT_IDX_KEY_STR					"pIdx"					    //	val: int,	partition index
#define	PT_FORCE_ON_REG_ARM_KEY_STR		"pFrceOnRegArm"			    //	val: bool,	force bypass open zones on regular arm
#define	PT_FORCE_ON_STAY_ARM_KEY_STR	"pFrceOnStayArm"		    //	val: bool,	force bypass open zones on stay arm
#define	PT_ED2_FOLLOW_KEY_STR			"pED2znFollow"			    //	val: bool,	if all ENTRY_DELAY_X zones are bypass, FOLLOW zones shall kick-off the delay timer (2) if enabled in partition options
#define	PT_ALRM_OUT_EN_KEY_STR			"pAlrmOutEn"			    //	val: bool,	alarm output enable
#define	PT_ALRM_LENGHT_KEY_STR			"pAlrmTime"				    //	val: int,	alarm time
#define	PT_NO_CUT_ON_FIRE_KEY_STR		"pNoCutOnFire"			    //	val: bool,	no cut on alarm on fire
#define	PT_ALARM_RECYC_TIME_KEY_STR		"pAlrmRecTime"			    //	val: int,	alarm recycle time
#define	PT_ED1_INTERVAL_KEY_STR			"pED1Intvl"				    //	val: int,	entry delay 1 (ED1) interval seconds
#define	PT_ED2_INTERVAL_KEY_STR			"pED2Intvl"				    //	val: int,	entry delay 2 (ED2) interval seconds
#define	PT_EXIT_DLY_KEY_STR				"pExitDly"				    //	val: int,	exit delay seconds
#define	PT_VALID_KEY_STR				"pValid"				    //	val: bool,	partition valid
// follow partition concept is kind of fuzzy. Idea is if partition which is followed is armed for example, this partition to arm as well)
// up to 8 partitions can be followed. If partition follows for examople partition 1, it will be armed/disarmed when the partition 1 is armed/disarmed.
#define	PT_FOLLOW_1_KEY_STR				"pFollow1"				    //	val: int,	follow partition idx - do the same as partition idx)		
#define	PT_FOLLOW_2_KEY_STR				"pFollow2"				    //	val: int,	follow partition idx - do the same as partition idx)
#define	PT_FOLLOW_3_KEY_STR				"pFollow3"				    //	val: int,	follow partition idx - do the same as partition idx)
#define	PT_FOLLOW_4_KEY_STR				"pFollow4"				    //	val: int,	follow partition idx - do the same as partition idx)
#define	PT_FOLLOW_5_KEY_STR				"pFollow5"				    //	val: int,	follow partition idx - do the same as partition idx)
#define	PT_FOLLOW_6_KEY_STR				"pFollow6"				    //	val: int,	follow partition idx - do the same as partition idx)
#define	PT_FOLLOW_7_KEY_STR				"pFollow7"				    //	val: int,	follow partition idx - do the same as partition idx)
#define	PT_FOLLOW_8_KEY_STR				"pFollow8"				    //	val: int,	follow partition idx - do the same as partition idx)

//
// ------------   PARTITIONS_CFG ------------------
// example partition control JSON:
// receive topic:	"/alarm/partitions/control",
// payload:			{"pName":"Main Floor","arm_method":"arm_force"}
// 
// Partition control supported JSON values (commands)  used as a value of arm_method JSON key in partition control command
#define	PT_ARM_METHOD_KEY_STR			"arm_method"
#define	PT_DISARM_VAL_STR				"disarm"
#define	PT_REG_ARM_VAL_STR				"arm"
#define	PT_FORCE_ARM_VAL_STR			"arm_force"
#define	PT_STAY_ARM_VAL_STR				"arm_stay"
#define	PT_INSTANT_ARM_VAL_STR			"arm_instant"
//#define	ARM_SLEEP_VAL_STR			"arm_sleep"
//#define	ARM_STAY_STAYD_VAL_STR		"arm_stay_stayd"
//#define	ARM_SLEEP_STAY_VAL_STR		"arm_sleep_stay"

// ------------   GLOBAL OPTIONS ------------------
//// example global config JSON:
// receive topic:	"/alarm/global-opt/config",
// payload:{"MaxSlaves": 2, "RestrSprvsL" : true, "RestrTamper" : true, "RestrACfail" : false,
//			"RestrBatFail" : true, "RestrOnBell" : false, "RestrOnBrdFail" : true, "RestrOnAmask" : false,
//			"TroubleLatch" : true, "TamperBpsOpt" : false, "TamperOpts" : "TROUBLE_ONLY", "AntiMaskOpt" : "ALARM_WHEN_ARMED",
//			"RfSprvsOpt" : "ALARM", "SprvsLoss" : 0, "ACfail" : 0, 	"BatFail" : 0, "BellFail" : 0, "BrdFail" : 0 }
// keys allowed in global options  JSON config
#define	GO_MAX_SLAVES_KEY_STR		 "MaxSlaves"	  // val: int, max number of slave boards allowed to be connected to the master board. 0..MAX_SLAVES
#define	GO_RESTR_SPRVS_LOSS_KEY_STR	 "RestrSprvsL"	  // val: bool, if true, restict arming  in supervision loss state
#define	GO_RESTR_TAMPER_KEY_STR		 "RestrTamper"	  // val: bool, if true, restrict arming if any of the zones is in tamper state
#define	GO_RESTR_AC_FAIL_KEY_STR	 "RestrACfail"	  // val: bool, if true, restrict arming in AC fail state
#define	GO_RESTR_BAT_FAIL_KEY_STR	 "RestrBatFail"	  // val: bool, if true, restrict arming in battery fail state
#define	GO_RESTR_BELL_KEY_STR		 "RestrOnBell"	  // val: bool, if true, restrict arming is siren is active (in alarm state)
#define	GO_RESTR_BOARD_FAIL_KEY_STR	 "RestrOnBrdFail" // val: bool, if true, restrict arming in board fail state
#define	GO_RESTR_AMASK_KEY_STR		 "RestrOnAmask"	  // val: bool, if true, restrict arming if any of the zones is in anti-mask state
#define	GO_TROUBLE_LATCH_KEY_STR	 "TroubleLatch"	  // val: bool, if true, latch trouble conditions
#define	GO_TAMPER_BPS_OPT_KEY_STR	 "TamperBpsOpt"	  // val: bool, if true, follow the global tamper settings else follow the zone tamper settings
#define	GO_TAMPER_OPTS_KEY_STR		 "TamperOpts"	  // val: string, tamper options see values defined above as LINE_ERR_OPT_XXXX_VAL_STR
#define	GO_AMASK_OPTS_KEY_STR		 "AntiMaskOpt"	  // val: string, anti-mask options see values defined above as LINE_ERR_OPT_XXXX_VAL_STR
#define	GO_RF_SPRVS_OPT_KEY_STR		 "RfSprvsOpt"	  // val: string, RF supervision options see values defined above as LINE_ERR_OPT_XXXX_VAL_STR

// ------------   PGM ------------------
//
// keys allowed in pgm JSON config
#define	PGM_NAME_KEY_STR				"pgmName"
#define	PGM_BRD_ID_KEY_STR				"pgmBrd"
#define	PGM_ID_KEY_STR					"pgmID"
#define	PGM_PULSE_LEN_KEY_STR			"pgmPulseLen"
#define	PGM_VALID_KEY_STR				"pgmValid"

//
// pgm (output) control supported payloads (commands). TODO - implement pulse command
#define PGM_ON_TTL      "on"
#define PGM_OFF_TTL     "off"
#define PGM_PULSE_TTL   "pulse"

//
// general keys allowed
#define	KEY_NOT_APPLICABLE			"NA"

//
enum ALARM_DOMAINS_t {
	RESERVED		= 0,				// offsets the IDs for the folloing items in order to match sbProps[] indexes TODO ???
	ZONES_CFG		= 0x10,
	ZONES_CMD		= 0x11,
	PARTITIONS_CFG	= 0x20,
	PARTITIONS_CMD	= 0x21,
	PGMS_CFG		= 0x40,
	PGMS_CMD		= 0x41,
	KEYSW_CFG		= 0x80,
	KEYSW_CMD		= 0x81,
	GLOBAL_OPT_CFG	= 0x100,
	GLOBAL_OPT_CMD	= 0x101,
};
//
//
enum PGM_CMDS_t {
	PGM_OFF			= 1,
	PGM_ON			= 2,
	PGM_PULSE		= 3,
};
//
// zone commands definitions
//
enum ZONE_CMDS_t {

	ZONE_BYPASS_CMD		= 1,    // keep BYPASS and UNBYPASS cmds as power of 2 as they can be bitwise or-ed in 
	ZONE_UNBYPASS_CMD	= 2,	
	ZONE_CLOSE_CMD		= 3,
	ZONE_OPEN_CMD		= 4,
	ZONE_AMASK_ON_CMD	= 5,
	ZONE_AMASK_OFF_CMD	= 6,
	ZONE_TAMPER_ON_CMD	= 7,
	ZONE_TAMPER_OFF_CMD = 8,
	ZONE_ANAL_SET_CMD	= 9,
	ZONE_DIG_SET_CMD	= 10,
	ZONE_RESERVED_CMD	= 0xFF, // used to determine if new command was received
};
//
//
enum  ARM_METHODS_t {
	INVALID_CMD		= -1,
	DISARM			= 0,
	REGULAR_ARM		= 0x1,
	FORCE_ARM		= 0x2,
	INSTANT_ARM		= 0x4,
	STAY_ARM		= 0x8,
};

////
//// zone commands definitions
////
//enum PARTITION_CMDS_t {
//	RESERVED = 0,
//	PRT_DISARM_CMD = 1,    // keep BYPASS and UNBYPASS cmds as power of 2 as they can be bitwise or-ed in 
//	PRT_REG_ARM_CMD = 2,
//	PRT_FORSE_ARM_CMD = 3,
//	PRT_INSTANT_ARM_CMD = 4,
//	PRT_STAY_ARM_CMD = 5,
//	PRT_RESERVED_CMD = 0xFF, // used to determine if new command was received
//};
////