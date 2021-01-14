/*
 * File: SHT.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#ifndef SHT_H
#define SHT_H

#include "HT.h"

typedef struct {
    Record record;
    int blockId;
} SecondaryRecord;

typedef struct {
    char surname[25];
    int blockID;
} SHT_Record;

typedef struct {
    int fileDesc;
    char* attrName;
    int attrLength;
    long int numBuckets;
    char* fileName;
} SHT_info;

#define SHT_MAX_RECORDS 17

/* General SHT functions ----------------------------------------------------------------*/

// TODO
int SHT_CreateSecondaryIndex(char *fileName, char attrType, char* attrName, int attrLength, int buckets);

// TODO
SHT_info* SHT_OpenSecondaryIndex(char *fileName);

// TODO
int SHT_CloseSecondaryIndex(SHT_info *header_info);

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record);

int SHT_SecondaryGetAllEntries(SHT_info header_info, void *value);

//int HashStatistics(char *filename);

// TODO
int SHT_InitFile(int fd, char type, const char *name, int length, unsigned long int bucket);

// TODO
int is_SHT_file(int fd);

/* SHT_info functions -------------------------------------------------------------------*/

void delete_SHT_info(SHT_info *info);

void* Get_SHT_info_Data(const SHT_info *info);

SHT_info* Get_SHT_info(int fd);

/* Bucket-Block functions ---------------------------------------------------------------*/

int SHT_InsertEntryToBucket(int fd, int starting_block_num, SecondaryRecord record, const char *key_name);

int SHT_GetAllBucketEntries(int fd, int starting_block_num, const char* key_name, void *key_value);

/* SHT_Record-Block functions -----------------------------------------------------------*/

int SHT_InsertRecordtoBlock(int fd, int block_num, SHT_Record rec);

int SHT_BlockHasRecord(void *block, const char* key_name, SHT_Record *rec);

void* SHT_GetLastRecord(void *block);

int SHT_PrintBlockRecordsWithKey(void *block, const char* key_name, void *value, int fd);

/* SHT_Record functions -----------------------------------------------------------------*/

int SHT_Record_size();

SHT_Record* Get_SHT_Record(const void *data);

void* get_SHT_Record_data(const SHT_Record *rec);

void* Next_SHT_Record(void *current);

int RecordsEqual(void *record, SHT_Record* other);

int SHT_RecordHasKeyValue(void *record, void *value);

/* The Hash function used by SHT. */
int SHT_HashCode(char* data, unsigned long int mod);

#endif
