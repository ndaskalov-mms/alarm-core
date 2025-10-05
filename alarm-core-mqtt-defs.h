#pragma once
//	MQTT defs
#define _CRT_SECURE_NO_WARNINGS
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif

//
//	MQTT defs
//
#define	MQTT_PREFIX						"/pdox"
#define	MQTT_CLIENT_NAME				"ESP32AlarmPanelClient"
//
#define TRUE_PAYLOAD	                "true"
#define FALSE_PAYLOAD	                "false"

#define MAX_MQTT_TOPIC					256
#define MAX_MQTT_PAYLOAD				32700
#define MAX_MQTT_TOKEN_LEN					32	// add some padding for \0
#define	SUBTOPIC_WILDCARD			"all"

//
// Partitions 
//
// topics
#define MQTT_PARTITIONS_CONTROL			"/pdox/control/partitions/%s"
#define PARTITIONS_STATES_TOPIC			"/pdox/states/partitions/%s/%s"
#define	SUBTOPIC_WILDCARD				"all"
// payloads
#define ARM_PAYLOAD		                "true"
#define DISARM_PAYLOAD	                "false"
#define ARM_PROPERTY	                "arm"
#define FORCE_ARM_PROPERTY              "force_arm"
#define STAY_ARM_PROPERTY               "stay_arm"
#define CURRENT_STATE_PROPERTY          "current_state"
#define EXIT_DELAY_PROPERTY             "exit_delay"
#define EXIT_DELAY_FINISHED_PROPERTY    "exit_delay_finished"
#define ENTRY_DELAY_PROPERTY            "entry_delay"
#define ENTRY_DELAY_FINISHED_PROPERTY   "entry_delay_finished"
#define CUR_STATE_ARMED_AWAY			"armed_away"
#define CUR_STATE_ARMED_HOME			"armed_home"
#define CUR_STATE_DISARMED				"disarmed"
#define CUR_STATE_ARMING				"arming"
#define CUR_STATE_ARMED_FORCE			"armed_force"
//
// Zones
//
#define MQTT_ZONES_CONTROL	            "/pdox/control/zones/%s"
#define ZONES_STATES_TOPIC              "/pdox/states/zones/%s/%s"
#define BYPASS_PROPERTY                 "bypassed"
#define TROUBLE_PROPERTY                "trouble"
#define TAMPER_PROPERTY                 "tamper"
#define ANTIMASK_PROPERTY               "antimask"
#define OPEN_PROPERTY                   "open"
#define OPEN_ZONES_PROPERTY             "open_zones"
#define TAMPER_ZONES_PROPERTY           "tamper_zones"
#define ANTIMASK_ZONES_PROPERTY         "antimask_zones"
#define OPEN_ED_ZONES_PROPERTY			"open_EDSD1_zones"
#define OPEN_SD_ZONES_PROPERTY			"open_EDSD2_zones"
#define	BYPASSED_ZONES_PROPERTY			"bypassed_zones"
#define IGNORRED_TAMPER_ZONES_PROPERTY	"ignorred_tampers"
#define IGNORRED_AMASK_ZONES_PROPERTY	"ignorred_amasks"
#define	ALARM_ZONES_PROPERTY			"zones_in_alarm"
#define NOT_BYPASSED_ED_ZONES_PROPERTY	"not_bypassed_ED_zones_cnt"
//
// OUTPUTS (PGM)
//
#define MQTT_OUTPUTS_CONTROL	        "/pdox/control/outputs/%s"
#define MQTT_OUTPUTS_STATES	        "/pdox/states/outputs/%s"
#define PGM_ON_PROPERTY                 "on"
#define PGM_OFF_PROPERTY                "off"
// 
// metrics exchange
//
#define MQTT_METRICS_EXCHANGE           "/pdox/metrics/exchange"
//
#define MQTT_GLOBAL_OPT_CONTROL		"/pdox/settings/global_options/%s"

//
#define ALARM_PROPERTY                  "alarm"
//
#define		MQTT_LAST_WILL				"/pdox/lastwill"
//
const char  willTopic[] = MQTT_LAST_WILL;		// the topic to be used by the will message
int         willQoS = 0;						// 0, 1 or 2 - the quality of service to be used by the will message
bool        willRetain = false;					// whether the will should be published with the retain flag
const char  willMessage[] = "disconnecting";	//the payload of the will message
//


//#define MQTT_PARTITIONS_CONTROL			(MQTT_PREFIX ## "/control/partitions/%s")
//#define MQTT_PARTITIONS_STATE				(MQTT_PREFIX ## "/states/partitions/%s/current_state")
//#define PARTITIONS_STATES_TOPIC			(MQTT_PREFIX ## "/states/partitions/%s/%s")
//#define MQTT_PARTITIONS_STATE				"/pdox/states/partitions/%s/current_state"

//#define MQTT_ZONES_CONTROL	            MQTT_PREFIX //## "/control/zones/%s"
//#define ZONES_STATES_TOPIC				MQTT_PREFIX //## "/states/zones/%s/%s"

//#define MQTT_OUTPUTS_CONTROL				MQTT_PREFIX //## "/control/outputs/%s"
//#define MQTT_OUTPUTS_STATES	            MQTT_PREFIX //## "/states/outputs/%s"

//#define MQTT_METRICS_EXCHANGE				MQTT_PREFIX //## "/metrics/exchange"

//#define MQTT_GLOBAL_OPT_CONTROL			MQTT_PREFIX //## "/settings/global_options/%s"

//#define		MQTT_LAST_WILL				MQTT_PREFIX //## "/lastwill"

#ifdef HASS_INTEGRATION
#define HASS_IDENTITY               "babaceca"
#define HASS_ID_MANUFACTURER        "pdox"
#define HASS_ID_MODEL               "EVO192"
#define HASS_ID_NAME                "EVO192"
#define HASS_ID_SW_VERS             "0.7"
//
#define HASS_AVAILABILITY_TOPIC     MQTT_PREFIX //## "/interface/availability"
//
#define HASS_AVAILABILITY           "\"availability_topic\": \"%s\",\n"                 // "paradox/interface/availability"
//
#define HASS_BIN_SENSOR             "homeassistant/binary_sensor/%s/%s/config"          // params: identity name - e.g. homeassistant/binary_sensor/0501b88a/Holl_Vrata_MUK_R/config
#define HASS_ALARM_PANEL            "homeassistant/alarm_control_panel/%s/%s/config"    // params: identity name - e.g. homeassistant/binary_sensor/0501b88a/Holl_Vrata_MUK_R/config
//
#define HASS_DEVICE                 "\"device\": {\n\t\t\"identifiers\": [\n\t\t\t\"%s\",\n\t\t\t\"%s\",\n\t\t\t\"%s\"\n\t\t],\n\t\t\"manufacturer\":  \"%s\",\n\t\t\"model\": \"%s\",\n\t\t\"name\": \"%s\",\n\t\t\"sw_version\": \"%s\"\n\t},"
//                                                                    HASS_ID_MANUFACTURER, HASS_ID_MODEL, HASS_IDENTITY,                 HASS_ID_MANUFACTURER,           HASS_ID_MODEL,         HASS_ID_NAME,                HASS_ID_SW_VERS
#define HASS_CLASS_ZONE             "motion" 
//
#define HAZZ_ZONE_DETAILS           "\"device_class\": \"%s\",\n\t\"name\": \"%s\",\n\t\"payload_off\": \"%s\",\n\t\"payload_on\": \"%s\",\n\t\"state_topic\": \"" //## ZONES_STATES_TOPIC ## "\",\n\t\"unique_id\": \"%s_zone_%s_%s\""
//
#define HASS_COMMAND_TOPIC          "\"command_topic\": \"" //## MQTT_PARTITIONS_CONTROL ## "\""
#define HASS_ALARM_PANEL_DETAILS    "\"name\": \"%s\",\n\t\"payload_arm_away\" : \"%s\",\n\t\"payload_arm_home\" : \"%s\",\n\t\"payload_arm_force\" : \"%s\",\n\t\"payload_disarm\" : \"%s\", \n\t\"state_topic\": \"" //## MQTT_PARTITIONS_STATE ## "\",\n\t\"unique_id\": \"%s_partition_%s\""
#endif