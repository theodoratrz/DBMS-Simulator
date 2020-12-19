#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BF.h"
#include "HP.h"

#define INPUT_FILE "records5.txt"

int main(void)
{
    char *fileName = "HP_file";

    BF_Init();
    HP_CreateFile(fileName, 'i', "id", 4);
    HP_info *info = HP_OpenFile(fileName);

    // Read file, parse each line into a record and insert it
    int current_field = 0;
    FILE *inputFile = fopen(INPUT_FILE, "r");
    if (inputFile == NULL)
    {
        perror("Unable to read file.\n");
        return 1;
    }
    int c = EOF;

    Record rec;
    char *id;
    char *name;
    char *surname;
    char *address;
    int end;
    
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
                /*processing*/
                rec.id = atoi(id);
                strcpy(rec.name, name);
                strcpy(rec.surname, surname);
                strcpy(rec.address, address);

                PrintRecord(rec);

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
    
    fclose(inputFile);
    //HP_GetAllEntries(*info, NULL); 
    
    if (HP_CloseFile(info) < 0) {return 1;}

    return 0;
}
