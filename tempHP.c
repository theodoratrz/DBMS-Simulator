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

    for (int i = 0; i < 205; i++)
    {
        rec.id = i;
        sprintf(rec.name, "name%d", i);
        sprintf(rec.surname, "surname%d", i);
        sprintf(rec.address, "address");
        HP_InsertEntry(*info, rec);
    }

    free(info->attrName);
    info->attrName = malloc(strlen("address") + 1 );
    strcpy(info->attrName, "address");
    int n;
    n =  HP_GetAllEntries(*info, "address");
    
    printf("%d\n", n);

    n =  HP_GetAllEntries(*info, "address");
    
    printf("%d\n", n);
    
    if (HP_CloseFile(info) < 0) {return 1;}
    
    return 0;
}
