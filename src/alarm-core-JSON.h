#pragma once
#include <stdio.h>
#include <string.h>
#include "..\alarm-core.h"
#include "alarm-core-internal-defs.h" // Include the internal definitions
#include "..\..\esp-json-parser\include\json_parser-code.h"

class alarmJSON {
public:
    /**
     * @brief Constructor that takes a reference to the Alarm instance to be populated.
     * @param alarm The Alarm object to configure.
     */
    alarmJSON(Alarm& alarm) : m_alarm(alarm) {}

    /**
     * @brief Parses the provided JSON configuration string and populates the Alarm object.
     * @param jsonString The null-terminated string containing the JSON configuration.
     * @return 0 on success, -1 on failure.
     */

    int parseConfigJSON(char* jsonBuffer)
    {
		jparse_ctx_t jctx, tmp_jctx;    // JSON parsing context
		int num_elem; 				    // number of elements in arrays     

        // Initialize JSON parser
		int ret = json_parse_start(&jctx, jsonBuffer, strlen(jsonBuffer)); 
        if (ret != OS_SUCCESS) {
            printf("Parser failed\n");
            return -1;
        }

        // Parse and display global options
        if (json_obj_get_object(&jctx, "globalOptions") == OS_SUCCESS) {
            parse_global_options(&jctx);
            json_obj_leave_object(&jctx);
        }

        // Parse and display zones
        if (json_obj_get_array(&jctx, "zones", &num_elem) == OS_SUCCESS) {
            printf("\n===== Zones (%d) =====\n", num_elem);
            tmp_jctx = jctx;                                // needed to properly parse arrays
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    printf("\nZone %d:\n", i + 1);
                    parse_zone(&jctx);
                    json_obj_leave_object(&jctx);
                }
                else
                    printf("\nInvalid Zone %d:\n", i + 1);

            }
            json_obj_leave_array(&jctx);
        }

        // Parse and display partitions
        if (json_obj_get_array(&jctx, "partitions", &num_elem) == OS_SUCCESS) {
            printf("\n===== Partitions (%d) =====\n", num_elem);
            tmp_jctx = jctx;
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    printf("\nPartition %d:\n", i + 1);
                    parse_partition(&jctx);
                    json_obj_leave_object(&jctx);
                }
            }
            json_obj_leave_array(&jctx);
        }

        // Parse and display pgms
        if (json_obj_get_array(&jctx, "pgms", &num_elem) == OS_SUCCESS) {
            printf("\n===== PGMs (%d) =====\n", num_elem);
            tmp_jctx = jctx;
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    printf("\nPGM %d:\n", i + 1);

                    //if (json_obj_get_string(&jctx, "pgmName", str_val, sizeof(str_val)) == OS_SUCCESS)
                    //    printf("  Name: %s\n", str_val);

                    //if (json_obj_get_int(&jctx, "pgmID", &int_val) == OS_SUCCESS)
                    //    printf("  ID: %d\n", int_val);

                    //if (json_obj_get_int(&jctx, "pgmPulseLen", &int_val) == OS_SUCCESS)
                    //    printf("  Pulse Length: %d seconds\n", int_val);

                    json_obj_leave_object(&jctx);
                }
            }
            json_obj_leave_array(&jctx);
        }

        // Parse and display keyswitches
        if (json_obj_get_array(&jctx, "keyswitches", &num_elem) == OS_SUCCESS) {
            printf("\n===== Keyswitches (%d) =====\n", num_elem);
            tmp_jctx = jctx;
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    printf("\nKeyswitch %d:\n", i + 1);

                    //if (json_obj_get_string(&jctx, "kswName", str_val, sizeof(str_val)) == OS_SUCCESS)
                    //    printf("  Name: %s\n", str_val);

                    //if (json_obj_get_string(&jctx, "type", str_val, sizeof(str_val)) == OS_SUCCESS)
                    //    printf("  Type: %s\n", str_val);

                    //if (json_obj_get_string(&jctx, "action", str_val, sizeof(str_val)) == OS_SUCCESS)
                    //    printf("  Action: %s\n", str_val);

                    json_obj_leave_object(&jctx);
                }
            }
            json_obj_leave_array(&jctx);
        }

        json_parse_end(&jctx);
        return 0;
    }

private:
    // Reference to the Alarm instance we are populating
    Alarm& m_alarm;

    /**
     * @brief Parses a single field (STRING, INT, BOOL) from the JSON object based on
      expected from the current JSON processor. JSON processor is selected based on particular JSON key.
	  For example, for zoneName key, we expect string value, for zoneType - int value, etc.
	  jsonValProcessor struct and all processors are defined in alarm-core-json-val-parsers.h
     */
    bool parseJSONval(jparse_ctx_t* jctx, const jsonValProcessor& processor, const char* print_prefix, parsedValue* result) {

        // Use a local buffer for parsing and printing.
		char str_val[NAME_LEN];                 // Temporary buffer for string values
		int int_val;                            // Temporary variable for integer values
		bool bool_val;                          // Temporary variable for boolean values 

		if (!result)                            // Sanity check
            return false;
      
        // Correctly get the address and size of the char array within the union to store the string values.
        char* target_buf = result->s;
        size_t target_size = sizeof(result->s);

        switch (processor.fieldType) {
        case VAL_TYP_INT:
            if (json_obj_get_int(jctx, processor.jsonValStr, &int_val) == OS_SUCCESS) {
                printf("%s%s: %d\n", print_prefix, processor.jsonValStr, int_val);
                result->i = int_val;
                return true;
            }
            break;
        case VAL_TYP_STR:
            // Use target_buf and target_size to parse the string directly into the union.
            if (json_obj_get_string(jctx, processor.jsonValStr, target_buf, target_size) == OS_SUCCESS) {
                printf("%s%s: %s\n", print_prefix, processor.jsonValStr, target_buf);
                return true;
            }
            break;
        case VAL_TYP_BOOL:
            if (json_obj_get_bool(jctx, processor.jsonValStr, &bool_val) == OS_SUCCESS) {
                printf("%s%s: %s\n", print_prefix, processor.jsonValStr, bool_val ? "true" : "false");
                result->b = bool_val;
                return true;
            }
            break;
        }
        // If we reach here, parsing failed.
        printf("Invalid or missing JSON value for: %s%s\n", print_prefix, processor.jsonValStr);
        return false;
    }

    /**
     * @brief Parses the "globalOptions" object from the JSON.
     */
    void parse_global_options(jparse_ctx_t* jctx) {
        printf("\n===== Global Options =====\n");
        for (size_t i = 0; i < GOPTS_KEYS_CNT; ++i) {
            parsedValue result;
            if (parseJSONval(jctx, gOptsValProcessors[i], "", &result)) {
                // TODO: Populate m_alarm.globalOptions using the result and patchCallBack
            }
        }
    }

    /**
     * @brief Parses the "zones" array from the JSON.
     */
    bool parse_zone(jparse_ctx_t* jctx) {
		int ret = 0;                            // to track if any error occurs during the conversion of JSON values
		int zoneIdx = -1;                       // to hold the index of the zone if it exists
        parsedValue result;
        ALARM_ZONE tempZone = {-1};              // Create a temporary local variable
     
        for (size_t j = 0; j < ZONE_KEYS_CNT; ++j) {
            // Pass the address of the 'result' union to parseJSONval
            if(parseJSONval(jctx, zoneValProcessors[j], "  ", &result)) {
                if (zoneValProcessors[j].patchCallBack) {
                    // Get the processor for context
                    const auto& processor = zoneValProcessors[j];
                    
                    // Call the patch function, passing a pointer to the entire 'result' union
                    if(!processor.patchCallBack((byte*)&tempZone, processor.patchOffset, processor.patchLen, &result))
                        ret++;
                }
            }
            else {                              // Consider adding an error log here
                printf("Failed to parse zone JSON value: %s\n", zoneValProcessors[j].jsonValStr);
                return false;
            }
        }
        if (ret) {                               // If any error occurred during patching, return false
            printf("Failed to parse zone completely. Errors occurred during patching.\n");
            return false;
		}
        // print temp zone          
        // m_alarm.printConfigData(zoneValProcessors, ZONE_KEYS_CNT, (byte*)&tempZone, PRTCLASS_ALL);

		// first check if we have zone name already in DB and add it if not exists, else overwtite it
		// we use zone name as unique identifier of the zone
        if ((zoneIdx = m_alarm.getZoneIndex(tempZone.zoneName)) == ERR_IDX_NOT_FND) { // zone name not found, we can add it
			if (zoneIdx=m_alarm.addZone(tempZone) >= 0)
                printf("Zone with name '%s' added successfully.\n", tempZone.zoneName);
            else {
                printf("Failed to add zone with name '%s'.\n", tempZone.zoneName);
                return false;
            }
        }
		m_alarm.zonesDB[zoneIdx] = tempZone; // Mark the zone as valid
		m_alarm.printAlarmZones(zoneIdx, zoneIdx+1); // Print the newly added or updated zone
        return true;
    }

    /**
     * @brief Parses the "partitions" array from the JSON.
     */
    void parse_partition(jparse_ctx_t* jctx) {
        int num_elem;
        //if (json_obj_get_array(jctx, "partitions", &num_elem) == OS_SUCCESS) {
        //    printf("\n===== Partitions (%d) =====\n", num_elem);
        //    jparse_ctx_t tmp_jctx = *jctx;
        //    m_alarm.partitionCount = (num_elem < MAX_PARTITION) ? num_elem : MAX_PARTITION;

        //    for (int i = 0; i < m_alarm.partitionCount; i++) {
        //        *jctx = tmp_jctx;
        //        if (json_arr_get_object(jctx, i) == OS_SUCCESS) {
        //            printf("\nPartition %d:\n", i + 1);
        //            for (size_t j = 0; j < PARTITION_KEYS_CNT; ++j) {
        //                ValueResult result;
        //                if(parse_and_print_field(jctx, partitionValProcessors[j], "  ", &result)) {
        //                    // TODO: Populate m_alarm.partitionsDB[i] using the result
        //                }
        //            }
        //            json_obj_leave_object(jctx);
        //        }
        //    }
        //    *jctx = tmp_jctx;
        //    json_obj_leave_array(jctx);
        //}
    }

    /**
     * @brief Parses the "pgms" array from the JSON.
     */
    void parse_pgm(jparse_ctx_t* jctx) {
        int num_elem;
        //if (json_obj_get_array(jctx, "pgms", &num_elem) == OS_SUCCESS) {
        //    printf("\n===== PGMs (%d) =====\n", num_elem);
        //    jparse_ctx_t tmp_jctx = *jctx;
        //    m_alarm.pgmCount = (num_elem < MAX_PGM) ? num_elem : MAX_PGM;

        //    for (int i = 0; i < m_alarm.pgmCount; i++) {
        //        *jctx = tmp_jctx;
        //        if (json_arr_get_object(jctx, i) == OS_SUCCESS) {
        //            printf("\nPGM %d:\n", i + 1);
        //            for (size_t j = 0; j < PGM_KEYS_CNT; ++j) {
        //                ValueResult result;
        //                if(parse_and_print_field(jctx, pgmValProcessors[j], "  ", &result)) {
        //                    // TODO: Populate m_alarm.pgmsDB[i] using the result
        //                }
        //            }
        //            json_obj_leave_object(jctx);
        //        }
        //    }
        //    *jctx = tmp_jctx;
        //    json_obj_leave_array(jctx);
        //}
    }

    /**
     * @brief Parses the "keyswitches" array from the JSON.
     */
    void parse_keyswitch(jparse_ctx_t* jctx) {
        int num_elem;
        //if (json_obj_get_array(jctx, "keyswitches", &num_elem) == OS_SUCCESS) {
        //    printf("\n===== Keyswitches (%d) =====\n", num_elem);
        //    jparse_ctx_t tmp_jctx = *jctx;
        //    m_alarm.keyswitchCount = (num_elem < MAX_KEYSWITCHES) ? num_elem : MAX_KEYSWITCHES;

        //    for (int i = 0; i < m_alarm.keyswitchCount; i++) {
        //        jctx = tmp_jctx;
        //        if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
        //            printf("\nKeyswitch %d:\n", i + 1);
        //            // TODO: Populate m_alarm.keyswitchesDB[i]
        //            json_obj_leave_object(&jctx);
        //        }
        //    }
        //    json_obj_leave_array(&jctx);
        //}
    }
};

//#define alarm_config_json "{\n\
//  \"zones\": [\n\
//    {\n\
//      \"zName\": \"Front Door\",\n\
//      \"zBRD\": 0,\n\
//      \"zID\": 0,\n\
//      \"zType\": \"ENTRY_DELAY1\",\n\
//      \"zPartn\": 1,\n\
//      \"zAlarmT\": \"STEADY_ALARM\",\n\
//      \"zShdnEn\": false,\n\
//      \"zBypEn\": true,\n\
//      \"zStayZ\": true,\n\
//      \"zFrceEn\": true,\n\
//      \"zIntelZ\": false,\n\
//      \"zDlyTRM\": false,\n\
//      \"zTmprGlb\": true,\n\
//      \"zTmprOPT\": \"TROUBLE_ONLY\",\n\
//      \"zAmskGlb\": true,\n\
//      \"zAmskOpt\": \"DISABLE\"\n\
//    },\n\
//    {\n\
//      \"zName\": \"Back Door\",\n\
//      \"zBRD\": 0,\n\
//      \"zID\": 1,\n\
//      \"zType\": \"ENTRY_DELAY2\",\n\
//      \"zPartn\": 1,\n\
//      \"zAlarmT\": \"PULSED_ALARM\",\n\
//      \"zShdnEn\": false,\n\
//      \"zBypEn\": true,\n\
//      \"zStayZ\": true,\n\
//      \"zFrceEn\": true,\n\
//      \"zIntelZ\": false,\n\
//      \"zDlyTRM\": false,\n\
//      \"zTmprGlb\": true,\n\
//      \"zTmprOPT\": \"TROUBLE_ONLY\",\n\
//      \"zAmskGlb\": true,\n\
//      \"zAmskOpt\": \"DISABLE\"\n\
//    },\n\
//    {\n\
//      \"zName\": \"Motion Sensor Living Room\",\n\
//      \"zBRD\": 0,\n\
//      \"zID\": 2,\n\
//      \"zType\": \"FOLLOW\",\n\
//      \"zPartn\": 1,\n\
//      \"zAlarmT\": \"STEADY_ALARM\",\n\
//      \"zShdnEn\": false,\n\
//      \"zBypEn\": true,\n\
//      \"zStayZ\": false,\n\
//      \"zFrceEn\": true,\n\
//      \"zIntelZ\": true,\n\
//      \"zDlyTRM\": false,\n\
//      \"zTmprGlb\": true,\n\
//      \"zTmprOPT\": \"ALARM\",\n\
//      \"zAmskGlb\": true,\n\
//      \"zAmskOpt\": \"ALARM_WHEN_ARMED\"\n\
//    },\n\
//    {\n\
//      \"zName\": \"Window Sensor Bedroom\",\n\
//      \"zBRD\": 0,\n\
//      \"zID\": 3,\n\
//      \"zType\": \"INSTANT\",\n\
//      \"zPartn\": 2,\n\
//      \"zAlarmT\": \"SILENT_ALARM\",\n\
//      \"zShdnEn\": false,\n\
//      \"zBypEn\": true,\n\
//      \"zStayZ\": false,\n\
//      \"zFrceEn\": true,\n\
//      \"zIntelZ\": false,\n\
//      \"zDlyTRM\": false,\n\
//      \"zTmprGlb\": false,\n\
//      \"zTmprOPT\": \"DISABLE\",\n\
//      \"zAmskGlb\": false,\n\
//      \"zAmskOpt\": \"DISABLE\"\n\
//    },\n\
//    {\n\
//      \"zName\": \"Smoke Detector\",\n\
//      \"zBRD\": 0,\n\
//      \"zID\": 4,\n\
//      \"zType\": \"H24_FIRE_STANDARD\",\n\
//      \"zPartn\": 2,\n\
//      \"zAlarmT\": \"STEADY_ALARM\",\n\
//      \"zShdnEn\": false,\n\
//      \"zBypEn\": false,\n\
//      \"zStayZ\": false,\n\
//      \"zFrceEn\": false,\n\
//      \"zIntelZ\": false,\n\
//      \"zDlyTRM\": false,\n\
//      \"zTmprGlb\": true,\n\
//      \"zTmprOPT\": \"TROUBLE_ONLY\",\n\
//      \"zAmskGlb\": true,\n\
//      \"zAmskOpt\": \"TROUBLE_ONLY\"\n\
//    }\n\
//  ],\n\
//  \"partitions\": [\n\
//    {\n\
//      \"pName\": \"Main Floor\",\n\
//      \"pIdx\": 1,\n\
//      \"pValid\": true,\n\
//      \"pFrceOnRegArm\": true,\n\
//      \"pFrceOnStayArm\": true,\n\
//      \"pED2znFollow\": true,\n\
//      \"pAlrmOutEn\": true,\n\
//      \"pAlrmTime\": 120,\n\
//      \"pNoCutOnFire\": true,\n\
//      \"pAlrmRecTime\": 30,\n\
//      \"pED1Intvl\": 30,\n\
//      \"pED2Intvl\": 45,\n\
//      \"pExitDly\": 60,\n\
//      \"pFollow1\": 1,\n\
//      \"pFollow2\": 0,\n\
//      \"pFollow3\": 0,\n\
//      \"pFollow4\": 0,\n\
//      \"pFollow5\": 0,\n\
//      \"pFollow6\": 0,\n\
//      \"pFollow7\": 0,\n\
//      \"pFollow8\": 0\n\
//    },\n\
//    {\n\
//      \"pName\": \"Upper Floor\",\n\
//      \"pIdx\": 2,\n\
//      \"pValid\": true,\n\
//      \"pFrceOnRegArm\": true,\n\
//      \"pFrceOnStayArm\": false,\n\
//      \"pED2znFollow\": true,\n\
//      \"pAlrmOutEn\": true,\n\
//      \"pAlrmTime\": 120,\n\
//      \"pNoCutOnFire\": true,\n\
//      \"pAlrmRecTime\": 30,\n\
//      \"pED1Intvl\": 30,\n\
//      \"pED2Intvl\": 45,\n\
//      \"pExitDly\": 60,\n\
//      \"pFollow1\": 0,\n\
//      \"pFollow2\": 0,\n\
//      \"pFollow3\": 0,\n\
//      \"pFollow4\": 0,\n\
//      \"pFollow5\": 0,\n\
//      \"pFollow6\": 0,\n\
//      \"pFollow7\": 0,\n\
//      \"pFollow8\": 0\n\
//    }\n\
//  ],\n\
//  \"globalOptions\": {\n\
//    \"MaxSlaves\": 2,\n\
//    \"RestrSprvsL\": true,\n\
//    \"RestrTamper\": true,\n\
//    \"RestrACfail\": false,\n\
//    \"RestrBatFail\": true,\n\
//    \"RestrOnBell\": false,\n\
//    \"RestrOnBrdFail\": true,\n\
//    \"RestrOnAmask\": false,\n\
//    \"TroubleLatch\": true,\n\
//    \"TamperBpsOpt\": false,\n\
//    \"TamperOpts\": \"TROUBLE_ONLY\",\n\
//    \"AntiMaskOpt\": \"ALARM_WHEN_ARMED\",\n\
//    \"RfSprvsOpt\": \"ALARM\",\n\
//    \"SprvsLoss\": 0,\n\
//    \"ACfail\": 0,\n\
//    \"BatFail\": 0,\n\
//    \"BellFail\": 0,\n\
//    \"BrdFail\": 0\n\
//  },\n\
//  \"pgms\": [\n\
//    {\n\
//      \"pgmName\": \"External Siren\",\n\
//      \"pgmBrd\": 0,\n\
//      \"pgmID\": 0,\n\
//      \"pgmPulseLen\": 5,\n\
//      \"pgmValid\": true\n\
//    },\n\
//    {\n\
//      \"pgmName\": \"Garage Door\",\n\
//      \"pgmBrd\": 0,\n\
//      \"pgmID\": 1,\n\
//      \"pgmPulseLen\": 3,\n\
//      \"pgmValid\": true\n\
//    },\n\
//    {\n\
//      \"pgmName\": \"Entrance Light\",\n\
//      \"pgmBrd\": 0,\n\
//      \"pgmID\": 2,\n\
//      \"pgmPulseLen\": 0,\n\
//      \"pgmValid\": true\n\
//    }\n\
//  ],\n\
//  \"keyswitches\": [\n\
//    {\n\
//      \"kswName\": \"Front Door Keyswitch\",\n\
//      \"partition\": 0,\n\
//      \"type\": \"MAINTAINED\",\n\
//      \"action\": \"REGULAR_ARM_ONLY\",\n\
//      \"boardID\": 0,\n\
//      \"zoneID\": 5\n\
//    }\n\
//  ]\n\
//}"