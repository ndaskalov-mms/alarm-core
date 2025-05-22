#pragma once
// credentials.h

#ifndef _credentials_h
#define _credentials_h
#endif
// WLAN
#define     NUMBER_OF_AP					1
byte        WiFi_AP = 0;										// The WiFi Access Point we are connected to 
const       char* ssid[NUMBER_OF_AP] = { /*"HlebarkaDE"*/	"hlebarka2" };
const       char* password[NUMBER_OF_AP] = { /*"Roksi6moksi"*/	 "fenwowmo" };
//
// MQTT
const char* mqttServer = "192.168.33.201"; //"192.168.0.228";
const int   mqttPort = 1883;
const char* mqttUser = ""; // "alarm";
const char* mqttPassword = ""; // "12345";
//
// config file(s)
const char masterDataPrefix[] = "../master/data";
const char jsonConfigFname[] = "/alarmConfig.json";
//
// //const char configFileName[]					= "/alarmConfig3.cfg";
//#define completeFilePath(prefix,filename)	prefix ## filename
//#define filename(x)						x
//#define configFileName					"/alarmConfig3.cfg"
//#define masterDataPrefix					"../master/data"
//#define configFilePath					("../master/data" ## "/alarmConfig3.cfg")
//#define csvConfigFname					= "/alarmConfig-ZoneTests-1.csv"; 
//#define csvBackupFname					= "/csvBackup.csv";
//const char csvConfigFname[]				= "C:/Users/Nik/source/repos/alarmTest/alarmConfig-ZoneTests-1.csv";// alarmConfig - 7.csv"; alarmConfig-SingleZone-PGM.csv";
//printf("Complete File name: %s\n", completeFilePath(filename(masterDataPrefix), filename(configFileName)));
