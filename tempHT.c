#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HT.h"
#include "BF.h"

int main(void)
{
    char *fileName = "file";
    int i;
    BF_Init();

    HT_CreateIndex(fileName, 'i', "id", 4, 10);

    HT_info *info = HT_OpenIndex(fileName);

    Record rec;
    for (i = 66; i < 70; i++)
    {
        rec.id = i;
        sprintf(rec.name, "name%d", i);
        sprintf(rec.surname, "surname%d", i);
        sprintf(rec.address, "address%d", i);

        HT_InsertEntry(*info, rec);
    }

    i = 68;
    HT_DeleteEntry(*info, &i);
    i = 70;
    rec.id = i;
    sprintf(rec.name, "name%d", i);
    sprintf(rec.surname, "surname%d", i);
    sprintf(rec.address, "address%d", i);
    HT_InsertEntry(*info, rec);

    i = 68;
    rec.id = i;
    sprintf(rec.name, "name%d", i);
    sprintf(rec.surname, "surname%d", i);
    sprintf(rec.address, "address%d", i);
    HT_InsertEntry(*info, rec);

    for (int i = 71; i < 77; i++)
    {
        rec.id = i;
        sprintf(rec.name, "name%d", i);
        sprintf(rec.surname, "surname%d", i);
        sprintf(rec.address, "address%d", i);

        HT_InsertEntry(*info, rec);
    }
    i = 66;
    HT_DeleteEntry(*info, &i);
    i = 68;
    HT_DeleteEntry(*info, &i);
    i = 70;
    HT_DeleteEntry(*info, &i);

    HT_CloseIndex(info);
    
    return 0;
}
