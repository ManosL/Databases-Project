#ifndef __HT_BLOCK__
#define __HT_BLOCK__

#include "../Common/Record.h"

#define MAX_RECORDS ((512-sizeof(short)-sizeof(long))/sizeof(Record))

typedef struct{
    Record records[MAX_RECORDS];
    short remaining_records;
    long next_block;
}HT_Block;

#endif
