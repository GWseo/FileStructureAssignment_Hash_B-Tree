#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(){
    float a = 0xFFFFFFFF;
    printf("%f\n",a);
    if(a > 0.0){
        printf("1");
    }else{
        printf("2");
    }
    return 0;
}
