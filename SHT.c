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

/* The hash function used by SHT. Uses SHA1. */
int SHT_Hashcode(char* data, unsigned long int mod)
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

/* Returns a pointer to the next SHT Record (as byte sequence). */
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

    // Copying surname
    memcpy(temp, rec->surname, surname_size);
    temp = (char*)temp + surname_size;

    // Copying block ID
    memcpy( temp, &(rec->blockID), sizeof(int) );

    // Done.
    return data;
}

/* SHT_info functions ------------------------------------------------------------------- */

int SHT_info_size()
{
    return sizeof(int) + (strlen("surname") + 1) + sizeof(int) + sizeof(long int) + 15;
}

/* Converts the specified SHT_info into a byte sequence. */
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

    // Copying primary file name
    memcpy( temp, info->fileName, file_name_size);

    // Done.
    return data;
}

/* Properly deletes the specified SHT_info. */
void delete_SHT_info(SHT_info *info)
{
    free(info->attrName);
    free(info);
}

/* Returns the header info of the secondary hash file with the specified file descriptor.
   In case of an error, NULL is returned. */
SHT_info* Get_SHT_info(int fd)
{
    void *block;

    // Return fail if this is not a hash file
    //if ( !IsHashFile(fd) ) { return NULL; }

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
    memcpy(info->fileName, temp, len);

    return info;
}

/* SHT_Record functions -----------------------------------------------------------------*/

/* Returns the size of each SHT_Record, without any padding. */
int SHT_Record_size()
{
    return 25 + sizeof(int);
}

/* Checks if the specified byte sequence SHT_Record is equal to OTHER.
   Returns 0 if this is the case, -1 otherwise.*/
int RecordsEqual(void *record, SHT_Record* other)
{
    // Comparing key values (surnames)
    if (strncmp( (char*)record, other->surname, 25 ) == 0)
    {
        // Comparing block ID's
        if (memcpy( (char*)record + 25, &(other->blockID), sizeof(int) ))
        {
            return 0;
        }
    }
    return -1;
}

/* Returns 0 if the specified byte sequence SHT_Record has key value equal to VALUE,
   -1 otherwise. */
int SHT_RecordHasKeyValue(void *record, void *value)
{
    if (strcmp(record, value) == 0)
    {
        return 0;
    }
    return -1;
}

/* Creates a record using the specified byte sequence. */
SHT_Record* Get_SHT_Record(const void *data)
{
    SHT_Record *record = malloc(sizeof(SHT_Record));

    const void *temp = data;        // Used to iterate over the sequence

    // Copying key value
    strcpy(record->surname, temp);
    temp = (char*)temp + 25;

    // Copying block ID
    memcpy(&(record->blockID), temp, sizeof(int));

    return record;
}

/* SHT_Record-Block functions -----------------------------------------------------------*/

/* Returns 0 if the specified block contains a SHT_Record with key value
   equal to the one in the specified Record. Returns -1 otherwise. */
int SHT_BlockHasRecord(void *block, SHT_Record *rec)
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
        // SHT_Record with same key value found
        {
            return 0;
        }
        curr_record = NextRecord(curr_record);
        curr_record_num++;
    }
    // No record with same key value value found, so return -1
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

/* Prints all the records in the specified block, with KEY_NAME field value equal to VALUE.
   Returns 0 if any records where printed, -1 otherwise.
   In case VALUE is NULL, all records will be printed. */
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
            // Get the record as a struct for easier handling
            sec_record = Get_SHT_Record(curr_record);

            // Read the corresponding block of the primary hash file
            // fd is the file descriptor of the primary hash file.
            if ( BF_ReadBlock(fd, sec_record->blockID, &primary_block) < 0) { return -1; }

            // Print all records with specified key value from that block
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
int SHT_InsertEntryToBucket(int fd, int starting_block_num, SecondaryRecord sec_record)
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
            if (SHT_BlockHasRecord(current, &record) == 0)
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

/*  Prints all Records with KEY_NAME field == KEY_VALUE, in the bucket starting
    at the block with the specified number.
    Returns the number of blocks read *up to the last printed record*, or -1 in case of error. */
int SHT_GetAllBucketEntries(int fd, int starting_block_num, const char* key_name, void *key_value, int pfd)
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
        // pfd is the file descriptor of the primary file
        if (SHT_PrintBlockRecordsWithKey(curr_block, key_name, key_value, pfd) == 0)
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

/*  Initializes all the recuired information for the specified Secondary Hash file
    (given attributes, blocks, buckets, etc.). Returns 0 if successful, -1 if failed. */
int SHT_InitFile(int fd, const char *name, int length, unsigned long int buckets, const char *primary_file_name)
{
    void* block;
    SHT_info info;
    void *data;

    // Creating the first block of the file
    if(BF_AllocateBlock(fd) < 0) { return -1; }

    // Since it is the first, block id  is 0
    if(BF_ReadBlock(fd, 0, &block) < 0) { return -1; }

    // Creating a header struct with all the required information
    info.fileDesc = fd;
    info.attrName = malloc(strlen(name) + 1);
    strcpy(info.attrName, name);
    info.attrLength = length;
    info.numBuckets = buckets;
    strcpy(info.fileName, primary_file_name);

    // Converting the struct into a byte sequence
    data = Get_SHT_info_Data(&info);

    free(info.attrName);

    // Storing a "sec_hash" string to identify that this is a Hash File
    memcpy(block, "sec_hash", strlen("sec_hash") + 1);

    // Storing the header sequence after it
    memcpy(block + strlen("sec_hash") + 1, data, SHT_info_size());

    free(data);

    // Initialize pointer to next block (-1)
    SetNextBlockNumber(block, -1);

    if (BF_WriteBlock(fd, 0) < 0) { return -1; }

    // Done.
    return 0;
}

/* Creates a Secondary Hash File with the specified information. Returns -1 in case of an error, 0 otherwise. */
int SHT_CreateSecondaryIndex(char *sfileName, char* attrName, int attrLength, int buckets, char* fileName)
{
    int fd;

    // Creating the file in block-level
    if(BF_CreateFile(sfileName) < 0) { return -1; }

    fd = BF_OpenFile(sfileName);
    if( fd < 0) { return -1; }

    // Initializing the new file
    if ( SHT_InitFile(fd, attrName, attrLength, buckets, fileName) < 0 ) { return -1; }

    // Creating bucket blocks
    if ( HT_CreateBuckets(fd, buckets) < 0) { return -1; }

    // File is ready
    if (BF_CloseFile(fd) < 0) { return -1; }
    
    return 0;
}

/* Opens the Secondary Hash File with the specified name.
   If successful, returns the header struct of this file, otherwise returns NULL. */
SHT_info* SHT_OpenSecondaryIndex(char *fileName)
{
    SHT_info *info;
    int fd;

    // Opening the file
    if ( (fd = BF_OpenFile(fileName)) < 0) { return NULL; }

    // Getting header information
    info = Get_SHT_info(fd);

    return info;
}

/* Closes the Secondary Hash File specified by the given header struct, which is deleted.
   Returns 0 if successful, -1 otherwise. */
int SHT_CloseSecondaryIndex(SHT_info *header_info)
{
    if (header_info == NULL) { return -1; }

    if (BF_CloseFile(header_info->fileDesc) < 0) { return -1; }

    delete_SHT_info(header_info);
    return 0;
}

/*  Inserts the specified record in the secondary hash file, as long as the key field (as specified in
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

    int hash_code = SHT_Hashcode(record.record.surname, header_info.numBuckets);
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
            return SHT_InsertEntryToBucket(fd, bucket_starting_block, record);            
        }        
    }
    return -1;    
}

/*  Prints all records record with key value = VALUE.
    Takes advantage of hashing to locate the corresponding bucket.
    Returns the number of blocks read until the record was found (or not found), -1 in case of error. */
int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value)
{
    void *current_block;
    
    if (BF_ReadBlock(header_info_sht.fileDesc, 0, &current_block) < -1) { return -1; }

    int current_block_num = GetNextBlockNumber(current_block);

    int hash_code = SHT_Hashcode((char*)value, header_info_sht.numBuckets);    
    int target_block = hash_code / MAX_BUCKETS;                 // The block where the bucket for this hashcode is (relative position, not the id)
    int *target_bucket;
    int block_counter = 0;                                      // Counts read blocks
    int total;

    // Iterating over the hashtable blocks
    while (current_block_num != -1)
    {
        if (BF_ReadBlock(header_info_sht.fileDesc, current_block_num, &current_block) < 0) { return -1; }

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
                total = block_counter + SHT_GetAllBucketEntries(header_info_sht.fileDesc, *target_bucket,
                                                               header_info_sht.attrName, value, header_info_ht.fileDesc);
                return total;
            }
        }        
    }
    return block_counter;
}