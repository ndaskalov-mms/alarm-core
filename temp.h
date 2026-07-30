// alarm-core-public-defs.h
// Zone status supported JSON values 
//#define ZN_BYPASSED_VAL					"BYPASSED"


// ------------   ZONE RUNTIME STATUS JSON KEYS and VALS ------------------

// Zone bypass status key and vals
#define ZN_BYPASSED_KEY_STR				"bypassed"
#define ZN_NO_BYPASS_VAL				"NO_BYPASS"
#define ZN_STAY_BYPASSED_VAL			"STAY_BYP"
#define ZN_EDx_BYPASSED_VAL				"ENT_DLY_BYP"
#define ZN_EX_D_BYPASSED_VAL			"EXIT_DLY_BYP"
#define ZN_FORCED_VAL					"FORCED"

#define ZN_ZONE_STAT_KEY_STR			"zoneStat"
#define ZN_OPEN_VAL						"OPEN"
#define ZN_ERROR_VAL					"ERROR"
#define ZN_CLOSE_VAL					"CLOSED"
	

#define ZN_ERROR_KEY_STR				"ERROR"
#define ZN_TAMPER_VAL					"TAMPER"
#define ZN_AMASK_VAL					"AMASK"

// ------------   ZONE RUNTIME STATUS JSON KEYS ------------------

#define ZN_IN_ALARM_KEY_STR				"inAlarm"		// val - YES if zone is in Alarm, NO if not in Alarm
#define ZN_IN_TROUBLE_KEY_STR			"inTrouble"		// val - YES if zone is in Trouble, NO if not in Trouble
#define ZN_IGNORED_TAMPER_KEY_STR		"ignTamper"		// val - YES if zone has Ignored Tamper, NO if not has Ignored Tamper
#define ZN_IGNORED_AMASK_KEY_STR		"ignAmask"		// val - YES if zone has Ignored Anti-Mask, NO if not has Ignored Anti-Mask
#define ZN_OPEN_EDSD1_ZONE_KEY_STR		"openEDSD1zn"	// val - YES if zone is in Open ED1/SD1 Zone, NO if not in Open ED1/SD1 Zone
#define ZN_OPEN_EDSD2_ZONE_KEY_STR		"openEDSD2zn"	// val - YES if zone is in Open ED2/SD2 Zone, NO if not in Open ED2/SD2 Zone

// zone status example JSON object
//{
//	ZN_ZONE_STAT_KEY_STR:		ZN_OPEN_VAL,		// ZN_CLOSE_VAL,  ZN_ERROR_VAL
//	ZN_BYPASSED_KEY_STR:		ZN_NO_BYPASS_VAL,	// ZN_STAY_BYPASSED_VAL, ZN_EDx_BYPASSED_VAL, ZN_EX_D_BYPASSED_VAL, ZN_NO_BYPASS_VAL, ZN_FORCED_VAL	
//	ZN_ERROR_KEY_STR:			ZN_NO_ERROR_VAL,	// ZN_TAMPER_VAL, ZN_AMASK_VAL
//	ZN_IN_ALARM_KEY_STR: 		YES,				// NO
//  ZN_IN_TROUBLE_KEY_STR:		YES,				// NO
//  ZN_IGNORED_TAMPER_KEY_STR:	YES,				// NO
//  ZN_IGNORED_AMASK_KEY_STR:	YES,				// NO
//	ZN_OPEN_EDSD1_ZONE_KEY_STR:	YES,				// NO
//	ZN_OPEN_EDSD2_ZONE_KEY_STR:	YES,				// NO
//}

//// bit masks for zone states, self explanatory 
//#define ZONE_CLOSE				0 
//#define ZONE_OPEN					0x1
//#define ZONE_AMASK				0x4
//#define ZONE_TAMPER				0x8
//#define ZONE_ERROR				(ZONE_AMASK|ZONE_TAMPER)        // 0xC
////
//// BYPASS DEFINITIONS !!! MAKE SURE FITS WITH BYPASS DEFS TO 8 BITS
//#define ZONE_BYPASSED			0x80     
//#define ZONE_FORCED				0x40    
//#define ZONE_STAY_BYPASSED		0x20
//#define ZONE_EDx_BYPASSED			0x02
//#define ZONE_EX_D_BYPASSED		0x01   
//#define ZONE_NO_BYPASS			0x00  

////
//struct ALARM_ZONE_RT {				// zone's run time staff
//	byte  	zoneStat;               // status of the zone switch. (open, close, short, line break
//	byte  	bypassed;			    // true if zone is bypassed, contains bitmas of bytass source(s)
//	byte    changed;				// butmap of what chaned in zone (so far status open/close + tamper + amask + bypass)
//	byte  	in_alarm;				// zone trigerred my_alarm
//	byte  	in_trouble;				// zone trigerred trouble
//	byte  	ignorredTamper;			// zone with ignorred tamper
//	byte  	ignorredAmask;			// zone with ignorred anti-mask
//	byte  	openEDSD1zone;			// open ED1/SD1 zone in ED1 interval
//	byte  	openEDSD2zone;			// open ED2/SD2 zone in ED2 interval
//};   




// zone status example JSON object
{
	ZN_ZONE_STAT_KEY_STR:		ZN_OPEN_VAL,		// ZN_CLOSE_VAL,  ZN_ERROR_VAL
	ZN_BYPASSED_KEY_STR:		ZN_NO_BYPASS_VAL,	// ZN_STAY_BYPASSED_VAL, ZN_EDx_BYPASSED_VAL, ZN_EX_D_BYPASSED_VAL, ZN_NO_BYPASS_VAL, ZN_FORCED_VAL	
	ZN_ERROR_KEY_STR:			ZN_NO_ERROR_VAL,	// ZN_TAMPER_VAL, ZN_AMASK_VAL
	ZN_IN_ALARM_KEY_STR: 		YES,				// NO
	ZN_IN_TROUBLE_KEY_STR:		YES,				// NO
	ZN_IGNORED_TAMPER_KEY_STR:	YES,				// NO
	ZN_IGNORED_AMASK_KEY_STR:	YES,				// NO
	ZN_OPEN_EDSD1_ZONE_KEY_STR:	YES,				// NO
	ZN_OPEN_EDSD2_ZONE_KEY_STR:	YES,				// NO
}
