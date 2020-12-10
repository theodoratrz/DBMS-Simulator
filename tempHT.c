#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HT.h"
#include "BF.h"

int main(void)
{
    char *fileName = "file";
    BF_Init();

    HT_CreateIndex(fileName, 'i', "id", 4, 3);

    HT_info *info = HT_OpenIndex(fileName);

    Record rec;
    for (int i = 66; i < 71; i++)
    {
        rec.id = i;
        sprintf(rec.name, "name%d", i);
        sprintf(rec.surname, "surname%d", i);
        sprintf(rec.address, "address");

        HT_InsertEntry(*info, rec);
    }

    HT_CloseIndex(info);
    
    return 0;
}
