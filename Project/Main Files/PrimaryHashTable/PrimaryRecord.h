#ifndef PRIMARYRECORD_H
#define PRIMARYRECORD_H

#include "../Common/Record.h"
#include "HT_info.h"

int HT_CreateIndex(char*,
                   char, //attrType
                   char*, //attrName
                   int, //attrLength
                   int); //buckets);

HT_info* HT_OpenIndex(char*);
int HT_CloseIndex(HT_info*);
int HT_InsertEntry(HT_info, Record);
int HT_DeleteEntry(HT_info, void*);
int HT_GetAllEntries(HT_info, void*);

#endif
