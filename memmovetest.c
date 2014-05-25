#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
    int* a;
    a=(int*)malloc(40*sizeof(int));
    memset(a,0x00,sizeof(int)*40);

    a[10]=1; 
    a[11]=2; 
    a[12]=3; 
    a[13]=4; 
    a[14]=5; 
    a[15]=6; 
    a[16]=7; 
    a[17]=8; 
    a[18]=9; 
    a[19]=10; 
    a[20]=11;
    a[21]=12; 
    a[22]=13; 
    memmove(a,a+10,sizeof(int)*13);



}
