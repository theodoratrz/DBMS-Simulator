#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HT.h"
#include "BF.h"

int main(void)
{
    char *fileName = "file";
    BF_Init();

    HT_CreateIndex(fileName, 'i', "id", 4, 10);

    BF_CloseFile(0);
    
    return 0;
}
