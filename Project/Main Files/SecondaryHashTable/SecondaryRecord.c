#include <stdio.h>

#include "SecondaryRecord.h"

void copy_sht_record(RealSecondaryRecord* dest,const RealSecondaryRecord* src){
    dest->blockId = src->blockId;

    strncpy(dest->sht_key,src->sht_key,40);

    dest->record_id = src->record_id;
}
