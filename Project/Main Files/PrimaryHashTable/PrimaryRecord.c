
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include "HashTableBlock.h"
#include "../../BF/BF.h"
#include "HT_info.h"
#include "../Common/info_static.h"
#include "PrimaryRecord.h"
#include "../Common/Record.h"
#include "../Common/Hash.h"

int HT_CreateIndex(char* filename,
                   char attrType,
                   char* attrName,
                   int attrLength,
                   int buckets){
    char* error_message = NULL;
    int fileDesc;
    void* block;
    HT_Block bucket = {};
    Static_info h_info = {};
    int blockNum;
    //assert that the struct is smaller than the block size
    assert(sizeof(Static_info) < 512);
    if(BF_CreateFile(filename) < 0){
        BF_PrintError("Error Creating File");
        return -1;
    }


    if((fileDesc = BF_OpenFile(filename)) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    if(BF_AllocateBlock(fileDesc) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    blockNum = BF_GetBlockCounter(fileDesc) - 1;
    assert(blockNum==0);


    if(BF_ReadBlock(fileDesc,blockNum,(void**) &block) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    //copy the info to the HT_info struct
    h_info.type = 'p';
    h_info.fileDesc = fileDesc;
    h_info.attrLength = attrLength;
    h_info.attrType = attrType;
    h_info.numBuckets = buckets;
    strncpy(h_info.attrName,attrName,attrLength);

    memcpy(block,&h_info,sizeof(Static_info));


    if(BF_WriteBlock(fileDesc,blockNum) < 0){
        BF_PrintError(error_message);
        return -1;
    }

    bucket.remaining_records=MAX_RECORDS;
    bucket.next_block = -1;

    //create the buckets for the hash table and initialize their values
    for(int i = 0;i < buckets;i++){
        if(BF_AllocateBlock(fileDesc) < 0){
            BF_PrintError(NULL);
            return -1;
        }

        blockNum = BF_GetBlockCounter(fileDesc) - 1;
        assert(blockNum == (i+1));

        if(BF_ReadBlock(fileDesc,blockNum,&block) < 0){
            BF_PrintError(NULL);
            return -1;
        }
        memcpy(block,&bucket,sizeof(HT_Block));

        if(BF_WriteBlock(fileDesc,blockNum) < 0){
            BF_PrintError(NULL);
            return -1;
        }
    }
     if(BF_CloseFile(fileDesc) < 0){
        BF_PrintError(NULL);
        return -1;
    }


    return 0;
}

HT_info* HT_OpenIndex(char* fileName){
    HT_info* ht_info_struct;
    int fileDesc, blockNum = 0;
    void* block;
    HT_info * ht_inform;
    Static_info *read_ht;

    if((fileDesc = BF_OpenFile(fileName)) < 0){
        BF_PrintError("Can't open File");
        return NULL;
    }

    if(BF_ReadBlock(fileDesc,blockNum,&block) < 0){
        BF_PrintError("Can't read block");
        return NULL;
    }

    ht_inform = malloc(sizeof(HT_info));
    read_ht = (Static_info*)block;
    // copy the information from the 0 block 
    ht_inform->attrName = malloc(sizeof(char)*(read_ht->attrLength+1));
    ht_inform->attrLength = read_ht->attrLength;
    ht_inform->attrType = read_ht->attrType;
    ht_inform->fileDesc = read_ht->fileDesc;
    ht_inform->numBuckets = read_ht->numBuckets;
    strncpy(ht_inform->attrName,read_ht->attrName,read_ht->attrLength + 1);


    return ht_inform;
}

int HT_CloseIndex(HT_info* header_info){
    int fileDesc = header_info->fileDesc;

    if(BF_CloseFile(fileDesc) < 0){
        BF_PrintError(NULL);
        return -1;
    }

    free(header_info->attrName);
    free(header_info);
    return 0;
}

int HT_InsertEntry(HT_info header_info, Record record){
    //get the id
    int id = record.id;
    char* error_message = NULL;
    int hash_index = hash_int(id,header_info.numBuckets);
    HT_Block *read_block;
    HT_Block *temp_block;

    char* hash_func_key;
    char* hash_func_attr;

    if (header_info.attrType == 'i'){
        hash_index = hash_int(id,header_info.numBuckets);
    }
    else{
        if(strcmp(header_info.attrName,"name") == 0){
            hash_func_key = record.name;
        }

        if(strcmp(header_info.attrName,"surname") == 0){
            hash_func_key = record.surname;
        }

        if(strcmp(header_info.attrName,"address") == 0){
            hash_func_key = record.address;
        }
        hash_index = Universal_Hash_Function(hash_func_key,header_info.numBuckets);
    }
    //Read the bucket with the hash index
    if(BF_ReadBlock(header_info.fileDesc,hash_index + 1,(void**) &read_block) < 0){
        BF_PrintError(error_message);
        return -1;
    }


    
    if (read_block->remaining_records>0){ //that means first block is not full
        int pos =6- read_block->remaining_records;

        read_block->records[pos] = record;
        read_block->remaining_records = read_block->remaining_records - 1;
        //write the records 
        //write the block
        if(BF_WriteBlock(header_info.fileDesc,hash_index+1) < 0){
            BF_PrintError(NULL);
            return -1;
        }
        
        return hash_index+1;
    }
    else if (read_block->next_block!= -1){ //means that first block is full
        int block_num = read_block->next_block;
        if(BF_ReadBlock(header_info.fileDesc,block_num,(void**) &read_block) < 0){
            BF_PrintError(error_message);
            return -1;
            }
        while(read_block->remaining_records==0 && read_block->next_block != -1){

            block_num = read_block->next_block;
            if(BF_ReadBlock(header_info.fileDesc,block_num,(void**) &read_block) < 0){
            BF_PrintError(error_message);
            return -1;
            }
        }

        if (read_block->next_block == -1 && read_block->remaining_records==0){ //means we need to create a new block
            //allocate new block
            if(BF_AllocateBlock(header_info.fileDesc) < 0){
                BF_PrintError(error_message);
                return -1;
            }
            int next_block = BF_GetBlockCounter(header_info.fileDesc) - 1;
            read_block->next_block = next_block;
            //write the entry on the hash
            if(BF_WriteBlock(header_info.fileDesc,block_num) < 0){
            BF_PrintError(NULL);
            return -1;
            }
            //read the new block
            if(BF_ReadBlock(header_info.fileDesc,next_block,(void**) &read_block) < 0){
            BF_PrintError(error_message);
            return -1;
            }

            read_block ->remaining_records = 5;
            read_block->next_block = -1;
            //write record
            read_block->records[0] = record;

            //write block
            if(BF_WriteBlock(header_info.fileDesc,next_block) < 0){
            BF_PrintError(NULL);
            return -1;
            }
            return next_block;
        }
        else{
            //write block
            int pos = 6-read_block->remaining_records;
            read_block->remaining_records = read_block->remaining_records - 1;
            read_block ->records[pos] = record;
            //printf("Pika\n");
            if(BF_WriteBlock(header_info.fileDesc,block_num) < 0){
            BF_PrintError(NULL);
            return -1;
            }

            return block_num;
        }

    }
    else{//we should create a new block

        //allocate a new block
        if(BF_AllocateBlock(header_info.fileDesc) < 0){
                BF_PrintError(error_message);
                return -1;
            }

        int new_block = BF_GetBlockCounter(header_info.fileDesc) - 1;
        read_block ->next_block = new_block;

        if(BF_WriteBlock(header_info.fileDesc,hash_index+1) < 0){
            BF_PrintError(NULL);
            return -1;
            }

        //insert the data to the new block

        if(BF_ReadBlock(header_info.fileDesc,new_block,(void**) &read_block) < 0){
            BF_PrintError(error_message);
            return -1;
            }

        read_block ->next_block = -1;
        read_block->remaining_records=5;
        read_block ->records[0] = record;

        if(BF_WriteBlock(header_info.fileDesc,new_block) < 0){
            BF_PrintError(NULL);
            return -1;
            }

        return new_block;    




    }
    return -1;
}


int HT_GetAllEntries(HT_info header_info, void* value){
    int int_key;
    char* string_key;
    void *block;
    int found = 0;
    HT_Block *ht_block;
    int blocks_read = 0;
    int block_num;
    char hash_func_key[40];
    int type = 0;
    
    //Hash the key 
    //Set the type of the key in an integer for future use
    if (header_info.attrType == 'i'){
        int_key = *((int*)value);
        block_num = hash_int(int_key,header_info.numBuckets)+1;
        type = 0;
    }
    else{
        if(strcmp(header_info.attrName,"name") == 0){
            type = 1;
        }

        if(strcmp(header_info.attrName,"surname") == 0){
            type = 2;
        }

        if(strcmp(header_info.attrName,"address") == 0){
            type = 3;
        }
        string_key = (char*)value;
        block_num = Universal_Hash_Function((char*)value,header_info.numBuckets)+1;
    }
    

    do{
        //read the disc block
        if(BF_ReadBlock(header_info.fileDesc,block_num,(void**) &ht_block) < 0){
        BF_PrintError("Can't read block");
        return -1;
        }
        //set the block num to the next block
        block_num = ht_block->next_block;
        blocks_read++;
        for(int i =0; i <( MAX_RECORDS -ht_block->remaining_records);i++){
            //Compare the key to the record, according to type
            switch(type){
                case 0:
                    if(ht_block->records[i].id == int_key){
                        found = 1;
                    }
                    break;
                case 1:
                    if(strcmp(ht_block->records[i].name,string_key)==0){
                        found = 1;
                    }
                    break;
                case 2:
                    if(strcmp(ht_block->records[i].surname,string_key)==0){
                        found = 1;
                    }
                    break;
                case 3:
                    if(strcmp(ht_block->records[i].address,string_key)==0){
                        found = 1;
                    }
                    break;
            }
            if(found){
                printf("Record found: Id: %d Name: %s Surname: %s Adress: %s\n",ht_block->records[i].id,ht_block->records[i].name,ht_block->records[i].surname,ht_block->records[i].address);
                return blocks_read;
            }
        }

    }while(block_num!=-1);

    printf("No Entry Found\n");

    return -1;
    
}
    

int HT_DeleteEntry(HT_info header_info, void* value){
    int int_key;
    int_key = *((int*)value);
    HT_Block *ht_block;
    int blocks_read = 0;
    int found = 0;
    int type = 0;
    int block_num;
    int prev_block;
    int max_pos;
    char *string_key;

        //Hash the key 
    if (header_info.attrType == 'i'){
        int_key = *((int*)value);
        block_num = hash_int(int_key,header_info.numBuckets)+1;
        type = 0;
    }
    else{
        if(strcmp(header_info.attrName,"name") == 0){
            type = 1;
        }

        if(strcmp(header_info.attrName,"surname") == 0){
            type = 2;
        }

        if(strcmp(header_info.attrName,"address") == 0){
            type = 3;
        }
        string_key = (char*)value;

        block_num = Universal_Hash_Function((char*)value,header_info.numBuckets)+1;
    }
    

    do{
        if(BF_ReadBlock(header_info.fileDesc,block_num,(void**)&ht_block) < 0){
        BF_PrintError("Can't read block");
        return -1;
        }
        prev_block = block_num;
        block_num = ht_block->next_block;
        max_pos = MAX_RECORDS - ht_block->remaining_records;
        for(int i =0; i <max_pos;i++){
            switch(type){
                case 0:
                    if(ht_block->records[i].id == int_key){
                        found = 1;
                    }
                    break;
                case 1:
                    if(strcmp(ht_block->records[i].name,string_key)==0){
                        found = 1;
                    }
                    break;
                case 2:
                    if(strcmp(ht_block->records[i].surname,string_key)==0){
                        found = 1;
                    }
                    break;
                case 3:
                    if(strcmp(ht_block->records[i].address,string_key)==0){
                        found = 1;
                    }
                    break;
            }

            if(found){
                //if its not the last position of the current records of the bucket
                if(i != max_pos - 1){
                    ht_block->records[i].id = ht_block->records[max_pos-1].id;
                    strncpy(ht_block->records[i].name,ht_block->records[max_pos-1].name,15);
                    strncpy(ht_block->records[i].surname,ht_block->records[max_pos-1].surname,20);
                    strncpy(ht_block->records[i].address,ht_block->records[max_pos-1].address,40);
                }

                
                (ht_block->remaining_records)++;
                if(BF_WriteBlock(header_info.fileDesc,prev_block) < 0){
                    BF_PrintError(NULL);
                    return -1;
                }

                return 0;
            }
        }

    }while(block_num!=-1);

    printf("No Entry Found\n");

    return -1;
}
