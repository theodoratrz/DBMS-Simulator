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

    for (int i = 0; i < 10000; i++)
    {
        rec.id = i;
        sprintf(rec.name, "name%d", i);
        sprintf(rec.surname, "surname%d", i);
        sprintf(rec.address, "address");
        HP_InsertEntry(*info, rec);
    }

    for (int i = 5000; i < 10000; i++)
    {
        HP_DeleteEntry(*info, &i);
    }

    for (int i = 10000; i < 15000; i++)
    {
        rec.id = i;
        sprintf(rec.name, "name%d", i);
        sprintf(rec.surname, "surname%d", i);
        sprintf(rec.address, "address");
        HP_InsertEntry(*info, rec);
    }

    /*
    for (int i = 0; i < 15000; i++)
    {
        HP_GetAllEntries(*info, &i);
    }
    */
    HP_GetAllEntries(*info, NULL); 
    
    if (HP_CloseFile(info) < 0) {return 1;}
    
    return 0;
}
