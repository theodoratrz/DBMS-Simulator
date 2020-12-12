/*
 * File: HT.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#ifndef HT_H
#define HT_H

#define HT_INFO_SIZE ( sizeof(int) + 1 + 3 + sizeof(int) + sizeof(unsigned long int) )
#define RECORD_SIZE (sizeof(int) + 15 + 25 + 50)
#define MAX_RECORDS 5
#define MAX_BUCKETS 127

typedef struct
{
    int fileDesc;
    char attrType;
    char *attrName;
    int attrLength;
    unsigned long int numBuckets;
} HT_info;

typedef struct{
    int id;
    char name[15];
    char surname[25];
    char address[50];
} Record;

int HT_CreateIndex(char *fileName, char attrType, char* attrName, int attrLength, int buckets);

HT_info* HT_OpenIndex(char *fileName);

int HT_CloseIndex(HT_info *header_info);

int HT_InsertEntry(HT_info header_info, Record record);

int HT_DeleteEntry(HT_info header_info, void *value);

int HT_GetAllEntries(HT_info header_info, void *value);

int HashStatistics(char *filename);

int HT_InitFile(int fd, char type, const char *name, int length, unsigned long int bucket);

int HT_CreateBuckets(int fd, int buckets);

int HT_GetUniqueEntry(HT_info header_info, void *value);

/* HT_info functions -----------------------------------------------------*/

void delete_HT_info(HT_info *info);

void* Get_HT_info_Data(const HT_info *info);

HT_info* Get_HT_info(int fd);

/* Bucket-Block functions -----------------------------------------------------*/

void InitBuckets(void *block);

void* GetNextBucket(void *current);

void SetBucket(void *current, int bn);

int InsertEntryToBucket(int fd, int starting_block_num, Record record, const char *key_name);

int DeleteEntryFromBucket(int fd, int starting_block_num, void *key_value, const char *key_name);

int GetAllBucketEntries(int fd, int starting_block_num, void *key_value, const char *key_name);

/* Record-Block Functions -----------------------------------------------------*/

int NewRecordBlock(int fd);

int GetBlockNumRecords(void *block);

void SetNextBlockNumber(void *current, int num);

int GetNextBlockNumber(void *current);

int GetNumRecords(void *block);

void SetNumRecords(void *block, int n);

int AddNextBlock(int fd, int current_num);

int InsertRecordtoBlock(int fd, int block_num, Record rec);

int BlockHasRecordWithKey(void *block, const char* key_name, Record *rec);

int DeleteRecordFromBlock(void *block, const char *key_name, void *value);

int PrintBlockRecordsWithKey(void *block, const char *key_name, void *value);

/* Record Functions -----------------------------------------------------*/

Record* GetRecord(const void *data);

void* GetRecordData(const Record *rec);

void CopyRecord(void *dest, void *src);

void* NextRecord(void *current);

void* GetLastRecord(void *block);

int RecordKeyHasValue(void *record, const char *key_name, void *value);

void PrintRecord(Record rec);

/* The Hash function used by HT. */
int GetHashcode(int id, unsigned long mod);

#endif