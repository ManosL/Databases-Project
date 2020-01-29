#ifndef __SHT_BLOCK__
#define __SHT_BLOCK__

#include "SecondaryRecord.h"

typedef struct{
    RealSecondaryRecord records[10];
    short remaining_records;
    short max_records;
    long  next_block;
}SHT_Block;

typedef struct{
    char hash_table_type;
    char fileName[40];
    char attrName[8];
    short attrLength;
    long numBuckets;
}SHT_InfoBlock;

#endif