#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "BF.h"
#include "HT.h"

/* The Hash function used by HT. Uses SHA1. */
int GetHashcode(int id, unsigned long int mod)
{
    int result;
    char *data = (char*)&id;
    char *hash = malloc(sizeof(SHA_DIGEST_LENGTH));
    unsigned long int hash_num;

    SHA1(data, 4, hash);

    memcpy(&hash_num, hash, sizeof(unsigned long int));

    free(hash);

    hash_num = hash_num % mod;
    result = hash_num;
    return result;
}

/* Record Functions */

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

void CopyRecord(void *dest, void *src)
{
    memcpy(dest, src, RECORD_SIZE);
}

void* NextRecord(void *current)
{
    return (char*)current + RECORD_SIZE;
}

void* GetLastRecord(void *block)
{
    return (char*)block + (GetBlockNumRecords(block) - 1)*RECORD_SIZE;
}

int RecordKeyHasValue(void *record, const char *key_name, void *value)
{
    Record *temp;

    temp = GetRecord(record);
    if ( strcmp(key_name, "id") == 0)
    {
        if ( memcmp( &(temp->id), value, sizeof(int) ) == 0 )
        {
            free(temp);
            return 0;
        }
    }
    else if ( strcmp(key_name, "name") == 0)
    {
        if ( strcmp(temp->name, value) == 0 )
        {
            free(temp);
            return 0;
        }
    }
    else if ( strcmp(key_name, "surname") == 0)
    {
        if ( strcmp(temp->surname, value) == 0 )
        {
            free(temp);
            return 0;
        }
    }
    else if ( strcmp(key_name, "address") == 0)
    {
        if ( strcmp(temp->address, value) == 0 )
        {
            free(temp);
            return 0;
        }
    }
    
    free(temp);
    return -1;
}

/* HT_info Functions */

HT_info* Get_HT_info(int fd)
{
    void *block;
    if (BF_ReadBlock(fd, 0, &block) < 0) { return NULL; }

    if (memcmp(block, "hash", strlen("hash") + 1) != 0)
    // Return fail if this is not a hash file
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

void delete_HT_info(HT_info *info)
{
    free(info->attrName);
    free(info);
}

/* Record-Block Functions */

int NewRecordBlock(int fd)
{
    if (BF_AllocateBlock(fd) < 0) { return -1; }

    int block_num = BF_GetBlockCounter(fd) - 1;
    void *block;
    if (BF_ReadBlock(fd, block_num, &block) < 0) { return -1; }

    SetNextBlockNumber(block, -1);
    SetNumRecords(block, 0);

    if (BF_WriteBlock(fd, block_num) < 0) { return -1; }

    return block_num;
}

int GetBlockNumRecords(void *block)
{
    int num;
    memcpy(&num, (char*)block + BLOCK_SIZE - 2*sizeof(int), sizeof(int));
    return num;
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

int GetNumRecords(void *block)
{
    int num;
    memcpy(&num, (char*)block + BLOCK_SIZE - 2*sizeof(int), sizeof(int));
    return num;
}

void SetNumRecords(void *block, int n)
{
    memcpy( (char*)block + BLOCK_SIZE - 2*sizeof(int), &n, sizeof(int) );
}

int AddNextBlock(int fd, int current_num)
{
    void *current, *new;
    int new_num;

    if (BF_ReadBlock(fd, current_num, &current) < 0) { return -1; }

    if (BF_AllocateBlock(fd) < 0) { return -1; }

    if ( (new_num = BF_GetBlockCounter(fd) - 1) < -1  ) { return -1; }

    SetNextBlockNumber(current, new_num);

    BF_WriteBlock(fd, current_num);

    if (BF_ReadBlock(fd, new_num, &new) < 0) { return -1; }
    SetNumRecords(new, 0);
    SetNextBlockNumber(new, -1);

    BF_WriteBlock(fd, new_num);

    return new_num;
}

int InsertRecordtoBlock(int fd, int block_num, Record rec)
{
    void *block;

    if (BF_ReadBlock(fd, block_num, &block) < 0) { return -1; }
    int num_records = GetNumRecords(block);

    if( num_records < MAX_RECORDS)
    {
        void* data = GetRecordData(&rec);

        memcpy( (char*)block + num_records*RECORD_SIZE, data, RECORD_SIZE );
        free(data);     // this won't be needed anymore

        SetNumRecords(block, num_records + 1);
        if (BF_WriteBlock(fd, block_num) < 0){ return -1; }

        return 0;
    }
    else
    {
        return -1;
    }
}

int BlockHasRecordWithKey(void *block, const char* key_name, Record *rec)
{
    int num_records = GetBlockNumRecords(block);
    if(num_records == 0) { return -1; }

    int curr_record_num = 1;
    void *curr_record;
    curr_record = block;

    while( curr_record_num <= num_records )
    {
        if (RecordKeyHasValue(curr_record, key_name, &(rec->id)) == 0)
        {
            return 0;
        }
        curr_record = NextRecord(curr_record);
        curr_record_num++;
    }
    return -1;
}

int DeleteRecordFromBlock(void *block, const char *key_name, void *value)
{
    int num_records = HP_GetNumRecords(block);
    if(num_records == 0) { return -1; }

    int curr_record_num = 1;
    void *curr_record;
    curr_record = block;

    while( curr_record_num <= num_records )
    {
        if (HP_RecordKeyHasValue(curr_record, key_name, value) == 0)
        {
            // Delete record (replace with last etc.)
            if (curr_record_num!= num_records)
            {
                CopyRecord(curr_record, GetLastRecord(block));
            }
            HP_SetNumRecords(block, num_records -1);
            return 0;
        }
        curr_record = NextRecord(curr_record);
        curr_record_num++;
    }
}

/* Bucket-Block functions */

void InitBuckets(void *block)
{
    int num_buckets = 0;
    void *temp = block;
    int init_value = -1;

    while (num_buckets < MAX_BUCKETS)
    {
        //memcpy(temp, &init_value, sizeof(int));
        //temp = (int*)temp + 1;

        SetBucket(temp, init_value);
        temp = GetNextBucket(temp);

        num_buckets++;
    }
}

void* GetNextBucket(void *current)
{
    return (int*)current + 1;
}

void SetBucket(void *current, int bn)
{
    memcpy(current, &bn, sizeof(int));
}

int InsertEntryToBucket(int fd, int starting_block_num, Record record, const char *key_name)
{
    void *current;
    
    int current_block_num = 0;
    int next_block_num = starting_block_num;
    int empty_block_found = 0;
    int empty_block_num = -1;
    int num_rec;

    while(next_block_num != -1)
    {
        current_block_num = next_block_num;
        if (BF_ReadBlock(fd, current_block_num, &current) < 0) { return -1; }
        num_rec = GetBlockNumRecords(current);

        if (num_rec == 0)
        {
            if ( !empty_block_found )
            {
                empty_block_num = current_block_num;
                empty_block_found = 1;
            }         
        }
        else
        {
            if ( num_rec < MAX_RECORDS )
            {
                if ( !empty_block_found )
                {
                    empty_block_num = current_block_num;
                    empty_block_found = 1;
                }
            }
            if (BlockHasRecordWithKey(current, key_name, &record) == 0)
            {
                return -1;
            }
        }
        next_block_num = GetNextBlockNumber(current);
    }

    if ( !empty_block_found)
    {
        int new_block_num = AddNextBlock(fd, current_block_num); 
        if (new_block_num < 0) { return -1; }

        if (InsertRecordtoBlock(fd, new_block_num, record) == 0)
        {
            return new_block_num;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (InsertRecordtoBlock(fd, empty_block_num, record) == 0)
        {
            return empty_block_num;
        }
    }
}

int DeleteEntryFromBucket(int fd, int starting_block_num, void *key_value, const char *key_name)
// TODO
// Very similar to HP_DeleteEntry...
{

}

/* More general HT functions */

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

HT_info* HT_OpenIndex(char *fileName)
{
    HT_info *info;
    int fd;

    if ( (fd = BF_OpenFile(fileName)) < 0) { return NULL; }

    info = Get_HT_info(fd);

    return info;
}

int HT_CloseIndex(HT_info *header_info)
{
    if (header_info == NULL) { return -1; }

    if (BF_CloseFile(header_info->fileDesc) < 0) { return -1; }

    delete_HT_info(header_info);
    return 0;
}

int HT_InsertEntry(HT_info header_info, Record record)
{
    int fd = header_info.fileDesc;
    void *current_block;
    
    if (BF_ReadBlock(fd, 0, &current_block) < -1) { return -1; }

    int current_block_num = 0;
    int next_block_num = GetNextBlockNumber(current_block);

    int hash_code = GetHashcode(record.id, header_info.numBuckets);
    int target_block = hash_code / MAX_BUCKETS;
    void *target_bucket;
    int bucket_starting_block;
    int block_counter = 0;

    while (next_block_num != -1)
    {
        current_block_num = next_block_num;
        if (BF_ReadBlock(fd, current_block_num, &current_block) < 0) { return -1; }

        if (block_counter != target_block)
        {
            block_counter++;
            next_block_num = GetNextBlockNumber(current_block);
        }
        else
        {
            target_bucket = (int*)current_block + hash_code - (block_counter*MAX_BUCKETS);
            memcpy(&bucket_starting_block, target_bucket, sizeof(int));

            if (bucket_starting_block == -1)
            // Bucket Empty
            {
                // Create new block
                bucket_starting_block = NewRecordBlock(fd);

                if (bucket_starting_block == -1) { return -1; }
                // Make the bucket point to the new block
                SetBucket(target_bucket, bucket_starting_block);
                //memcpy(target_bucket, &bucket_starting_block, sizeof(int));
                if ( BF_WriteBlock(fd, current_block_num) < 0 ) { return -1; }
            }

            return InsertEntryToBucket(fd, bucket_starting_block, record, header_info.attrName);            
        }        
    }
    return -1;    
}

int HT_DeleteEntry(HT_info header_info, void *value)
{
    int fd = header_info.fileDesc;
    void *current_block;
    
    if (BF_ReadBlock(fd, 0, &current_block) < -1) { return -1; }

    int current_block_num = 0;
    int next_block_num = GetNextBlockNumber(current_block);

    // Could take advantage of header_info.attrType here as well...
    int hash_code = GetHashcode(*((int*)value), header_info.numBuckets);
    
    int target_block = hash_code / MAX_BUCKETS;
    void *target_bucket;
    int bucket_starting_block;
    int block_counter = 0;

    while (next_block_num != -1)
    {
        current_block_num = next_block_num;
        if (BF_ReadBlock(fd, current_block_num, &current_block) < 0) { return -1; }

        if (block_counter != target_block)
        {
            block_counter++;
            next_block_num = GetNextBlockNumber(current_block);
        }
        else
        {
            target_bucket = (int*)current_block + hash_code - (block_counter*MAX_BUCKETS);
            memcpy(&bucket_starting_block, target_bucket, sizeof(int));

            if (bucket_starting_block == -1)
            // Bucket Empty
            {
                return -1;
            }
            else
            {
                return DeleteEntryFromBucket(fd, bucket_starting_block, value, header_info.attrName);
            }
        }        
    }
    return -1;
}