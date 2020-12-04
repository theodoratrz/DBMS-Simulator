#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HP.h"
#include "BF.h"

int main(void)
{
    char *fileName = "file";
    BF_Init();

    HP_CreateFile(fileName, 'i', "id", 4);
    HP_info *info = HP_OpenFile(fileName);
    Record rec;

    for (int i = 0; i < 5; i++)
    {
        rec.id = i;
        strcpy (rec.name, "_name_");
        strcpy(rec.surname, "_surname_");
        strcpy(rec.address, "_address_");
        HP_InsertEntry(*info, rec);
    }

    for (int i = 5; i < 10; i++)
    {
        rec.id = i;
        strcpy (rec.name, "@name@");
        strcpy(rec.surname, "@surname@");
        strcpy(rec.address, "@address@");
        HP_InsertEntry(*info, rec);
    }

    for (int i = 10; i < 15; i++)
    {
        rec.id = i;
        strcpy (rec.name, "_name_");
        strcpy(rec.surname, "_surname_");
        strcpy(rec.address, "_address_");
        HP_InsertEntry(*info, rec);
    }

    free(info->attrName);
    info->attrName = malloc(strlen("name") + 1 );
    strcpy(info->attrName, "name");
    int n;
    n =  HP_GetAllEntries(*info, "_name_");
    
    printf("%d\n", n);

    n = HP_GetAllEntries(*info, "@name@");

    printf("%d\n", n);
    
    if (HP_CloseFile(info) < 0) {return 1;}
    
    return 0;
}
