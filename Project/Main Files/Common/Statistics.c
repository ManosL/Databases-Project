#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "../../BF/BF.h"
#include "info_static.h"
#include "../PrimaryHashTable/HashTableBlock.h"
#include "../PrimaryHashTable/HT_info.h"
#include "../PrimaryHashTable/PrimaryRecord.h"

#include "../SecondaryHashTable/SecondaryHashTableBlock.h"
#include "../SecondaryHashTable/SecondaryHashTable.h"

int HashStatistics(char* filename){
    int fileDesc = BF_OpenFile(filename);
    Static_info *info;
    int TotalBuckets, nblock,nrecords,recbuck,block_counter=0;
    HT_Block *ht_block;
    SHT_Block *sht_block;
    int min = INT_MAX, max = 0;
    int *overflowBuckets;
    int totalBlocks;
    int counter = 0;


    if(BF_ReadBlock(fileDesc,0,(void**) &info) < 0){
            BF_PrintError(NULL);
            return -1;
        }
    char type = info->type;
    BF_CloseFile(fileDesc);
    if (type=='p'){
        printf("\n---Statistics for the primary hashtable---\n");
        HT_info *info_block= HT_OpenIndex(filename);
        TotalBuckets = info_block->numBuckets;
        fileDesc = info_block->fileDesc;
        nrecords = 0;
        overflowBuckets = malloc(TotalBuckets*sizeof(int));
        for(int i=1; i<=TotalBuckets;i++){
            nblock = i;
            recbuck=0;
            block_counter = 0;
            do{
                block_counter++;
               if(BF_ReadBlock(fileDesc,nblock,(void**) &ht_block) < 0){
                    BF_PrintError(NULL);
                    return -1;
                } 
                recbuck+=(MAX_RECORDS - ht_block->remaining_records);
                nblock = ht_block->next_block;
            }while(nblock!=-1);
            //write the overflow blocks for this bucket
            overflowBuckets[i-1] = block_counter-1;

            nrecords+=recbuck;
            if(min>recbuck){
                min = recbuck;
            }
            if(max<recbuck){
                max = recbuck;
            }
        }
        double average = (double)nrecords/(TotalBuckets);

        totalBlocks = BF_GetBlockCounter(fileDesc);
        printf("Total Blocks: %d\n",totalBlocks);
        printf("Records per bucket:\n Average: %lf\n Minimum: %d\n Maximum: %d\n",average,min,max);
        printf("Average blocks per bucket: %lf\n",(double)(totalBlocks-1)/(TotalBuckets));
        for (int i = 0; i < TotalBuckets;i++){
            if(overflowBuckets[i]>0){
                printf("Bucket %d has %d overflow blocks.\n",i,overflowBuckets[i]);
                counter++;
            }
        }
        printf("%d overflow buckets in total.\n",counter);
        HT_CloseIndex(info_block);
        free(overflowBuckets);
    }
    else if (type=='s'){
        printf("\n---Statistics for the secondary hashtable---\n");
        SHT_info *info_block= SHT_OpenSecondaryIndex(filename);
        TotalBuckets = info_block->numBuckets;
        fileDesc = info_block->fileDesc;
        nrecords = 0;
        overflowBuckets = malloc(TotalBuckets*sizeof(int));
        for(int i=1; i<=TotalBuckets;i++){
            nblock = i;
            recbuck=0;
            block_counter=0;
            do{
                block_counter++;
               if(BF_ReadBlock(fileDesc,nblock,(void**) &sht_block) < 0){
                    BF_PrintError(NULL);
                    return -1;
                } 
                recbuck+=(sht_block->max_records - sht_block->remaining_records);
                nblock = sht_block->next_block;
            }while(nblock!=-1);
            //write the overflow blocks for this bucket
            overflowBuckets[i-1] = block_counter-1;
            nrecords+=recbuck;
            if(min>recbuck){
                min = recbuck;
            }
            if(max<recbuck){
                max = recbuck;
            }
        }
        double average = (double)nrecords/(TotalBuckets);


        totalBlocks = BF_GetBlockCounter(fileDesc);
        printf("Total Blocks: %d\n",totalBlocks);
        printf("Records per bucket:\n Average: %lf\n Minimum: %d\n Maximum: %d\n",average,min,max);
        printf("Average blocks per bucket: %lf\n",(double)(totalBlocks-1)/(TotalBuckets));
        for (int i = 0; i < TotalBuckets;i++){
            if(overflowBuckets[i]>0){
                printf("Bucket %d has %d overflow blocks.\n",i,overflowBuckets[i]);
                counter++;
            }
        }
        printf("%d overflow buckets in total.\n",counter);
        SHT_CloseSecondaryIndex(info_block);
        free(overflowBuckets);
    }
    else{
        printf("Not a hash file!\n");
        return -1;
    }

    return 0;

}