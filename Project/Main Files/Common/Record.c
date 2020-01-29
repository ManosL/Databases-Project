#include <stdio.h>
#include "Record.h"

void print_record(const Record rec){
    printf("ID: %d\n",rec.id);
    printf("Name: %s\n",rec.name);
    printf("Surname: %s\n",rec.surname);
    printf("Address: %s\n",rec.address);
}