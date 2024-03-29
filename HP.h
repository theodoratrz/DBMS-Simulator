/*
 * File: HP.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#ifndef HP_H
#define HP_H

/*
 * File: HP.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#define HP_INFO_SIZE 12
#define RECORD_SIZE 94
#define MAX_RECORDS 5

typedef struct{
    int id;
    char name[15];
    char surname[25];
    char address[50];
} Record;

typedef struct {
    int fileDesc;       /* File Descriptor */
    char attrType;      /* Key type: 'c', 'i' */
    char *attrName;     /* Key field name */
    int attrLength;     /* Key length (in bytes) */
} HP_info;

/* General HP functions ----------------------------------------------------------------- */

int HP_CreateFile(  const char *fileName,       /* File name */
                    const char attrType,        /* Key type: 'c', 'i' */
                    const char* attrName,       /* Key field name */
                    const int attrLength        /* Key length (in bytes?) */ );

HP_info* HP_OpenFile( const char *fileName );

int HP_InitFile(int fd, char type, const char *name, int length);

int HP_CloseFile( HP_info* header_info );

int HP_InsertEntry( HP_info header_info, /* File Header information */
                    Record record        /* Record to be inserted */ );

int HP_DeleteEntry( HP_info header_info,    /* File Header information */
                    void *value             /* Key value of record to be deleted */ );

int HP_GetAllEntries( HP_info header_info,    /* File Header information */
                      void *value             /* Key value of record to be found */ );

/* HP_info functions ----------------------------------------------------------------- */

void* HP_GetInfoData(const HP_info* info);

void delete_HP_info(HP_info *info);

HP_info* Get_HP_info(int fd);

/* Block functions ------------------------------------------------------------------- */

void HP_SetNextBlockNumber(void *current, int num);

int HP_GetNextBlockNumber(void *current);

void HP_SetNumRecords(void *block, int n);

int HP_AddNextBlock(int fd, int current_num);

int HP_InsertRecordtoBlock(int fd, int current_num, Record rec);

int HP_GetNumRecords(void *block);

void* GetLastRecord(void *block);

int HP_DeleteRecordFromBlock(void *block, const char *key_name, void *value);

int BlockHasRecordWithKey(void *block, const char* key_name, Record *rec);

int PrintBlockRecordsWithKey(void *block, const char *key_name, void *value);

/* Record functions ------------------------------------------------------------------ */

void* GetRecordData(const Record *rec);

Record* GetRecord(const void *data);

void PrintRecord(Record rec);

void CopyRecord(void *dest, void *src);

void* NextRecord(void *current);

int HP_RecordKeyHasValue(void *record, const char *key_name, void *value);

#endif