#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "BF.h"
#include "HT.h"

Record* GetRecord(const void *data)
{
    Record *record = malloc(sizeof(Record));

    const void *temp = data;

    memcpy(&(record->id), temp, sizeof(int));
    temp = (int*)temp + 1;

    strcpy(record->name, (char*)temp);
    temp = (char*)temp + sizeof(record->name);

    strcpy(record->surname, (char*)temp);
    temp = (char*)temp + sizeof(record->surname);

    strcpy(record->address, (char*)temp);

    return record;
}

void* GetRecordData(const Record *rec)
{
    void *data;
    void *temp;
    int id_size = sizeof(rec->id);
    int name_size = 15;
    int surname_size = 25;
    int address_size = 50;

    data = malloc(RECORD_SIZE);
    temp = data;

    memcpy(temp, &(rec->id), id_size);
    temp = (int*)temp + 1;
    memcpy( temp, rec->name, name_size );
    temp = (char*)temp + name_size;
    memcpy( temp, rec->surname, surname_size );
    temp = (char*)temp + surname_size;
    memcpy( temp, rec->address, address_size);

    return data;
}

HT_info* Get_HT_info(int fd)
{
    void *block;
    if (BF_ReadBlock(fd, 0, &block) < 0) { return NULL; }

    if (memcmp(block, "hash", strlen("hash") + 1) != 0)
    // Return fail if this is not a heap file
    {
        return NULL;
    }

    HT_info *info = malloc(sizeof(HT_info));
    void *temp = (char*)block + strlen("hash") + 1;
    int len;

    memcpy(&(info->fileDesc), temp, sizeof(int));
    temp = (int*)temp + 1;

    memcpy(&(info->attrType), temp, 1);
    temp = (char*)temp + 1;

    len = strlen((char*)temp) + 1;
    info->attrName = malloc(len);
    memcpy(info->attrName, temp, len);
    temp = (char*)temp + len;

    memcpy(&(info->attrLength), temp, sizeof(int));
    temp = (int*)temp + 1;

    memcpy(&(info->numBuckets), temp, sizeof(unsigned long int));

    return info;
}

void SetNextBlockNumber(void *current, int num)
{
    memcpy((char*)current + BLOCK_SIZE - sizeof(int), &num, sizeof(int) );
}

int GetNextBlockNumber(void *current)
{
    int next;
    memcpy(&next, (char*)current + BLOCK_SIZE - sizeof(int), sizeof(int) );
    return next;
}

void* Get_HT_info_Data(const HT_info *info)
{
    void *data;
    void *temp;
    int fd_size = sizeof(int);
    int type_size = 1;
    int name_size = 3;      // id (2 chars) + null char
    int length_size = sizeof(int);
    int buckets_size = sizeof(unsigned long int);

    data = malloc(HT_INFO_SIZE);
    temp = data;

    memcpy(temp, &(info->fileDesc), fd_size);
    temp = (int*)temp + 1;

    memcpy( temp, &(info->attrType), type_size );
    temp = (char*)temp + type_size;

    memcpy( temp, info->attrName, name_size );
    temp = (char*)temp + name_size;

    memcpy( temp, &(info->attrLength), length_size);
    temp = (int*)temp + 1;

    memcpy( temp, &(info->numBuckets), buckets_size);

    return data;
}

int HT_InitFile(int fd, char type, const char *name, int length, unsigned long int buckets)
{
    void* block;
    HT_info info;
    void *data;

    if(BF_AllocateBlock(fd) < 0) { return -1; }

    if(BF_ReadBlock(fd, 0, &block) < 0) { return -1; }

    info.fileDesc = fd;
    info.attrType = type;
    info.attrName = malloc(strlen(name) + 1);
    strcpy(info.attrName, name);
    info.attrLength = length;
    info.numBuckets = buckets;

    data = Get_HT_info_Data(&info);

    memcpy(block, "hash", strlen("hash") + 1);

    memcpy(block + strlen("hash") + 1, data, HT_INFO_SIZE);

    // set pointer to next block (-1)
    SetNextBlockNumber(block, -1);

    if (BF_WriteBlock(fd, 0) < 0) { return -1; }

    return 0;
}

void InitBuckets(void *block)
{
    int num_buckets = 0;
    void *temp = block;
    int init_value = -1;

    while (num_buckets < MAX_BUCKETS)
    {
        memcpy(temp, &init_value, sizeof(int));
        temp = (int*)temp + 1;
        num_buckets++;
    }
}

int HT_CreateBuckets(int fd, int buckets)
{
    int nof_blocks = (buckets / BLOCK_SIZE) + 1;

    void *curr_block;
    int block_counter = 0;
    int next_block_num = 0;
    int curr_block_num = 0;

    if ( BF_ReadBlock(fd, 0, &curr_block) < 0) { return -1; }

    while(block_counter < nof_blocks)
    {       
        if (BF_AllocateBlock(fd) < 0 ) { return -1; }
        curr_block_num = next_block_num;
        next_block_num = BF_GetBlockCounter(fd) - 1;
        SetNextBlockNumber(curr_block, next_block_num);

        if ( BF_WriteBlock(fd, curr_block_num) < 0) { return -1; }

        block_counter++;
        if ( BF_ReadBlock(fd, next_block_num, &curr_block) < 0) { return -1; }

        InitBuckets(curr_block);
    }
    SetNextBlockNumber(curr_block, -1);
    if ( BF_WriteBlock(fd, next_block_num) < 0) { return -1; }

    return 0;
}

int HT_CreateIndex( char *fileName, char attrType, char* attrName, int attrLength, int buckets )
{
    int fd;

    if(BF_CreateFile(fileName) < 0) { return -1; }

    fd = BF_OpenFile(fileName);
    if( fd < 0) { return -1; }

    if ( HT_InitFile(fd, attrType, attrName, attrLength, buckets) < 0 ) { return -1; }

    if ( HT_CreateBuckets(fd, buckets) < 0) { return -1; }

    if (BF_CloseFile(fd) < 0) { return -1; }
    
    return 0;
}