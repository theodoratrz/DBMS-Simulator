/*
 * File: HP.c
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "HP.h"
#include "BF.h"

/* Record functions ------------------------------------------------------------------ */

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

/* Prints the specified Record. */
void PrintRecord(Record rec)
{
    printf("{ %d, %s, %s, %s }\n", rec.id, rec.name, rec.surname, rec.address);
}

/* Returns 0 if the KEY_NAME field value of the specified record is equal to VALUE, -1 otherwise. */
int HP_RecordKeyHasValue(void *record, const char *key_name, void *value)
{
    Record *temp;

    // The specified record is a byte sequence, so convert it to a struct Record to make thing simpler
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

/* HP_info functions ----------------------------------------------------------------- */

/* Returns the header info of the heap file with the specified file descriptor. */
HP_info* Get_HP_info(int fd)
{
    void *block;

    // The header is placed in block 0
    if (BF_ReadBlock(fd, 0, &block) < 0) { return NULL; }

    if (memcmp(block, "heap", strlen("heap") + 1) != 0)
    // Return fail if this is not a heap file
    {
        return NULL;
    }

    // The header is a byte sequence, so convert it to a struct HP_info
    HP_info *info = malloc(sizeof(HP_info));
    void *temp = (char*)block + strlen("heap") + 1;     // Used to iterate over the sequence
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

    return info;
}

/* Converts the specified HP_info into a byte sequence. */
void* Get_HP_info_Data(const HP_info *info)
{
    void *data;
    void *temp;
    int fd_size = sizeof(int);
    int type_size = 1;
    int name_size = 3;              // 3 because "id" -> 2 chars + null character
    int length_size = sizeof(int);

    // Allocating space
    data = malloc(HP_INFO_SIZE);
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

    // Done.
    return data;
}

/* Properly deletes the specified HP_info. */
void delete_HP_info(HP_info *info)
{
    free(info->attrName);
    free(info);
}

/* Block functions ------------------------------------------------------------------- */

// TO BE DELETED (Not used)
void* HP_GetNextBlock(int fd, void *current)
{
    int next_block_num;
    void *next_block;

    next_block_num = HP_GetNextBlockNumber(current);

    if (next_block_num == -1) { return NULL; }
    if (BF_ReadBlock(fd, next_block_num, &next_block) < 0) { return NULL; }

    return next_block;
}

/* Sets the next block number of the specified block to NUM. */
void HP_SetNextBlockNumber(void *current, int num)
{
    // The number is an int located at the end of the block
    memcpy((char*)current + BLOCK_SIZE - sizeof(int), &num, sizeof(int) );
}

/* Returns the next block number in the specified block. */
int HP_GetNextBlockNumber(void *current)
{
    int next;
    // The number is an int located at the end of the block
    memcpy(&next, (char*)current + BLOCK_SIZE - sizeof(int), sizeof(int) );
    return next;
}

/* Sets the number of records in the specified block to N. */
void HP_SetNumRecords(void *block, int n)
{
    // The number of records is placed before the next block number
    memcpy( (char*)block + BLOCK_SIZE - 2*sizeof(int), &n, sizeof(int) );
}

/* Returns the number of records in the specified block. */
int HP_GetNumRecords(void *block)
{
    int num;
    memcpy(&num, (char*)block + BLOCK_SIZE - 2*sizeof(int), sizeof(int));
    return num;
}

/* Creates and adds a new block after the one with the specified number. */
int HP_AddNextBlock(int fd, int current_num)
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
    HP_SetNextBlockNumber(current, new_num);

    // Write back the current block
    BF_WriteBlock(fd, current_num);

    // Reade the new block and initialize it properly
    if (BF_ReadBlock(fd, new_num, &new) < 0) { return -1; }
    HP_SetNumRecords(new, 0);
    HP_SetNextBlockNumber(new, -1);

    // Write back the new block
    BF_WriteBlock(fd, new_num);

    // Return the number of the new block
    return new_num;
}

/* Inserts the specified record in the block with the specified number.
   Whether a record with the same key already exists must have been previously checked.
   If the record was successfully inserted, returns 0, otherwise -1.*/
int HP_InsertRecordtoBlock(int fd, int block_num, Record rec)
{
    void *block;

    // Reading the block using file descriptor and getting number of block in it
    if (BF_ReadBlock(fd, block_num, &block) < 0) { return -1; }
    int num_records = HP_GetNumRecords(block);

    if( num_records < MAX_RECORDS)
    // The block still has space for a new record
    {
        // Convert record into byte sequence
        void* data = GetRecordData(&rec);

        // Write the sequence right after the last record
        memcpy( (char*)block + num_records*RECORD_SIZE, data, RECORD_SIZE );
        free(data);     // this won't be needed anymore

        // Increase the number of records
        HP_SetNumRecords(block, num_records + 1);

        // The block is ready to be written back to the disk
        if (BF_WriteBlock(fd, block_num) < 0){ return -1; }

        // Return success
        return 0;
    }
    else
    // In case the block is full, return fail
    {
        return -1;
    }
}

/* Returns 0 if the specified block contains a record with KEY_NAME
   field value equal to the one in the specified Record. Returns -1 otherwise. */
int BlockHasRecordWithKey(void *block, const char* key_name, Record *rec)
{
    int num_records = HP_GetNumRecords(block);
    if(num_records == 0) { return -1; }

    int curr_record_num = 1;
    void *curr_record;
    curr_record = block;

    while( curr_record_num <= num_records )
    {
        if (HP_RecordKeyHasValue(curr_record, key_name, &(rec->id)) == 0)
        {
            return 0;
        }
        curr_record = NextRecord(curr_record);
        curr_record_num++;
    }
    return -1;
}

/* Returns a pointer to the last record of the specified block. */
void* GetLastRecord(void *block)
{
    return (char*)block + (HP_GetNumRecords(block) - 1)*RECORD_SIZE;
}

/* Deletes the first record from the block where KEY_NAME field value is equal to VALUE.
   Returns 0 if the record was eventually deleted, -1 otherwise. */
int HP_DeleteRecordFromBlock(void *block, const char *key_name, void *value)
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

    // TEST THAT
    return -1;
}

int PrintBlockRecordsWithKey(void *block, const char *key_name, void *value)
{
    int num_records = HP_GetNumRecords(block);
    if(num_records == 0) { return -1; }
    int found_records = 0;

    int curr_record_num = 1;

    void *curr_record;
    curr_record = block;
    Record *record;

    if (value == NULL)
    {
        while( curr_record_num <= num_records )
        {
            record = GetRecord(curr_record);
            PrintRecord(*record);
            found_records++;
            free(record);
            curr_record = NextRecord(curr_record);
            curr_record_num++;
        }
    }
    else
    {
        while( curr_record_num <= num_records )
        {
            if (HP_RecordKeyHasValue(curr_record, key_name, value) == 0)
            {
                record = GetRecord(curr_record);
                PrintRecord(*record);
                found_records++;
                free(record);
            }
            curr_record = NextRecord(curr_record);
            curr_record_num++;
        }
    }

    if (found_records) { return 0; }
    
    return -1;
}

/* General HP functions ----------------------------------------------------------------- */

int HP_CreateFile(const char *fileName, const char attrType, const char* attrName, const int attrLength)
{
    int fd;

    if(BF_CreateFile(fileName) < 0) { return -1; }

    fd = BF_OpenFile(fileName);
    if( fd < 0) { return -1; }

    if ( HP_InitFile(fd, attrType, attrName, attrLength) < 0 ) { return -1; }

    if (BF_CloseFile(fd) < 0) { return -1; }
    
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

    delete_HP_info(header_info);
    return 0;
}

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

    free(info.attrName);

    memcpy(block, "heap", strlen("heap") + 1);

    //memcpy(block + strlen("heap") + 1, data, sizeof(HP_info));
    memcpy(block + strlen("heap") + 1, data, HP_INFO_SIZE);

    free(data);

    // set pointer to next block (-1)
    HP_SetNextBlockNumber(block, -1);

    if (BF_WriteBlock(fd, 0) < 0) { return -1; }

    return 0;
}

int HP_InsertEntry( HP_info header_info, Record record )
{
    int fd = header_info.fileDesc;
    
    void *current;
    if (BF_ReadBlock(fd, 0, &current) < -1) { return -1; }
    
    int current_block_num = 0;
    int next_block_num = HP_GetNextBlockNumber(current);
    int empty_block_found = 0;
    int empty_block_num = -1;
    int num_rec;

    while(next_block_num != -1)
    {
        current_block_num = next_block_num;
        if (BF_ReadBlock(fd, current_block_num, &current) < 0) { return -1; }
        num_rec = HP_GetNumRecords(current);

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
            if (BlockHasRecordWithKey(current, header_info.attrName, &record) == 0)
            {
                return -1;
            }
        }
        next_block_num = HP_GetNextBlockNumber(current);
    }

    if ( !empty_block_found)
    {
        int new_block_num = HP_AddNextBlock(fd, current_block_num); 
        if (new_block_num < 0) { return -1; }

        if (HP_InsertRecordtoBlock(fd, new_block_num, record) == 0)
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
        if (HP_InsertRecordtoBlock(fd, empty_block_num, record) == 0)
        {
            return empty_block_num;
        }
    }
}

int HP_DeleteEntry(HP_info header_info, void *value )
{
    int fd = header_info.fileDesc;
    void *curr_block;

    if ( BF_ReadBlock(fd, 0, &curr_block) < 0) { return -1; }

    int curr_block_num = HP_GetNextBlockNumber(curr_block);

    while(curr_block_num != -1)
    {
        if (BF_ReadBlock(fd, curr_block_num, &curr_block) < 0 ) { return -1; }

        if (HP_DeleteRecordFromBlock(curr_block, header_info.attrName, value) == 0)
        // Record Deleted
        {
            if (BF_WriteBlock(fd, curr_block_num) < 0) { return -1; }
            return 0;
        }
        curr_block_num = HP_GetNextBlockNumber(curr_block);
    }
    return -1;
}

int HP_GetAllEntries(HP_info header_info, void *value)
{
    int fd = header_info.fileDesc;
    void *curr_block;

    if ( BF_ReadBlock(fd, 0, &curr_block) < 0) { return -1; }

    int curr_block_num = HP_GetNextBlockNumber(curr_block);
    int read_blocks_until_rec = 0;
    int blocks_from_last_rec = 1;

    while(curr_block_num != -1)
    {
        if (BF_ReadBlock(fd, curr_block_num, &curr_block) < 0 ) { return -1; }

        if (PrintBlockRecordsWithKey(curr_block, header_info.attrName, value) == 0)
        {
            read_blocks_until_rec += blocks_from_last_rec;
            blocks_from_last_rec = 1;
        }
        else
        {
            blocks_from_last_rec++;
        }

        curr_block_num = HP_GetNextBlockNumber(curr_block);
    }
    return read_blocks_until_rec;
}