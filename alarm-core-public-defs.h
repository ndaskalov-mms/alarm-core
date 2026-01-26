#pragma once
// MQTT topics defs
#define MQTT_PARTITIONS_CONTROL_TOPIC		"/alarm/partitions/control"
#define MQTT_PARTITIONS_STATES_TOPIC		"/alarm/partitions/states"
#define MQTT_ZONES_CONTROL_TOPIC	        "/alarm/zones/control"
#define MQTT_ZONES_STATES_TOPIC             "/alarm/zones/states"
#define MQTT_OUTPUTS_CONTROL_TOPIC			"/alarm/outputs/control"
#define MQTT_OUTPUTS_STATES_TOPIC			"/alarm/outputs/states"
#define MQTT_GLOBAL_OPT_CONTROL_TOPIC		"/alarm/global_options/control"

// JSON sections
#define JSON_SECTION_ZONES				"zones"
#define JSON_SECTION_PARTITIONS			"partitions"
#define JSON_SECTION_PGMS				"pgms"
#define JSON_SECTION_GLOBAL_OPTIONS		"global_options"
	

// MQTT JSON keys

// ------------   ZONES_CFG ------------------
// keys allowed in zone JSON config 
#define ZN_NAME_KEY_STR					"zName"
#define ZN_BRD_ID_KEY_STR				"zBRD"
#define ZN_ID_KEY_STR					"zID"
#define ZN_TYPE_KEY_STR					"zType"
#define ZN_PRT_KEY_STR					"zPartn"
#define ZN_ALARM_KEY_STR				"zAlarmT"
#define ZN_SHD_EN_KEY_STR				"zShdnEn"
#define ZN_BYP_EN_KEY_STR				"zBypEn"
#define ZN_STAY_KEY_STR					"zStayZ"
#define ZN_FORCE_EN_KEY_STR				"zFrceEn"
#define ZN_INTEL_KEY_STR				"zIntelZ"
#define ZN_DLYTRM_KEY_STR				"zDlyTRM"
#define ZN_TMPR_GLB_KEY_STR				"zTmprGlb"
#define ZN_TMPR_OPT_KEY_STR				"zTmprOPT"
#define ZN_AMSK_GLB_KEY_STR				"zAmskGlb"
#define ZN_AMSK_OPT_KEY_STR				"zAmskOpt"
#define ZN_CONTROL_KEY_STR				"zControl"
#define ZN_STATUS_KEY_STR				"zStatus"

//
// Zone control (ZN_CONTROL_KEY_STR) supported JSON values (commands)
#define ZONE_BYPASS_VAL_STR				"bypass"      
#define ZONE_TAMPER_VAL_STR				"tamper"      
#define ZONE_OPEN_VAL_STR				"open"        
#define ZONE_ANTI_MSK_VAL_STR			"anti-mask"   

//
// Zone status (ZN_STATUS_KEY_STR) supported JSON values 
#define ZN_BYPASSED_TTL					"BYPASSED"
#define ZN_TAMPER_TTL					"TAMPER"
#define ZN_OPEN_TTL						"OPEN"
#define ZN_ERROR_TTL					"ERROR"
#define ZN_FORCED_TTL					"FORCED"
#define ZN_CLOSE_TTL					"CLOSED"
#define ZN_AMASK_TTL					"AMASK"

//
// value strings allowed for zone type (ZN_TYPE_TTL) in zone config JSON
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
// value strings allowed for alarm type (ZN_ALARM_TTL) in zone config JSON
#define	STEADY_ALARM_VAL_STR			"STEADY_ALARM"
#define	SILENT_ALARM_VAL_STR			"SILENT_ALARM"
#define	PULSED_ALARM_VAL_STR			"PULSED_ALARM"
#define	REPORT_ALARM_VAL_STR			"REPORT_ALARM"

//
// value strings allowed as values for ZN_TMPR_OPT_TTL and ZN_AMSK_GLB_TTL control in zone config JSON
#define LINE_ERR_OPT_DISABLED_VAL_STR			"DISABLE"
#define LINE_ERR_OPT_TROUBLE_ONLY_VAL_STR		"TROUBLE_ONLY"
#define LINE_ERR_OPT_ALARM_WHEN_ARMED_VAL_STR	"ALARM_WHEN_ARMED"
#define LINE_ERR_OPT_ALARM_ONLY_VAL_STR			"ALARM"

// ------------   PARTITIONS_CFG ------------------
// keys allowed in partition JSON config 
#define	PT_NAME_KEY_STR					"pName"
#define	PT_IDX_KEY_STR					"pIdx"
#define	PT_FORCE_ON_REG_ARM_KEY_STR		"pFrceOnRegArm"
#define	PT_FORCE_ON_STAY_ARM_KEY_STR	"pFrceOnStayArm"
#define	PT_ED2_FOLLOW_KEY_STR			"pED2znFollow"
#define	PT_ALRM_OUT_EN_KEY_STR			"pAlrmOutEn"
#define	PT_ALRM_LENGHT_KEY_STR			"pAlrmTime"
#define	PT_NO_CUT_ON_FIRE_KEY_STR		"pNoCutOnFire"
#define	PT_ALARM_RECYC_TIME_KEY_STR		"pAlrmRecTime"
#define	PT_ED1_INTERVAL_KEY_STR			"pED1Intvl"
#define	PT_ED2_INTERVAL_KEY_STR			"pED2Intvl"
#define	PT_EXIT_DLY_KEY_STR				"pExitDly"
#define	PT_VALID_KEY_STR				"pValid"
#define	PT_FOLLOW_1_KEY_STR				"pFollow1"
#define	PT_FOLLOW_2_KEY_STR				"pFollow2"
#define	PT_FOLLOW_3_KEY_STR				"pFollow3"
#define	PT_FOLLOW_4_KEY_STR				"pFollow4"
#define	PT_FOLLOW_5_KEY_STR				"pFollow5"
#define	PT_FOLLOW_6_KEY_STR				"pFollow6"
#define	PT_FOLLOW_7_KEY_STR				"pFollow7"
#define	PT_FOLLOW_8_KEY_STR				"pFollow8"

//
// Partition control supported JSON values (commands)
// used as a value of arm_method JSON key in partition control command
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
//
// keys allowed in global options  JSON config
#define	GO_MAX_SLAVES_KEY_STR			"MaxSlaves"
#define	GO_RESTR_SPRVS_LOSS_KEY_STR		"RestrSprvsL"
#define	GO_RESTR_TAMPER_KEY_STR			"RestrTamper"
#define	GO_RESTR_AC_FAIL_KEY_STR		"RestrACfail"
#define	GO_RESTR_BAT_FAIL_KEY_STR		"RestrBatFail"
#define	GO_RESTR_BELL_KEY_STR			"RestrOnBell"
#define	GO_RESTR_BOARD_FAIL_KEY_STR		"RestrOnBrdFail"
#define	GO_RESTR_AMASK_KEY_STR			"RestrOnAmask"
#define	GO_TROUBLE_LATCH_KEY_STR		"TroubleLatch"
#define	GO_TAMPER_BPS_OPT_KEY_STR		"TamperBpsOpt"
#define	GO_TAMPER_OPTS_KEY_STR			"TamperOpts"
#define	GO_AMASK_OPTS_KEY_STR			"AntiMaskOpt"
#define	GO_RF_SPRVS_OPT_KEY_STR			"RfSprvsOpt"

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
	RESERVED = 0,						// offsets the IDs for the folloing items in order to match sbProps[] indexes TODO ???
	ZONES_CFG = 0x10,
	ZONES_CMD = 0x11,
	PARTITIONS_CFG = 0x20,
	PARTITIONS_CMD = 0x21,
	PGMS_CFG = 0x40,
	PGMS_CMD = 0x41,
	KEYSW_CFG = 0x80,
	KEYSW_CMD = 0x81,
	GLOBAL_OPT_CFG = 0x100,
	GLOBAL_OPT_CMD = 0x101,
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
	INVALID_CMD = -1,
	DISARM = 0,
	REGULAR_ARM = 0x1,
	FORCE_ARM = 0x2,
	INSTANT_ARM = 0x4,
	STAY_ARM = 0x8,
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