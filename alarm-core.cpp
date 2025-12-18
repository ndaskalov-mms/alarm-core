// this is heeded for windows JSON library
//#include "json.hpp"
#include <stdio.h>
typedef unsigned char byte;
#include "alarm-core-config.h"
#include "src\alarm-core-debug.h"
//#include "src\alarm-core-timers.h"
#include "alarm-FS-wrapper.h"
#include "alarm-core.h"
#include "src\alarm-core-JSON.h" // Include the new header-only parser
//#include "alarm-core-mqtt.h" // Include the new header
//#include "alarm-core-CSV-parser.h" // Include the new header
#include <string.h>
//#include "..\esp-json-parser\include\json_parser-code.h"

// Sample JSON configuration for testing
// #define alarm_config_json "{\"zones\":[{\"zName\":\"Front\"},{\"zName\":\"Back\"}]}"

// global storage definitions
char prnBuf[1024];
char token[256];
//TimerManager alarmTimerManager;


void debugPrinter(const char* message, size_t length) {
    printf("[DEBUG] %.*s\n", (int)length, message);
}

// instance of the Alarm class
Alarm alarm;

int main() {
    // Print a welcome message
    //std::cout << "Windows Console Application: ALARM JSON Example\n";

    alarm.setDebugCallback(GlobalDebugLogger);
    alarm.debugCallback(LOG_ERR_OK, "test\n");

    storageSetup();

    IOptr configFile = alarmFileOpen(jsonConfigFname, "r");
    if (configFile) {
        int fileSize = alarmFileSize(configFile);
        char* jsonBuffer = (char*)malloc(fileSize + 1);

        if (jsonBuffer) {
            size_t bytesRead = alarmFileRead((byte*)jsonBuffer, fileSize, configFile);
            if (bytesRead == fileSize) {
                jsonBuffer[fileSize] = '\0'; // Null-terminate the buffer
                alarmJSON parser(alarm);
                parser.parseConfigJSON(jsonBuffer);
            }
            else {
                GlobalDebugLogger(LOG_ERR_CRITICAL, "Failed to read config file: %s", jsonConfigFname);
            }
            free(jsonBuffer);
        }
        else {
            GlobalDebugLogger(LOG_ERR_CRITICAL, "Failed to allocate memory for config file.");
        }
        alarmFileClose(configFile);
    }
    else {
        GlobalDebugLogger(LOG_ERR_CRITICAL, "Failed to open config file: %s", jsonConfigFname);
    }

    storageClose();

    //runJsonMQTTTests(alarm);
    return 0;
}
//
// wrapers for the Alarm class methods
//
int getZoneCount() {
    return alarm.getZoneCount();
}
//
int getZoneIndex(const char* name) {
    return alarm.getZoneIndex(name);
}
//
//const char* getZoneName(int zoneIdx) {
//    return alarm.getZoneName(zoneIdx);
//}
//
//bool isZoneValid(int zoneIdx) {
//    return alarm.isZoneValid(zoneIdx);
//}
//
//int getPartitionCount() {
//    return alarm.getPartitionCount();
//}
////
//int getPartitionIndex(const char* name) {
//    return alarm.getPartitionIndex(name);
//}
////
//const char* getPartitionName(int partIdx) {
//    return alarm.getPartitionName(partIdx);
//}
////
//bool isPartitionValid(int partIdx) {
//    return alarm.isPartitionValid(partIdx);
//}
////
//void setPartitionTarget(int partIdx, int action) {
//    alarm.setPartitionTarget(partIdx, static_cast<ARM_METHODS_t>(action));
////}
////
//int getPgmCount() {
//    return alarm.getPgmCount();
//}
//
//bool isPgmValid(int pgmIdx) {
//    return alarm.isPgmValid(pgmIdx);
//}
//
//int getPgmIndex(const char* name) {
//    return alarm.getPgmIndex(name);
//}
////
//const char* getPgmName(int pgmIdx) {
//    return alarm.getPgmName(pgmIdx);
//}
////
//bool setGlobalOptions(const char* opt_name, const char* opt_val) {
//    return alarm.setGlobalOptions(opt_name, opt_val);
//}
////
//int getGlobalOptionsCnt() {
//    return alarm.getGlobalOptionsCnt();
//}
////
//const char* getGlobalOptionKeyStr(int idx) {
//    return alarm.getGlobalOptionKeyStr(idx);
//}
//
//void zoneCmd(Alarm& alarm_instance, void* param1, void* param2, void* param3) {
//	return alarm.modifyZn(param1, param2, param3);
//}
////
//void pgmCmd(Alarm& alarm_instance, void* param1, void* param2, void* param3) {
//	return alarm_instance.modifyPgm(param1, param2, param3);
//}

//
// ------------------------  JSON MQTT TEST vectors ------------------------
//
struct JsonMQTTTestVector {
    const char* topic;       // The MQTT topic to test
    const char* payload;     // The JSON payload to send
    const char* description; // Description of the test case
};

// This array contains the test vectors for the JSON MQTT commands
JsonMQTTTestVector jsonTestVectors[] = {
    // Zone control tests
    {"/alarm/zones/control",
     "{\"zone\":\"Front Door\",\"action\":\"bypass\"}",
     "Bypass Front Door zone"},

    {"/alarm/zones/control",
     "{\"zone\":\"Back Door\",\"action\":\"clear_bypass\"}",
     "Clear bypass on Back Door zone"},

    {"/alarm/zones/control",
     "{\"zone\":\"Motion Sensor\",\"action\":\"tamper\"}",
     "Trigger tamper on Motion Sensor zone"},

    {"/alarm/zones/control",
     "{\"zone\":\"Window Sensor\",\"action\":\"open\"}",
     "Open Window Sensor zone"},

    {"/alarm/zones/control",
     "{\"zone\":\"Smoke Detector\",\"action\":\"close\"}",
     "Close Smoke Detector zone"},

    {"/alarm/zones/control",
     "{\"zone\":\"PIR Living Room\",\"action\":\"anti-mask\"}",
     "Trigger anti-mask on PIR Living Room zone"},

     // Invalid zone tests
     {"/alarm/zones/control",
      "{\"zone\":\"InvalidZone\",\"action\":\"bypass\"}",
      "Bypass an invalid zone"},

     {"/alarm/zones/control",
      "{\"zone\":\"Front Door\",\"action\":\"invalid-action\"}",
      "Invalid action for a valid zone"},

     {"/alarm/zones/control",
      "{\"invalid-key\":\"Front Door\",\"action\":\"bypass\"}",
      "Invalid JSON key for zone control"},

     {"/alarm/zones/control",
      "{\"zone\":\"Front Door\"}",
      "Missing action key in JSON"},

     {"/alarm/zones/control",
      "{\"action\":\"bypass\"}",
      "Missing zone key in JSON"},

      // Partition control tests
      {"/alarm/partitions/control",
       "{\"partition\":\"Main Floor\",\"action\":\"disarm\"}",
       "Disarm Main Floor partition"},

      {"/alarm/partitions/control",
       "{\"partition\":\"Upper Floor\",\"action\":\"arm\"}",
       "Arm Upper Floor partition"},

      {"/alarm/partitions/control",
       "{\"partition\":\"Basement\",\"action\":\"arm_force\"}",
       "Force arm Basement partition"},

      {"/alarm/partitions/control",
       "{\"partition\":\"Garage\",\"action\":\"arm_stay\"}",
       "Stay arm Garage partition"},

      {"/alarm/partitions/control",
       "{\"partition\":\"Office\",\"action\":\"arm_instant\"}",
       "Instant arm Office partition"},

       // Invalid partition tests
       {"/alarm/partitions/control",
        "{\"partition\":\"InvalidPartition\",\"action\":\"arm\"}",
        "Arm an invalid partition"},

       {"/alarm/partitions/control",
        "{\"partition\":\"Main Floor\",\"action\":\"invalid-action\"}",
        "Invalid action for a valid partition"},

        // PGM control tests
        {"/alarm/pgms/control",
         "{\"pgm\":\"Siren\",\"action\":\"on\"}",
         "Turn on Siren PGM"},

        {"/alarm/pgms/control",
         "{\"pgm\":\"Garage Door\",\"action\":\"off\"}",
         "Turn off Garage Door PGM"},

        {"/alarm/pgms/control",
         "{\"pgm\":\"Outdoor Lights\",\"action\":\"pulse\"}",
         "Pulse Outdoor Lights PGM"},

         // Invalid PGM tests
         {"/alarm/pgms/control",
          "{\"pgm\":\"InvalidPGM\",\"action\":\"on\"}",
          "Turn on an invalid PGM"},

         {"/alarm/pgms/control",
          "{\"pgm\":\"Siren\",\"action\":\"invalid-action\"}",
          "Invalid action for a valid PGM"},

          // Global options tests
          {"/alarm/global/options",
           "{\"option\":\"MaxSlaves\",\"value\":\"5\"}",
           "Set MaxSlaves option to 5"},

          {"/alarm/global/options",
           "{\"option\":\"RestrSprvsL\",\"value\":\"true\"}",
           "Enable restriction on supervision loss"},

          {"/alarm/global/options",
           "{\"option\":\"RestrTamper\",\"value\":\"false\"}",
           "Disable restriction on tamper"},

          {"/alarm/global/options",
           "{\"option\":\"RestrACfail\",\"value\":\"true\"}",
           "Enable restriction on AC failure"},

           // Invalid global options tests
           {"/alarm/global/options",
            "{\"option\":\"InvalidOption\",\"value\":\"true\"}",
            "Set an invalid global option"},

           {"/alarm/global/options",
            "{\"option\":\"MaxSlaves\",\"value\":\"invalid\"}",
            "Set MaxSlaves to an invalid value"},

            // Malformed JSON tests
            {"/alarm/zones/control",
             "{\"zone\":\"Front Door\",\"action\":\"bypass\"",
             "Malformed JSON - missing closing brace"},

            {"/alarm/partitions/control",
             "This is not JSON",
             "Invalid JSON format"},

            {"/alarm/pgms/control",
             "",
             "Empty payload"},

             // Empty JSON tests
             {"/alarm/zones/control",
              "{}",
              "Empty JSON object"},

              // Complex JSON tests
              {"/alarm/zones/control",
               "{\"zone\":\"Front Door\",\"action\":\"bypass\",\"extra\":\"This should be ignored\"}",
               "JSON with extra fields that should be ignored"},
};

/**
 * @brief Run the JSON MQTT test vectors
 *
 * This function iterates through all the JSON test vectors and calls
 * the processJsonMessage method of the Alarm class to test the JSON
 * processing functionality.
 *
 * @param alarm Reference to the Alarm class instance
 */
 //void runJsonMQTTTests(Alarm& alarm) {
 //    printf("\n=================== STARTING JSON MQTT TESTS ===================\n");
 //
 //    int passCount = 0;
 //    int totalTests = sizeof(jsonTestVectors) / sizeof(jsonTestVectors[0]);
 //
 //    for (int i = 0; i < totalTests; i++) {
 //        const auto& test = jsonTestVectors[i];
 //        printf("\n--------------------- Test #%d ----------------------\n", i + 1);
 //        printf("Description: %s\n", test.description);
 //        printf("Topic: %s\n", test.topic);
 //        printf("Payload: %s\n", test.payload);
 //
 //        // Process the JSON message
 //        bool result = alarm.processMqttMessage(
 //            test.topic,
 //            (byte*)test.payload,
 //            strlen(test.payload)
 //        );
 //
 //        printf("Result: %s\n", result ? "SUCCESS" : "FAILED");
 //        if (result) passCount++;
 //
 //        printf("------------------------------------------------------------\n");
 //
 //        // Optionally call alarm_loop to process any changes
 //        //alarm.alarm_loop();
 //    }
 //
 //    printf("\n=================== JSON MQTT TEST SUMMARY ===================\n");
 //    printf("Tests passed: %d / %d (%.1f%%)\n",
 //        passCount, totalTests, (float)passCount / totalTests * 100);
 //    printf("============================================================\n\n");
 //}
