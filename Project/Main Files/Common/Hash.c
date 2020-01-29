#include <string.h>

long Universal_Hash_Function(char* str,long numBuckets){
    int a = 31;
    long init_val = 0;
    long p = 1302397;

    int str_length = strlen(str);

    for(int i = 0;i < str_length;i++){
        init_val = ((init_val*a) + str[i])%p;
    }

    return init_val % numBuckets;
}

int hash_int(int num,int nbuckets){
    long p = 1302397;
    long a = 35759;
    long b = 128932;
    
    return ((a*num+b) % p) % nbuckets; 
}