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
#define DEL_IDS 1000 
#define HT_BUCKS_NUM 50
#define SHT_BUCKS_NUM 50

int main(void){
    FILE* records_file = fopen(FILENAME,"r");
    char curr_line[100];
    
    int ids[RECORDS];
    int deleted_ids[DEL_IDS];

    char names[RECORDS / 2][15];

    Record current = {};
    SecondaryRecord sht_current = {};

    srand(time(NULL));

    if(records_file == NULL){
        printf("Cannot open file\n");
        return -1;
    }

    BF_Init();

    HT_CreateIndex("primary.index",'i',"id",4,HT_BUCKS_NUM);
    HT_info*  my1 = HT_OpenIndex("primary.index");

    SHT_CreateSecondaryIndex("secondary.index","name",15,SHT_BUCKS_NUM,"primary.index");
    SHT_info* my = SHT_OpenSecondaryIndex("secondary.index");

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
        
        /* Passing the last RECORDS/2 records to secondary hash table 
        and only them because when the SHT created the HT was empty */
        if(i > (RECORDS / 2) - 1){
            sht_current.blockId = res;
            sht_current.record  = current;

            if(SHT_SecondaryInsertEntry(*my,sht_current) == -1){
                printf("SHT_Insert Failed\n");
                return -1;
            }

            /* Remembering the names that are in the SHT */
            strcpy(names[i - RECORDS / 2],sht_current.record.name);
        }

        /* Remembering the ids that are in the HT */
        ids[i] = current.id;
        i++;
    }

    /* Searching for all entries at the HT */
    for(int j = 0;j < RECORDS;j++){
        int return_val;

        return_val = HT_GetAllEntries(*my1,(void*) &ids[j]);

        if(return_val == -1){
            printf("Failed at inserting some entry or some search failed.\n");
            return -1;
        }
    }

    /* Searching for the names that are on the SHT */
    for(int i = 0;i < RECORDS / 2 ;i++){
        int res;
        res = SHT_SecondaryGetAllEntries(*my,*my1,(void*) names[i]);

        if(res == -1){
            printf("SHT_GetAllEntries Failed\n");
            return -1;
        }
    }

    /* Deleting DEL_IDS records from the HT */
    for(int i = 0;i < DEL_IDS;i++){
        int curr_num = rand() % RECORDS;

        int curr_num_exists = 0;

        do{
            curr_num_exists = 0;

            for(int j = 0;j < i;j++){
                if(ids[curr_num] == deleted_ids[j]){
                    curr_num_exists = 1;

                    if(curr_num == RECORDS - 1){
                        curr_num = 0;
                    }
                    else
                        curr_num++;
                }
            }
        }while(curr_num_exists);

        deleted_ids[i] = ids[curr_num];

        if(HT_DeleteEntry(*my1,(void*) &deleted_ids[i]) == -1){
            printf("Delete Failed\n");
            return -1;
        }
    }
    
    /* Searching for all IDs at the HT and we want to don't find DEL_IDS */
    int not_found = 0;
    for(int i = 0;i < RECORDS;i++){
        int return_val;

        return_val = HT_GetAllEntries(*my1,(void*) &ids[i]);

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
    
    if(SHT_CloseSecondaryIndex(my) == -1){
        printf("Failed to close SHT File\n");
        return -1;
    }
    

    /* Getting the statistics */
    HashStatistics("primary.index");
    HashStatistics("secondary.index");
    
    fclose(records_file);

    remove("primary.index");
    remove("secondary.index");
    
    return 0;
}
