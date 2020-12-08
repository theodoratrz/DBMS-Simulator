#ifndef HT_H
#define HT_H

/*
 * File: HT.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

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

int HT_CreateIndex(char *fileName, char attrType, char* attrName,int attrLength, int buckets);

HT_info* HT_OpenIndex(char *fileName);

int HT_CloseIndex(HT_info *header_info);

int HT_InsertEntry(HT_info header_info, Record record);

int HT_DeleteEntry(HT_info header_info, void *value);

int HT_GetAllEntries(HT_info header_info, void *value);

int HashStatistics(char *filename);

Record* GetRecord(const void *data);

void* GetRecordData(const Record *rec);

void delete_HT_info(HT_info *info);

void* Get_HT_info_Data(const HT_info *info);

HT_info* Get_HT_info(int fd);

int HT_InitFile(int fd, char type, const char *name, int length, unsigned long int bucket);

int HT_CreateBuckets(int fd, int buckets);

#endif