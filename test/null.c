#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(){
    int* a=NULL;
    a= (int*)malloc(sizeof(int));
    free(a);
    printf("%u",a);
    return 0;
}
