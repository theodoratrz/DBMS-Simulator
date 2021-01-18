/*
 * File: HT.c
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

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

/* Record Functions --------------------------------------------------------------*/

/* Creates a record using the specified byte sequence. */
Record* GetRecord(const void *data)
{
    Record *record = malloc(sizeof(Record));

    const void *temp = data;        // Used to iterate over the sequence

    // Copying ID
    memcpy(&(record->id), temp, sizeof(int));
    temp = (int*)temp + 1;

    // Copying name
    strcpy(record->name, (char*)temp);
    temp = (char*)temp + sizeof(record->name);

    // Copying surname
    strcpy(record->surname, (char*)temp);
    temp = (char*)temp + sizeof(record->surname);

    // Copying address
    strcpy(record->address, (char*)temp);

    return record;
}

/* Converts the specified Record into a byte sequence (no padding). */
void* GetRecordData(const Record *rec)
{
    void *data;
    void *temp;
    int id_size = sizeof(rec->id);
    int name_size = 15;
    int surname_size = 25;
    int address_size = 50;

    data = malloc(RECORD_SIZE);     // Allocating space for the sequence
    temp = data;                    // Used to iterate over the sequence

    // Copying ID
    memcpy(temp, &(rec->id), id_size);
    temp = (int*)temp + 1;

    // Copying name
    memcpy( temp, rec->name, name_size );
    temp = (char*)temp + name_size;

    // Copying surname
    memcpy( temp, rec->surname, surname_size );
    temp = (char*)temp + surname_size;

    // Copying address
    memcpy( temp, rec->address, address_size);

    // Done.
    return data;
}

/* Copies the Record (as byte sequence) pointed by SRC into DEST. */
void CopyRecord(void *dest, void *src)
{
    memcpy(dest, src, RECORD_SIZE);
}

/* Returns a pointer to the next Record (as byte sequence). */
void* NextRecord(void *current)
{
    return (char*)current + RECORD_SIZE;
}

/* Returns 0 if the KEY_NAME field value of the specified record is equal to VALUE, -1 otherwise. */
int RecordKeyHasValue(void *record, const char *key_name, void *value)
{
    Record *temp;

    // The specified record is a byte sequence, so convert it to a struct Record to make things simpler
    temp = GetRecord(record);

    // Different case for every possible field name
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

    // If this point is reached, the specified field value was not the same, so return -1
    // Note that this point will also be reached in case KEY_NAME is invalid
    return -1;
}

/* Prints the specified Record. */
void PrintRecord(Record rec)
{
    printf("{ %d, %s, %s, %s }\n", rec.id, rec.name, rec.surname, rec.address);
}

/* HT_info Functions -------------------------------------------------------------*/

/* Returns the header info of the hash file with the specified file descriptor.
   In case of an error, NULL is returned. */
HT_info* Get_HT_info(int fd)
{
    void *block;

    // Return fail if this is not a hash file
    if ( !IsHashFile(fd) ) { return NULL; }

    // The header is placed in block 0
    if (BF_ReadBlock(fd, 0, &block) < 0) { return NULL; }

    // The header is a byte sequence, so convert it to a struct HT_info
    HT_info *info = malloc(sizeof(HT_info));
    void *temp = (char*)block + strlen("hash") + 1;
    int len;

    // Copying file descriptor field
    memcpy(&(info->fileDesc), temp, sizeof(int));
    temp = (int*)temp + 1;

    // Copying attrType field
    memcpy(&(info->attrType), temp, 1);
    temp = (char*)temp + 1;

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

    return info;
}

/* Converts the specified HT_info into a byte sequence. */
void* Get_HT_info_Data(const HT_info *info)
{
    void *data;
    void *temp;
    int fd_size = sizeof(int);
    int type_size = 1;
    int name_size = 3;                      // 3 because "id" -> 2 chars + null character
    int length_size = sizeof(int);
    int buckets_size = sizeof(unsigned long int);

    // Allocating space for the struct
    data = malloc(HT_INFO_SIZE);
    temp = data;                    // Used to iterate over the sequence

    // Copying file descriptor
    memcpy(temp, &(info->fileDesc), fd_size);
    temp = (int*)temp + 1;

    // Copying attribute type
    memcpy( temp, &(info->attrType), type_size );
    temp = (char*)temp + type_size;

    // Copying attribute name
    memcpy( temp, info->attrName, name_size );
    temp = (char*)temp + name_size;

    // Copying attribute length
    memcpy( temp, &(info->attrLength), length_size);
    temp = (int*)temp + 1;

    // Copying number of buckets
    memcpy( temp, &(info->numBuckets), buckets_size);

    // Done.
    return data;
}

/* Properly deletes the specified HT_info. */
void delete_HT_info(HT_info *info)
{
    free(info->attrName);
    free(info);
}

/* Record-Block Functions */

/* Creates a new, initiallized record block.
   Returns the number of the new block, or -1 in case of an error. */
int NewRecordBlock(int fd)
{
    // Allocation the new block
    if (BF_AllocateBlock(fd) < 0) { return -1; }

    int block_num = BF_GetBlockCounter(fd) - 1;
    void *block;
    if (BF_ReadBlock(fd, block_num, &block) < 0) { return -1; }

    // Initializing the new block
    SetNextBlockNumber(block, -1);
    SetNumRecords(block, 0);

    // Write it back
    if (BF_WriteBlock(fd, block_num) < 0) { return -1; }

    // Return the number of the new block
    return block_num;
}

/* Returns the number of records in the specified block. */
int GetBlockNumRecords(void *block)
{
    int num;
    // The number of records is placed before the next block number
    memcpy(&num, (char*)block + BLOCK_SIZE - 2*sizeof(int), sizeof(int));
    return num;
}

/* Sets the next block number of the specified block to NUM. */
void SetNextBlockNumber(void *current, int num)
{
    // The number is an int located at the end of the block
    memcpy((char*)current + BLOCK_SIZE - sizeof(int), &num, sizeof(int) );
}

/* Returns the next block number in the specified block. */
int GetNextBlockNumber(void *current)
{
    int next;
    memcpy(&next, (char*)current + BLOCK_SIZE - sizeof(int), sizeof(int) );
    return next;
}

/* Sets the number of records in the specified block to N. */
void SetNumRecords(void *block, int n)
{
    memcpy( (char*)block + BLOCK_SIZE - 2*sizeof(int), &n, sizeof(int) );
}

/* Creates and adds a new block after the one with the specified number.
   Returns the number of the new block, or -1 in case of an error. */
int AddNextBlock(int fd, int current_num)
{
    void *current, *new;
    int new_num;

    // Read the current block
    if (BF_ReadBlock(fd, current_num, &current) < 0) { return -1; }

    // Allocate a new block
    if (BF_AllocateBlock(fd) < 0) { return -1; }

    // Get the new block number
    if ( (new_num = BF_GetBlockCounter(fd) - 1) < -1  ) { return -1; }

    // Set the next block number of the current block
    SetNextBlockNumber(current, new_num);

    // Write back the current block
    BF_WriteBlock(fd, current_num);

    // Read the new block and initialize it properly
    if (BF_ReadBlock(fd, new_num, &new) < 0) { return -1; }
    SetNumRecords(new, 0);
    SetNextBlockNumber(new, -1);

    // Write back the new block
    BF_WriteBlock(fd, new_num);

    // Return the number of the new block
    return new_num;
}

/* Inserts the specified record in the block with the specified number.
   Whether a record with the same key already exists must have been previously checked.
   If the record was successfully inserted, returns 0, otherwise -1.*/
int InsertRecordtoBlock(int fd, int block_num, Record rec)
{
    void *block;

    // Reading the block using file descriptor and getting number of records in it
    if (BF_ReadBlock(fd, block_num, &block) < 0) { return -1; }
    int num_records = GetBlockNumRecords(block);

    if( num_records < MAX_RECORDS)
    // The block still has space for a new record
    {
        // Convert record into byte sequence
        void* data = GetRecordData(&rec);

        // Write the sequence right after the last record
        memcpy( (char*)block + num_records*RECORD_SIZE, data, RECORD_SIZE );
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

/* Returns 0 if the specified block contains a record with KEY_NAME
   field value equal to the one in the specified Record. Returns -1 otherwise. */
int BlockHasRecordWithKey(void *block, const char* key_name, Record *rec)
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
        if (RecordKeyHasValue(curr_record, key_name, &(rec->id)) == 0)
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

/* Returns a pointer to the last record of the specified block. */
void* GetLastRecord(void *block)
{
    return (char*)block + (GetBlockNumRecords(block) - 1)*RECORD_SIZE;
}

/* Deletes the first record from the block where KEY_NAME field value is equal to VALUE.
   Returns 0 if the record was deleted, -1 otherwise. Note that only the first occurence is deleted.*/
int DeleteRecordFromBlock(void *block, const char *key_name, void *value)
{
    int num_records = GetBlockNumRecords(block);
    if(num_records == 0) { return -1; }

    int curr_record_num = 1;
    void *curr_record;
    curr_record = block;

    // Iterate over the records in the block
    while( curr_record_num <= num_records )
    {
        if (RecordKeyHasValue(curr_record, key_name, value) == 0)
        // Found record to delete
        {
            if (curr_record_num!= num_records)
            // This was not the last record
            {
                // Replace it the last record
                CopyRecord(curr_record, GetLastRecord(block));
            }
            // Decrement record counter
            SetNumRecords(block, num_records -1);
            return 0;
        }
        curr_record = NextRecord(curr_record);
        curr_record_num++;
    }

    // No record was deleted
    return -1;
}

/* Prints all the records in the specified block, with KEY_NAME field value equal to VALUE.
   Returns 0 if any records where printed, -1 otherwise.
   In case VALUE is NULL, all records will be printed. */
int PrintBlockRecordsWithKey(void *block, const char *key_name, void *value)
{
    int num_records = GetBlockNumRecords(block);
    if(num_records == 0) { return -1; }
    int found_records = 0;

    int curr_record_num = 1;

    void *curr_record;
    curr_record = block;
    Record *record;

    if (value == NULL)
    // If VALUE is NULL, print all the records
    {
        // Iterate over the records
        while( curr_record_num <= num_records )
        {
            // Each record is stored as a byte sequence, so convert it in a struct Record
            record = GetRecord(curr_record);
            // And print it
            PrintRecord(*record);
            found_records++;
            free(record);

            // Go to the next record
            curr_record = NextRecord(curr_record);
            curr_record_num++;
        }
    }
    else
    {
        // Iterate over the records
        while( curr_record_num <= num_records )
        {
            if (RecordKeyHasValue(curr_record, key_name, value) == 0)
            // Found record to print
            {
                // Each record is stored as a byte sequence, so convert it in a struct Record
                record = GetRecord(curr_record);
                // And print it
                PrintRecord(*record);
                found_records++;
                free(record);
            }
            // Go to the next record
            curr_record = NextRecord(curr_record);
            curr_record_num++;
        }
    }
    if (found_records) { return 0; }
    
    // No records were printed
    return -1;
}

/* Bucket-Block functions --------------------------------------------------------*/

/* Initializes all the buckets of the given block to -1 (empty). */
void InitBuckets(void *block)
{
    int num_buckets = 0;
    void *temp = block;
    int init_value = -1;

    // Iterate over the buckets
    while (num_buckets < MAX_BUCKETS)
    {
        // Set each to -1
        SetBucket(temp, init_value);
        temp = GetNextBucket(temp);

        // Continue to the next one
        num_buckets++;
    }
}

/* Returns the address of the next bucket (for better readability). */
void* GetNextBucket(void *current)
{
    return (int*)current + 1;
}

/* Sets the value of the specified bucket to BN. */
void SetBucket(void *current, int bn)
{
    memcpy(current, &bn, sizeof(int));
}

/* Inserts the specified Record in the bucket block (specified by number), if the key field value is unique.
   Returns the number of the block where the Record was inserted.
   If the Record was not inserted (duplicate key or error), -1 is returned. */
int InsertEntryToBucket(int fd, int starting_block_num, Record record, const char *key_name)
{
    void *current;
    
    int current_block_num = 0;
    int next_block_num = starting_block_num;    // Begin from the first block of the bucket (where the bucket points to)
    int empty_block_found = 0;                  // Indicates whether a non-full block has been found
    int empty_block_num = -1;
    int num_rec;

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
            if ( num_rec < MAX_RECORDS )
            // If the current block has free space
            {
                if ( !empty_block_found )
                // If this is the first non-full block found, save it
                {
                    empty_block_num = current_block_num;
                    empty_block_found = 1;
                }
            }
            if (BlockHasRecordWithKey(current, key_name, &record) == 0)
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
        if (InsertRecordtoBlock(fd, new_block_num, record) == 0)
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
        if (InsertRecordtoBlock(fd, empty_block_num, record) == 0)
        {
            return empty_block_num;
        }
    }
}

/* Deletes the Record with key field == VALUE from the bucket block (specified by number). 
   Returns 0 if the Record was deleted, -1 otherwise.
   Note that only the first occurence is deleted. */
int DeleteEntryFromBucket(int fd, int starting_block_num, void *key_value, const char *key_name)
{
    void *current;    
    // Begin from the first block of the bucket (where the bucket points to)
    int current_block_num = starting_block_num;
    int num_rec;

    // Iterate over the blocks of the bucket
    while(current_block_num != -1)
    {
        if (BF_ReadBlock(fd, current_block_num, &current) < 0) { return -1; }
        num_rec = GetBlockNumRecords(current);

        // Ignore empty blocks
        if (num_rec != 0)
        {
            // Attempt to delete the record from every block, until it is deleted successfully
            if (DeleteRecordFromBlock(current, key_name, key_value) == 0)
            // Record Deleted
            {
                // Write-back the block and return success
                if (BF_WriteBlock(fd, current_block_num) < 0) { return -1; }
                return 0;
            }
        }
        current_block_num = GetNextBlockNumber(current);
    }

    // The entry was not deleted, so it was not found
    return -1;
}

/*  Prints all Records with KEY_NAME field == KEY_VALUE, in the bucket starting
    at the block with the specified number.
    Returns the number of blocks read *up to the last printed record*, or -1 in case of error. */
int GetAllBucketEntries(int fd, int starting_block_num, void *key_value, const char *key_name)
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
        if (PrintBlockRecordsWithKey(curr_block, key_name, key_value) == 0)
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

/* Calculates statistics for the bucket that starts at the block with the specified number.
   Stores the number of total blocks and total records in bucket, in TOTAL_BLOCKS and TOTAL_RECORDS respectively.
   Returns 0 if successful, -1 if failed. */
int GetBucketStats(int fd, int starting_block_num, int *total_blocks, int *total_records)
{
    void *curr_block;
    
    // Begin from the first block of the bucket (where the bucket points to)
    int curr_block_num = starting_block_num;
    int block_counter = 0;
    int record_counter = 0;

    // Iterate over the blocks of the bucket
    while(curr_block_num != -1)
    {
        if (BF_ReadBlock(fd, curr_block_num, &curr_block) < 0 ) { return -1; }

        // Update counters after evaluating this block
        record_counter += GetBlockNumRecords(curr_block);
        block_counter++;
        // And continue to the next one
        curr_block_num = GetNextBlockNumber(curr_block);
    }

    *total_blocks = block_counter;
    *total_records = record_counter;

    return 0;
}

/* More general HT functions -----------------------------------------------------*/

/*  Initializes all the recuired information for the specified Hash file
    (given attributes, blocks, buckets, etc.)
    Returns 0 if succesful, -1 if failed. */
int HT_InitFile(int fd, char type, const char *name, int length, unsigned long int buckets)
{
    void* block;
    HT_info info;
    void *data;

    // Creating the first block of the file
    if(BF_AllocateBlock(fd) < 0) { return -1; }

    // Since it is the first, block id  is 0
    if(BF_ReadBlock(fd, 0, &block) < 0) { return -1; }

    // Creating a header struct with all the required information
    info.fileDesc = fd;
    info.attrType = type;
    info.attrName = malloc(strlen(name) + 1);
    strcpy(info.attrName, name);
    info.attrLength = length;
    info.numBuckets = buckets;

    // Converting the struct into a byte sequence
    data = Get_HT_info_Data(&info);

    free(info.attrName);

    // Storing a "hash" string to identify that this is a Hash File
    memcpy(block, "hash", strlen("hash") + 1);

    // Storing the header sequence
    memcpy(block + strlen("hash") + 1, data, HT_INFO_SIZE);

    free(data);

    // Initialize pointer to next block (-1)
    SetNextBlockNumber(block, -1);

    if (BF_WriteBlock(fd, 0) < 0) { return -1; }

    // Done.
    return 0;
}

/* Creates the buckets of the file with the speicfied file descriptor.
   Returns 0 if successful, -1 if failed. */
int HT_CreateBuckets(int fd, int buckets)
{
    // Calculating how many blocks will be needed to store the buckets
    int nof_blocks = (buckets / MAX_BUCKETS) + 1;

    void *curr_block;
    int block_counter = 0;
    int next_block_num = 0;
    int curr_block_num = 0;

    // Begin from header block
    if ( BF_ReadBlock(fd, 0, &curr_block) < 0) { return -1; }

    // Adding a new block after the last one in every loop
    while(block_counter < nof_blocks)
    {       
        // Creating new block and connecting it with the last one
        if (BF_AllocateBlock(fd) < 0 ) { return -1; }
        curr_block_num = next_block_num;
        next_block_num = BF_GetBlockCounter(fd) - 1;
        SetNextBlockNumber(curr_block, next_block_num);

        if ( BF_WriteBlock(fd, curr_block_num) < 0) { return -1; }

        // Reading the new block (which now becomes the current one)
        block_counter++;
        if ( BF_ReadBlock(fd, next_block_num, &curr_block) < 0) { return -1; }

        // Initializing the buckets of the new block
        InitBuckets(curr_block);
    }
    // The last block has -1 as next block number
    SetNextBlockNumber(curr_block, -1);
    if ( BF_WriteBlock(fd, next_block_num) < 0) { return -1; }

    return 0;
}

/* Creates a Hash File with the specified information. Returns -1 in case of an error, 0 otherwise. */
int HT_CreateIndex( char *fileName, char attrType, char* attrName, int attrLength, int buckets )
{
    int fd;

    // Creating the file in block-level
    if(BF_CreateFile(fileName) < 0) { return -1; }

    fd = BF_OpenFile(fileName);
    if( fd < 0) { return -1; }

    // Initializing the new file
    if ( HT_InitFile(fd, attrType, attrName, attrLength, buckets) < 0 ) { return -1; }

    // Creating bucket blocks
    if ( HT_CreateBuckets(fd, buckets) < 0) { return -1; }

    // File is ready
    if (BF_CloseFile(fd) < 0) { return -1; }
    
    return 0;
}

/* Opens the Hash File with the specified name.
   If succesful, returns the header struct of this file, otherwise returns NULL. */
HT_info* HT_OpenIndex(char *fileName)
{
    HT_info *info;
    int fd;

    // Opening the file
    if ( (fd = BF_OpenFile(fileName)) < 0) { return NULL; }

    // Getting header information
    info = Get_HT_info(fd);

    return info;
}

/* Closes the Hash File specified by the given header struct, which is deleted.
   Returns 0 if successful, -1 otherwise. */
int HT_CloseIndex(HT_info *header_info)
{
    if (header_info == NULL) { return -1; }

    if (BF_CloseFile(header_info->fileDesc) < 0) { return -1; }

    delete_HT_info(header_info);
    return 0;
}

/*  Inserts the specified record in the hash file, as long as the key field (as specified in
    the header) does not have a duplicate value.
    Takes advantage of hashing in order to quickly locate the bucket where the block should be inserted. 
    Returns the number of the block where the record was inserted, otherwise -1.*/
int HT_InsertEntry(HT_info header_info, Record record)
{
    int fd = header_info.fileDesc;
    void *current_block;
    
    // Starting from the header block
    if (BF_ReadBlock(fd, 0, &current_block) < -1) { return -1; }

    int current_block_num = 0;
    int next_block_num = GetNextBlockNumber(current_block);

    int hash_code = GetHashcode(record.id, header_info.numBuckets);
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
            return InsertEntryToBucket(fd, bucket_starting_block, record, header_info.attrName);            
        }        
    }
    return -1;    
}

/* Deletes the Record with key field == VALUE from the file. 
   Returns 0 if the Record was deleted, -1 otherwise.
   Takes advantage of hashing to quickly locate the bucket where the entry is.
   Note that only the first occurence is deleted. */
int HT_DeleteEntry(HT_info header_info, void *value)
{
    int fd = header_info.fileDesc;
    void *current_block;
    
    if (BF_ReadBlock(fd, 0, &current_block) < -1) { return -1; }

    int current_block_num = 0;
    int next_block_num = GetNextBlockNumber(current_block);

    int hash_code = GetHashcode(*((int*)value), header_info.numBuckets);
    
    int target_block = hash_code / MAX_BUCKETS;                 // The block where the bucket for the hashcode is (relative position, not the id)
    void *target_bucket;
    int bucket_starting_block;
    int block_counter = 0;

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
        {
            // Finding the *relative* position of the bucket in the block
            target_bucket = (int*)current_block + hash_code - (block_counter*MAX_BUCKETS);

            memcpy(&bucket_starting_block, target_bucket, sizeof(int));

            if (bucket_starting_block == -1)
            // Bucket Empty, so nothing to delete
            {
                return -1;
            }
            else
            // Bucket not empty, so attempt to delete
            {
                return DeleteEntryFromBucket(fd, bucket_starting_block, value, header_info.attrName);
            }
        }        
    }
    return -1;
}

/* Prints all the records in the heap file with key field (attrName) value == VALUE.
   Returns the number of read blocks **up to the last printed Record** (-1 in case of an error).
   It works for any possible key field, but does not take advantage of hashing.*/
int HT_GetAllEntries(HT_info header_info, void *value)
{
    int fd = header_info.fileDesc;
    void *current_block;
    
    if (BF_ReadBlock(fd, 0, &current_block) < -1) { return -1; }

    int current_block_num = GetNextBlockNumber(current_block);

    int bucket_starting_block;

    // Counts total read blocks
    int block_counter = 0;
    int *current_bucket;
    int bucket_counter;

    // Iterating over the Hash Table blocks (indexes)
    while (current_block_num != -1)
    {
        if (BF_ReadBlock(fd, current_block_num, &current_block) < 0) { return -1; }
        // Iterating over the buckets of this block
        bucket_counter = 0;
        while (bucket_counter < MAX_BUCKETS)
        {
            // Without this, current_block is reset at some point and the functionality totally breaks
            if (BF_ReadBlock(fd, current_block_num, &current_block) < 0) { return -1; }

            // Getting the bucket value
            current_bucket = (int*)current_block + bucket_counter;

            // If bucket empty, skip it
            if ( *current_bucket != -1 )
            // Bucket is not empty, so get the entries in it
            {
                block_counter += GetAllBucketEntries(fd, *current_bucket, value, header_info.attrName);
            }
            bucket_counter++;
        }
        block_counter++;
        current_block_num = GetNextBlockNumber(current_block);
    }

    return block_counter;
}

/*  Prints a (unique) record for which key(id) == VALUE.
    Takes advantage of hashing to locate the corresponding bucket.
    Returns the number of blocks read until the record was found (or not found), -1 in case of error. */
int HT_GetUniqueEntry(HT_info header_info, void *value)
{
    int fd = header_info.fileDesc;
    void *current_block;
    
    if (BF_ReadBlock(fd, 0, &current_block) < -1) { return -1; }

    int current_block_num = GetNextBlockNumber(current_block);

    int hash_code = GetHashcode(*((int*)value), header_info.numBuckets);    
    int target_block = hash_code / MAX_BUCKETS;                 // The block where the bucket for this hashcode is (relative position, not the id)
    int *target_bucket;
    int block_counter = 0;                                      // Counts read blocks

    // Iterating over the hashtable blocks
    while (current_block_num != -1)
    {
        if (BF_ReadBlock(fd, current_block_num, &current_block) < 0) { return -1; }

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
                return block_counter + GetAllBucketEntries(fd, *target_bucket, value, header_info.attrName);
            }
        }        
    }

    return block_counter;
}

/* Returns 1 if the file with the specified file descriptor is a hash file, otherwise returns 0. */
int IsHashFile(int fd)
{
    void *block;

    // Reading first block in the file
    if(BF_ReadBlock(fd, 0, &block) < 0) { return -1; }

    // Comparing the beginning of the file for "hash" string
    if (memcmp(block, "hash", strlen("hash") + 1) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }    
}