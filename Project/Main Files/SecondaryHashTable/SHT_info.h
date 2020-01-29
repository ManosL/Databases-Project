#ifndef __SHT_INFO__
#define __SHT_INFO__

typedef struct {
    int fileDesc;
    char* attrName;
    int attrLength;
    long int numBuckets;
    char* fileName;
}SHT_info;

#endif