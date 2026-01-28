
#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include "WiFi.h"
WiFiClient espClient;
#include "PubSubClient.h"
extern const char* mqttServer;
extern const int   mqttPort;
PubSubClient MQTTclient(mqttServer, mqttPort, espClient);

#include "..\..\alarm-core-config.h"
#include "..\..\src\alarm-core-debug.h"
#include "..\..\alarm-FS-wrapper.h"
#include "..\..\alarm-core.h"
#include "..\..\src\alarm-core-JSON.h" // Include the new header-only parser
#include "..\..\alarm-core-mqtt.h"

#define WiFiRetryCnt 10
// 
// WIFIsetup - try to connect to WLAN. Expects to be called by ARDUINO loop function
// rtry is internal static counter how many times WIFIsetup was called. The goal is 
// to just initiate the connection if not established and to return to master loop function
// If after WiFiRetryCnt trys is still not connected, initiates new connection request to AP.
// rtry is 0 only if the link is established and all post connection settings are done
// otherwise rtry counts from 0 up to WiFiRetryCnt. The only exception is when the function is called for the first time.
// delay 10 ms is added, seems for proper WIFI operation it is needed
//
int WIFIsetup() {
#ifndef ARDUINO
	return true;
#endif
	static unsigned long rtry = 0;
	// connect to WiFi Access Point
	if (WiFi.status() == WL_CONNECTED) {
		if (rtry) {									// do it only once after connected,
			lprintf("Connected after %ld ms\n", rtry / 10);
			WiFi.setAutoReconnect(true);
			WiFi.persistent(true);
			rtry = 0;
		}
		return true;
	}
	if ((rtry % WiFiRetryCnt) == 0) {					// will execute only if rtry  is 0 or WiFiRetryCnt  try to connect again
		rtry = 1;									//  try to connect again
		lprintf("Start new attempt to connect to WiFi AP %s\n", ssid[WiFi_AP]);
		WiFi.mode(WIFI_STA);
		WiFi.begin(ssid[WiFi_AP], password[WiFi_AP], 0, NULL, true);
	}
	else {
		delay(10);
		rtry++;
	}
	return false;
}
//
// global storage definitions
char prnBuf[1024];
char token[256];
char jsonBuffer[32768];

void runJsonMQTTTests(Alarm& alarm);

void debugPrinter(const char* message, size_t length) {
    printf("[DEBUG] %.*s\n", (int)length, message);
}

// instance of the Alarm class
Alarm my_alarm;
// instance of the JSON parser class
alarmJSON parser(my_alarm);
// Create the MqttProcessor, "injecting" the dependencies (myAlarm and myJsonParser).
MqttProcessor myMqttProcessor(parser);




void setup() {
    Serial.begin(115200);
    delay(100);
    log_i("Init start\n");
    storageSetup();
    //initAlarmLoop()

    LOG_DEBUG("Starting Alarm Core JSON MQTT Tests...\n");
	// debug callback setup
    my_alarm.setDebugCallback(GlobalDebugLogger);
    my_alarm.debugCallback(LOG_ERR_OK, "test\n");

	// MQTT publisher setup
    // args passed to the Alarm class: static wrapper function (mqttPublishWrapper) and pointer of your client object (&mqttClient)
    //my_alarm.setPublisher(mqttPublishWrapper, &mqttClient);


    if (!loadConfig(jsonConfigFname, (byte *)jsonBuffer, sizeof(jsonBuffer))) {
        printf("Failed to load config file\n");
        return;
    }
    if(!parser.parseConfigJSON(jsonBuffer)) {
        printf("Failed to parse config JSON\n");
        return -1;
	}

    parser.parseConfigJSON(jsonBuffer);

    //runJsonMQTTTests(my_alarm);


    listDir(LittleFS, "/", 3);

    //readFile2str(LittleFS, "/alarm-config.json", jsonString);
    //printf ("JSON string = %s\n", jsonString);

    // Print the zone configuration regardless of warnings
    //alarm.printConfigData(zoneTags, ZONE_TAGS_CNT, (byte*)&zone, PRTCLASS_ALL);
    //alarm.printAlarmOpts((byte*)&alarmGlobalOpts);
    my_alarm.printAlarmPartition(0, MAX_PARTITION);
    my_alarm.printAlarmZones(0, MAX_ALARM_ZONES);
    my_alarm.printAlarmPgms();
    //return 0;
    printf("Done\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}


//if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
//    Serial.println("LittleFS Mount Failed");
//    return;
//}

/* You only need to format LittleFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin
   If you test two partitions, you need to use a custom
   partition.csv file, see in the sketch folder */


#define FORMAT_LITTLEFS_IF_FAILED false

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}


void readFile2str(fs::FS &fs, const char *path, char * buffer) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }
  int i = 0;
  Serial.println("- read from file:");
  while (file.available()) {
      // Read one character at a time and append to the string
      char c = file.read();
      Serial.write(c);
      buffer[i++] = c;
  }
  buffer[i] = NULL;
  file.close();
}

// void createDir(fs::FS &fs, const char *path) {
//   Serial.printf("Creating Dir: %s\n", path);
//   if (fs.mkdir(path)) {
//     Serial.println("Dir created");
//   } else {
//     Serial.println("mkdir failed");
//   }
// }

// void removeDir(fs::FS &fs, const char *path) {
//   Serial.printf("Removing Dir: %s\n", path);
//   if (fs.rmdir(path)) {
//     Serial.println("Dir removed");
//   } else {
//     Serial.println("rmdir failed");
//   }
// }

// void readFile(fs::FS &fs, const char *path) {
//   Serial.printf("Reading file: %s\r\n", path);

//   File file = fs.open(path);
//   if (!file || file.isDirectory()) {
//     Serial.println("- failed to open file for reading");
//     return;
//   }

//   Serial.println("- read from file:");
//   while (file.available()) {
//     Serial.write(file.read());
//   }
//   file.close();
// }


// void writeFile(fs::FS &fs, const char *path, const char *message) {
//   Serial.printf("Writing file: %s\r\n", path);

//   File file = fs.open(path, FILE_WRITE);
//   if (!file) {
//     Serial.println("- failed to open file for writing");
//     return;
//   }
//   if (file.print(message)) {
//     Serial.println("- file written");
//   } else {
//     Serial.println("- write failed");
//   }
//   file.close();
// }

// void appendFile(fs::FS &fs, const char *path, const char *message) {
//   Serial.printf("Appending to file: %s\r\n", path);

//   File file = fs.open(path, FILE_APPEND);
//   if (!file) {
//     Serial.println("- failed to open file for appending");
//     return;
//   }
//   if (file.print(message)) {
//     Serial.println("- message appended");
//   } else {
//     Serial.println("- append failed");
//   }
//   file.close();
// }

// void renameFile(fs::FS &fs, const char *path1, const char *path2) {
//   Serial.printf("Renaming file %s to %s\r\n", path1, path2);
//   if (fs.rename(path1, path2)) {
//     Serial.println("- file renamed");
//   } else {
//     Serial.println("- rename failed");
//   }
// }

// void deleteFile(fs::FS &fs, const char *path) {
//   Serial.printf("Deleting file: %s\r\n", path);
//   if (fs.remove(path)) {
//     Serial.println("- file deleted");
//   } else {
//     Serial.println("- delete failed");
//   }
// }

// // SPIFFS-like write and delete file, better use #define CONFIG_LITTLEFS_SPIFFS_COMPAT 1

// void writeFile2(fs::FS &fs, const char *path, const char *message) {
//   if (!fs.exists(path)) {
//     if (strchr(path, '/')) {
//       Serial.printf("Create missing folders of: %s\r\n", path);
//       char *pathStr = strdup(path);
//       if (pathStr) {
//         char *ptr = strchr(pathStr, '/');
//         while (ptr) {
//           *ptr = 0;
//           fs.mkdir(pathStr);
//           *ptr = '/';
//           ptr = strchr(ptr + 1, '/');
//         }
//       }
//       free(pathStr);
//     }
//   }

//   Serial.printf("Writing file to: %s\r\n", path);
//   File file = fs.open(path, FILE_WRITE);
//   if (!file) {
//     Serial.println("- failed to open file for writing");
//     return;
//   }
//   if (file.print(message)) {
//     Serial.println("- file written");
//   } else {
//     Serial.println("- write failed");
//   }
//   file.close();
// }

// void deleteFile2(fs::FS &fs, const char *path) {
//   Serial.printf("Deleting file and empty folders on path: %s\r\n", path);

//   if (fs.remove(path)) {
//     Serial.println("- file deleted");
//   } else {
//     Serial.println("- delete failed");
//   }

//   char *pathStr = strdup(path);
//   if (pathStr) {
//     char *ptr = strrchr(pathStr, '/');
//     if (ptr) {
//       Serial.printf("Removing all empty folders on path: %s\r\n", path);
//     }
//     while (ptr) {
//       *ptr = 0;
//       fs.rmdir(pathStr);
//       ptr = strrchr(pathStr, '/');
//     }
//     free(pathStr);
//   }
// }

// void testFileIO(fs::FS &fs, const char *path) {
//   Serial.printf("Testing file I/O with %s\r\n", path);

//   static uint8_t buf[512];
//   size_t len = 0;
//   File file = fs.open(path, FILE_WRITE);
//   if (!file) {
//     Serial.println("- failed to open file for writing");
//     return;
//   }

//   size_t i;
//   Serial.print("- writing");
//   uint32_t start = millis();
//   for (i = 0; i < 2048; i++) {
//     if ((i & 0x001F) == 0x001F) {
//       Serial.print(".");
//     }
//     file.write(buf, 512);
//   }
//   Serial.println("");
//   uint32_t end = millis() - start;
//   Serial.printf(" - %u bytes written in %lu ms\r\n", 2048 * 512, end);
//   file.close();

//   file = fs.open(path);
//   start = millis();
//   end = start;
//   i = 0;
//   if (file && !file.isDirectory()) {
//     len = file.size();
//     size_t flen = len;
//     start = millis();
//     Serial.print("- reading");
//     while (len) {
//       size_t toRead = len;
//       if (toRead > 512) {
//         toRead = 512;
//       }
//       file.read(buf, toRead);
//       if ((i++ & 0x001F) == 0x001F) {
//         Serial.print(".");
//       }
//       len -= toRead;
//     }
//     Serial.println("");
//     end = millis() - start;
//     Serial.printf("- %u bytes read in %lu ms\r\n", flen, end);
//     file.close();
//   } else {
//     Serial.println("- failed to open file for reading");
//   }
// }
