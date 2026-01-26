#pragma once
#include <stdio.h>
#include <string.h>
#include "..\alarm-core.h"
#include "alarm-core-internal-defs.h" // Include the internal definitions
#include "..\..\esp-json-parser\include\json_parser-code.h"

class alarmJSON {
    // Reference to the Alarm instance we are populating
    Alarm& m_alarm;
public:
    /**
     * @brief Constructor that takes a reference to the Alarm instance to be populated.
     * @param alarm The Alarm object to configure.
     */
    alarmJSON(Alarm& alarm) : m_alarm(alarm) {}

    // Define the JSON processor functions (payload handlers)
    bool processJsonPayload(const char* jsonBuffer, size_t length, ALARM_DOMAINS_t domain) {
        jparse_ctx_t jctx;    // JSON parsing context

        int ret = json_parse_start(&jctx, jsonBuffer, length);
        if (ret != OS_SUCCESS) {
            LOG_ERROR("Parser fstart failed\n");
            return false;
        }
        ret = 0;
        switch(domain) {
            case GLOBAL_OPT_CFG:
                ret = parseGlobalOptionsCfg(&jctx);
                break;
            case ZONES_CFG:
                ret = parseZoneCfg(&jctx);
                break;
            case ZONES_CMD:
                ret = parseZoneCmd(&jctx);
                break;
            case PARTITIONS_CFG:
                ret = parsePartitionCfg(&jctx);
                break;
            case PARTITIONS_CMD:
                ret = parsePartitionCmd(&jctx);
                break;
            case PGMS_CFG:
                //ret = parsePgmCfg(&jctx);
                break;
            default:
                printf("Unknown domain type\n");
				ret = 0;
		}
        json_parse_end(&jctx);
        return ret;
    }

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
		int ret = json_parse_start(&jctx, jsonBuffer, (int)strlen(jsonBuffer)); 
        if (ret != OS_SUCCESS) {
            printf("Parser failed\n");
            return -1;
        }

        // Parse and display global options
        if (json_obj_get_object(&jctx, "globalOptions") == OS_SUCCESS) {
            parseGlobalOptionsCfg(&jctx);
            json_obj_leave_object(&jctx);
        }

        // Parse and display zones
        if (json_obj_get_array(&jctx, "zones", &num_elem) == OS_SUCCESS) {
            LOG_DEBUG("\n===== Zones (%d) =====\n", num_elem);
            tmp_jctx = jctx;                                // needed to properly parse arrays
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    printf("\nZone %d:\n", i + 1);
                    parseZoneCfg(&jctx);
                    json_obj_leave_object(&jctx);
                }
                else
                    LOG_ERROR("\nInvalid Zone %d:\n", i + 1);

            }
            json_obj_leave_array(&jctx);
        }

        // Parse and display partitions
        if (json_obj_get_array(&jctx, "partitions", &num_elem) == OS_SUCCESS) {
            LOG_DEBUG("\n===== Partitions (%d) =====\n", num_elem);
            tmp_jctx = jctx;
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    printf("\nPartition %d:\n", i + 1);
                    parsePartitionCfg(&jctx);
                    json_obj_leave_object(&jctx);
                }
            }
            json_obj_leave_array(&jctx);
        }

        // Parse and display pgms
        if (json_obj_get_array(&jctx, "pgms", &num_elem) == OS_SUCCESS) {
            LOG_DEBUG("\n===== PGMs (%d) =====\n", num_elem);
            tmp_jctx = jctx;
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    LOG_DEBUG("\nPGM %d:\n", i + 1);
                    //parsePgmCfg(&jctx);
                    json_obj_leave_object(&jctx);
                }
            }
            json_obj_leave_array(&jctx);
        }

        // Parse and display keyswitches
        if (json_obj_get_array(&jctx, "keyswitches", &num_elem) == OS_SUCCESS) {
            LOG_DEBUG("\n===== Keyswitches (%d) =====\n", num_elem);
            tmp_jctx = jctx;
            for (int i = 0; i < num_elem; i++) {
                jctx = tmp_jctx;
                if (json_arr_get_object(&jctx, i) == OS_SUCCESS) {
                    LOG_DEBUG("\nKeyswitch %d:\n", i + 1);
                    //parse_keyswitch(&jctx);
                    json_obj_leave_object(&jctx);
                }
            }
            json_obj_leave_array(&jctx);
        }

        json_parse_end(&jctx);
        return 0;
    }

private:

    /**
     * @brief Parses a single field (STRING, INT, BOOL) from the JSON object based on
      expected from the current JSON processor. JSON processor is selected based on particular JSON key.
	  For example, for zoneName key, we expect string value, for zoneType - int value, etc.
	  jsonValProcessor struct and all processors are defined in alarm-core-json-val-parsers.h
     */
    bool parseJSONval(jparse_ctx_t* jctx, const jsonKeyValProcessor& processor, parsedValue* result) {

        // Use a local buffer for parsing and printing.
		// char str_val[NAME_LEN];                 // Temporary buffer for string values - UNUSED
		int int_val;                            // Temporary variable for integer values
		bool bool_val;                          // Temporary variable for boolean values 

		if (!result)                            // Sanity check
            return false;
      
        // Correctly get the address and size of the char array within the union to store the string values.
        char* target_buf = result->s;
        size_t target_size = sizeof(result->s);

        switch (processor.fieldType) {
        case VAL_TYP_INT:
            if (json_obj_get_int(jctx, processor.jsonKeyStr, &int_val) == OS_SUCCESS) {
                //printf("%s: %d\n", processor.jsonKeyStr, int_val);
                result->i = int_val;
                return true;
            }
            break;
        case VAL_TYP_STR:
            // Use target_buf and target_size to parse the string directly into the union.
            if (json_obj_get_string(jctx, processor.jsonKeyStr, target_buf, (int)target_size) == OS_SUCCESS) {
                //printf("%s: %s\n", processor.jsonKeyStr, target_buf);
                return true;
            }
            break;
        case VAL_TYP_BOOL:
            if (json_obj_get_bool(jctx, processor.jsonKeyStr, &bool_val) == OS_SUCCESS) {
                //printf("%s: %s\n", processor.jsonKeyStr, bool_val ? "true" : "false");
                result->b = bool_val;
                return true;
            }
            break;
        }
        // If we reach here, parsing failed.
        //printf("Invalid or missing JSON value for KEY : %s\n", processor.jsonKeyStr);
        return false;
    }

    /**
 * @brief Copies only the members that were present in the JSON from a temporary source object to a destination object.
 * @param dest_item Pointer to the destination object (e.g., in the database).
 * @param src_item Pointer to the source temporary object.
 * @param processors The array of value processors for this item type.
 * @param processor_count The number of processors in the array.
 */
    void patch_db_item(void* dest_item, const void* src_item, const jsonKeyValProcessor* processors, size_t processor_count) {
        for (size_t i = 0; i < processor_count; ++i) {
            if (processors[i].pos != 0) { // Check if the value was present in the JSON
                byte* dest = (byte*)dest_item + processors[i].patchOffset;
                const byte* src = (const byte*)src_item + processors[i].patchOffset;
                memcpy(dest, src, processors[i].patchLen);
            }
        }
    }

    /**
    * @brief Parses the "zones" array from the JSON.
    */
    bool parse_object(jparse_ctx_t* jctx, jsonKeyValProcessor* processors, size_t processor_count, byte* tempObj) {
        int ret = 0;                            // to track if any error occurs during the conversion of JSON values
        parsedValue result;

        // iterate over all zone JSON key:val pairs defined in zoneCfgKeyValProcessors[]
        for (size_t j = 0; j < processor_count; ++j) {
            processors[j].pos = 0;        // Reset presence flag before parsing
            // try get the JSON val for the key stored in zoneCfgKeyValProcessors[j].jsonKeyStr
            // Pass the address of the 'result' union to parseJSONval
            LOG_DEBUG("Looking for %s key: ", processors[j].jsonKeyStr);
            if (parseJSONval(jctx, processors[j], &result)) {
                // found key, the val is stored in result
                LOG_DEBUG("Found\n");
                if (processors[j].patchCallBack) {
                    // Get the processor for context
                    auto& processor = processors[j];
                    // Call the patch function, passing a pointer to the entire 'result' union
                    if (!processor.patchCallBack(tempObj, processor.patchOffset, processor.patchLen, &result)) {
                        LOG_ERROR("Failed to process VAL for zone json KEY: %s\n", processor.jsonKeyStr);
                        ret++;                  // Increment error count if patching fails and go to next key:val
                    }
                    else {
                        processor.pos = 1;      // Mark as present and converted successfully
                    }
                }
            }
            else {
                LOG_ERROR("Not found\n");
                //printf("Failed to find zone json KEY: %s\n", zoneCfgKeyValProcessors[j].jsonKeyStr);
            }
        }
        if (ret) {                               // If any error occurred during patching, return false
            LOG_ERROR("Failed to parse zone completely. Errors occurred during VAL parsing/patching.\n");
            return false;
        }
        return true;
    }

    /**
    * @brief Parses the "globalOptions" object from the JSON.
    */
    bool parseGlobalOptionsCfg(jparse_ctx_t* jctx) {
        int ret = 0;                            // to track if any error occurs during the conversion of JSON values
        ALARM_GLOBAL_OPTS_t temp_gOpts; // Create a temporary local variable
        memset(&temp_gOpts, -1, sizeof(temp_gOpts));

        if (!parse_object(jctx, gOptsKeyValProcessors, GOPTS_KEYS_CNT, (byte*)&temp_gOpts))
            return false;

        // print temp gOpts
        // m_alarm.printConfigData(gOptsKeyValProcessors, GOPTS_KEYS_CNT, (byte*)&temp_gOpts, PRTCLASS_ALL);

        // Copy only the members that were present in the JSON to the target zone
        patch_db_item(&m_alarm.alarmGlobalOpts, &temp_gOpts, gOptsKeyValProcessors, GOPTS_KEYS_CNT);
        m_alarm.printAlarmOpts((byte*)&m_alarm.alarmGlobalOpts); // Print the newly added or updated zone
        return true;
    }

    /**
    * @brief Parses the "zones" array from config JSON.
    */
    bool parseZoneCfg(jparse_ctx_t* jctx) {
        int ret = 0;                            // to track if any error occurs during the conversion of JSON values
        int zoneIdx = -1;                       // to hold the index of the zone if it exists
        ALARM_ZONE tempZone;              // Create a temporary local variable
        memset(&tempZone, 0, sizeof(tempZone));

        if (!parse_object(jctx, zoneCfgKeyValProcessors, ZONE_CFG_KEYS_CNT, (byte*)&tempZone))
            return false;
        // print temp zone          
        //m_alarm.printConfigData(zoneValProcessors, ZONE_CFG_KEYS_CNT, (byte*)&tempZone, PRTCLASS_ALL);

        // first check if we have zone name already in DB and add it if not exists, else overwtite it
        // we use zone name as unique identifier of the zone
        if ((zoneIdx = m_alarm.getZoneIndex(tempZone.zoneName)) == ERR_IDX_NOT_FND) { // zone name not found, we can add it
            LOG_DEBUG("Zone with name %s not found. Adding new zone.\n", tempZone.zoneName);
            if ((zoneIdx = m_alarm.addZone(tempZone)) >= 0)
                LOG_DEBUG("Zone with name '%s' added successfully at index %d.\n", tempZone.zoneName, zoneIdx);
            else {
                LOG_ERROR("Failed to add zone with name %s.\n", tempZone.zoneName);
                return false;
            }
        }
        // Copy only the members that were present in the JSON to the target zone
        patch_db_item(&m_alarm.zonesDB[zoneIdx], &tempZone, zoneCfgKeyValProcessors, ZONE_CFG_KEYS_CNT);
        m_alarm.printAlarmZones(zoneIdx, zoneIdx + 1); // Print the newly added or updated zone
        return true;
    }

    /**
    * @brief Parses the zone command JSON
    */
    bool parseZoneCmd(jparse_ctx_t* jctx) {
        int ret = 0;                            // to track if any error occurs during the conversion of JSON values
        int zoneIdx = -1;                       // to hold the index of the zone if it exists
        ALARM_ZONE_CMD_t tempZone;
        tempZone.open = tempZone.bypass = tempZone.tamper = tempZone.antiMask = ZONE_RESERVED_CMD ; 
		tempZone.zoneName[0] = '\0';           // initialize zone name to empty string
        int res;

        if (!parse_object(jctx, zoneCmdKeyValProcessors, ZONE_CMD_KEYS_CNT, (byte*)&tempZone))
            return false;

        if (tempZone.zoneName[0] == '\0') {
            LOG_ERROR("Zone name must be provided in zone command!\n");
            return false;
        }
        if ((zoneIdx = m_alarm.getZoneIndex(tempZone.zoneName)) == ERR_IDX_NOT_FND) { // zone name not found, we can add it
            LOG_ERROR("No zone with name %s exists.\n", tempZone.zoneName);
            return false;
        }
        if (tempZone.bypass != ZONE_RESERVED_CMD) {
            res = tempZone.bypass ? ZONE_BYPASS_CMD : ZONE_UNBYPASS_CMD;
            m_alarm.modifyZn((void *)&zoneIdx, &res, NULL);
        }
        if (tempZone.open != ZONE_RESERVED_CMD) {
            res = tempZone.open ? ZONE_OPEN_CMD : ZONE_CLOSE_CMD;
            m_alarm.modifyZn((void*)&zoneIdx, &res, NULL);
        }
        if (tempZone.tamper != ZONE_RESERVED_CMD) {
            res = tempZone.tamper ? ZONE_TAMPER_ON_CMD : ZONE_TAMPER_OFF_CMD;
            m_alarm.modifyZn((void*)&zoneIdx, &res, NULL);
        }
        if (tempZone.antiMask != ZONE_RESERVED_CMD) {
            res = tempZone.antiMask ? ZONE_AMASK_ON_CMD : ZONE_AMASK_OFF_CMD;
            m_alarm.modifyZn((void*)&zoneIdx, &res, NULL);
        }
        return true;
    }

    /**
    * @brief Parses the zone command JSON
    */
    bool parsePartitionCmd(jparse_ctx_t* jctx) {
        int ret = 0;                            // to track if any error occurs during the conversion of JSON values
        int partitionIdx = -1;                  // to hold the index of the zone if it exists
        ALARM_PARTITION_CMD_t tempPartition;
        tempPartition.partitionName[0] = '\0';  // initialize zone name to empty string
		tempPartition.armMethod[0] = '\0';      // initialize arm method to empty string
        //int res;

        if (!parse_object(jctx, partitionCmdKeyValProcessors, PARTITION_CMD_KEYS_CNT, (byte*)&tempPartition))
            return false;

        if (tempPartition.partitionName[0] == '\0') {
            LOG_ERROR("Partition name must be provided in partition command!\n");
            return false;
        }
        if ((partitionIdx = m_alarm.getPartitionIndex(tempPartition.partitionName)) == ERR_IDX_NOT_FND) { // zone name not found, we can add it
            LOG_ERROR("No partition with name %s exists.\n", tempPartition.partitionName);
            return false;
        }
        // look-up the command in commands array
        ARM_METHODS_t cmd = INVALID_CMD;
        for (int i = 0; i < PARTITION_CMDS_CNT; i++) {
            if (_stricmp(tempPartition.armMethod, partitionCmdsInt2Str[i].valStr) == 0) {
                cmd = (ARM_METHODS_t) partitionCmdsInt2Str[i].val;
                break;
            }
        }
        if (cmd != -1) {
            // Execute the found command
            ret = m_alarm.trigerArm(partitionIdx, cmd);
        }
        else {
            LOG_ERROR("Unknown partition command: %s\n", tempPartition.armMethod);
            return false;
        }
        return true;
    }
    /**
     * @brief Parses the "partitions" array from the JSON.
    */
    bool parsePartitionCfg(jparse_ctx_t* jctx) {
        int ret = 0;                            // to track if any error occurs during the conversion of JSON values
        int partitionIdx = -1;                  // to hold the index of the partition if it exists
        ALARM_PARTITION_t tempPartition;        // Create a temporary local variable
        memset(&tempPartition, -1, sizeof(tempPartition));

        if (!parse_object(jctx, partitionKeyValProcessors, PARTITION_KEYS_CNT, (byte*)&tempPartition))
            return false;

        // first check if we have partition name already in DB and add it if not exists, else overwrite it
        // we use partition name as unique identifier of the partition
        if ((partitionIdx = m_alarm.getPartitionIndex(tempPartition.partitionName)) == ERR_IDX_NOT_FND) { // partition name not found, we can add it
            LOG_DEBUG("Partition with name '%s' not found. Adding new partition.\n", tempPartition.partitionName);
            if ((partitionIdx = m_alarm.addPartition(tempPartition)) >= 0)
                LOG_DEBUG("Partition with name '%s' added successfully at index %d.\n", tempPartition.partitionName, partitionIdx);
            else {
                LOG_ERROR("Failed to add partition with name '%s'.\n", tempPartition.partitionName);
                return false;
            }
        }

        // Copy only the members that were present in the JSON to the target partition
        patch_db_item(&m_alarm.partitionDB[partitionIdx], &tempPartition, partitionKeyValProcessors, PARTITION_KEYS_CNT);
        m_alarm.printAlarmPartition(partitionIdx, partitionIdx + 1); // Print the newly added or updated partition
        return true;
    }

    /**
     * @brief Parses the "pgms" array from the JSON.
     */
    bool parsePgmCfg(jparse_ctx_t* jctx) {
        int ret = 0;
        int pgmIdx = -1;
        // parsedValue result; // UNUSED
        ALARM_PGM tempPgm;
        memset(&tempPgm, -1, sizeof(tempPgm));

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
        return true; // Added return
    }

    /**
     * @brief Parses the "keyswitches" array from the JSON.
     */
    void parse_keyswitch(jparse_ctx_t* jctx) {
        // int num_elem; // UNUSED
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



