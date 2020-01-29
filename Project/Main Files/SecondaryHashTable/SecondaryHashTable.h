#ifndef __SECONDARY_HT__
#define __SECONDARY_HT__

#include "../../BF/BF.h"
#include "SecondaryRecord.h"
#include "SecondaryHashTableBlock.h"

#include "../PrimaryHashTable/HashTableBlock.h"
#include "../PrimaryHashTable/HT_info.h"

#include "SHT_info.h"

int SHT_CreateSecondaryIndex(char*,char*,int,int,char*);
SHT_info* SHT_OpenSecondaryIndex(char*);
int SHT_CloseSecondaryIndex(SHT_info*);
int SHT_SecondaryInsertEntry(SHT_info,SecondaryRecord);
int SHT_SecondaryGetAllEntries(SHT_info,HT_info,void*);

#endif