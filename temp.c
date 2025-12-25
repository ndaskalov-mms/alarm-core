/**
 * @brief Parses the "zones" array from the JSON.
 */
bool parse_zone(jparse_ctx_t* jctx, const jsonKeyValProcessor* processors, size_t processor_count) {
	int ret = 0;                            // to track if any error occurs during the conversion of JSON values
	int zoneIdx = -1;                       // to hold the index of the zone if it exists
    parsedValue result;
    ALARM_ZONE tempZone;              // Create a temporary local variable
    memset(&tempZone, -1, sizeof(tempZone));

    // iterate over all zone JSON key:val pairs defined in zoneKeyValProcessors[]
    for (size_t j = 0; j < processor_count; ++j) {
		processors[j].pos = 0;        // Reset presence flag before parsing
        // try get the JSON val for the key stored in zoneKeyValProcessors[j].jsonKeyStr
        // Pass the address of the 'result' union to parseJSONval
        printf("Looking for %s key: ", processors[j]);
        if(parseJSONval(jctx, processors[j], &result)) {
            // found key, the val is stored in result
			printf("Found\n");
            if (processors[j].patchCallBack) {
                // Get the processor for context
                auto& processor = processors[j];
                // Call the patch function, passing a pointer to the entire 'result' union
                if (!processor.patchCallBack((byte*)&tempZone, processor.patchOffset, processor.patchLen, &result)) {
                    printf("Failed to process VAL for zone json KEY: %s\n", processor.jsonKeyStr);
                    ret++;                  // Increment error count if patching fails and go to next key:val
                }
                else {
                    processor.pos = 1;      // Mark as present and converted successfully
                }
            }
        }
        else {       
            printf("Not found\n");              
            //printf("Failed to find zone json KEY: %s\n", zoneKeyValProcessors[j].jsonKeyStr);
        }
    }
    if (ret) {                               // If any error occurred during patching, return false
        printf("Failed to parse zone completely. Errors occurred during parsing/patching.\n");
        return false;
	}
    // print temp zone          
    // m_alarm.printConfigData(zoneValProcessors, ZONE_KEYS_CNT, (byte*)&tempZone, PRTCLASS_ALL);

	// first check if we have zone name already in DB and add it if not exists, else overwtite it
	// we use zone name as unique identifier of the zone
    if ((zoneIdx = m_alarm.getZoneIndex(tempZone.zoneName)) == ERR_IDX_NOT_FND) { // zone name not found, we can add it
		printf("Zone with name '%s' not found. Adding new zone.\n", tempZone.zoneName);
        if ((zoneIdx=m_alarm.addZone(tempZone)) >= 0)
            printf("Zone with name '%s' added successfully at index %d.\n", tempZone.zoneName, zoneIdx);
        else {
            printf("Failed to add zone with name '%s'.\n", tempZone.zoneName);
            return false;
        }
    }
	// Copy only the members that were present in the JSON to the target zone
	patch_db_item(&m_alarm.zonesDB[zoneIdx], &tempZone, processors, processor_count);
	m_alarm.printAlarmZones(zoneIdx, zoneIdx+1); // Print the newly added or updated zone
    return true;
}
