/*
 * File: HP.h
 * Pavlos Spanoudakis (sdi1800184)
 * Theodora Troizi (sdi1800197)
 */

#define HP_INFO_SIZE 12
#define RECORD_SIZE 94

typedef struct{
    int id;
    char name[15];
    char surname[25];
    char address[50];
} Record;

typedef struct {
    int fileDesc;       /* File Descriptor */
    char attrType;      /* Key type: 'c', 'i' */
    char *attrName;     /* Key field name */
    int attrLength;     /* Key length (in bytes?) */
} HP_info;


int HP_CreateFile(  const char *fileName,       /* File name */
                    const char attrType,        /* Key type: 'c', 'i' */
                    const char* attrName,       /* Key field name */
                    const int attrLength        /* Key length (in bytes?) */ );

HP_info* HP_OpenFile( const char *fileName );

int HP_CloseFile( HP_info* header_info );

int HP_InsertEntry( HP_info header_info, /* File Header information */
                    Record record        /* Record to be inserted */ );

int HP_DeleteEntry( HP_info header_info,    /* File Header information */
                    void *value             /* Key value of record to be deleted */ );

int HP_GetAllEntries( HP_info header_info,    /* File Header information */
                      void *value             /* Key value of record to be found */ );

void* HP_GetInfoData(HP_info* info);

int HP_InitFile(int fd, char type, const char *name, int length);

void HP_SetNextBlockNumber(void *current, int num);

int HP_GetNextBlockNumber(void *current);

void* HP_GetNextBlock(  int fd, 
                        void *current_block);

void* GetRecordData(const Record *rec);

HP_info* Get_HP_info(int fd);

void* Get_HP_info_Data(const HP_info *info);