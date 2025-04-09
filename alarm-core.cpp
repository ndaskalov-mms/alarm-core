// alarm-core.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
	/* vsprintf example */
#include <stdarg.h>


#define MASTER
#define lprintf			printf
#define isWhitespace	isspace
#define isSpace			isspace
typedef unsigned char	byte;
#define HIGH			1
#define LOW				0
//
//#include "common_defs.h"
#include <windows.h>
unsigned long millis() {
	SYSTEMTIME st;
	GetSystemTime(&st);
	//printf("The system time is: %02d:%02d:%02d\n", st.wHour, st.wMinute,st.wSecond);
	unsigned long res = (st.wHour * 60 * 60 * 1000 + st.wMinute * 60 * 1000 + st.wSecond * 1000 + st.wMilliseconds);
	//printf("Time in mS: %lu\n", res);
	return res;
}

/* DUMMY STAFF TO COMPILE ARDUINO SPECIFIC STAFF ON WIN */
#define OUTPUT 1
#define INPUT 0
void	digitalWrite(int pin, int value) {}
int		digitalRead(int pin) { return 1; }
void	pinMode(int pin, int mode) {}
int		analogRead(int pin) { return 100; }
int     check4msg(int, int, int) { return true; }
void	master2slave(void) {}
int		WIFIsetup(void) { return 1; }
int		MasterMsgChannel = 0;
int		rcvMsg = 0;
void	masterProcessMsg(int) {};
int		isTimeFor(int, int) { return true; };
int		POLL_ZONES = 0;
int		pollSlaveZones(int, byte*) { return 0 /*ERR_OK*/; }


//void	copyFakeZonesStat() {}
//void	copyZonesStat() {}
//void	synchPGMstates() {}
//
//
FILE* scriptFile = NULL;
char	scriptPld[256];					// to hold scrit line MQTT payload
char	scriptTopic[256];				// to hold scrit line MQTT topic
//
struct	scriptReply {
	char topic[sizeof(scriptTopic)];
	char payload[sizeof(scriptTopic)];
	int  result;
	int  valid;
};
#define maxMQTT 32
struct scriptReply mustHave[maxMQTT];												// arrays to store what script prescribed to expect from alarm
struct scriptReply mustNotHave[maxMQTT];												// as MQTT output

//WiFi.status() == WiFi.status)
//WiFi.setAutoReconnect(true);
//WiFi.persistent(true);
//WiFiRetryCnt == 0) 
//WiFi.mode(WIFI_STA);
//WiFi.begin(ssid[WiFi_AP], password[WiFi_AP], 0, NULL, true);
//
class WLAN
{
public:
	int status()
	{
		return true;
	}
	void setAutoReconnect(int set)
	{
		return;
	}
	void persistent(int set)
	{
		return;
	}
	void mode(int set)
	{
		return;
	}
	void begin(const char*, const char*, int, int, int) { return; }

};
WLAN WiFi;
//
#define WL_CONNECTED	0
//#define	WiFiRetryCnt	2
#define	WIFI_STA		1
// 
class serial
{
public:
	void begin(int, int) { return; }
};
serial logger;
//
void delay(int) { return; }
#define	LOG_BITRATE 1200
#define	SERIAL_8N1	1
//
//
class MQTT
{
public:
	int bufLen = 0;
	int conn = 0;
	int connected()
	{
		return conn;
	}
	int disconnect()
	{
		return true;
	}
	int subscribe(char* topic)
	{
		return true;
	}
	void setCallback(void (*f)(char*, byte*, unsigned int)) {
	}
	int setBufferSize(int len) {
		bufLen = len;
		return true;
	}
	int getBufferSize() {
		return bufLen;
	}
	int connect(const char* topic, const char[], const char[], const char[], int, int, const char[], int) {
		conn = true;
		return true;
	}
	int state() {
		return 5;
	}
	void loop() {

	}
	//;
	void publish(const char* tempMQTTbuf, const char* payload)
	{
		int i;
		lprintf("MQTT send: %s %s\n", tempMQTTbuf, payload);
		for (i = 0; i < maxMQTT; i++) {
			if (mustHave[i].valid == false)
				continue;
			if (!strcmp(tempMQTTbuf, mustHave[i].topic)) {												// send script CMD?
				if (!strcmp(payload, mustHave[i].payload)) {
					mustHave[i].result = true;
					continue;
				}
			}
		}
		for (i = 0; i < maxMQTT; i++) {
			if (mustNotHave[i].valid == false)
				continue;
			if (!strcmp(tempMQTTbuf, mustNotHave[i].topic)) {												// send script CMD?
				if (!strcmp(payload, mustNotHave[i].payload)) {
					mustNotHave[i].result = true;
					continue;
				}
			}
		}
	}
};
MQTT MQTTclient;
/* END OF DUMMY STAFF TO COMPILE ARDUINO SPECIFIC STAFF ON WIN */
//
//int WIFIsetup(void) {
//	return true;
//}
//
// ------------------------- global variables definition -----------------------------
//
//int newConfigAvailable = false;
//
//
//#include "credentials.h"
//#include "errors.h"
//#include "zonen.h"
//#include "alarm-defs.h"
//#include "loadStore.h"
//#include "parserHelpers.h"
//#include "mqtt.h"
//#include "alarmHelpers.h"
//
//#include "parser.h"
//#include "alarm_logic.h"
//#include "json.h"

#include "../master.h"
//
unsigned long analogReadMilliVolts(int pin) {
	for (int i = 0; i < (sizeof(MzoneDB) / sizeof(MzoneDB[0])); i++) {
		if (MzoneDB[i].gpio == pin) {
			return MzoneDB[i].mvValue;
		}
	}
	ErrWrite(ERR_CRITICAL, "CAnnot find master zone with GPIO %d", pin);
	return 0;
}
//
//
//
//  Arduino setup function - call all local setups her
//
//
//void processMasterPGMs() {
//	lprintf("Setting master PGM\n");
//	for (int i = 0; i < MAX_ALARM_PGM; i++) {
//		if (pgmsDB[i].boardID != MASTER_ADDRESS)
//			continue;
//		setPgm(MpgmDB, pgmsDB[i].pgmID, pgmsDB[i].tValue);
//		pgmsDB[i].cValue = pgmsDB[i].tValue;
//
//		//PublishMQTT(pgmsDB[i].cValue?PGM_ON_PROPERTY:PGM_OFF_PROPERTY, MQTT_OUTPUTS_STATES, pgmsDB[i].pgmName);
//	}
//}
//
//
//
//void setup() {
//	lprintf("\n\nStarting setup\n\n");
//	zoneHWSetup();                                  // init mux for zones selection
//	pgmSetup(SpgmDB, SLAVE_PGM_CNT);              // init PGMs (output and default value)
//	//printErrorsDB();
//	ErrWrite(ERR_DEBUG, "\n\nSetup finished\n\n");
//	//
//	/*
//	if (!storageSetup()) {                      // mount file system
//		while (true) {                          // loop forever
//			Sleep(1000);
//			PublishMQTT(ERROR_TOPIC, "Error initializing storage");
//		}
//	}
//	// read config file from storage and init all alarm internals and databases for zones, pgms, partitions, keswitches, etc
//	if (!initAlarmLoop()) {        				// read setting from storage and set all variables
//		setAlarmDefaults(false);                // init zones and pgms DBs with default data and set valid flag to false to all zones and pgms 
//		while (true) {
//			Sleep(1000);
//			lprintf("Looping for getting alarm settings from MQTT\n"); // something wrong happened, decide what to do here
//		}
//	}
//	storageClose();								// unmount FS
//	*/
//	newConfigAvailable = 1;
//	//zoneHWSetup();                              // init mux for zones selection
//	//pgmSetup(MpgmDB, MASTER_PGM_CNT);			// init PGMs (output and default value)
//}
//
//
void processKbd() {
	int c; static int i = 0;
	if (!_kbhit())							// no keypress, return
		return;
	// we have a keypress, process it
	c = _getch();
	c &= 0x7F;								// arrows generate negative values which cause assert in strlen
	_putch(c);								// output to console
	if (c == '\r') {						// end of line?
		_putch('\n');						// yes
		line[i] = 0;						// add terminating char
		//lprintf("Got line: %s\r", line);	
		//cmdGetLine();						// process input line here
		memset(line, ' ', strlen(line));	// prepare for the next input
		//lprintf("%s\r", line);
		i = 0;
	}
	else {
		line[i++] = c;						// store the input in line buffer
		if (i == sizeof(line)) {			// check for overflow
			lprintf("Input overflow\r");
			i = 0;
		}
	}
}
//
void cleanMQTTarrays() {
	for (int i = 0; i < maxMQTT; i++) {
		mustHave[i].result = false;
		mustHave[i].valid = false;
		mustHave[i].topic[0] = 0;
		mustNotHave[i].result = false;
		mustNotHave[i].valid = false;
		mustNotHave[i].topic[0] = 0;
	}
}
//
int  checkMQTTarrays() {
	int res = 0;
	for (int i = 0; i < maxMQTT; i++) {
		if (mustHave[i].valid) {
			res += mustHave[i].result ? 0 : 1;
			if (!mustHave[i].result)
				lprintf("!!!!! \a\a\aERROR - MUST HAVE: %s %s \t\t\tNOT RECEIVED\n", mustHave[i].topic, mustHave[i].payload);
		}
		if (mustNotHave[i].valid) {
			res += mustNotHave[i].result;
			if (mustNotHave[i].result)
				lprintf("!!!!! \a\a\a ERROR - MUST NOT HAVE: %s %s \t\t\tRECEIVED\n", mustNotHave[i].topic, mustNotHave[i].payload);

		}
	}
	if (res) {
		//		printf("Press any key to continue!\n");
		while (!_kbhit())							// no keypress, return
			;
	}
	return res;
}
//
char* fgetline(char* str, int num, FILE* stream) {
	int i = 0;
	int c;
	if (stream == NULL) {
		lprintf("Null file ptr in getline\n");
		return NULL;
	}
	while (true) {									// reserve space for terminating null) {
		c = fgetc(stream);
		if (feof(stream)) {
			if (i)
				break;                              // EOF with no LF/CR, there is something to return
			else
				return NULL;                        // nothing in the file
		}
		if ((c == '\n') || (c == '\r')) {           // 
			//printf("\n");
			while (!feof(stream)) {                 // read all conrol characters after CR
				if (!isWhitespace(c = fgetc(stream))) {
					ungetc(c, stream);
					break;
				}
			}
			break;
		}
		//lprintf("%c", c);
		if (i < num)
			str[i++] = c;
		else
			lprintf("Too long input line, skipping everything past 255th character\n ");
	}
	str[i] = 0;
	fixEmptyCellsAtLineEnd(str);                     // excell will generate empty cells 
	return str;
}
//
int processScript(FILE* scriptFilePtr) {
	//
	int posIdx = 0; int negIdx = 0;
	static int wait = 0;
	unsigned long interval = SCRIP_WAIT_INTERVAL;
	static int skipMode = false;
	if (wait) {
		if (!timeoutOps(GET, SCRIPT_WAIT_TIMER))									// wait?							
			return ~EOF;															// yes, return to run the alarmLoop
		else
			wait = false;
	}
	while (!wait)
	{
		if (!fgetline(line, sizeof(line), scriptFilePtr)) {
			lprintf("Script file processed\n");
			return EOF;																// no more commands in script
		}
		if (!_strnicmp(line, "SKIP_MODE_START", strlen("SKIP_MODE_START"))) {
			skipMode = true;
		}
		if (!_strnicmp(line, "SKIP_MODE_STOP", strlen("SKIP_MODE_STOP"))) {
			skipMode = false;
		}
		if (skipMode)																// skip all
			continue;
		if (line[0] == '#') {														// comment
			lprintf("\t\t\t\t\t\t\t\t\t"); lprintf(line); lprintf("\n");
			if (!strncmp(line, "#-STOP_HERE", 10))
				printf("On the spot\n");
			continue;
		}
		lprintf("\t\t\t\t\t\t\t\t\t"); lprintf(line); lprintf("\n");
		if (!getToken(line, token, cmdDelim, true, sizeof(token) - 1))				// empty line?
			continue;
		if (!_strnicmp(token, "-->", 3)) {											// send script CMD?
			if (!getToken(line, scriptTopic, cmdDelim, false, sizeof(scriptTopic))) {	// yes, get topic
				lprintf("Script error: no topic found in: %s\n", line);
				continue;
			}
			if (!getToken(line, scriptPld, cmdDelim, false, sizeof(scriptPld))) {	// get payload
				lprintf("Script error: no payload found in: %s\n", line);
				continue;
			}
			MQTTcallback(scriptTopic, (byte*)scriptPld, strlen(scriptPld));		// call to process
		}
		else if (!_strnicmp(token, "<--", 3)) {										// must receive MQTT reply
			if (!getToken(line, scriptTopic, cmdDelim, false, sizeof(scriptTopic))) {	// yes, get topic
				lprintf("Script error: no topic found in: %s\n", line);
				continue;
			}
			if (!getToken(line, scriptPld, cmdDelim, false, sizeof(scriptPld))) {// get payload
				lprintf("Script error: no payload found in: %s\n", line);
				continue;
			}
			if (posIdx < 32) {
				strncpy(mustHave[posIdx].payload, scriptPld, strlen(scriptTopic) + 1);	// store in the positive commands array
				strncpy(mustHave[posIdx].topic, scriptTopic, strlen(scriptTopic) + 1);	// add terminating NULL as well
				mustHave[posIdx].result = false;
				mustHave[posIdx].valid = true;
				posIdx++;
			}
			else
				lprintf("Script error: no room for return MQTTs (<--)\n");
		}
		else if (!_strnicmp(token, "x--", 3)) {										// must NOT receive MQTT reply
			if (!getToken(line, scriptTopic, cmdDelim, false, sizeof(scriptTopic))) {	// yes, get topic
				lprintf("Script error: no topic found in: %s\n", line);
				continue;
			}
			if (!getToken(line, scriptPld, cmdDelim, false, sizeof(scriptPld))) {// get payload
				lprintf("Script error: no payload found in: %s\n", line);
				continue;
			}
			if (negIdx < 32) {
				strncpy(mustNotHave[negIdx].payload, scriptPld, strlen(scriptTopic) + 1);
				strncpy(mustNotHave[negIdx].topic, scriptTopic, strlen(scriptTopic) + 1);
				mustNotHave[negIdx].result = false;
				mustNotHave[negIdx].valid = true;
				negIdx++;
			}
			else
				lprintf("Script error: no room for return MQTTs (<--)\n");
		}
		else if (!_strnicmp(token, "wait", 4)) {
			if (!getToken(line, scriptTopic, cmdDelim, false, sizeof(scriptTopic))) {	// yes
				interval = SCRIP_WAIT_INTERVAL;					// no waiting time specified
			}
			else {
				interval = atol(scriptTopic) * 1000;
			}
			timerSetInterval(SCRIPT_WAIT_TIMER, interval);
			timeoutOps(SET, SCRIPT_WAIT_TIMER);
			wait = true;
		}
		else if (!_strnicmp(token, "check", 5)) {
			if (!checkMQTTarrays()) {
				printf("------------------------------EXPECTED RESULTS MATCH--------------------------------------\n\n");
			}
			else {
				printf("!!!!!!!!!!!!!!!!!!!!!!!!------EXPECTED RESULTS DOES NOT MATCH--------!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
			}
			cleanMQTTarrays();														// prepare for the next command
		}
		else {
			lprintf("Unrecognized script command %s", token);
		}
	}
	return ~EOF;
}
// 

//
//
int main(int argc, char* argv[])
{
	int fromScript = false;

	// check if we will execute scripts from file or will process keyboard
	if (argc == 2) {
		scriptFile = fopen((argv[1]), "r");
		if (scriptFile == NULL)
			ErrWrite(ERR_CRITICAL, "Problem opening script file\n");
		else
			fromScript = true;
	}

	// clear  arrays are used in test mode to check that what is sent via MQTT matches what is specified in script file
	cleanMQTTarrays();

	// in PC mode commands are read from script file orfrom console
	while (true) {
		//lprintf("Master loop\n");
		master();
		if (fromScript) {
			if (EOF == processScript(scriptFile))
				exit(0);								// done with script
		}
		else
			processKbd();								// process keyboard
		//
	}
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
