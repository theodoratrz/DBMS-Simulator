/*
 * File: HP_main.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BF.h"
#include "HP.h"

#define INPUT_FILE "record_examples/records5K.txt"

int main(void)
{
    char *fileName = "HP_file";

    BF_Init();
    if (HP_CreateFile(fileName, 'i', "id", 4) < 0)
    {
        perror("Could not create heap file.\n");
        exit(EXIT_FAILURE);
    }
    HP_info *info = HP_OpenFile(fileName);

    if (info == NULL)
    {
        perror("Could not open heap file.\n");
        exit(EXIT_FAILURE);
    }

    // Read input file, parse each line into a record and insert it in the heap file
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

                // Inserting record in heap file
                if (HP_InsertEntry(*info, rec) < 0)
                {
                    perror("Error Inserting Record");
                    fclose(inputFile);
                    if (HP_CloseFile(info) < 0)
                    {
                        perror("Error closing HP file");
                    }
                    exit(EXIT_FAILURE);
                }
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

    // Deleting some of them
    for (int i = 0; i < (records/2); i++)
    {
        // Deleting record with id == i
        if (HP_DeleteEntry(*info, &i) < 0 )
        {
            perror("Error Deleting Record");
            fclose(inputFile);
            if (HP_CloseFile(info) < 0)
            {
                perror("Error closing HP file");
            }
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < records; i++)
    {
        if ( HP_GetAllEntries(*info, &i) < 0)
        {
            perror("Error Getting Record");
            fclose(inputFile);
            if (HP_CloseFile(info) < 0)
            {
                perror("Error closing HT file");
            }
            exit(EXIT_FAILURE);
        }
    }

    // We can also make this call alternatively (prints all stored records)
    //HP_GetAllEntries(*info, NULL); 
    
    if (HP_CloseFile(info) < 0) {return 1;}

    return 0;
}
