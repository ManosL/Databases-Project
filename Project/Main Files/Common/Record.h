#ifndef __RECORD__
#define __RECORD__

typedef struct{
    int id;
    char name[15];
    char surname[20];
    char address[40];
}Record;

void print_record(const Record);

#endif