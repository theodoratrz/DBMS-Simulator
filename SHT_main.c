/*
 * File: HT_main.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BF.h"
#include "HT.h"
#include "SHT.h"

#define INPUT_FILE "record_examples/records1K.txt"
#define NUM_BUCKETS 2000

int main(void)
{
    char *fileName = "HT_file";
    char *sfileName = "SHT_file";
    int buckets = NUM_BUCKETS;

    BF_Init();
    HT_CreateIndex(fileName, 'i', "id", 4, buckets);
    HT_info *info = HT_OpenIndex(fileName);
    SHT_CreateSecondaryIndex(sfileName, "surname", 25, NUM_BUCKETS, fileName);
    SHT_info *sec_info = SHT_OpenSecondaryIndex(sfileName);

    // Read input file, parse each line into a record and insert it in the hash file
    int current_field = 0;
    FILE *inputFile = fopen(INPUT_FILE, "r");
    if (inputFile == NULL)
    {
        perror("Unable to read file.\n");
        return 1;
    }
    int c = EOF;

    // Used for parsing
    Record rec;
    SecondaryRecord srec;
    char *id;
    char *name;
    char *surname;
    char *address;
    int end;

    // Counts stored records
    int records = 0;
    
    while ( (c = getc(inputFile) ) != EOF )
    {
        if (c == ',') {
            if (current_field >= 4)
            {
                perror("Invalid input\n");
                return 1;
            }
            else
            {
                current_field++;
                continue;
            }                      
        }
        else if (c == '\n')
        {
            if ( !(current_field == 4) )
            {
                printf("Input Error.\n");
                return 1;
            }
            else
            {
                // A record has been fully read at this point
                // Storing data at the record
                rec.id = atoi(id);
                strcpy(rec.name, name);
                strcpy(rec.surname, surname);
                strcpy(rec.address, address);

                srec.record = rec;

                // Inserting record in hash file
                srec.blockId = HT_InsertEntry(*info, rec);
                SHT_SecondaryInsertEntry(*sec_info, srec);

                if (srec.blockId < 0) { exit(EXIT_FAILURE); }

                records++;

                // Prepare for next record
                current_field = 0;
                free(id);
                free(name);
                free(surname);
                free(address);
                continue;
            }            
        }
        else if ( c == '{' || c == '}' || c == '"')
        {
            continue;
        }     
        switch (current_field)
        {
            case 0:
                id = malloc(sizeof(char));
                id[0] = '\0';
                name = malloc(sizeof(char));
                name[0] = '\0';
                surname = malloc(sizeof(char));
                surname[0] = '\0';
                address = malloc(sizeof(char));
                address[0] = '\0';
                current_field = 1;
            case 1:
                end = strlen(id);
                id = realloc(id, end + 2);
                id[end] = c;
                id[end + 1] = '\0';
                break;
            case 2:
                end = strlen(name);
                name = realloc(name, end + 2);
                name[end] = c;
                name[end + 1] = '\0';
                break;
            case 3:
                end = strlen(surname);
                surname = realloc(surname, end + 2);
                surname[end] = c;
                surname[end + 1] = '\0';
                break;
            case 4:
                end = strlen(address);
                address = realloc(address, end + 2);
                address[end] = c;
                address[end + 1] = '\0';
                break;      
            default:
                break;
        }
    }
    // At this point, all records in file have been inserted
    fclose(inputFile);
    HT_CloseIndex(info);
    
    char temp[25];
    for (int i = 0; i < records; i++)
    {
        sprintf(temp, "surname_%d", i);
        SHT_SecondaryGetAllEntries(*sec_info, temp);
    }

    return 0;
}