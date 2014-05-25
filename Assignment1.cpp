/*
 *  FileStrucutre DBMS Assignment
 *  Student ID  : 200924472
 *  Name        : Gyuwon Seo
 *  Date        : 12/5/14
 *  Object      : make hashTable & dat File for input data
 *  repo        : https://github.com/GWseo/FileStructureAssignment_Hash_B-Tree/blob/master/Assignment1.cpp
 *  
 */

/*
 * Block Structure(102 element per block)
 * +----------------+-------+---+---+-----------------------+---+
 * | TotalNum(uint) |       | R | R |                       | R |
 * +----------------+       | E | E |                       | E |
 * | blockNumber    |       | C | C |                       | C |
 * +----------------|       | O | O |                       | O |
 * |    element     |       | R | R |                       | R |
 * |     hash       |       | D | D |        ..........     | D |
 * |    value       |       |   |   |                       | S |
 * |    (102#)      |       |   |   |                       |   |
 * |                |       |   |   |                       |   |
 * |                |       |   |   |                       |   |
 * |                |       |   |   |                       |   |
 * +----------------+-------+---+---+-----------------------+---+
 *
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <string.h>
#include <functional>
#include <stdlib.h>
#include <sys/time.h>



//#define DEBUG
//#define DEBUGHIGH
#define CHECKTIME

#define BLOCKSIZE 8192 

//#define BLOCK30

#ifdef BLOCK30
#define MAXBLOCKDATA 102 
#define NAMELEN 30
#define DEPTLEN 30
#else
#define MAXBLOCKDATA 170 
#define NAMELEN 20
#define DEPTLEN 10
#endif
#define MAXBUCKETDATA 1022 
#define MAXNODEDATA 1021
#define MAXLEAFDATA MAXNODEDATA
#define LEFTNODEMOVESIZE 511
#define RIGHTNODEMOVESIZE 510

typedef struct{
  char name[NAMELEN];
  unsigned ID;
  float score;
  char dept[DEPTLEN];
}Data;

typedef struct{
    unsigned elementCount;
    unsigned blockNumber;
}DataHeader;

typedef struct{
    unsigned elementCount;
    unsigned referenceCount;
    unsigned minIdx;
    unsigned maxIdx;
}BucketAttr;

typedef struct{
    unsigned hashValue;
    unsigned blockNumber;
}BucketData;

typedef struct{
    BucketAttr Attr;
    BucketData Data[MAXBUCKETDATA];
}Bucket;

typedef struct{
    Bucket* bucket;
    unsigned blockNumber;
}hashTableData;

typedef struct{
    unsigned blockNumber;
    float score;
}btreeData;

typedef struct{
    unsigned elementCount;
    unsigned depth;
    unsigned nodeNum;
    unsigned parentNum;
}btreeNodeHeader;

typedef struct{
    btreeNodeHeader header;
    btreeData Data[MAXNODEDATA+1];
}btreeNode;

typedef struct{
    unsigned elementCount;
    unsigned depth;
    unsigned leafNum;
    unsigned parentNum;
}btreeLeafHeader;

typedef struct {
    btreeLeafHeader header;
    btreeData Data[MAXLEAFDATA];
    unsigned nextLeafNum;
    float Dummy;
}btreeLeaf;

using namespace std;

FILE* hashFile;
FILE* idxFile;
FILE* datFile;

//hashTableData* hashTable;
Bucket** hashTable;
Bucket* currentBucket;
int hashPrefix=0;
unsigned currentBucketNum;
unsigned currentBucketCount;
//Btree
btreeNode *rootTree;
btreeNode *currentNode;
unsigned maxDepth;
unsigned currentNodeNum;
unsigned currentNodeCount;

unsigned char currentBlock[BLOCKSIZE];
unsigned currentBlockNum;
unsigned currentBlockCount;

unsigned RJhash( unsigned source)
{
   unsigned a=source;
   a = (a+0x7ed55d16) + (a<<12);
   a = (a^0xc761c23c) ^ (a>>19);
   a = (a+0x165667b1) + (a<<5);
   a = (a+0xd3a2646c) ^ (a<<9);
   a = (a+0xfd7046c5) + (a<<3);
   a = (a^0xb55a4f09) ^ (a>>16);
   return a;
}

// =========================You don't need to touch=========================
typedef struct {
    unsigned blockNumber;
    unsigned studentId;
} SaveInfo;

vector<SaveInfo> infoVector;

void insertBlockNumber(unsigned blockNumber, unsigned studentId){
            
            SaveInfo s;
            s.blockNumber = blockNumber;
            s.studentId = studentId;
            infoVector.push_back(s);
}
       
bool checkBlockNumber(unsigned blockNumber, unsigned studentId){
	bool isThere = false;
	for(int i = 0; i < infoVector.size(); i++){
			if(infoVector[i].studentId == studentId){
				  if(infoVector[i].blockNumber == blockNumber){
					  isThere = true;
				  }              
			}                
	}     
	
	 return isThere;
}
// ======================You don't need to touch (End)======================

// =========================Your code here down!!=========================
// Database file open 
int initDataBase();
bool openDB(const char* filename);
int checkInsert(unsigned blockNumber);
/*
 *  1. check need to split hash table
 *  2. splite hashtable
 *  3. increase hashprefix
 *  4. increase blockCounter
 *  5. splite block
 *  6. assign block num to hashtable
 *
 */
unsigned branchBlock(unsigned blockNumber, unsigned hashValue);
/*
 * look up empty block
 * total 180 elements and if hash idx == -1 empty slot 
 */
unsigned insertData(unsigned blockNumber, Data D,unsigned hashValue);
/*
 * B+Tree Functions
 */
unsigned insertNode(unsigned blockNumber, float score,unsigned currentD);
unsigned spliteNode(unsigned blockNumber, float score,unsigned currentD);
void nodeWrite(unsigned, void*);
void* nodeRead(unsigned);


/*
 * hash functions 
 */
unsigned insertBucket(unsigned blockNumber,unsigned hashValue);
void packingBucket(BucketData*);
unsigned branchBucket(unsigned hashIdx);
Bucket* swapBucket(unsigned from, unsigned to);
hashTableData* newBucket(Bucket* b);
unsigned getHashIndex(unsigned hashValue);
/*
 * fwrite when hashprefix change
 * maybe need to realloc or memcpy & malloc
 *
 * just reference block num;
 */

unsigned findBlockNumFromBucket(unsigned BucketIndex,unsigned hashValue);
unsigned getBlockNumFromHash(unsigned hashValue);
/*
 * generate hashvalue
 * translate value to index
 * get block number from hash table
 */
unsigned insertHash(char* name, unsigned ID, float score, char* dept);
/*
 * Insert record into myDB
 * 1. insert data current DataBlock
 * 2. insert at the hash 
 * 3. insert at B+tree
*/
unsigned insertRecord(char* name, unsigned ID, float score, char* dept);

void outputTest();


// Delete record 
bool deleteRecord(unsigned ID){
        // add code here
        
        return false;
}


// Search by ID
unsigned searchID(unsigned ID){
         unsigned blockNumber = 0;
         // add code here 
         unsigned hashValue = RJhash(ID);
         blockNumber= getBlockNumFromHash(hashValue);
          
         return blockNumber;
}


// Search by score
unsigned searchScore(float lower, float upper){
         unsigned numOfScore = 0;
         // add code here
         
         return numOfScore;
}
// =========================Your code here down!!(Encdcdd)=========================


// =========================You don't need to touch=========================         
// main
int main(){
#ifdef CHECKTIME
    struct timeval start_point, end_point;
    double operating_time;

    gettimeofday(&start_point, NULL);
#endif
    openDB("myDB");

    
    // Input data
  //  ifstream fin("Assignment1.inp", ios::in);
    ifstream fin("input.in",ios::in);
    // for ESPA
    ofstream fout("Assignment1.out", ios::out);
    
    while(!fin.eof()){
    
        string type = "";
        fin >> type;
        
                
        if(type == "i"){ // insertRecord
               string firstName = "";
               string lastName = "";
               string fullName = "";
               unsigned studentId = 0;
               float score = 0;
               string deptStr = "";

               fin >> firstName >> lastName >> studentId >> score >> deptStr;
               fullName = firstName + "  " + lastName;
               char name[30];
               strcpy(name, fullName.c_str());
               // for Visual Studio 2012
               // strcpy_s(name, fullName.c_str());
               char dept[30];
               strcpy(dept, deptStr.c_str());
               // for Visual Studio 2012
               // strcpy_s(dept, deptStr.c_str());
               
               //cout<<"type:" << type << "\t" <<"fiN:" <<name << "\t"<<"id:"  << studentId << "\t"<<"sc:"  << score << "\t"  <<"dep:"<< deptStr << endl;
               
               // insertRecord
               unsigned blockNumber = insertRecord(name, studentId, score, dept);
    //            unsigned blockNumber = 0;
               // for ESPA
               insertBlockNumber(blockNumber, studentId);
               
        } else if(type == "s"){  // searchID
               unsigned studentId = 0;
               
               fin >> studentId;
               //cout << type << "\t"  << studentId << endl;
               
               // searchID
               unsigned blockNumber = searchID(studentId);
               // for ESPA
               bool isThere = false;
               isThere = checkBlockNumber(blockNumber, studentId);
               if(isThere)
                   fout << "true" << endl;
               else
                   fout << "false" << endl;
                   
        }else if(type == "c"){
            float lower = 0;
            float upper = 0;
            fin >> lower >> upper;
            //cout << type << "\t" << lower << "\t" << upper <<endl;
            
            // searchScore
            unsigned result = searchScore(lower,upper);
            fout<<result<<endl;
        }
    }
#ifdef DEBUG   
  outputTest(); 
#endif
    fout.close();
    fin.close();
#ifdef CHECKTIME
    gettimeofday(&end_point, NULL); 

    operating_time = (double)(end_point.tv_sec)+(double)(end_point.tv_usec)/1000000.0-(double)(start_point.tv_sec)-(double)(start_point.tv_usec)/1000000.0;

    printf("%f\n",operating_time); 
#endif
    // for Dev C++
    //system("PAUSE");
    //return EXIT_SUCCESS;
    return 0;
}

// ======================You don't need to touch (End)======================

int initDataBase(){
    //TODO : make default hashTable, make root B+tree
    unsigned *blockPtr;
    Bucket* b_New;
    BucketAttr *b_NewAttr; 
    btreeNodeHeader tempHeader={0,0,0,-1};
#ifdef DEBUGHIGH
    printf("initDataBase -- \n");
#endif
   // hashTable = (hashTableData*)malloc(sizeof(hashTableData*));
    hashTable = (Bucket**)malloc(sizeof(Bucket*));
    memset(hashTable,0x00,sizeof(Bucket*));
   
    b_New = (Bucket*)malloc(sizeof(Bucket)); 
    memset(b_New,0x00,sizeof(Bucket));
    b_NewAttr = (BucketAttr*)b_New;
    b_NewAttr->referenceCount++;
   // hashTable[0].bucket=b_New; 
    hashTable[0]=b_New;

    currentBucket = (Bucket*)malloc(sizeof(Bucket*));
    currentBucketNum=0;
    currentBucketCount=1;

    blockPtr=(unsigned*)currentBlock;
    memset(blockPtr+2,0xFF,sizeof(unsigned)*MAXBLOCKDATA); 

    rootTree = (btreeNode*)malloc(sizeof(btreeNode));
    memset(rootTree,0x00,sizeof(btreeData)*MAXNODEDATA+1);
    memcpy(rootTree, &tempHeader, sizeof(btreeNodeHeader)); 
    currentNode = rootTree;

    currentBlockNum=0;
    currentBlockCount=1;
    return 0;
}
bool openDB(const char* filename){
        int namelen = 0;
        char* file;

#ifdef DEBUGHIGH
    printf("open DB-- : \n");
#endif
    namelen= strlen(filename);
    file = (char*)malloc(namelen+6);
    memset(file,0x00, namelen+6);
    memcpy(file,filename,namelen);


    memcpy(file+namelen,".dat",4);
    datFile = fopen(file,"wb+");
    memset(file+namelen,0x00,5);
    memcpy(file+namelen,".hash",5);
    hashFile = fopen(file,"wb+");
    memset(file+namelen,0x00,5);
    memcpy(file+namelen,".idx",4);
    idxFile = fopen(file,"wb+");
    
    initDataBase();
#ifdef DEBUG
        printf("sizeof Data %lu , sizeof hashData : %lu \n",sizeof(Data),sizeof(Bucket));
#endif
        return true;
}
unsigned insertData(Data D,unsigned hashValue){
    int insertPos;
    unsigned blockNumber=currentBlockNum;
    unsigned* currentBlockElementCount;
    int* blockReference = (int*)currentBlock+2;
    insertPos = checkInsert(blockNumber);
#ifdef DEBUGHIGH
    printf("insert Data --#%u, at %u: \n",currentBlockCount,insertPos);
#endif
    currentBlockElementCount = (unsigned*)currentBlock;
    (*currentBlockElementCount)++;
    *(blockReference+insertPos)=(int)hashValue;
    memcpy(currentBlock+BLOCKSIZE-(insertPos+1) * sizeof(Data), &D, sizeof(Data));
#ifdef DEBUG
    printf("name : %s, id:%u, sc : %f, dep:%s\n",D.name,D.ID,D.score,D.dept);
    printf("current Block elementCoutn : %u, current Position : %d\n",*currentBlockElementCount,insertPos);
#endif
    return currentBlockNum;
}

unsigned getHashIndex(unsigned hV){
    unsigned hashTableIdx= 0;
    unsigned hashValue = hV;
    int i = 0;
    for(i=0; i < hashPrefix; i++){
        hashTableIdx <<=1;
        hashTableIdx += hashValue & 0x1;
        hashValue >>=1;
    }
#ifdef DEBUG
    printf("hash Idx: %u, hash Value %u, hashPrefix : %d\n", hashTableIdx,hashValue,hashPrefix);
#endif
    return hashTableIdx;
}

int checkInsert(unsigned blockNumber){
    int numOfDataInBlock;
    unsigned idx =0;
    int currentBlockElementCount;
    int* blockPtr = (int*)currentBlock; 
    DataHeader *H;
#ifdef DEBUGHIGH
    printf("insert checkInsert -- :#%u \n",blockNumber);
#endif
    
    currentBlockElementCount = *(blockPtr);

    if(currentBlockElementCount >= MAXBLOCKDATA){
#ifdef DEBUG
        printf("----------------------------------------------not current block %u, %u\n",currentBlockNum,blockNumber);
#endif
        fseek(datFile,currentBlockNum*BLOCKSIZE,SEEK_SET);
        fwrite(currentBlock, sizeof(char),BLOCKSIZE,datFile);
        currentBlockNum=currentBlockCount;
        fseek(datFile,currentBlockNum * BLOCKSIZE ,SEEK_SET);
        memset(currentBlock,0x00,BLOCKSIZE);
        memset(blockPtr+2,0xFF,sizeof(unsigned)*MAXBLOCKDATA); 
        H=(DataHeader*)currentBlock;
        H->blockNumber = currentBlockNum;
        currentBlockCount++;
    }

    for(idx = 0;idx < MAXBLOCKDATA ; idx++) {
        if(blockPtr[idx+2]== -1)break;
    }
    return idx;
}
unsigned branchBlock(unsigned blockNumber, unsigned hashValue){
    return 0;
}
unsigned findBlockNumFromBucket(Bucket* B,unsigned hashValue){
    int i;
    unsigned hV = hashValue;
    BucketData *cBucket = B->Data;
#ifdef DEBUGHIGH
    printf("insert findBlockNumFromBucket -- : \n");
#endif
    for(i = 0 ; i < MAXBUCKETDATA ; i++){
        if (cBucket[i].hashValue == hV){
//            printf("find Block Num From Bucket ---  return\n");
            return cBucket[i].blockNumber;
        }
    }
    return -1;
}
unsigned getBlockNumFromHash(unsigned hashValue){
    Bucket* B;
    unsigned blockNumber;
    unsigned DynamicHashIdx = getHashIndex(hashValue);
#ifdef DEBUGHIGH
    printf("insert getBlockNumFromHash -- : \n");
#endif
    // read hash table from memory(RAM!!) not to try file I/O 
    // No need to do File I/O
    B = hashTable[DynamicHashIdx];
   // B = swapBucket(currentBucketNum,DynamicHashIdx);

    blockNumber = findBlockNumFromBucket(B,hashValue);
    if(blockNumber == 0xFFFFFFFF){
        printf("ERROR!! blocknumbr = -1!\n");
    }
    return blockNumber;
}
unsigned insertBucket(unsigned blockNumber,unsigned hashValue){
    unsigned hV = hashValue;
    unsigned hashIdx = getHashIndex(hV);
    Bucket* B = hashTable[hashIdx];
//    Bucket* B = swapBucket(currentBucketNum,hashIdx);
    BucketAttr* Attr = (BucketAttr*)B;
    BucketData* Data = (BucketData*)B+2;
    
#ifdef DEBUGHIGH
    printf("insert insertBucket -- :idx:%u hash:%u \n",hashIdx,hashValue);
#endif
#ifdef DEBUG
        printf("Debug 0   %p\n",Attr);
    printf("insertBuceket ---- Attr: %u , hashIdx:%u, %u, Data : %p\n",Attr->elementCount,hashIdx,hV,Data) ;
#endif
    if(Attr->elementCount >= MAXBUCKETDATA){
        branchBucket(hashIdx); 
        insertBucket(blockNumber,hashValue);
    }
    else{
        BucketData D = {hashValue,blockNumber};
        memcpy(Data + Attr->elementCount, &D, sizeof(BucketData));
        Attr->elementCount++;
    }
    return blockNumber;
}
/*
 *  1. hashprefix ++
 *  2. realloc hashtable( pow(2, hashprefix))
 *  3. update Bucket Attr
 *  3-1. 
 *  3-1. check bucket is duplicated
 *  5. make 1 Bucket 
 *  6. update min,max index
 */
unsigned branchBucket(unsigned hI){
    char isTableSizeChange;
    int i;
    int hashTableSize;

    unsigned BucketElementCount;
    unsigned hashIdx;
    unsigned tempHashIndex;
    unsigned maxhashIdx,minhashIdx;
    int referenceCount=0;
//    hashTableData* tempHashTable;
    Bucket** tempHashTable;
    Bucket* b_New;
    Bucket* b_Branch; 
    BucketAttr b_Attr, new_Attr;
    BucketData* b_Data, *new_Data;
     

#ifdef DEBUGHIGH
    printf("insert branchBucket -- : \n");
#endif
    
    isTableSizeChange=hashTableSize = 1<<hashPrefix; //need to chekc reference count of Bucket
    tempHashTable = hashTable;
    if(((BucketAttr*)hashTable[hI])->referenceCount == 1){
//    if(((BucketAttr*)swapBucket(currentBucketNum,hI))->referenceCount ==1){
        hashPrefix++; 
        hashTableSize = 1<<hashPrefix; //need to chekc reference count of Bucket
        //tempHashTable = (hashTableData*)malloc(sizeof(hashTableData*)*hashTableSize);
        tempHashTable = (Bucket**)malloc(sizeof(hashTableData*)*hashTableSize);
        for (i = 0 ; i < hashTableSize ; i ++){
            memcpy(tempHashTable+i, hashTable+i/2, sizeof(Bucket*));
            ((BucketAttr*)tempHashTable[i])->referenceCount=0;
           // ((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount=0;
        }
    
    }
    else{
        for (i = 0 ; i < hashTableSize ; i ++){
            ((BucketAttr*)tempHashTable[i])->referenceCount=0;
          // ((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount=0;
        }
      //  tempHashTable = (hashTableData*)malloc(sizeof(hashTableData*)*hashTableSize);
        tempHashTable = (Bucket**)malloc(sizeof(Bucket*)*hashTableSize);
        memcpy(tempHashTable,hashTable,sizeof(hashTableData*)*hashTableSize);
    }


    
#ifdef DEBUG
    printf("debug1\n");
#endif

    free(hashTable);
 
    b_New = (Bucket*)malloc(sizeof(Bucket)); 
    memset(b_New, 0x00, sizeof(Bucket));
   

    new_Attr = b_New->Attr;
    new_Data = b_New->Data;
    
#ifdef DEBUG
    printf("debug2\n");
#endif
    if(isTableSizeChange != hashTableSize) 
        hashIdx = hI<<1; 
    else {
        hashIdx = hI&0xFFFFFFFE;
    }
    b_Branch = tempHashTable[hashIdx]; 
//    b_Branch = swapBucket(currentBucketNum,hashIdx);
    b_Attr = b_Branch->Attr;
    b_Data = b_Branch->Data;

    BucketElementCount = b_Attr.elementCount; 
    maxhashIdx = hashIdx; 
    minhashIdx = hashIdx;
    for(i = 0 ; i < BucketElementCount; i++  ){
        tempHashIndex = getHashIndex((b_Data+i)->hashValue); 
        if(tempHashIndex != hashIdx){
#ifdef DEBUG
            printf("i : %d hash IDx:%u when prefiex : %d, new_Attr.ele: %u, b_Attr.ele : %u\n",i, tempHashIndex,hashPrefix,new_Attr.elementCount, b_Attr.elementCount);
#endif

            memcpy(new_Data+new_Attr.elementCount, b_Data+i, sizeof(BucketData));
            memset(b_Data+i,0x00,sizeof(BucketData)); 
            new_Attr.elementCount++;
            b_Attr.elementCount--;
        }
        if(tempHashIndex > maxhashIdx) maxhashIdx = tempHashIndex;
        if(tempHashIndex < minhashIdx) minhashIdx = tempHashIndex;
    }
    // packing up bucket
#ifdef DEBUG
    printf("debug3\n");
#endif
    packingBucket(b_Data);    

    // need to check duplicated
    for(i = hashIdx + 1 ; i < maxhashIdx+1 ; i++) {
        tempHashTable[i]=b_New;
        //tempHashTable[i]=*(newBucket(b_New));
    }
    /*******apply attribute change*********/
    new_Attr.minIdx=hashIdx+1;
    new_Attr.maxIdx=maxhashIdx;
    b_Attr.minIdx = minhashIdx;
    b_Attr.maxIdx = hashIdx;

    memcpy(&b_New->Attr ,&new_Attr,sizeof(BucketAttr));
    memcpy(&b_Branch->Attr,& b_Attr,sizeof(BucketAttr));

    for ( i =0 ; i< hashTableSize ; i++ ){
        ((BucketAttr*)tempHashTable[i])->referenceCount++;
       // ((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount++;
    }
    for (i = maxhashIdx+1; i < hashTableSize; i++){
        ((BucketAttr*)tempHashTable[i])->minIdx=i;
        i= ((BucketAttr*)tempHashTable[i])->maxIdx=i+((BucketAttr*)tempHashTable[i])->referenceCount-1;
       // ((BucketAttr*)swapBucket(currentBucketNum,i))->minIdx=i;
       // i= ((BucketAttr*)swapBucket(currentBucketNum,i))->maxIdx = i+((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount-1;
    }
   // hashTable = (hashTableData*)malloc(sizeof(hashTableData*)*hashTableSize);
    hashTable = (Bucket**)malloc(sizeof(Bucket*)*hashTableSize);
    memcpy(hashTable,tempHashTable,sizeof(hashTableData*)*hashTableSize);
    return currentBlockNum;
}
void packingBucket(BucketData* Data){
    int i=0,j=0,k=0;
    int movestartIndex=0,movesize=0, emptyIndex=0;
//  printf("Packing....\n"); 
    BucketData* D = Data;
    BucketData* tempD = Data;
#ifdef DEBUGHIGH
    printf("insert PackingBucket -- : \n");
#endif
    for(i = 0 ; i < MAXBUCKETDATA ; i++,D++){
        if(D->hashValue == 0 )
        {
            if(emptyIndex == 0 ) emptyIndex = i; 
            for (j = i+1; j<MAXBUCKETDATA; j++){
                if((tempD+j)->hashValue != 0){
                    movestartIndex = j;
                    movesize=0; 
                    for(k = j+1 ; k < MAXBUCKETDATA; k++){
#ifdef DEBUG
                        printf("i : %d j : %d k : %d hash : %x\n", i , j , k , (tempD+k)->hashValue);                    
#endif
                        movesize++;
                        if((tempD+k)->hashValue == 0)
                            break;
                    }
                    memcpy(tempD+emptyIndex,tempD+movestartIndex,sizeof(BucketData)*movesize);
                    memset(tempD+movestartIndex,0x00, sizeof(BucketData)*movesize);
                    break;
                }
            }
            i=j+movesize-1;
            D=tempD+i;
            emptyIndex+=movesize;
        }
    }
}
Bucket* swapBucket(unsigned from, unsigned to ){
    unsigned maxIdx, minIdx;
    BucketAttr b_Attr;

    b_Attr=*((BucketAttr*)currentBucket);
    maxIdx = b_Attr.maxIdx;
    minIdx = b_Attr.minIdx;
    if((to<=maxIdx) && (minIdx<=to) ){
        printf("currentBucket\n");
        return currentBucket;
    }
    else if(to < minIdx){
        printf("min bucket\n");
        fseek(hashFile,from*BLOCKSIZE,SEEK_SET);
        fwrite(currentBucket, sizeof(Bucket),1,hashFile);
        currentBucketNum=to;
        fseek(hashFile,currentBlockNum * BLOCKSIZE ,SEEK_SET);
        memset(currentBlock,0x00,BLOCKSIZE);
        fread(currentBucket,sizeof(Bucket),1,hashFile);

    }
    else{
        printf("new bucket\n");
        fseek(hashFile,from*BLOCKSIZE,SEEK_SET);
        fwrite(currentBucket, sizeof(Bucket),1,hashFile);
        currentBucketNum=to;
        currentBucket = (Bucket*)malloc(sizeof(Bucket));
        memset(currentBucket,0x00,sizeof(Bucket));

        currentBucketCount++;
    }
    return currentBucket;

}
hashTableData* newBucket(Bucket* B){
    hashTableData* hTD;
    
    hTD = (hashTableData*)malloc(sizeof(hashTableData));
    memset(hTD,0x00,sizeof(hashTableData));

    fseek(hashFile, currentBucketNum * BLOCKSIZE, SEEK_SET);
    fwrite(B,sizeof(Bucket),1,hashFile);
     
    currentBucketNum++;

    hTD->blockNumber = currentBucketNum;
    hTD->bucket=B;
    return hTD;

}
unsigned insertHash(char* name, unsigned ID, float score, char* dept){
        unsigned blockNumber = 0;
        unsigned hashValue = RJhash(ID);

#ifdef DEBUGHIGH
    printf("insert Hash -- :\t\t ID:%u \n",ID);
#endif
        Data D;
        D.ID = ID;
        D.score =score;
        strcpy(D.name,name);
        strcpy(D.dept,dept);

        blockNumber = insertData(D,hashValue);
        insertBucket(blockNumber,hashValue);
        blockNumber = getBlockNumFromHash(hashValue);


        return blockNumber;
}
unsigned insertRecord(char* name, unsigned ID, float score, char* dept){
         unsigned blockNumber = 0;
         // add code heres
        
#ifdef DEBUGHIGH
    printf("insert Record -- : \n");
    printf("name: %s, ID : %u, score : %f, dept : %s\n",name,ID, score,dept);
#endif
    //insert Hash
        if( (blockNumber = insertHash(name, ID, score, dept))==-1){
            printf("Error!!\n");
        }
#ifdef DEBUGHIGH
        printf("insert Record --- return b#: %u, currentBucket:%p \n",blockNumber,currentBucket);
#endif
//insert btreeNode

        currentNode = rootTree;
        insertNode(blockNumber,score,0);
         return blockNumber;
}
void outputTest(){
    int elementCount;
    int i;
    Data temp;
    int* t;
    t=(int*)currentBlock;
    elementCount = t[0];
    for (i = 0 ; i < elementCount; i++){
        memcpy(&temp,currentBlock+BLOCKSIZE-i*sizeof(Data),sizeof(Data));
       // printf("index : %d, name : %s, id:%u, sc : %f, dep:%s\n",i,temp.name,temp.ID,temp.score,temp.dept);
    }
}
//search from root
unsigned insertNode(unsigned blockNumber, float score,unsigned currentD){
    float sc = score;
    int i = 0;
    unsigned elementC;
    unsigned currentDepth, currentNodeN;
    
    btreeData* Data;
    elementC = currentNode->header.elementCount;
    currentDepth = currentNode->header.depth;
    Data=currentNode->Data;
    currentNodeN = currentNode->header.nodeNum;
    
    for ( i = 0 ; i < elementC; i++ ){
        if(Data->score >= score){
           Data++; 
        }else if(Data->score < score){
            if(currentDepth == maxDepth){
                btreeData tempD = {blockNumber,score};
                if(MAXNODEDATA <= elementC){
                    //branchNode
                    spliteNode(blockNumber,score,currentD);
                }
                if(currentNodeN != currentNode->header.nodeNum){
                    nodeWrite(currentNode->header.nodeNum, currentNode);
                    free(currentNode);
                    currentNode = (btreeNode*)nodeRead(Data->blockNumber);
                }
                Data=currentNode->Data+i;
                memmove(Data+1,Data,sizeof(btreeData)*(elementC-i));
                //insert data
                memcpy(Data,&tempD,sizeof(btreeData));
                currentNode->header.elementCount++;            
                break;
            }
            else{
                //swap currentNode
                if(currentNodeNum != currentNode->header.nodeNum){
                    nodeWrite(currentNode->header.nodeNum, currentNode);
                    free(currentNode);
                    currentNode = (btreeNode*)nodeRead(Data->blockNumber);
                    
                }
                insertNode(blockNumber,score,currentD++);
            }
        }else{
            printf("what the..?\n");
        }
    }
    //when need to move nextblock
    //
    
    //
}
//manage nodenum, nodecount, new node num, new node type, 
//if prev or next or parent == -1 => non 
//check depth 
//
unsigned spliteNode(unsigned blockNumber, float score,unsigned currentD){
    int i=0; 
    float cmpScore = currentNode->Data[LEFTNODEMOVESIZE].score;
    unsigned leftNodeNum, rightNodeNum, parentNodeNum, tempNextNodeNum;
    btreeLeaf *newLeftLeaf,*newRightLeaf;
    btreeNode *newParentNode,*newLeftNode,*newRightNode;
    btreeLeafHeader tempLeafHeader = {0,0,0,-1};
    btreeNodeHeader tempNodeHeader = {0,0,0,-1}; 
    btreeData tempNewNodeData, *tempCurrentNodeData, tempNextNodeData={0xFFFFFFFF,0};

    parentNodeNum = currentNode->header.parentNum;
    leftNodeNum = currentNode->header.nodeNum;     

    if(parentNodeNum == 0xFFFFFFFF){
       newParentNode = (btreeNode*)malloc(sizeof(btreeNode));

       tempNodeHeader.elementCount=1;
       tempNodeHeader.nodeNum = currentNodeCount+1;

       memcpy(newParentNode,&tempNodeHeader,sizeof(btreeNodeHeader));
        //insert middle of currentData;
       tempNewNodeData.score=((currentNode->Data)+LEFTNODEMOVESIZE-1)->score; 
       tempNewNodeData.blockNumber= currentNode->header.nodeNum;
        memcpy(newParentNode->Data,
                &tempNewNodeData,
                sizeof(btreeData));
        tempNextNodeData.blockNumber = tempNodeHeader.nodeNum+1;
        memcpy(newParentNode->Data,
                &tempNextNodeData,
                sizeof(btreeData));
    //TODO : update depth for all node
    
    }
    else{
        newParentNode = (btreeNode*)nodeRead(parentNodeNum);
        //check elementCount;
        currentNode = newParentNode;
        currentNodeNum = newParentNode->header.nodeNum;

        if(newParentNode->header.elementCount >= MAXNODEDATA){
            spliteNode(blockNumber, score, currentD-1); 
        }
        if(currentNodeNum != parentNodeNum){
            nodeWrite(currentNodeNum,currentNode);
            free(newParentNode);
            newParentNode = (btreeNode*)nodeRead(parentNodeNum);
            currentNode = newParentNode;
            currentNodeNum = newParentNode->header.nodeNum;
        } 
        tempCurrentNodeData = newParentNode->Data;
        for(i = 0 ; i < newParentNode->header.elementCount+1; i++ ){
            if(tempCurrentNodeData->score > cmpScore) break;
            tempCurrentNodeData++; 
        } 
        memmove(tempCurrentNodeData+1,tempCurrentNodeData,sizeof(btreeData)*(MAXNODEDATA-i+1) );       
        memcpy(newParentNode->Data+newParentNode->header.elementCount,
                currentNode+LEFTNODEMOVESIZE-1,
                sizeof(btreeData));
        tempCurrentNodeData->score=cmpScore;
        newParentNode->header.elementCount++;

    }

    nodeWrite(newParentNode->header.nodeNum , newParentNode);
    parentNodeNum = newParentNode->header.nodeNum;

//when leaf split
    if(currentD == maxDepth){
        //make new next(right) Node    
        if(parentNodeNum == 0xFFFFFFFF) 
           maxDepth++; 
        newLeftLeaf = (btreeLeaf*)nodeRead(leftNodeNum);
        newRightLeaf = (btreeLeaf*)malloc(sizeof(btreeLeaf));
        memset(newRightLeaf,0x00,sizeof(btreeLeaf));
        
        memcpy(newRightLeaf->Data,newLeftLeaf->Data+LEFTNODEMOVESIZE, sizeof(btreeData)*RIGHTNODEMOVESIZE);
        //get next leaf node number from parent
        newRightLeaf->nextLeafNum = ((newParentNode->Data)+i)->blockNumber;

        tempLeafHeader.elementCount = RIGHTNODEMOVESIZE;
        tempLeafHeader.leafNum =currentBlockCount+1;
        tempLeafHeader.depth = maxDepth;
        memcpy(&newRightLeaf->header,&tempLeafHeader,sizeof(btreeLeafHeader));

        //write to the file (leftleaf)
        nodeWrite(currentNodeCount+1,newRightLeaf);

        //modify leftleaf
        memset(newLeftLeaf->Data+LEFTNODEMOVESIZE,0x00,sizeof(btreeData)*RIGHTNODEMOVESIZE);
        newLeftLeaf->nextLeafNum =newRightLeaf->header.leafNum;
        newLeftLeaf->header.elementCount=LEFTNODEMOVESIZE;
        newLeftLeaf->header.depth=maxDepth; 
        nodeWrite(newLeftLeaf->header.leafNum,newLeftLeaf);
        free(newRightLeaf);
        free(newLeftLeaf);
    }
    else{   //when node splite

    }
    free(newParentNode);
}

void nodeWrite(unsigned nodeNumber ,void* Node ){
    if(nodeNumber > currentNodeCount)currentNodeCount++;

    fseek(idxFile,nodeNumber*BLOCKSIZE,SEEK_SET);
    fwrite(Node, sizeof(char),BLOCKSIZE,idxFile);
}
void* nodeRead(unsigned nodeNumber){
    void* node;
    fseek(idxFile,nodeNumber * BLOCKSIZE, SEEK_SET);
    node = malloc(sizeof(BLOCKSIZE));
    fread(node,sizeof(BLOCKSIZE),1,idxFile); 
    return node;
}
