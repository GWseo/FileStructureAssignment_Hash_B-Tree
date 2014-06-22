#include<stdio.h>
#include<stdlib.h>
#include<string.h>
void fff(char* b){
    free(b);
}
int main(int argc, char* argvp[] ){
    char* buffer = NULL;

    buffer = (char*)malloc(sizeof(char)*10);
    fff(buffer);
    free(buffer);
    return 0;
}
