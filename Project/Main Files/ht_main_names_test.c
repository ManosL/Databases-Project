#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "PrimaryHashTable/HT_info.h"
#include "PrimaryHashTable/PrimaryRecord.h"
#include "SecondaryHashTable/SecondaryHashTable.h"
#include "SecondaryHashTable/SecondaryRecord.h"

#include "Common/Statistics.h"

#include "../BF/BF.h"

#define FILENAME "Main\ Files/Input\ Files/Class\ Input/records15K.txt"

#define RECORDS 15000
#define DEL_IDS 0
#define HT_BUCKS_NUM 50

int main(void){
    FILE* records_file = fopen(FILENAME,"r");
    char curr_line[100];
    
    char names[RECORDS][15];
    char deleted_names[DEL_IDS][15];

    Record current = {};
    SecondaryRecord sht_current = {};

    srand(time(NULL));

    if(records_file == NULL){
        printf("Cannot open file\n");
        return -1;
    }

    BF_Init();

    HT_CreateIndex("primary.index",'c',"name",15,HT_BUCKS_NUM);
    HT_info*  my1 = HT_OpenIndex("primary.index");

    int i = 0;
    while(i < RECORDS){
        int res;
        char* token = NULL;

        /* Reads a line from file */
        fgets(curr_line,100,records_file);

        if(feof(records_file)){
            printf("Too small file. File should have at least %d records\n",RECORDS);
            return -1;
        }

        /* Tokenizes it to get the information
        about the record */

        token = strtok(curr_line,"\",");
        sscanf(token,"{%d",&current.id);

        token = strtok(NULL,"\",");
        sscanf(token,"%s",current.name);

        token = strtok(NULL,"\",");
        sscanf(token,"%s",current.surname);

        token = strtok(NULL,"\",");
        sscanf(token,"%s}",current.address);   

        /*Inserts it to hash table */
        if((res = HT_InsertEntry(*my1,current)) == -1){
            printf("HT_Insert Failed");
            return -1;
        }

        /* Remembering the names that are in the HT */
        strcpy(names[i],current.name);

        i++;
    }

    /* Searching for all entries at the HT */
    for(int j = 0;j < RECORDS;j++){
        int return_val;

        return_val = HT_GetAllEntries(*my1,(void*) names[j]);

        if(return_val == -1){
            printf("Failed at inserting some entry or some search failed.\n");
            return -1;
        }
    }

    /* Deleting DEL_IDS records from the HT */
    for(int i = 0;i < DEL_IDS;i++){
        int curr_num = rand() % RECORDS;

        int curr_name_exists = 0;

        do{
            curr_name_exists = 0;

            for(int j = 0;j < i;j++){
                if(!strcmp(names[curr_num],deleted_names[j])){
                    curr_name_exists = 1;

                    if(curr_num == RECORDS - 1){
                        curr_num = 0;
                    }
                    else
                        curr_num++;
                }
            }
        }while(curr_name_exists);

        strcpy(deleted_names[i],names[curr_num]);

        if(HT_DeleteEntry(*my1,(void*) deleted_names[i]) == -1){
            printf("Delete Failed\n");
            return -1;
        }
    }
    
    /* Searching for all IDs at the HT and we want to don't find 2000 */
    int not_found = 0;
    for(int i = 0;i < RECORDS;i++){
        int return_val;

        return_val = HT_GetAllEntries(*my1,(void*) names[i]);

        if(return_val == -1){
            not_found++;
        }
    }

    if(not_found != DEL_IDS){
        if(not_found > DEL_IDS){
            printf("I didn't find more records than I deleted\n");
            return -1;
        }

        if(not_found < DEL_IDS){
            printf("I didn't find less records than I deleted\n");
            return -1;
        }
    }

    /* Closing the files */
    if(HT_CloseIndex(my1) == -1){
        printf("Failed to close HT File\n");
        return -1;
    }
    
    
    /* Getting the statistics */
    HashStatistics("primary.index");
    
    fclose(records_file);

    remove("primary.index");
    
    return 0;
}