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

/* Returns a pointer to the last record of the specified block. */
void* SHT_GetLastRecord(void *block)
{
    return (char*)block + (GetBlockNumRecords(block) - 1)*SHT_Record_size();
}