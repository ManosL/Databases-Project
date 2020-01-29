#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"

typedef struct{
    int nums[1000];
    char* desc;
}yolo;

int main(void){
    yolo* str_yolo;
    yolo* str_yolo2;
    char* str;

    srand(0);

    BF_Init();

    BF_CreateFile("manolakis");

    int fileDesc = BF_OpenFile("manolakis");

    BF_AllocateBlock(fileDesc);

    BF_ReadBlock(fileDesc,0,(void**) &str_yolo);

    for(int i = 0;i < 1000;i++){
        str_yolo->nums[i] = rand();
    }
    str_yolo->desc = malloc(21);
    strcpy(str_yolo->desc,"Merry Christmas");

    BF_WriteBlock(fileDesc,0);

    BF_ReadBlock(fileDesc,0,(void**) &str_yolo2);

    for(int i = 0;i< 1000;i++){
        printf("%d\n",str_yolo2->nums[i]);
    }

    printf("%s\n",str_yolo2->desc);

    printf("%d\n",sizeof(short));
}