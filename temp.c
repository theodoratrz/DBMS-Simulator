#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HP.h"
#include "BF.h"

int main(void)
{
    /*
    Record rec;
    rec.id = 25;
    sprintf(rec.name, "Bob");
    sprintf(rec.surname, "Jones");
    sprintf(rec.address, "Avenue 36");

    void *data = GetRecordData(&rec);
    int id;
    void *temp = data;

    memcpy(&id, temp, sizeof(int));
    temp = (int*)temp + 1;

    char* name = malloc(15);
    memcpy(name, temp, 15);
    temp = (char*)temp + 15;

    char* surname = malloc(25);
    memcpy(surname, temp, 25);
    temp = (char*)temp + 25;

    char* address = malloc(50);
    memcpy(address, temp, 50);
    
    printf("id: %d\nname: %s\nsurname: %s\naddress: %s\n", id, name, surname, address);

    free(data);
    free(name);
    free(surname);
    free(address);
    */

    char *fileName = "file";
    BF_Init();

    HP_CreateFile(fileName, 'i', "id", 4);
    HP_info *info = HP_OpenFile(fileName);
    Record rec1, rec2;
    rec1.id = 33;
    strcpy (rec1.name, "noname");
    strcpy(rec1.surname, "nosurname");
    strcpy(rec1.address, "noaddress");
    HP_InsertEntry(*info, rec1);

    rec2.id = 66;
    strcpy (rec2.name, "aname");
    strcpy(rec2.surname, "asurname");
    strcpy(rec2.address, "aaddress");
    HP_InsertEntry(*info, rec2);

    HP_InsertEntry(*info, rec1);

    HP_DeleteEntry(*info, &(rec1.id));

    rec1.id = 51;
    strcpy (rec1.name, "name1");
    strcpy(rec1.surname, "surname1");
    strcpy(rec1.address, "address1");
    HP_InsertEntry(*info, rec1);

    if (HP_CloseFile(info) < 0) {return 1;}
    
    return 0;
}
