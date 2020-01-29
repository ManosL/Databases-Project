#ifndef __SECONDARY_RECORD__
#define __SECONDARY_RECORD__

#include <string.h>

#include "../Common/Record.h"

typedef struct{
    Record record;
    int blockId;
}SecondaryRecord;

/* I keep that structure because to find an element
at the primary hash table I just need the ID of that
record because each ID is unique, the HT blockId to
find that at the primary HT and the sht_key to find 
if the value is equal to that record */

typedef struct{
    char sht_key[40];
    int record_id;
    int blockId;
}RealSecondaryRecord;

void copy_sht_record(RealSecondaryRecord*,const RealSecondaryRecord*);


#endif