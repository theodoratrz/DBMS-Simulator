#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "HP.h"
#include "BF.h"

int HP_InitFile(int fd, char type, const char *name, int length)
{
    void* block;
    HP_info info;
    void *data;

    if(BF_AllocateBlock(fd) < 0) { return -1; }

    if(BF_ReadBlock(fd, 0, &block) < 0) { return -1; }

    info.fileDesc = fd;
    info.attrType = type;
    info.attrName = malloc(strlen(name) + 1);
    strcpy(info.attrName, name);
    info.attrLength = length;

    data = Get_HP_info_Data(&info);

    memcpy(block, "heap", strlen("heap") + 1);

    memcpy(block + strlen("heap") + 1, data, sizeof(HP_info));

    // set pointer to next block (-1)
    HP_SetNextBlockNumber(block, -1);

    if (BF_WriteBlock(fd, 0) < 0) { return -1; }

    if (BF_CloseFile(fd) < 0) { return -1; }

    return 0;
}

void* HP_GetNextBlock(int fd, void *current)
{
    int next_block_num;
    void *next_block;

    next_block_num = HP_GetNextBlockNumber(current);
    BF_ReadBlock(fd, next_block_num, &next_block);

    return next_block;
}

void HP_SetNextBlockNumber(void *current, int num)
{
    memcpy((char*)current + BLOCK_SIZE - sizeof(int), &num, sizeof(int) );
}

int HP_GetNextBlockNumber(void *current)
{
    int next;
    memcpy(&next, (char*)current + BLOCK_SIZE - sizeof(int), sizeof(int) );
    return next;
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

HP_info* Get_HP_info(int fd)
{
    void *block;
    if (BF_ReadBlock(fd, 0, &block) < 0) { return NULL; }

    if (memcmp(block, "heap", strlen("heap") + 1) != 0)
    // Return fail if this is not a heap file
    {
        return NULL;
    }

    HP_info *info = malloc(sizeof(HP_info));
    void *temp = (char*)block + strlen("heap") + 1;
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

    return info;

}

void* Get_HP_info_Data(const HP_info *info)
{
    void *data;
    void *temp;
    int fd_size = sizeof(int);
    int type_size = 1;
    int name_size = 3;      // id (2 chars) + null char
    int length_size = sizeof(int);

    data = malloc(HP_INFO_SIZE);
    temp = data;

    memcpy(temp, &(info->fileDesc), fd_size);
    temp = (int*)temp + 1;

    memcpy( temp, &(info->attrType), type_size );
    temp = (char*)temp + type_size;

    memcpy( temp, info->attrName, name_size );
    temp = (char*)temp + name_size;

    memcpy( temp, &(info->attrLength), length_size);

    return data;
}

int HP_CreateFile(const char *fileName, const char attrType, const char* attrName, const int attrLength)
{
    int fd;

    if(BF_CreateFile(fileName) < 0) { return -1; }

    fd = BF_OpenFile(fileName);
    if( fd < 0) { return -1; }

    if ( HP_InitFile(fd, attrType, attrName, attrLength) < 0 ) { return -1; }
    
    return 0;
}

HP_info* HP_OpenFile(const char *fileName)
{
    HP_info* info;
    int fd;

    if ( (fd = BF_OpenFile(fileName)) < 0) { return NULL; }

    info = Get_HP_info(fd);
    
    return info;
}

int HP_CloseFile(HP_info* header_info)
{
    if (header_info == NULL) { return -1; }

    if (BF_CloseFile(header_info->fileDesc) < 0) { return -1; }

    free(header_info);
    return 0;
}

void HP_SetNumRecords(void *block, int n)
{
    memcpy( (char*)block + BLOCK_SIZE - 2*sizeof(int), &n, sizeof(int) );
}

int HP_AddNextBlock(int fd, int current_num)
{
    void *current, *new;
    int new_num;

    if (BF_ReadBlock(fd, current_num, &current) < 0) { return -1; }

    if (BF_AllocateBlock(fd) < 0) { return -1; }

    if ( (new_num = BF_GetBlockCounter(fd) - 1) < -1  ) { return -1; }

    HP_SetNextBlockNumber(current, new_num);

    BF_WriteBlock(fd, current_num);

    if (BF_ReadBlock(fd, new_num, &new) < 0) { return -1; }
    HP_SetNumRecords(new, 0);
    HP_SetNextBlockNumber(new, -1);

    BF_WriteBlock(fd, new_num);

    return new_num;
}