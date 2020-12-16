#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HT.h"
#include "BF.h"

int main(void)
{
    char *fileName = "file";
    int i;
    int limit = 10000;
    BF_Init();

    HT_CreateIndex(fileName, 'i', "id", 4, 10000);

    HT_info *info = HT_OpenIndex(fileName);

    
    Record rec;
    for (i = 0; i < limit; i++)
    {
        rec.id = i;
        sprintf(rec.name, "name%d", i);
        sprintf(rec.surname, "surname%d", i);
        sprintf(rec.address, "address%d", i);

        HT_InsertEntry(*info, rec);
    }
    
    /*
    for (i = 5000; i < 10000; i++)
    {
        HT_DeleteEntry(*info, &i);
    }
    */
    
    for (i = 0; i < limit; i ++)
    {
        //HT_GetUniqueEntry(*info, &i);
        HT_GetAllEntries(*info, &i);
    }
    
    //HT_GetAllEntries(*info, NULL);

    HT_CloseIndex(info);
    
    return 0;
}
