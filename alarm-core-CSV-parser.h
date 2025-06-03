/**
 * @file parser.h
 *
 * Parses CSV file with alarm settings. Currently supports zones, partitions, pgms and global options settings.
 * CSV file supports two types of lines - headers and data lines. All tags (header line HEADER tag and DATA tags, as well as
 * data line HEADER tag) are defined as strings in alarm-defs.h
 * Header line format: <HEADER tag>  <DATA tag>  <DATA tag>  <DATA tag>  .....
 * Data line format:   <HEADER tag>  <data>  <data>  <data>  .....
 * Example of header line followed by data line:
 *  .---- HEADER  tag               .---- DATA tag
 *  zHeader; zName;	        zID;	zType;	        zPartn;	    zAlarmT;		zBypEn;	    zStayZ;	    zFrceEn;	zBRD;	    zShdnEn;	zIntelZ;	zDlyTRM;	zTmprGlb;	    zTmprOPT;	        zAmskGlb;	    zAmskOpt
 *  zone;	1_ED1_!S_!B_!F;	0;	    ENTRY_DELAY1;	1;			STEADY_ALARM;	FALSE;		FALSE;		FALSE;		0;			FALSE;		FALSE;		FALSE;		FALSE;			DISABLE;			FALSE;			DISABLE;
 *  ^- data line HEADER tag
 * Header lines begin with HEADER tag as "zHeader", "pgmHdr", etc, while data lines begin data line HEADER tags as "zone", "partition", etc
 * The header line shall preceed the data line. Header line DATA tags follows header line HEADER tag and define the content in the coresponding column. of the data line.
 * The tags allowed in the header line are defined in alarm-defs.h (ZN_* for zone header, PT_* for partition header GO_* for global options, etc)
 * and can be changed there. The way to handle the DATA tags are defined in corresponding struct tagAccess arrays as gOptsTags[], zoneTags[], ....
 * Before any data line to be imported, global options data shall be imported as there  global settings affecting the way
 * the data are interpreted - e.g. max slave (extender) boards are defined.
 * The way the HEADER tags are processed is defined by csvHeaders[] array, where for each header or data line HEADER tag corresponds entry of struct csvHeader_t.
 * Each entry contains of struct csvHeader_t, containing the  string for the HEADER tag to be parsed, and data needed for the tag processing,
 * including type of the data carried by the line with this tag, ponters to arrays with valid DATA tags, etc.
 * All HEADER tags are defined in csvHeaders[], while DATA tags are defined in corresponding arrays zoneTags[], partitionTags[], pgmTags[],
 * consisting of struct tagAccess entries. Poniters to one of this arrays is provided in entry of csvHeaders[].
 * Parsing is implemented in parseConfigFile(char buffer[], int bufferLen, int saveFlag), data to be parsed shall reside in buffer[] and save flag
 * says if parsing only or to store data as well. The CSV config file are fetched one line at the time by alarmGetLine() funct and
 * GET_FIRST_TOKEN and GET_NEXT_TOKEN macros are used to tokenize the stream.
 * The first token for the line (line HEADER tag candidate) is compared vs string member hdrTTL of struct csvHeader_t for each csvHeaders[] entry.
 * If found, the corresponding struct member callbacks for parsing and storing data contained in corresponding csvHeaders[] entry are called.
 * All header line tags parsing is implemented in getHeaderLine() funct and reduces to find if the current token (DATA tag candidate) is defined in
 * corresponding array of struct tagAccess.
 * If yes, the position of the DATA tag relative to the HEADER tag is noted in <pos> member of struct tagAccess. This offset is used later when parsing
 * the data line.
 * Parsing of the data lines is implemented in  getDataline() and is based of the relative offset of the current column
 * (first column after data line HEADER tag is 0). For each column the offset of the column is compared to
 * the offset noted in corresponding struct tagAccess array <pos> member during the processing of the DATA tags of the header line.
 * When matched, we know the data in this column for which header line DATA tag correspond. Then, the position in the memory and len of the current data,
 * is found from hdrParamArrPtr member of csvHeader_t, which points to struct tagAccess, specific for each header line DATA tag.
 * Basically processing in  * getDataLine is only storing the data in temporary storage tmpMax.
 * The data specific processing and moving to configuarion memory of the system is done by another callback defined in struct csvHeader_t csvHeaders[] - dataStoreCB().
 *
 * Adding new line HEADER. To add new line (header or data), add new entry to csvHeaders[]. The data tags shall be defined as struct tagAccess xxxxxTags[],
 * and correspondig pointer to it in the new csvHeaders[] entry. The corresponding DATA tags shall be added to struct tagAccess array.
 * Lines starting with '#' are comment line and DATA tag '#' specifies that this column shall be skipped
*/
#ifndef ARDUINO
#define isWhitespace(c) ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))  //!< is character whitespace?
#define isSpace(c) ((c == ' ') || (c == '\t'))  //!< is character space or tab?
#endif
//
// tmp storage 
union maxTmp_t {
    struct ALARM_ZONE           tmpZn;
    struct ALARM_PGM            tmpPgm;
    struct ALARM_GLOBAL_OPTS_t  tmpGopts;
    struct ALARM_PARTITION_t    tmpPart;
} maxTmp;

/**
 * @brief Contains all data needed to process line header
*/
struct csvHeader_t {
    const           char* hdrTTL;                                   //!< header title - first cell in each line to identify the line content (e.g. zone, partition, zHeader, pgmHeader, etc.)
    int			    domain_idx;						                //!< index in dbProps to find the database for the corresponding entry
    int             (*handlerCB)  (int dataTyp, char* strbuf);      //!< pointer to function to handle the rest of the line
    struct          tagAccess* hdrParamArrPtr;                     //!< array with all possible parameters in the header
    int             paramCnt;                                       //!< size of the above array
    unsigned int    setFlags;                                       //!< we need to have certain data in order to know how to interpreate current one - e.g. we need to get global options before any data, or to get header before the data line
    unsigned int    checkFlags;                                     //!< we need to have certain data in order to know how to interpreate current one - e.g. we need to get global options before any data, or to get header before the data line
    byte* dataPtr;                                        //!< pointer to temp storage to store the imported data
    int             (*dataStoreCB) (int dataTyp, maxTmp_t* src, int saveFl);    //!< fuct callback to postproces and copy line data from tmp storage to corrsponding database
};

// forward declarations of functions used in csvHeaders[]
int getHeaderLine(int hdrTyp, char* strbuf);
int getDataLine(int hdrTyp, char* strbuf);
int postprocesStoreImportedCSVData(int srcTyp, maxTmp_t* srcPtr, int saveFlag);

/**
 * @brief Contains all line headers allowed in CSV file
*/
struct csvHeader_t csvHeaders[] = {
        {CSV_ZONE_HDR_TTL,	    RESERVED,   &getHeaderLine, zoneTags,       ZONE_TAGS_CNT,            HAVE_ZN_HDR,            CLEAR_FLAGS,                                NULL,           NULL},
        {CSV_PRT_HDR_TTL,	    RESERVED,   &getHeaderLine, partitionTags,  PARTITION_TAGS_CNT,       HAVE_PRT_HDR,           CLEAR_FLAGS,                                NULL,           NULL},
        {CSV_PGM_HDR_TTL,	    RESERVED,   &getHeaderLine, pgmTags ,       PGM_TAGS_CNT,             HAVE_PGM_HDR,           CLEAR_FLAGS,                                NULL,           NULL},
        {CSV_GOPT_HDR_TTL,      RESERVED,   &getHeaderLine, gOptsTags,      GLOBAL_OPTIONS_TAGS_CNT,  HAVE_GOPT_HDR,          CLEAR_FLAGS,                                NULL,           NULL},
        {CSV_ZONE_TTL,		    ZONES,      &getDataLine,   zoneTags,       ZONE_TAGS_CNT,            CLEAR_FLAGS,            HAVE_GOPT_HDR | HAVE_ZN_HDR | HAVE_G_OPT,      (byte*)&maxTmp, &postprocesStoreImportedCSVData},
        {CSV_PRT_TTL,	        PARTITIONS, &getDataLine,   partitionTags,  PARTITION_TAGS_CNT,       CLEAR_FLAGS,            HAVE_GOPT_HDR | HAVE_PRT_HDR | HAVE_G_OPT,      (byte*)&maxTmp, &postprocesStoreImportedCSVData},
        {CSV_PGM_TTL,	        PGMS,       &getDataLine,   pgmTags ,       PGM_TAGS_CNT,             CLEAR_FLAGS,            HAVE_GOPT_HDR | HAVE_PGM_HDR | HAVE_G_OPT,      (byte*)&maxTmp, &postprocesStoreImportedCSVData},
        {CSV_GOPT_TTL,          GLOBAL_OPT, &getDataLine,   gOptsTags,      GLOBAL_OPTIONS_TAGS_CNT,  HAVE_G_OPT,             HAVE_GOPT_HDR,                              (byte*)&maxTmp, &postprocesStoreImportedCSVData},
        //{CSV_KEYSW_HDR_TTL,   KEYSW,      &getHeaderLine,   KEYSW_HDR   },
};
#define CSV_HEADERS_CNT         (sizeof(csvHeaders)/sizeof(csvHeaders[0]))

// flags used in csv parsing. used to mark that global options and header lines are imported in order to import data lines properly
unsigned long	csvParserFlags = 0;
//
char csvDel[] = ";";          // white space is not valid delimiter
char cmdDelim[] = " ";          // used for console commands, to be removed
char skipField[] = "NA";  // KEY_NOT_APPLICABLE;
//
#define CSV_COMMENT_CHAR '#'

/**
 * @brief   Get next token from line.
 * @details Saves the separator at the end of the isolated token in local static variable and replaces it with /0. 
 *
 * @param str   The input line buffer.
 * @param seps  Array of allowed separators.
 * @return      Index (position) of the token relative from start of the buffer.
*/
char* paxtok(char* str, const char* seps) {
    static char *tpos, *pos = NULL;        
    static const char* tkn;
    static char savech;

    // Specific actions for first and subsequent calls.

    if (str != NULL) {
        // First call, set pointer.
        pos = str;                                          // points to the start of the last token marked
        savech = 'x';                                       // shall contain last found separator in the string, it is replaced by NULL to form a NULL terminated token
    }
    else {
        // Subsequent calls, check we've done first.
        if (pos == NULL)
            return NULL;
        // Then put character back and advance.
        while (*pos != '\0')                                // find the end of the previous token first                              
            pos++;
        *pos++ = savech;                                    // and put the separator back in place (can be '\0' if this was the last token in line)
    }
    // Detect previous end of string.
    if (savech == '\0')                                     // was the separator '\0'? this means this was the last token in line
        return NULL;                                        // nothing to do, the line is exhausted
    // Now we have pos pointing to first character.         // TODO - WHAT's happening if we have several separators in raw?????
    // Find first separator or nul.
    tpos = pos;                                             // pos will point to the first char after separator at the end of the prev token
    // TODO - add code here to skip leading WS
    while (*tpos != '\0') {                                 // check if the current character of the string belongs to separators
        tkn = strchr(seps, *tpos);
        if (tkn != NULL)
            break;                                          // yes
        tpos++;
    }
    // TODO - add code here to skip treiling WS
    savech = *tpos;                                         // store the current char (separator)
    *tpos = '\0';                                           // and replace it with NULL to mark the end of the token
    return pos;                                             // return the start of the token
}
//
// !!!! TODO - add support for header row !!!!!
//



#define GET_FIRST_TOKEN if(!getToken(line, token, csvDel, true, NAME_LEN)) {lprintf("Invalid input designator, must be zone, partiton globalOpts or comment starting with #, aboring current line parsing\n"); continue;}
#define GET_NEXT_TOKEN if(!getToken(line, token, csvDel, false, NAME_LEN)) {lprintf("Invalid token, aboring current line parsing\n"); continue;}
//


/**
 * @brief   Cleans white space around token if any.
 * @param tok   Token
 * @return      Pointer to token.
*/
char* cleanWS(char* tok) {
    int i, j;
    for (i = 0; i < (int)strlen(tok); i++) {
        if (!isspace(tok[i]))
            break;
    }
    for (j = i; j < (int)strlen((const char*)tok); j++) {
        if (isspace(tok[j]))
            break;
    }
    tok[j] = 0;
    for (int s = i, d = 0; s <= j; s++, d++)         // copy in-place to fix WS in front
        tok[d] = tok[s];
    return &tok[0];
}

/**
 * @brief   Get the next token from buffer
 * @param inptr     Input buffer
 * @param outptr    Pointer to token
 * @param delim     Array with allowed delimiters
 * @param first     True if this is the first call of the funct for this buffer
 * @param maxLen    Maximum token lengt
 * @return          Pointer to token
*/
char* getToken(char* inptr, char* outptr, const char* delim, int first, int maxLen) {
    char* tok;
    if (!(tok = paxtok(first ? inptr : NULL, delim)))
        return NULL;
    if(strlen(tok) > (unsigned) maxLen -1)
        lprintf("Too long input token, max is %d chars... Trunkating\n", maxLen);
    //strncpy(outptr, tok, maxLen -1);
	snprintf(outptr, maxLen - 1, "%s", tok); // safer than strncpy, but still truncates if too long
    outptr[maxLen - 1] = '\0';
    cleanWS(outptr);
    return outptr;
}
/**
 * @brief   Imports and converts CSV data line values
 * @param hdrTyp    Type of data line to be processed - index in csvHeaders[]
 * @param strbuf    Pointer to buffer containing the data values - points to line to be processed after the line HEADER 
 * @return          1 if all data was converted successfully, othervise 0.
*/
int getDataLine(int hdrTyp, char* strbuf) {
    int i, j;
    // use global token[]
    if ((csvParserFlags & csvHeaders[hdrTyp].checkFlags) ^ csvHeaders[hdrTyp].checkFlags) {                      // check that corresponding header and global options are already imported
        LOG_CRITICAL("No header and/or global options imported yet, don't know how to handle data\n");
        return 0;
    }
    // find out for which parameter the current token must be converted
    // j contains relative token position in line (line header excludded), means first param has position 0
    // iterate for all entries in struct tagAccess array using i for index
    // and find content of which struct tagAccess [].pos entry matches the current position
    for (j = 0; getToken(strbuf, token, csvDel, false, NAME_LEN); j++) {
        if (!_stricmp(token, skipField))
            continue;
        for (i = 0; i < csvHeaders[hdrTyp].paramCnt; i++) {
            if (j == csvHeaders[hdrTyp].hdrParamArrPtr[i].pos) {
                //lprintf("Found option: key %s value %s at position %d\n", csvHeaders[hdrTyp].hdrParamArrPtr[i].keyStr, token, j);
                if (!csvHeaders[hdrTyp].hdrParamArrPtr[i].patchCallBack(csvHeaders[hdrTyp].dataPtr, csvHeaders[hdrTyp].hdrParamArrPtr[i].patchOffset, csvHeaders[hdrTyp].hdrParamArrPtr[i].patchLen, token))
                    return false;
               break;
            }
        }
        if (csvHeaders[hdrTyp].paramCnt == i)
            ;                                                                   // all keys checked till the end but current parma is not found
            //lprintf("Data value %s at position %d skipped\n", token, j);
    }
    csvParserFlags |= csvHeaders[hdrTyp].setFlags;                              // mark that header was imported by setting the appropriate flag in global var
    return 1;
}
/**
 * @brief   Parses the header line to identify and store the position of each field in header  in corresponding struct tagAccess 'pos' member 
 *          Which exact struct tagAccess is know from index (hdrTyp) in csvHeaders[].
 * 
 * The function uses the csvDel delimiter to tokenize the header line in strbuf.
 *
 * @param hdrTyp    The type of header to be parsed - zoneHdr, partititonHdr, pgmHdr, etc, representing index in csvHeaders[]
 * @param strbuf    The buffer containing the header line to parse.
 *
 * @return 1 if the parsing is successful, 0 otherwise.
 * 
 * Modifies global csvParserFlags;
 */
int getHeaderLine(int hdrTyp, char* strbuf) {
    int i, j;
    char token[NAME_LEN];
    for (j = 0; getToken(strbuf, token, csvDel, false, NAME_LEN); j++) {
        if (token[0] == '#')
            continue;
        for (i = 0; i < csvHeaders[hdrTyp].paramCnt; i++) {
            if (!_stricmp(token, csvHeaders[hdrTyp].hdrParamArrPtr[i].keyStr)) {
                csvHeaders[hdrTyp].hdrParamArrPtr[i].pos = j;
                //lprintf("Storing header field %s at position %d\n", csvHeaders[hdrTyp].hdrParamArrPtr[i].keyStr, j);
                break;
            }
        }
        if (i== csvHeaders[i].paramCnt) {
            LOG_CRITICAL("Unrecognized token: %s while parsing header\n", token);
            return 0;
        }
    }   
    csvParserFlags |= csvHeaders[hdrTyp].setFlags;                     // mark that header was imported by setting the appropriate flag in global var
    //ErrWrite(ERR_DEBUG, "CSV header %s successfully imported\n", csvHeaders[hdrTyp].hdrTTL);
    //printConfigHeader(keyArr, entryCnt);
    return 1;
}
/**
 * @brief   Postproces and copy the data from imported data line (located in temp storage) to corresponding index
 *          in corresponding database (zoneDB, partitionDB, etc)
 *          Which exact struct tagAccess is know from index (hdrTyp) in csvHeaders[].
 *
 * @param srcTyp    source data type - type of imported data in srcPtr - zone, partititon, pgm, etc,as index in dbPtrArr[]
 *                  allows to find the target for the imported data -zoneDB, pgmDB, etc 
 * @param srcPtr    The buffer containing the source data
 * @param saveFlag  parse only or parse and save
 *
 * @return 1 if is successful, 0 otherwise.
 *
 */
int postprocesStoreImportedCSVData(int srcTyp, maxTmp_t* srcPtr, int saveFlag) {
    int     idx;
    struct ALARM_ZONE* tmpZn;
    struct ALARM_PARTITION_t* tmpPart;
    struct ALARM_PGM* tmpPgm;
    struct ALARM_GLOBAL_OPTS_t* tmpGopts;
//
    switch (srcTyp) {
    case RESERVED:
        return 1;
        break;
    case ZONES:
        tmpZn = (struct ALARM_ZONE*)srcPtr;
        if (tmpZn->zoneType == ZONE_DISABLED)
            return 1;
        if (saveFlag) {
            if(alarm.addZone(*tmpZn) == -1) {
                LOG_CRITICAL("No space for zone, Zone %s not added\n", tmpZn->zoneName);
                return 0;
			}
            lprintf("Zone def %s successfully imported/stored\n", tmpZn->zoneName);
            //alarm.printConfigHeader(zoneTags, ZONE_TAGS_CNT);
            //alarm.printConfigData(zoneTags, ZONE_TAGS_CNT, (byte*)&zonesDB[idx], PRTCLASS_ALL);
        }
        return 1;
    case PARTITIONS:
        tmpPart = (struct ALARM_PARTITION_t *)srcPtr;
        if (tmpPart->partIdx >= MAX_PARTITION) {
            LOG_CRITICAL("Partition %d out of range\n", tmpPart->partIdx);
            return 0;
        }
        if (saveFlag) {
            idx = tmpPart->partIdx;
            //partitionDB[idx] = *tmpPart;
            if(alarm.addPartition(*tmpPart) == -1) {
                LOG_CRITICAL("No space for partition, Partition %s not added\n", tmpPart->partitionName);
                return 0;
			}
            lprintf("Partition def %s successfully imported/stored\n", tmpPart->partitionName);
            //alarm.printConfigHeader(partitionTags, PARTITION_TAGS_CNT);
            //alarm.printConfigData(partitionTags, PARTITION_TAGS_CNT, (byte*)&partitionDB[idx], PRTCLASS_ALL);
        }
        return 1;
    case PGMS:
        tmpPgm = (struct ALARM_PGM*)srcPtr;
        //if ((tmpPgm->boardID > maxSlaves) || (tmpPgm->pgmID > (tmpPgm->boardID ? SLAVE_PGM_CNT : MASTER_PGM_CNT))) {
        //    LOG_CRITICAL("PGM getDataLine: Invalid pgm No - Board: %d pgmID: %d\n", tmpPgm->boardID, tmpPgm->pgmID);
        //    return 0;
        //}
        //else if(saveFlag) {
        //    idx = BOARD_PGM_2_IDX(tmpPgm->boardID, tmpPgm->pgmID);
        //    pgmsDB[idx] = *tmpPgm;
        //    lprintf("PGM def %s successfully imported/stored\n", tmpPgm->pgmName);
        //    //printConfigHeader(pgmTags, PGM_TAGS_CNT);
        //    //printConfigData(pgmTags, PGM_TAGS_CNT,(byte*)&pgmsDB[idx], PRTCLASS_ALL);
        //}
        return 1;
    case GLOBAL_OPT:
        tmpGopts = (struct ALARM_GLOBAL_OPTS_t*)srcPtr;
        //if (tmpGopts->maxSlaveBrds > MAX_SLAVES) {
        //    lprintf("Global opts # of slave board: %d > compile time MAX_SLAVES: %d\n", tmpGopts->maxSlaveBrds, MAX_SLAVES);
        //    return 0;
        //}
        //if (saveFlag) {
        //    maxSlaves = tmpGopts->maxSlaveBrds;
        //    alarmGlobalOpts = *tmpGopts;
        //    lprintf("Global options successfully imported/stored\n");
        //    //printAlarmOpts((byte *)&alarmGlobalOpts);
        //}
        return 1;
    case KEYSW:
        LOG_CRITICAL("KEYSWITCHES are not supported yet !!!\n");
        return 0;
    default:
        LOG_CRITICAL("Postproces/Store CSV data error\n");
        return 0;
    }
}

/**
 * @brief   Resets header line DATA tags positions
 * @param keysArr 
 * @param cnt 
*/
void initHeaderPositions(struct tagAccess keysArr[], int cnt) {
    for (int i = 0; i < cnt; i++) {
        keysArr[i].pos = -1;
    }
}
//
void initAllHeadersPositions() {
    initHeaderPositions(zoneTags,    ZONE_TAGS_CNT);
    initHeaderPositions(partitionTags,    PARTITION_TAGS_CNT);
    initHeaderPositions(gOptsTags,   GLOBAL_OPTIONS_TAGS_CNT);
    initHeaderPositions(pgmTags,     PGM_TAGS_CNT);
}
// 
/**
 * Removes trailing whitespace and delimiter characters from a CSV line.
 * The line is modified in-place.
 *
 * @param line The CSV line to be modified.
 */
void fixEmptyCellsAtLineEnd(char* line) {
    size_t length = strlen(line);
    // Iterate backwards from the end of the line, skipping whitespace and delimiter(if any)
    while (length > 0 && (isWhitespace(line[length - 1]) || strchr(csvDel, line[length - 1]))) {
        line[--length] = '\0';          // Null-terminate the line at the last whitespace character
    }
}
/**
 * Reads a line from the input buffer and stores it in lineBuf.
 * The line is terminated by a newline character ('\n') or carriage return character ('\r'),
 * and any trailing whitespace is trimmed.
 *
 * @param lineBuf   The buffer to store the line read from the input.
 * @param lineLen   The maximum length of the line buffer.
 * @param inBuf     The input buffer containing the data to read from.
 * @param inBufLen  The length of the input buffer.
 * @param offset    The starting offset within the input buffer.
 *
 * @return The new offset within the input buffer after reading the line.
 */
unsigned int alarmGetLine(char lineBuf[], int lineLen, char inBuf[], unsigned int inBufLen, unsigned int offset) {
    int i = 0; int c;

    if (!inBuf) {
        printf("Null file ptr in alarmGetLine\n");
        return 0;
    }
    while (offset < inBufLen) {                             // (stream.available())
        c = inBuf[offset++];                                // read the next char
        if ((c == '\n') || (c == '\r')) {                   // check for LF/CR
            while (offset < inBufLen && isWhitespace(inBuf[offset]))
                offset++;                                   // Skip trailing whitespace after the line terminator
            break;
        }
        if (i < (lineLen - 1))                              // store in output buf
            lineBuf[i++] = c;
        else
            lprintf("Too long input line, skipping everything past 254th character\n ");
    }
    if (!i)                                                 // something in lineBuf ?
        return 0;                                           // nothing to return
    lineBuf[i] = 0;                                         // terminate the string in lineBuf
    fixEmptyCellsAtLineEnd(lineBuf);                        // excell will generate empty cells 
    return offset;
} 
 /**
 * @brief Parses a CSV configuration file stored in a buffer and updates the configuration data structure.
 *
 * The CSV file is expected to consist of sections for zones, partitions, global options, etc. Each section is composed of a header
 * line and one or more data lines. The header line is parsed for specific strings (tokens), and the position of each token is recorded
 * in corresponding arrays like zoneKeys, partitionKeys, gOptsTags. Later, when the data line is parsed, the data value at each
 * position is retrieved according to the recipe in xxxxKeys[], and the retrieved value is stored at the offset and length relative to
 * the pointer to the beginning of the corresponding data structure (e.g., ALARM_ZONE, ALARM_GLOBAL_OPTS_t, ALARM_PARTITION_t).
 * 
 * @param config A pointer to the CONFIG_t structure. If the data line is parsed correctly, the data from temporary data structures
 *               (e.g., ALARM_ZONE, ALARM_GLOBAL_OPTS_t, ALARM_PARTITION_t) is copied to the corresponding offset in the configuration
 *               data. NULL means no copy, just parse.
 * @param buffer The buffer containing the CSV data.
 * @param bufferLen The total length of the CSV data.
 * @param saveFlag Flag indicating whether to save the parsed data to the configuration or only parse it.
 *                 0 means parse only, non-zero means save to the configuration.
 * 
 * @return 0 on success, non-zero on errors.
 *
 * @note The function relies on global variables for certain operations, such as csvParserFlags, csvHeaders, and maxTmp.
 *       Ensure that these variables are correctly initialized and accessible before calling this function.
 *       The function returns 0 on success, and non-zero values on error conditions.
 */
int parseConfigFile(char buffer[], int bufferLen, int saveFlag) {
    unsigned int offset = 0;                                // stores the offset in the input line from which the next token shall be retrieved
    csvParserFlags = 0;                                     // global flag to track import dependancies e.g. header must be imported before data line
    int i;
    //
    // For flexibility, config data for particular item(zones, partitions, PGMS, etc) are organized as tables where every column
    // has header on top. Header line is parsed first and the what data to be expected in this collumn in the data line is recorded 
    // Reset first all 
    initAllHeadersPositions();  
    //
    // in a loop, fetch the data lines and depends on the content of the first column in each line, process it
    // it shall contain valid identifier from csvHeaders[];
    while ((offset = alarmGetLine(line, sizeof(line), buffer, bufferLen, offset))) {
        //printf("offset = %d\n", offset);  //printf("\nParsing input line:%s\n", line);
        memset((void*)&maxTmp, 0, sizeof(maxTmp));                          // clear temp storage
        //
        if (line[0] == CSV_COMMENT_CHAR)                                         // comment                     
            continue;
        GET_FIRST_TOKEN;                                                    // line specifier - zone, partition or globaOptions?
        printf("\nToken:%s\n", token);
        for (i = 0; i < CSV_HEADERS_CNT; i++) {                             // for all allowed headers
            if (_stricmp(token, csvHeaders[i].hdrTTL))                      // try to match the current token to any of the allowed headers
                continue;                                                   // not matching
            if (!csvHeaders[i].handlerCB(i, line)) {                        //  call callback to import the header/dataline content
                LOG_CRITICAL("CSV header/dataline %s import error\n", line);
                return 1;
            }
            if (csvHeaders[i].dataStoreCB) {                                // callback == NULL means we don't have to call it
                if (!csvHeaders[i].dataStoreCB(csvHeaders[i].domain_idx, &maxTmp, saveFlag)) {      //  call callback to postpocess and save the header/dataline content
                    LOG_CRITICAL("CSV header/dataline %s save error\n", line);
                    return 1;
                }
            }
            break;                                                          // we found what we needed, exit the loop
        }
        if (i == CSV_HEADERS_CNT) {
            LOG_CRITICAL("Unrecognized top-level token %s\n", token);
            return 1;
        }
    }
    return 0;                                        // TODO - shall have some config data validation

}

//
// match keyboard command to implemented commands
// parms: 
//      struct inpPlds_t Cmds[] - array of input commands: {const char cmd[32]; const char MQTTcmd[32]; unsigned int action; void (*callBack)(byte zoneOrPart, byte action);}
//      int CmdsCnt             - number of entries in array
//      const char payld[]      - string to match again commands
// return: 
//      NULL                    - if nothing matches
//      inpPlds_t*              - pointer to array entry (struct) with command
//
//struct inpPlds_t* processKBQcontrol(struct inpPlds_t Cmds[], int CmdsCnt, const char payld[]) {
//    int i;
//    for (i = 0; i < CmdsCnt; i++) {
//        if (!_stricmp(payld, Cmds[i].consoleCmd)) {
//            if (strlen(Cmds[i].consoleCmd) != strlen(payld))
//                continue;
//            lprintf("Found CMD: KBD cmd %s, action = %d\n", Cmds[i].consoleCmd, Cmds[i].action);
//            return &Cmds[i];
//        }
//    }
//    //lprintf("Unrecognized KBD command payload:  %s\n", payld);
//    return NULL;
//}


