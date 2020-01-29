#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../BF/BF.h"
#include "SecondaryRecord.h"
#include "SecondaryHashTableBlock.h"
#include "SecondaryHashTable.h"

#include "../PrimaryHashTable/PrimaryRecord.h"
#include "../PrimaryHashTable/HashTableBlock.h"
#include "../PrimaryHashTable/HT_info.h"

#include "../Common/info_static.h"
#include "../Common/Hash.h"

#include "SHT_info.h"

int SHT_CreateSecondaryIndex(char* sfileName,char* attrName,int attrLength,int buckets,char* fileName){
    char* error_message = NULL;
    int fileDesc;

    void* temp;
    Static_info* infoBlock;
    SHT_Block* bucket;

    /* Checkin if attrName combined with attrLength is valid */
    if(strcmp(attrName,"name") && strcmp(attrName,"address") && strcmp(attrName,"surname")){
        printf("Pass valid attrName\n");
        return -1;
    }

    if(!strcmp(attrName,"name") && attrLength != 15){
        printf("Pass valid attrLength for name which is 15\n");
        return -1;
    }

    if(!strcmp(attrName,"address") && attrLength != 40){
        printf("Pass valid attrLength for address which is 40\n");
        return -1;
    }

    if(!strcmp(attrName,"surname") && attrLength != 20){
        printf("Pass valid attrLength for surname which is 20\n");
        return -1;
    }  

    int HT_fileDesc;

    /* Opening a primary hash table file and checking 
    if it is really a primary hash table file */

    if((HT_fileDesc = BF_OpenFile(fileName)) == -1){
        BF_PrintError(NULL);
        return -1;
    }

    if(strlen(fileName) + 1 > 150){
        printf("Give a smaller fileName\n");
        return -1;
    }

    infoBlock = malloc(sizeof(Static_info));

    if(BF_ReadBlock(HT_fileDesc,0,&temp) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    memcpy(infoBlock,temp,sizeof(Static_info));

    if(infoBlock->type != 'p'){
        printf("File with name %s is not a primary hash table file\n",fileName);
        return -1;
    }

    /* Checkin if has as a key the ID of the record */
    if(strcmp(infoBlock->attrName,"id") || infoBlock->attrType != 'i'){
        printf("Primary Hash Table does not have ID as key\n");
        return -1;
    }

    if(BF_CloseFile(HT_fileDesc) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    /* Creating the secondary hash table file */
    if(BF_CreateFile(sfileName) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    if((fileDesc = BF_OpenFile(sfileName)) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    /*Creating the first block that contains the information
    about secondary hash table file */

    if(BF_AllocateBlock(fileDesc) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    int blockNum = BF_GetBlockCounter(fileDesc) - 1;

    if(BF_ReadBlock(fileDesc,blockNum,&temp) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    memcpy(infoBlock,temp,sizeof(Static_info));

    infoBlock->attrLength = attrLength;
    strncpy(infoBlock->attrName,attrName,strlen(attrName) + 1);
    infoBlock->type = 's';
    infoBlock->attrLength = attrLength;
    infoBlock->numBuckets = buckets;
    infoBlock->fileDesc = fileDesc;

    strncpy(infoBlock->fileName,fileName,strlen(fileName) + 1);

    /* Writing the info back to file */
    memcpy(temp,infoBlock,sizeof(Static_info));

    if(BF_WriteBlock(fileDesc,blockNum) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    /* Creating the first block of each bucket so 
    I can easily find the first block of each bucket.
    Each kth bucket's first block has block num equal to k */

    for(int i = 0;i < buckets;i++){
        if(BF_AllocateBlock(fileDesc) < 0){
            BF_PrintError(NULL);
            return -1;
        }

        blockNum = BF_GetBlockCounter(fileDesc) - 1;

        if(blockNum == -2){
            BF_PrintError(NULL);
            return -1;
        }

        if(BF_ReadBlock(fileDesc,blockNum,&temp) < 0){
            BF_PrintError(NULL);
            return -1;
        }

        bucket = malloc(sizeof(SHT_Block));
        memcpy(bucket,temp,sizeof(SHT_Block));

        /*The newly allocated block will not point
        to other block and it's remaining records num
        will be the maximum records num */

        bucket->max_records = 10;
        bucket->remaining_records = 10;
        bucket->next_block = -1;

        memcpy(temp,bucket,sizeof(SHT_Block));
        free(bucket);

        if(BF_WriteBlock(fileDesc,blockNum) < 0){
            BF_PrintError(NULL);
            return -1;
        }
    }

    if(BF_CloseFile(fileDesc) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    /* Synchronizing the secondary hash table records
    with the primary hash table records with name fileName */

    SHT_info* sht_info = SHT_OpenSecondaryIndex(sfileName);

    int ht_fileDesc;    
    HT_Block* ht_block = malloc(sizeof(HT_Block));

    /* Opening hash table file */
    if((ht_fileDesc = BF_OpenFile(fileName)) == -1){
        BF_PrintError("Failed to open hash table file");
        return -1;
    }

    /* Getting how many block HT file has. */
    int ht_blocksNum = BF_GetBlockCounter(ht_fileDesc);

    if(sht_info == NULL){
        printf("SHT_OpenSecondaryIndex Failed\n");
        return -1;
    }

    if(ht_blocksNum == -1){
        BF_PrintError("Failed to read the number of blocks");
        return -1;
    }

    /* For each block of HT file that I read except
    the first one which only contains info about the
    HT file */

    for(int i = 1;i < ht_blocksNum;i++){
        if(BF_ReadBlock(ht_fileDesc,i,&temp) == -1){ /*Reading the block */
            BF_PrintError(NULL);
            return -1;
        }

        memcpy(ht_block,temp,sizeof(HT_Block));

        /* For each record that the current block contains */
        for(int j = 0;j < MAX_RECORDS - ht_block->remaining_records;j++){ 
            SecondaryRecord sht_record;

            sht_record.blockId = i;            /*Converting it to SecondaryRecord */
            sht_record.record.id = ht_block->records[j].id;

            strcpy(sht_record.record.name,ht_block->records[j].name);
            strcpy(sht_record.record.surname,ht_block->records[j].surname);
            strcpy(sht_record.record.address,ht_block->records[j].address);

            /*Inserting it to Secondary Hash Table */

            if(SHT_SecondaryInsertEntry(*sht_info,sht_record) == -1){
                printf("Failed to insert Secondary Record\n");
                return -1;
            }
        }
    }

    /* Closing the current SHT Index so there won't be any
    confusion with the file descriptors */
    
    if(SHT_CloseSecondaryIndex(sht_info) == -1){
        printf("SHT_CloseSecondaryIndex Failed\n");
        return -1;
    }

    /* Closing the HT file */

    if(BF_CloseFile(ht_fileDesc) == -1){
        BF_PrintError("Failed to close hash table file");
        return -1;
    }

    free(infoBlock);
    free(ht_block);

    return 0;
}

SHT_info* SHT_OpenSecondaryIndex(char* sfileName){
    SHT_info* sht_info_struct;
    
    int fileDesc, blockNum = 0;

    void* temp;
    Static_info* read_block;

    if((fileDesc = BF_OpenFile(sfileName)) < 0){
        BF_PrintError(NULL);
        return NULL;
    }
    
    if(BF_ReadBlock(fileDesc,0,&temp) == -1){
        BF_PrintError(NULL);
        return NULL;
    }

    read_block = malloc(sizeof(Static_info));

    memcpy(read_block,temp,sizeof(Static_info));

    /* Changing the file descriptor at the first block of the file 
    because it will probably change so I don't want to keep an old
    file descriptor */

    read_block->fileDesc = fileDesc;

    memcpy(temp,read_block,sizeof(Static_info));

    free(read_block);
    
    if(BF_WriteBlock(fileDesc,0) == -1){
        BF_PrintError(NULL);
        return NULL;
    }

    /* Reading again the first block */

    if(BF_ReadBlock(fileDesc,blockNum,&temp) < 0){
        BF_PrintError(NULL);
        return NULL;
    }

    read_block = malloc(sizeof(Static_info));

    memcpy(read_block,temp,sizeof(Static_info));

    sht_info_struct = malloc(sizeof(SHT_info));

    if(sht_info_struct == NULL){
        printf("Memory Error\n");
        return NULL;
    }

    /* and copying it's info into a SHT_info structure */

    sht_info_struct->fileDesc = read_block->fileDesc;
    sht_info_struct->attrLength = read_block->attrLength;

    sht_info_struct->numBuckets = read_block->numBuckets;

    sht_info_struct->attrName = malloc((strlen(read_block->attrName) + 1)*sizeof(char));
    strcpy(sht_info_struct->attrName,read_block->attrName);

    sht_info_struct->fileName = malloc((strlen(read_block->fileName) + 1) * sizeof(char));
    strcpy(sht_info_struct->fileName,read_block->fileName);
    
    free(read_block);

    /* Returning a pointer to the structure that I previously allocated */
    return sht_info_struct;
}

int SHT_CloseSecondaryIndex(SHT_info* header_info){
    int fileDesc = header_info->fileDesc;

    if(BF_CloseFile(fileDesc) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    /* De-allocate the sht_info structure that I allocated at
    OpenIndex function */

    free(header_info->attrName);
    free(header_info->fileName);

    free(header_info);

    return 0;
}

int SHT_SecondaryInsertEntry(SHT_info header_info,SecondaryRecord record){
    char hash_func_attr[8];
    char hash_func_key[40];
    long bucketsNum = header_info.numBuckets;
    long hash_table_index;

    RealSecondaryRecord real_record;

    void* temp_block = NULL;
    SHT_Block replace_block;

    HT_info* ht_info;
    
    strcpy(hash_func_attr,header_info.attrName);

    /* Getting the hash table key */
    if(strcmp(hash_func_attr,"name") == 0){
        strcpy(hash_func_key,record.record.name);

        if(strlen(hash_func_key) + 1 > header_info.attrLength){
            printf("Give smaller name\n");
            return -1;
        }
    }

    if(strcmp(hash_func_attr,"surname") == 0){
        strcpy(hash_func_key,record.record.surname);

        if(strlen(hash_func_key) + 1 > header_info.attrLength){
            printf("Give smaller surname\n");
            return -1;
        }
    }

    if(strcmp(hash_func_attr,"address") == 0){
        strcpy(hash_func_key,record.record.address);
    
        if(strlen(hash_func_key) + 1 > header_info.attrLength){
            printf("Give smaller address\n");
            return -1;
        }
    }

    /*Configuring real record(record that will writen on Secondary HT) */
    real_record.blockId = record.blockId;
    real_record.record_id = record.record.id;
    strncpy(real_record.sht_key,hash_func_key,strlen(hash_func_key) + 1);

    /* Getting the bucket that I should insert the new record */

    hash_table_index = Universal_Hash_Function(hash_func_key,bucketsNum);

    long prev_block_num = -1;
    long curr_block_num = hash_table_index + 1;

    /* This loop will run until I reach the end of the list */
    while(curr_block_num != -1){
        if(BF_ReadBlock(header_info.fileDesc,curr_block_num,&temp_block) < 0){
            BF_PrintError(NULL);
            return -1;
        }

        memcpy(&replace_block,(SHT_Block*) temp_block,sizeof(SHT_Block));

        /*If the current block has space for another record
        I will just insert that record to it and I will write
        the block to the disk */

        if(replace_block.remaining_records > 0){
            short pos = replace_block.max_records - replace_block.remaining_records;

            strncpy(replace_block.records[pos].sht_key,real_record.sht_key,40);
            replace_block.records[pos].record_id = real_record.record_id;
            replace_block.records[pos].blockId = real_record.blockId;

            (replace_block.remaining_records)--;

            memcpy(temp_block,(void*) &replace_block,sizeof(SHT_Block));

            if(BF_WriteBlock(header_info.fileDesc,curr_block_num) < 0){
                BF_PrintError(NULL);
                return -1;
            }

            return 0;
        }

        temp_block = NULL;
        prev_block_num = curr_block_num;
        curr_block_num = replace_block.next_block;
    }

    /* If I reached the end of the list and I didn't insert my
    record I would allocate a new block I will change the pointer
    to next block of the last block that belongs to the bucket
    that I insert the record */

    if(BF_AllocateBlock(header_info.fileDesc) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    long newBlockNum = BF_GetBlockCounter(header_info.fileDesc) - 1;

    if(newBlockNum == -2){
        BF_PrintError(NULL);
        return -1;
    }

    /* Changing the pointer of the last block of the bucket
    to point to the newly allocated block */

    if(BF_ReadBlock(header_info.fileDesc,prev_block_num,&temp_block) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    memcpy(&replace_block,(SHT_Block*) temp_block,sizeof(SHT_Block));

    replace_block.next_block = newBlockNum;

    memcpy(temp_block,(void*) &replace_block,sizeof(SHT_Block));

    if(BF_WriteBlock(header_info.fileDesc,prev_block_num) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    temp_block = NULL;

    /* Initializing the newly allocated block */

    if(BF_ReadBlock(header_info.fileDesc,newBlockNum,&temp_block) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    memcpy(&replace_block,(SHT_Block*) temp_block,sizeof(SHT_Block));

    strncpy(replace_block.records[0].sht_key,real_record.sht_key,40);
    replace_block.records[0].record_id = real_record.record_id;
    replace_block.records[0].blockId = real_record.blockId;

    replace_block.remaining_records = 9;

    replace_block.max_records = 10;

    replace_block.next_block = -1;

    memcpy(temp_block,(void*) &replace_block,sizeof(SHT_Block));

    if(BF_WriteBlock(header_info.fileDesc,newBlockNum) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    return 0;
}

/* The Hash Table header info I will add it later */

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht,HT_info header_info_ht,void* value){
    int blocks_read = 0;
    int printed_records = 0;
    int curr_block_num;

    /* Getting the number of the bucket that I should seek the records */
    int hash_table_index = Universal_Hash_Function((char*) value,header_info_sht.numBuckets);

    void* temp;
    SHT_Block*  curr_block_sht = malloc(sizeof(SHT_Block));
    HT_Block*   curr_block_ht = malloc(sizeof(HT_Block));

    if(strlen(value) + 1 > header_info_sht.attrLength){
        printf("Give a %d size string\n",header_info_sht.attrLength);
        return -1;
    }

    curr_block_num = hash_table_index + 1;

    /* Traversing the blocks of the proper bucket */

    while(curr_block_num != -1){
        blocks_read++;

        if(BF_ReadBlock(header_info_sht.fileDesc,curr_block_num,&temp) < 0){
            BF_PrintError(NULL);
            return -1;
        }

        memcpy(curr_block_sht,temp,sizeof(SHT_Block));

        /* For each record of the current block */
        for(int i = 0;i < curr_block_sht->max_records - curr_block_sht->remaining_records;i++){
            /* If the record's key is equal to value */

            if(strcmp(curr_block_sht->records[i].sht_key,value) == 0){

                long ht_blockID = curr_block_sht->records[i].blockId;

                if(BF_ReadBlock(header_info_ht.fileDesc,ht_blockID,&temp) < 0){
                    BF_PrintError(NULL);
                    return -1;
                }

                memcpy(curr_block_ht,temp,sizeof(HT_Block));

                short rem_recs_ht = MAX_RECORDS - curr_block_ht->remaining_records;

                /* Checking If that record exist at the hash table */
                for(int j = 0;j < rem_recs_ht;j++){
                    if(curr_block_ht->records[j].id == curr_block_sht->records[i].record_id){
                        printf("Entry found\n");
                        
                        /* If exists I just print it */
                        print_record(curr_block_ht->records[j]);

                        printf("\n");

                        printed_records++;

                        break;
                    }
                }
            }
        }

        curr_block_num = curr_block_sht->next_block;
    }

    printf("Printed %d records.\n",printed_records);
    printf("Read %d blocks at Secondary Hash Table.\n",blocks_read);

    free(curr_block_ht);
    free(curr_block_sht);
    
    /* If I didn't printed any record I just return -1 */
    if(printed_records == 0){
        return -1;
    }
    else 
        return blocks_read;
}