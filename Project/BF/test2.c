#include <stdio.h>
#include <string.h>
#include "BF.h"

typedef struct{
    char name[15];
    int k;
}abc;

int main(void){
    int fileDesc;
    BF_Init();
    BF_CreateFile("manolakis");
    int i = 0;
    while(i < 5){
        fileDesc = BF_OpenFile("manolakis");
        printf("%d\n",fileDesc);
        i++;
    }

}