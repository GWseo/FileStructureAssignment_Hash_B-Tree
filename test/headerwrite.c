#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct{
    unsigned a;
    unsigned b;
    unsigned c;
    unsigned d;
}header;

typedef struct{
    unsigned q;
    unsigned p;
}body;
typedef struct{
    header h;
    body b[100];
}node;

int main(){
    FILE *f;
    node n[10] ={0,};
    f=fopen("input.in","wb+");
    n[0].h.a=1;
    n[0].h.b=2;
    n[0].h.d=1;
    n[1].h.a=1;
    n[3].h.a=1;
    n[4].h.a=1;
    n[4].h.b=1;
    n[4].h.d=1;
    n[6].h.a=1;

    fwrite(&n, sizeof(node)*10,1,f);

    fseek(f,0,SEEK_SET);
    fread(n,sizeof(node),1,f);
    printf("%u %u %u %u\n",n[0].h.a,n[0].h.b,n[0].h.c,n[0].h.d);
    close(f);
    return 0;
}
