/*
 * File: SHT.c
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "BF.h"
#include "SHT.h"

int SHT_HashCode(char* data, unsigned long int mod)
{
    int result;
    
    // Initial hashcode is stored here
    char *hash = malloc(SHA_DIGEST_LENGTH);
    unsigned long int hash_num;

    // Getting the hashcode
    SHA1(data, 4, hash);

    // Copying part of it and storing it
    memcpy(&hash_num, hash, sizeof(unsigned long int));

    free(hash);

    // Calculating modulo and returning the result in a typical int
    hash_num = hash_num % mod;
    result = hash_num;
    return result;
}

void* Next_SHT_Record(void *current)
{
    return (char*)current + SHT_Record_size();
}

/* Converts the specified SHT_Record into a byte sequence (no padding). */
void* get_SHT_Record_data(const SHT_Record *rec)
{
    void *data;
    void *temp;
    int surname_size = 25;

    data = malloc(SHT_Record_size());     // Allocating space for the sequence
    temp = data;                    // Used to iterate over the sequence

    // Copying ID
    memcpy(temp, &(rec->surname), surname_size);
    temp = (int*)temp + 1;

    // Copying name
    memcpy( temp, rec->blockID, sizeof(int) );

    // Done.
    return data;
}

/* SHT_info functions ------------------------------------------------------------------- */

int SHT_info_size()
{
    return sizeof(int) + (strlen("surname") + 1) + sizeof(int) + sizeof(long int) + /*file name size?*/ 15;
}

void* Get_SHT_info_Data(const SHT_info *info)
{
    void *data;
    void *temp;
    int name_size = strlen(info->attrName) + 1;
    int file_name_size = strlen(info->fileName) + 1;

    // Allocating space for the struct
    data = malloc( SHT_info_size() );
    temp = data;                    // Used to iterate over the sequence

    // Copying file descriptor
    memcpy(temp, &(info->fileDesc), sizeof(int));
    temp = (int*)temp + 1;

    // Copying attribute name
    memcpy( temp, info->attrName, name_size );
    temp = (char*)temp + name_size;

    // Copying attribute length
    memcpy( temp, &(info->attrLength), sizeof(int));
    temp = (int*)temp + 1;

    // Copying number of buckets
    memcpy( temp, &(info->numBuckets), sizeof(unsigned long int));
    temp = (unsigned long int*)temp + 1;

    memcpy( temp, &(info->fileName), file_name_size);

    // Done.
    return data;
}

/* Properly deletes the specified SHT_info. */
void delete_SHT_info(SHT_info *info)
{
    free(info->attrName);
    free(info->fileName);
    free(info);
}

/* Returns the header info of the secondary hash file with the specified file descriptor.
   In case of an error, NULL is returned. */
SHT_info* Get_SHT_info(int fd)
{
    void *block;

    // Return fail if this is not a hash file
    if ( !IsHashFile(fd) ) { return NULL; }

    // The header is placed in block 0
    if (BF_ReadBlock(fd, 0, &block) < 0) { return NULL; }

    // The header is a byte sequence, so convert it to a struct HT_info
    SHT_info *info = malloc( SHT_info_size() );
    void *temp = (char*)block + strlen("sec_hash") + 1;
    int len;

    // Copying file descriptor field
    memcpy(&(info->fileDesc), temp, sizeof(int));
    temp = (int*)temp + 1;

    // Copying attrName field (size must be determined first)
    len = strlen((char*)temp) + 1;
    info->attrName = malloc(len);
    memcpy(info->attrName, temp, len);
    temp = (char*)temp + len;

    // Copying attrLength field
    memcpy(&(info->attrLength), temp, sizeof(int));
    temp = (int*)temp + 1;

    // Copying numBuckets field
    memcpy(&(info->numBuckets), temp, sizeof(unsigned long int));
    temp = (unsigned long int*)temp + 1;

    // Copying fileName field (size must be determined first)
    len = strlen((char*)temp) + 1;
    info->fileName = malloc(len);
    memcpy(info->fileName, temp, len);

    return info;
}

/* SHT_Record functions -----------------------------------------------------------------*/

int SHT_Record_size()
{
    return 25 + sizeof(int);
}

int RecordsEqual(void *record, SHT_Record* other)
{
    // Different case for every possible field name
    if (strncmp( (char*)record, other->surname, 25 ) == 0)
    {
        if (memcpy( (char*)record + 25, &(other->blockID), sizeof(int) ))
        {
            return 0;
        }
    }
    return -1;
}

// Probably won't be needed.
/* Returns a pointer to the last record of the specified block. */
void* SHT_GetLastRecord(void *block)
{
    return (char*)block + (GetBlockNumRecords(block) - 1)*SHT_Record_size();
}

int SHT_RecordHasKeyValue(void *record, void *value)
{
    return ( strcmp(record, value) == 0 );
}

SHT_Record* Get_SHT_Record(const void *data)
{
    SHT_Record *record = malloc(sizeof(SHT_Record));

    const void *temp = data;        // Used to iterate over the sequence

    // Copying key value
    strcpy(record->surname, temp);
    temp = (char*)temp + (strlen(temp) + 1);

    // Copying block ID
    memcpy(record->blockID, temp, sizeof(int));

    return record;
}

/* SHT_Record-Block functions -----------------------------------------------------------*/

int SHT_BlockHasRecord(void *block, char* key_name, SHT_Record *rec)
{
    int num_records = GetBlockNumRecords(block);

    // Return -1 if the block has no records
    if(num_records == 0) { return -1; }

    int curr_record_num = 1;
    void *curr_record;
    curr_record = block;

    // Iterate over the records in the block
    while( curr_record_num <= num_records )
    {
        if (RecordsEqual(curr_record, rec) == 0)
        // Record with same value in KEY_NAME found
        {
            return 0;
        }
        curr_record = NextRecord(curr_record);
        curr_record_num++;
    }
    // No record with same KEY_NAME field value found, so return -1
    return -1;
}

/* Inserts the specified record in the block with the specified number.
   Whether a record with the same key already exists must have been previously checked.
   If the record was successfully inserted, returns 0, otherwise -1.*/
int SHT_InsertRecordtoBlock(int fd, int block_num, SHT_Record rec)
{
    void *block;

    // Reading the block using file descriptor and getting number of records in it
    if (BF_ReadBlock(fd, block_num, &block) < 0) { return -1; }
    int num_records = GetBlockNumRecords(block);

    if( num_records < SHT_MAX_RECORDS)
    // The block still has space for a new record
    {
        // Convert record into byte sequence
        void* data = get_SHT_Record_data(&rec);

        // Write the sequence right after the last record
        memcpy( (char*)block + (num_records*SHT_Record_size()), data, SHT_Record_size());
        free(data);     // this won't be needed anymore

        // Increase the number of records
        SetNumRecords(block, num_records + 1);

        // The block is ready to be written back to the disk
        if (BF_WriteBlock(fd, block_num) < 0){ return -1; }

        // Return success
        return 0;
    }
    else
    {
        // In case the block is full, return fail
        return -1;
    }
}

int SHT_PrintBlockRecordsWithKey(void *block, const char* key_name, void *value, int fd)
{
    int num_records = GetBlockNumRecords(block);
    if(num_records == 0) { return -1; }
    int found_records = 0;

    int curr_record_num = 1;

    void *curr_record;
    curr_record = block;
    void *primary_block;
    SHT_Record* sec_record;
 
    // Iterate over the records
    while( curr_record_num <= num_records )
    {
        if (SHT_RecordHasKeyValue(curr_record, value) == 0)
        // Found record to print
        {
            sec_record = Get_SHT_Record(curr_record);
            if ( BF_ReadBlock(fd, sec_record->blockID, &primary_block) < 0) { return -1; }
            PrintBlockRecordsWithKey(primary_block, key_name, value);
            found_records++;
            free(sec_record);
        }
        // Go to the next record
        curr_record = Next_SHT_Record(curr_record);
        curr_record_num++;
    }
    if (found_records) { return 0; }
    
    // No records were printed
    return -1;
}

/* Bucket-Block functions ---------------------------------------------------------------*/

/* Inserts the specified Record in the bucket block (specified by number), if the key field value is unique.
   Returns the number of the block where the Record was inserted.
   If the Record was not inserted (duplicate key or error), -1 is returned. */
int SHT_InsertEntryToBucket(int fd, int starting_block_num, SecondaryRecord sec_record, const char *key_name)
{
    void *current;
    
    int current_block_num = 0;
    int next_block_num = starting_block_num;    // Begin from the first block of the bucket (where the bucket points to)
    int empty_block_found = 0;                  // Indicates whether a non-full block has been found
    int empty_block_num = -1;
    int num_rec;

    SHT_Record record;
    strcpy(record.surname, sec_record.record.surname);
    record.blockID = sec_record.blockId;

    // The idea is to iterate over the blocks of the bucket to check for duplicate key,
    // and save the first non-full block to store the record
    while(next_block_num != -1)
    {
        current_block_num = next_block_num;
        if (BF_ReadBlock(fd, current_block_num, &current) < 0) { return -1; }
        num_rec = GetBlockNumRecords(current);

        if (num_rec == 0)
        // If the current block has no records, don't check for duplicate key
        {
            if ( !empty_block_found )
            // If this is the first non-full block found, save it
            {
                empty_block_num = current_block_num;
                empty_block_found = 1;
            }         
        }
        else
        {
            if ( num_rec < SHT_MAX_RECORDS )
            // If the current block has free space
            {
                if ( !empty_block_found )
                // If this is the first non-full block found, save it
                {
                    empty_block_num = current_block_num;
                    empty_block_found = 1;
                }
            }
            if (SHT_BlockHasRecord(current, key_name, &record) == 0)
            // If the block already has a record with key equal to the specified one's,
            {
                // Don't insert the record
                return -1;
            }
        }
        next_block_num = GetNextBlockNumber(current);
    }

    if ( !empty_block_found)
    // No non-full block has been found
    {
        // Create a new block after the last one
        int new_block_num = AddNextBlock(fd, current_block_num); 
        if (new_block_num < 0) { return -1; }

        // And attempt to insert the Record there
        if (SHT_InsertRecordtoBlock(fd, new_block_num, record) == 0)
        // Successfully inserted
        {
            return new_block_num;
        }
        else
        {
            return -1;
        }
    }
    else
    // A non-full block has been found,
    {
        // So insert the Record there
        if (SHT_InsertRecordtoBlock(fd, empty_block_num, record) == 0)
        {
            return empty_block_num;
        }
    }
}

int SHT_GetAllBucketEntries(int fd, int starting_block_num, const char* key_name, void *key_value)
{
    void *curr_block;

    // Begin from the first block of the bucket (where the bucket points to)
    int curr_block_num = starting_block_num;
    int read_blocks_until_rec = 0;              // The block counter to be returned
    int blocks_from_last_rec = 1;

    // Iterate over the blocks
    while(curr_block_num != -1)
    {
        if (BF_ReadBlock(fd, curr_block_num, &curr_block) < 0 ) { return -1; }

        // Print all Records of the current block with the specified key field value
        if (SHT_PrintBlockRecordsWithKey(curr_block, key_name, key_value, fd) == 0)
        {
            read_blocks_until_rec += blocks_from_last_rec;
            blocks_from_last_rec = 1;
        }
        else
        {
            blocks_from_last_rec++;
        }

        curr_block_num = GetNextBlockNumber(curr_block);
    }
    // If no entry is found, 0 is returned
    return read_blocks_until_rec;
}

/* General SHT functions ----------------------------------------------------------------*/

/*  Inserts the specified record in the hash file, as long as the key field (as specified in
    the header) does not have a duplicate value.
    Takes advantage of hashing in order to quickly locate the bucket where the block should be inserted. 
    Returns the number of the block where the record was inserted, otherwise -1.*/
int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record)
{
    int fd = header_info.fileDesc;
    void *current_block;
    
    // Starting from the header block
    if (BF_ReadBlock(fd, 0, &current_block) < -1) { return -1; }

    int current_block_num = 0;
    int next_block_num = GetNextBlockNumber(current_block);

    int hash_code = GetHashcode(record.record.id, header_info.numBuckets);
    int target_block = hash_code / MAX_BUCKETS;             // The block where the bucket for insertion is (relative position, not the id)
    void *target_bucket;
    int bucket_starting_block;
    int block_counter = 0;                                  // Counts the iterated blocks

    // Iterating over the blocks of the hashtable
    while (next_block_num != -1)
    {
        current_block_num = next_block_num;
        if (BF_ReadBlock(fd, current_block_num, &current_block) < 0) { return -1; }

        if (block_counter != target_block)
        // If this is not the target block, continue to the next one
        {
            block_counter++;
            next_block_num = GetNextBlockNumber(current_block);
        }
        else
        // This is the block we are looking for
        {
            // Finding the *relative* position of the bucket in the block
            target_bucket = (int*)current_block + hash_code - (block_counter*MAX_BUCKETS);
            memcpy(&bucket_starting_block, target_bucket, sizeof(int));

            if (bucket_starting_block == -1)
            // The bucket is empty and must be initiallized
            {
                // Create new block
                bucket_starting_block = NewRecordBlock(fd);

                if (bucket_starting_block == -1) { return -1; }
                // Make the bucket point to the new block
                SetBucket(target_bucket, bucket_starting_block);

                if ( BF_WriteBlock(fd, current_block_num) < 0 ) { return -1; }
            }

            // Ready to insert the record
            return SHT_InsertEntryToBucket(fd, bucket_starting_block, record, header_info.attrName);            
        }        
    }
    return -1;    
}

/*  Prints a (unique) record for which key(id) == VALUE.
    Takes advantage of hashing to locate the corresponding bucket.
    Returns the number of blocks read until the record was found (or not found), -1 in case of error. */
int SHT_SecondaryGetAllEntries(SHT_info header_info, void *value)
{
    HT_info* primary_header = HT_OpenIndex(header_info.fileName);
    if (primary_header == NULL) { return -1; }
    void *current_block;
    
    if (BF_ReadBlock(header_info.fileDesc, 0, &current_block) < -1) { return -1; }

    int current_block_num = GetNextBlockNumber(current_block);

    int hash_code = GetHashcode(*((int*)value), header_info.numBuckets);    
    int target_block = hash_code / MAX_BUCKETS;                 // The block where the bucket for this hashcode is (relative position, not the id)
    int *target_bucket;
    int block_counter = 0;                                      // Counts read blocks

    // Iterating over the hashtable blocks
    while (current_block_num != -1)
    {
        if (BF_ReadBlock(header_info.fileDesc, current_block_num, &current_block) < 0) { return -1; }

        if (block_counter != target_block)
        // If this is not the target block, continue to the next one
        {
            block_counter++;
            current_block_num = GetNextBlockNumber(current_block);
        }
        else
        // This is the block we are looking for
        {
            // Finding the *relative* position of the bucket in the block
            target_bucket = (int*)current_block + hash_code - (block_counter*MAX_BUCKETS);

            if (*target_bucket == -1)
            // Bucket Empty, so nothing to search here
            {
                return -1;
            }
            else
            // Bucket not empty, so search here
            {
                return block_counter + SHT_GetAllBucketEntries(primary_header->fileDesc,
                                                               *target_bucket, header_info.attrName, value);
            }
        }        
    }

    return block_counter;
}