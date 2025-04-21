#pragma once
//
//
#define	MAX_ALARM_ZONES			256
#define MAX_ALARM_PGM			32				// 
#define NAME_LEN				32
#define MAX_KEYSW_CNT			32

//#define PGM_STATES_LEN			1							// contains bitmap of PGM states (PGM1->bit 0, PGM2->bit1) 
//#define SZONERES_LEN			(SLAVE_ZONES_CNT/2 + SLAVE_ZONES_CNT%2) // 
//#define POLL_RES_PAYLD_LEN		SZONERES_LEN+PGM_STATES_LEN	// poll payload is 4bits per zone  + one byte for PGMs states
//#define PGM_STATES_OFFSET		SZONERES_LEN				// PGMs states are stord in the last byte of the POLL payload                                          

//#define MAX_MQTT_TOPIC			256
//#define MAX_MQTT_PAYLOAD		32700

#define SW_VERSION			100				// UPDATE ON EVERY CHANGE OF ZONEs, PGMs, etc
//
// --------------- error codes -----------------//
enum errorID {
	ERR_INFO = 2,                             // just print if INFO is set
	ERR_OK = 0,                           	// no error
	ERR_CRITICAL = -1,                        // critical error occured, 
	ERR_DEBUG = -2, 							// debug print, can be enabled/disable by #define DEBUG
	ERR_WARNING = -3, 						// warning print  can be enabled/disable by #define WARNING
	ERR_DB_INDEX_NOT_FND = -10,				// cannot find error in the database
};
//
//
#define UNKNOWN_TTL "unknown"
//
// ----------- alarm-defs.h -------------------
//
#define MAX_PARTITION			8
#define MAX_FOLLOW_PART			8
//
// keys allowed in zone header in CSV config file
//
#define ZN_NAME_TTL					"zName"
#define ZN_BRD_ID_TTL				"zBRD"
#define ZN_ID_TTL					"zID"
#define ZN_TYPE_TTL					"zType"
#define ZN_PRT_TTL					"zPartn"
#define ZN_ALARM_TTL				"zAlarmT"
#define ZN_SHD_EN_TTL				"zShdnEn"
#define ZN_BPS_EN_TTL				"zBypEn"
#define ZN_STAY_TTL					"zStayZ"
#define ZN_FORCE_TTL				"zFrceEn"
#define ZN_INTEL_TTL				"zIntelZ"
#define ZN_DLYTRM_TTL				"zDlyTRM"
#define ZN_TMPR_GLB_TTL				"zTmprGlb"
#define ZN_TMPR_OPT_TTL				"zTmprOPT"
#define ZN_AMSK_GLB_TTL				"zAmskGlb"
#define ZN_AMSK_OPT_TTL				"zAmskOpt"
//
#define ZN_BYPASSED_TTL				"BYPASSED"
#define ZN_TAMPER_TTL				"TAMPER"
#define ZN_OPEN_TTL					"OPEN"
#define ZN_ERROR_TTL				"ERROR"
#define ZN_FORCED_TTL				"FORCED"
#define ZN_CLOSE_TTL				"CLOSED"
#define ZN_AMASK_TTL				"AMASK"
//
//
// keys allowed in partition header in CSV config file
//
#define	PT_NAME_TTL					"pName"
#define	PT_IDX						"pIdx"
#define	PT_FORCE_ON_REG				"pFrceOnRegArm"
#define	PT_FORCE_ON_STAY			"pFrceOnStayArm"
#define	PT_ED2_FOLLOW				"pED2znFollow"
#define	PT_ALRM_OUT_EN				"pAlrmOutEn"
#define	PT_ALRM_LENGHT				"pAlrmTime"
#define	PT_NO_CUT_ON_FIRE			"pNoCutOnFire"
#define	PT_ALARM_RECYCLE_TIME		"pAlrmRecTime"
#define	PT_ED1_INTERVAL				"pED1Intvl"
#define	PT_ED2_INTERVAL				"pED2Intvl"
#define	PT_EXIT_DELAY				"pExitDly"
#define	PT_VALID					"pValid"
#define	PT_FOLLOW_1					"pFollow1"
#define	PT_FOLLOW_2					"pFollow2"
#define	PT_FOLLOW_3					"pFollow3"
#define	PT_FOLLOW_4					"pFollow4"
#define	PT_FOLLOW_5					"pFollow5"
#define	PT_FOLLOW_6					"pFollow6"
#define	PT_FOLLOW_7					"pFollow7"
#define	PT_FOLLOW_8					"pFollow8"
//
// keys allowed in global options  header in CSV config file
//
#define	GO_MAX_SLAVES				"MaxSlaves"
#define	GO_RESTR_SPRVS_LOSS			"RestrSprvsL"
#define	GO_RESTR_TAMPER				"RestrTamper"
#define	GO_RESTR_AC_FAIL			"RestrACfail"
#define	GO_RESTR_BAT_FAIL			"RestrBatFail"
#define	GO_RESTR_BELL				"RestrOnBell"
#define	GO_RESTR_ON_BRDFAIL			"RestrOnBrdFail"
#define	GO_RESTR_AMASK				"RestrOnAmask"
#define	GO_TROUBLE_LATCH			"TroubleLatch"
#define	GO_TAMPER_BPS_OPT			"TamperBpsOpt"
#define	GO_TAMPER_OPTS				"TamperOpts"
#define	GO_AMASK_OPTS				"AntiMaskOpt"
#define	GO_RF_SPRVS_OPTS			"RfSprvsOpt"
//
// keys allowed in pgm header in CSV config file
//
#define	PGM_NAME					"pgmName"
#define	PGM_BRD						"pgmBrd"
#define	PGM_ID						"pgmID"
#define	PGM_PULSE_LEN				"pgmPulseLen"
#define	PGM_VALID					"pgmValid"
//
// general keys allowed
//
#define	KEY_NOT_APPLICABLE			"NA"
//
// strings allowed as values for zone type in zone config in CSV
//
#define	ZONE_DISABLED_TTL			"DISABLED"             
#define	INSTANT_TTL					"INSTANT"              		
#define	ENTRY_DELAY1_TTL			"ENTRY_DELAY1"         	
#define	ENTRY_DELAY2_TTL			"ENTRY_DELAY2"         	
#define	FOLLOW_TTL					"FOLLOW"               			
#define	STAY_DELAY1_TTL				"STAY_DELAY1"          
#define	STAY_DELAY2_ANTI_MASK_TTL	"STAY_DELAY2_ANTI_MASK"	
#define	H24_BUZZER_TTL				"H24_BUZZER"           
#define	H24_BURGLAR_TTL				"H24_BURGLAR"          
#define	H24_HOLDUP_TTL				"H24_HOLDUP"           
#define	H24_GAS_TTL					"H24_GAS"              
#define	H24_HEAT_TTL				"H24_HEAT"             
#define	H24_WATER_TTL				"H24_WATER"            
#define	H24_FREEZE_TTL				"H24_FREEZE"           
#define	H24_FIRE_DELAYED_TTL		"H24_FIRE_DELAYED"     
#define	H24_FIRE_STANDARD_TTL		"H24_FIRE_STANDARD"    
//
// strings allowed as values for alarm type in zone config in CSV
//
#define	STEADY_ALARM_TTL			"STEADY_ALARM"
#define	SILENT_ALARM_TTL			"SILENT_ALARM"
#define	PULSED_ALARM_TTL			"PULSED_ALARM"
#define	REPORT_ALARM_TTL			"REPORT_ALARM"
//
// strings allowed as values for line errors control in zone config in CSV
//
#define LINE_ERR_OPT_DISABLED_TTL			"DISABLE"
#define LINE_ERR_OPT_TROUBLE_ONLY_TTL		"TROUBLE_ONLY"
#define LINE_ERR_OPT_ALARM_WHEN_ARMED_TTL	"ALARM_WHEN_ARMED"
#define LINE_ERR_OPT_ALARM_TTL				"ALARM"
//
// here are the identifying string for different configurable items 
// the parser will look in CSV file for lines starting with these identificators and will process accordingly
//
#define RESERVED_TTL				SUBTOPIC_WILDCARD	//all' is reserved and cannot be used in CSV because of function to convert string to index
#define CSV_ZONE_TTL				"zone"
#define CSV_ZONE_HDR_TTL			"zHeader"
#define CSV_PGM_TTL					"pgm"
#define CSV_PGM_HDR_TTL				"pgmHdr"
#define CSV_PRT_TTL					"partition"
#define CSV_PRT_HDR_TTL				"pHeader"
#define CSV_GOPT_TTL				"globalOpt"
#define CSV_GOPT_HDR_TTL			"gOptHeader"
#define CSV_KEYSW_TTL				"keysw"
#define CSV_KEYSW_HDR_TTL			"kswHeader"
//
#define BACKUP						false
#define CHECK_4_CHANGE				true
#define SET_BYPASS					false
#define CLEAR_BYPASS				true
#define PRTCLS_LINE_ERR				2
#define PRTCLS_GENERAL				1
//
// FSM states to track command execution. For PGM for example:
// TO_IMPLEMENT means new PGM state received and it must be set.
// TO_VERIFY means it is set, but not read back (note for slaves this requires receiving PGMs statuses from slave
// TO_REPORT means we need to communicate the new state (printing, MQTT)
//
enum CMD_EXEC_t {
	ALL_DONE					= 0,
	TO_IMPLEMENT				= 1,
	TO_VERIFY					= 2,
	TO_REPORT					= 3,
};
//
enum PGM_CMDS_t {
	PGM_OFF						= 1,
	PGM_ON						= 2,
	PGM_PULSE					= 3,
};
//
// zone commands definitions
//
enum ZONE_CMDS_t {
	ZONE_BYPASS_CMD				= 1,    // keep BYPASS and UNBYPASS cmds as power of 2 as they can be or-ed in 
	ZONE_UNBYPASS_CMD			= 2,    // in zoneNewCmd TODO - check if this is correct
	ZONE_CLOSE_CMD				= 3,
	ZONE_OPEN_CMD				= 4,
	ZONE_AMASK_CMD				= 5,
	ZONE_TAMPER_CMD				= 6,
	ZONE_ANAL_SET_CMD			= 7,
	ZONE_DIGITAL_SET_CMD		= 8,
};
//
// zone definitions
//
enum ZONE_DEFS_t {
    ZONE_DISABLED				= 0x0 ,
    INSTANT						= 0x1 ,
	FOLLOW						= 0x2,
    ENTRY_DELAY1				= 0x4 ,
    ENTRY_DELAY2				= 0x8 ,
	STAY_DELAY1					= 0x10,
    STAY_DELAY2_ANTI_MASK		= 0x20,
    H24_BUZZER					= 0x40,
    H24_BURGLAR					= 0x80,
	// special zones follow - non bypassable
    H24_HOLDUP					= 0x100,
    H24_GAS						= 0x200,
    H24_HEAT					= 0x400,
    H24_WATER					= 0x800,
    H24_FREEZE					= 0x1000,
    H24_FIRE_DELAYED			= 0x2000,
    H24_FIRE_STANDARD			= 0x4000,
};
//
// Special zones are not bypassable, cannot be forced, etc. TODO - Not sure what H24_HOLDUP is
#define SPECIAL_ZONES			(H24_HOLDUP|H24_GAS|H24_HEAT|H24_WATER|H24_FREEZE|H24_FIRE_DELAYED|H24_FIRE_STANDARD)
#define EXIT_DELAY_ZONES		(INSTANT | FOLLOW | ENTRY_DELAY1 | ENTRY_DELAY2 | STAY_DELAY1 | STAY_DELAY2_ANTI_MASK)
#define ENTRY_DELAY1_ZONES		(FOLLOW | ENTRY_DELAY1 | STAY_DELAY1)
#define ENTRY_DELAY2_ZONES		(FOLLOW | ENTRY_DELAY2 | STAY_DELAY2_ANTI_MASK)
//
// bit masks for zone states, self explanatory 
#define ZONE_CLOSE				0 
#define ZONE_OPEN				0x1
#define ZONE_AMASK				0x4
#define ZONE_TAMPER				0x8
#define ZONE_ERROR				(ZONE_AMASK|ZONE_TAMPER)        // 0xC
//
// BYPASS DEFINITIONS !!! MAKE SURE FITS WITH BYPASS DEFS TO 8 BITS
#define ZONE_BYPASSED			0x80     
#define ZONE_FORCED				0x40    
#define ZONE_STAY_BYPASSED		0x20
#define ZONE_EDx_BYPASSED		0x02
#define ZONE_EX_D_BYPASSED		0x01   
#define ZONE_NO_BYPASS			0x00  
//
enum ZONE_STAT_CHANGES_t {
	ZONE_STATE_CHANGED			= 0x1,
	ZONE_BYPASS_CHANGED			= 0x2,
	ZONE_USR_BYPASS_CHANGED		= 0x4,
};
//
struct zoneStates_t {
	unsigned int state; 
	const char	stateTitle[NAME_LEN];
};
//
enum PRINT_CLASS_t	{
	PRTCLASS_ALL		= 0,
	PRTCLASS_LINE_ERR,
	PRTCLASS_GENERAL
};
//
// alarms and trubles
#define NO_TROUBLE						0x0
#define TROUBLE							0x1
//
#define NO_ALARM						0x0
#define STEADY_ALARM					0x1
#define PULSED_ALARM					0X2
#define SILENT_ALARM					0x4
#define REPORT_ALARM 					0x8
//
enum LINE_ERR_OPT_t {
	LINE_ERR_OPT_DISABLED				= 0,
	LINE_ERR_OPT_TROUBLE_ONLY			= 0x1,
	LINE_ERR_OPT_ALARM_WHEN_ARMED		= 0x2,
	LINE_ERR_OPT_ALARM					= 0x3,
};
//
enum PARTITIONS {
    PARTITION1			= 0,
    PARTITION2			= 1,
    PARTITION3			= 2,
    PARTITION4			= 3,
    PARTITION5			= 4,
    PARTITION6			= 5,
    PARTITION7			= 6,
    PARTITION8			= 7,
};
//
//  bitmaps of what changed in partition, used for MQTT reporting
//
enum PART_STAT_CHANGES_t {
	CHG_NO_CHANGE			= 0,
	CHG_ENTRY_DELAY1_TIMER	= 1,
	CHG_ENTRY_DELAY2_TIMER	= 2,
	CHG_EXIT_DELAY_TIMER	= 4,
	//CHG_PARTITION_STATS	= 8,
	CHG_ZONE_CHANGED		= 16,
	CHG_NEW_CMD				= 32,
};
//
//	partition timers ID - EXIT DELAY, ENTRY_DELAY1,  ENTRY_DELAY2
#define EXIT_DELAY_TIMER		0
#define ENTRY_DELAY1_TIMER		1
#define ENTRY_DELAY2_TIMER		2
//
#define	MAX_PARTITION_TIMERS	3		
//
enum  ARM_METHODS_t {
    DISARM = 0,
	REGULAR_ARM			= 0x1,
	FORCE_ARM 			= 0x2,
    INSTANT_ARM			= 0x4,                                    
    STAY_ARM			= 0x8,
};
//
enum  BYPASS_REQ_t {
	USER_REQ			= 0,
	REGULAR_ARM_REQ		= 0x1,
	FORCE_ARM_REQ		= 0x2,
	INSTANT_ARM_REQ		= 0x4,
	STAY_ARM_REQ		= 0x8,
};
//
// Keysw
//
enum  KEYSW_OPTS_t {
    ENABLED 						= 0x1,                            
    MAINTAINED           			= 0x2, 				 // MOMENTARY = 0; MAINTAINED = 1  
    GEN_UTL_KEY_ON_OPEN_AND_CLOSE	= 0x4,         // GEN_UTL_KEY_ON_OPEN_AND_CLOSE = 1; ON_OPEN_ONLY = 0;
};
//
enum  KEYSW_ACTS_t {
    DISARM_ONLY						= 0x1,                            
    STAY_INSTANT_DISARM_ONLY, 				 
    ARM_ONLY,
	REGULAR_ARM_ONLY,
	STAY_ARM_ONLY,
	FORCE_ARM_ONLY,
	INSTANT_ARM_ONLY,
};
//
enum entryDelay_t {
	NOT_STARTED						= 0,
	RUNNING							= 0x1,
	DONE							= 0x2,
};
//
// alarm zones structure to hold all alarm zones related info
//
struct ALARM_ZONE {
	uint16_t  valid;				// TODO - why uint16_t?
	char	zoneName[NAME_LEN];		// user friendly name
	byte	boardID;
	byte	zoneID;
	uint16_t zoneType;				// zone type - enable, entry delay, follow, instant, stay, etc see ZONE_DEFS_t
	byte	zonePartition;			// assigned to partition X
	byte	zoneAlarmType;			// zone alarm type - steady, pulsed, etc
	byte	zoneShdwnEn;
	byte	zoneBypassEn;
	byte	zoneStayZone;
	byte	zoneForceEn;
	byte	zoneIntelizone;
	byte	delayTrm;
	byte	zoneTamperFpanel;		// on tamper follow panel settings
	byte	zoneTamperOpts;			// zone tamper options - when/how to generate alarm when tamper
	byte	zoneAmaskFpanel;		// on antimask follow panel settings
	byte	zoneAmaskOpts;			// zone tamper options - when/how to generate alarm when tamper
};
//
struct ALARM_ZONE_RT {				// zone's run time staff
	byte  	zoneStat;               // status of the zone switch. (open, close, short, line break
	byte  	bypassed;			    // true if zone is bypassed, contains bitmas of bytass source(s)
	byte    changed;				// butmap of what chaned in zone (so far status open/close + tamper + amask + bypass)
	byte  	in_alarm;				// zone trigerred alarm
	byte  	in_trouble;				// zone trigerred trouble
	byte  	ignorredTamper;			// zone with ignorred tamper
	byte  	ignorredAmask;			// zone with ignorred anti-mask
	byte  	openEDSD1zone;			// open ED1/SD1 zone in ED1 interval
	byte  	openEDSD2zone;			// open ED2/SD2 zone in ED2 interval
};      
//
// alarm pgms records structure to hold all alarm pgms related info
struct ALARM_PGM {
	uint16_t  valid;				// TODO - why uint16_t?
	byte	boardID;
	byte	pgmID;					// 0 based index, for MASTER is 0-7, for SLAVE is 0 or 1
	byte    pgmFSM;					// holds state number of command implementation state machine see enum CMD_EXEC_t
	byte	cValue;             	// current value
	byte	tValue;             	// target  value
	byte	sValue;            		// slave   value (this the value returned from Slave as part of POLL_ZONES)
	byte	pulseLen;				// TODO
	char	pgmName[NAME_LEN];		// user friendly name
};
//
// keyswitch related staff
struct ALARM_KEYSW {
	uint16_t valid;					// TODO - why uint16_t?	
	byte  partition;				// Keyswitch can be assigned to one partition only. If == NO_PARTITION (0) the keyswitch is not defined/valid
	byte  type;						// disabled, momentary, maintained,  generate utility key on open/close, .... see enum  KEYSW_OPTS_t 
	byte  action;					// keyswitch action definition - see enum  KEYSW_ACTS_t 
	byte  boardID;					// the board of which zone will e used as keyswitch belong. Master ID is 0	
	byte  zoneID;					// the number of zone that will e used as keyswitch
	char  keyswName[NAME_LEN];			// user friendly name
};  
//
struct ALARM_GLOBAL_OPTS_t {		// TODO - add SW version
	byte maxSlaveBrds;				// how many slaves are installed. Run-time this value is copied to maxSlaves
	byte restrOnSprvsLoss;
	byte restrOnTamper;             // default ON
	byte restrOnACfail;
	byte restrOnBatFail;            // default ON
	byte restrOnBellFail;
	byte restrOnBrdFail;
	byte restrOnAntimask;
	byte troubleLatch;				// if trouble, latch it or not
	byte tamperBpsOpt;				// true - if zone is bypassed ignore tamper; false - follow global or local tamper settings
	byte tamperOpts;				// global tamper optons, same as local - see #define ZONE_TAMPER_OPT_XXXXXX
	byte antiMaskOpt;				// global antimask optons, same as local - see #define ZONE_ANTI_MASK_SUPERVISION__XXXXXX
	byte rfSprvsOpt;				// wireless sensors supervision see RF_SUPERVISION_XXXX
//	unsigned long entryDelay1Start;	//to store the time when entry delay 1 zone opens
//	unsigned long entryDelay2Start; //to store the time when entry delay 2 zone opens
	byte SprvsLoss;
	byte ACfail;
	byte BatFail;
	byte BellFail;
	byte BrdFail;
};
//
// alarm partition 
struct ALARM_PARTITION_t {
	// configuration data
	uint16_t valid;					// partition is valid if true
	byte partIdx;					// partition index (barely used)
	byte forceOnRegularArm;			// allways use force arm (bypass open zones) when regular arming
	byte forceOnStayArm;			// allways use force arm (bypass open zones) when stay arming
	byte followEntryDelay2;			// if and entry delay zone is bypassed and follow zone is opens, the alarm will be postponed by EntryDelay2 
	byte alarmOutputEn;				// enable to triger bell or siren once alarm condition is detected in partition
	byte alarmCutOffTime;			// cut alarm output after 1-255 seconds
	byte noCutOffOnFire;			// disable cut-off for fire alarms
	byte alarmRecycleTime;			// re-enable after this time if alarm condition not fixed
	byte entryDelay1Intvl;			// entry delay 1 delay in seconds 1-255
	byte entryDelay2Intvl;			// entry delay 2 delay in seconds 1-255
	byte exitDelay;					// exit delay in seconds 1-255
	char partitionName[NAME_LEN];	// user readable name
	// array of bytes, each contains partition  ID, WARNING - partitionDB[0] is has ID 1, partitionDB[1] is 2, etc
	// this is done because when initialized, default value of 0 will means invalid follow-ing partition 
	byte follows[MAX_PARTITION];	
};
//
struct partitionTimer_t {
	byte	timerFSM;				// current state of timer FSM.States are : NOT_STARTED, RUNNING, DONE
	byte	timerDelay;				// delay in seconds 1-255
	byte	changedMask;			// bitmask to indicate that FSM state has changed, used for MQTT reporting
	byte	bypassWhat;				// zones  types bypassed when the timer is running: EXIT_DELAY_ZONES, ENTRY_DELAY1_ZONES, ...
	byte	bypassMask;				// bitmask to use to set/clear in zoneXXX.bypassed: ZONE_EDx_BYPASSED, 	ZONE_EX_D_BYPASSED, ...
	unsigned long timerStart_ms;	// timer start time in mS
};
//
//
struct ALARM_PARTITION_RUN_TIME_t {
	byte newCmd;					// new partition cmd received
	byte armStatus;					// current arm status: DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM 
	byte targetArmStatus;			// set when arm command received:DISARM = 0, REGULAR_ARM, FORCE_ARM, INSTANT_ARM, STAY_ARM 
	unsigned long armTime;			// arm time
	byte changed;					// contains bitmap of the reaso for partition change see enum PART_STAT_CHANGES_t 
	struct partitionTimer_t partitionTimers[MAX_PARTITION_TIMERS] = {{NOT_STARTED,0,CHG_EXIT_DELAY_TIMER,	EXIT_DELAY_ZONES,	ZONE_EX_D_BYPASSED, 0},
																	 {NOT_STARTED,0,CHG_ENTRY_DELAY1_TIMER,	ENTRY_DELAY1_ZONES, ZONE_EDx_BYPASSED, 0},
																	 {NOT_STARTED,0,CHG_ENTRY_DELAY2_TIMER,	ENTRY_DELAY2_ZONES, ZONE_EDx_BYPASSED, 0}};
};
// 
//
struct ALARM_PARTITION_STATS_t {
	byte openZonesCnt;				// count of open zones
	byte bypassedZonesCnt;			// count of bypassed zones
	byte openZonesCntEDSD1;			// count of Entry Delay 1 open zones
	byte openZonesCntEDSD2;			// count of Entry Delay 1 open zones
	byte notBypassedEntyDelayZones; // used to triger follow zones to use ENTRY_DELAY2 if no more notBypassedEntyDelayZones 
	byte tamperZonesCnt;			// count of zones with tamper 
	byte ignorredTamperZonesCnt;	// count of ignorred zones with tamper  
	byte amaskZonesCnt;				// count of zones with  anti-mask 
	byte ignorredAmaskZonesCnt;		// count of ignorred zones  and anti-mask  
	byte alarmZonesCnt;				// count of zones causing alarms
};

// -------------- copied from various files -------------

#define ALARM_LOOP_INTERVAL		1000
#define STATUS_REPORT_INTERVAL	30000
#define	ALARM_PUBLISH_INTERVAL	500


// ------------------------ this code is copied to alarmClass.h ------------------------
	//// 
	//// top level desctribing boards 
	//struct ALARM_BRD_t {
	//	byte valid;						// board is present if true
	//	byte brdFail;					// true if last comm with board failed
	//	unsigned long totalErrors;		// total errors in comm
	//};
	////
	//// flags used in csv parsing. used to mark that global options and header lines are imported in order to import data lines properly
	//unsigned long	csvParserFlags = 0;

	//
	// zoneDB - database with all zones (master&slaves) CONFIG info. 
	// zonesRT = all run time zones data. Info from slaves are fetched via pul command over RS485
	//struct ALARM_ZONE zonesDB[MAX_ALARM_ZONES];
	//struct ALARM_ZONE_RT zonesRT[MAX_ALARM_ZONES];
	////
	//// MASTER PGMs organized as 1D array - MASTER_PGM_CNT followed by MAX_SLAVES * SLAVE_PGM_CNT
	//struct ALARM_PGM pgmsDB[MAX_ALARM_PGM];
	////
	//// alarm keysw records structure to hold all alarm pgms related info     
	//struct ALARM_KEYSW keyswDB[MAX_KEYSW_CNT];
	////
	//// alarm global options storage
	//struct ALARM_GLOBAL_OPTS_t  alarmGlobalOpts;
	//
	//// alarm partition options storage
	//struct ALARM_PARTITION_t					partitionDB[MAX_PARTITION];		// Partitions CONFIG data
	//struct ALARM_PARTITION_RUN_TIME_t			partitionRT[MAX_PARTITION];		// Partitions Entry Delay data
	//struct ALARM_PARTITION_STATS_t				partitionSTATS[MAX_PARTITION];	// Partitons Run-Time statistics
	//
	// TODO - use it
	//struct ALARM_BRD_t brdsDB[MAX_SLAVES + 1]; // for each board, includding master
	// 
	// 
// ------------------------ end this code is copied to alarmClass.h ------------------------
// 
// ------------------------ shall not be used  --------------------------
//// struct describing parameters for array representing specific alarm domain like zones, partitions, pgms, etc
//struct dbProps_t {
//	const char  title[NAME_LEN];			// string to identify for what are the data of CSV line (needed by CSV parser), not used
//	const char*	dbBaseAddr;					// base address of the DB (zonesDB, pgmsDB, keyswDB,..)
//	int			elmCnt;						// number of entries (elements) in DB
//	int			elmLen;						// length of one element in bytes (stride)
//	int			nameOffs;					// offstet of the name of entry (zone name, part name, ...)
//	int			validOffs;					// offstet of the valid flag of entry. !0 means valid
//};
////
//// indexes in dbPtrArr[] to find key parameters for the different alarm entities (zones, part, pgms,.. )
//enum ALARM_DOMAINS_t {
//	RESERVED	= -1,						// offsets the IDs for the folloing items in order to match sbProps[] indexes TODO ???
//	ZONES		= 0,
//	PARTITIONS	= 1,
//	PGMS		= 2,
//	KEYSW		= 3,
//	GLOBAL_OPT	= 4,
//};
////
//// array containing parameters for data(bases) for all alarm domains. used to get runtime parameters of the domains
////
//struct dbProps_t dbPtrArr[] = {  //{NULL,				NULL,						0,				0,						0													,0										 	},
//								{CSV_ZONE_TTL,		(const char*)&zonesDB,			MAX_ALARM_ZONES,sizeof(zonesDB[0]),		offsetof(struct ALARM_ZONE, zoneName)				,offsetof(struct ALARM_ZONE, zoneType)	 	},
//								{CSV_PRT_TTL,		(const char*)&partitionDB,		MAX_PARTITION,	sizeof(partitionDB[0]),	offsetof(struct ALARM_PARTITION_t, partitionName)	,offsetof(struct ALARM_PARTITION_t, valid)  },
//								{CSV_PGM_TTL,		(const char*)&pgmsDB,			MAX_ALARM_PGM,	sizeof(pgmsDB[0]),		offsetof(struct ALARM_PGM, pgmName)					,offsetof(struct ALARM_PGM, valid)		 	},
//								{CSV_KEYSW_TTL,		(const char*)&keyswDB,			MAX_KEYSW_CNT,	sizeof(keyswDB[0]),		offsetof(struct ALARM_KEYSW,keyswName)				,offsetof(struct ALARM_KEYSW, valid)		},
//								{CSV_GOPT_TTL,		(const char*)&alarmGlobalOpts,	1,				sizeof(alarmGlobalOpts),0													,0										 	},
//};
//#define DB_PTR_ARR_CNT (sizeof(dbPtrArr)/sizeof(struct dbProps_t))
//
// // tmp storage used in parserHelpers
//union maxTmp_t {
//	struct ALARM_ZONE           tmpZn;
//	struct ALARM_PGM            tmpPgm;
//	struct ALARM_GLOBAL_OPTS_t  tmpGopts;
//	struct ALARM_PARTITION_t    tmpPart;
//} maxTmp;
////
//// array of zone states as integer:string couples. used mainly for printing 
//struct  zoneStates_t zoneStatesTitles[] = {
//	{ZONE_BYPASSED,  ZN_BYPASSED_TTL},		{ZONE_FORCED,  ZN_FORCED_TTL},
//	{ZONE_TAMPER,    ZN_TAMPER_TTL	},		{ZONE_CLOSE,   ZN_CLOSE_TTL	},
//	{ZONE_OPEN,      ZN_OPEN_TTL	},		{ZONE_AMASK,   ZN_AMASK_TTL	},
//	{ZONE_ERROR,	 ZN_ERROR_TTL	},
//};
//#define ZONE_STATES_TITLE_CNT (sizeof(zoneStatesTitles)/sizeof(struct zoneStates_t))
// 
// char 		token[256];		//used in parser to store tokenized string
// ------------------------ end shall not be used  --------------------------







