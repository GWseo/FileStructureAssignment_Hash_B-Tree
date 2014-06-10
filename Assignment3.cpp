/*
 *  FileStrucutre DBMS Assignment
 *  Student ID  : 200924472
 *  Name        : Gyuwon Seo
 *  Date        : 12/6/9
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
//#define BTREEDEBUG

//#define CHECKTIME

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

unsigned InsertCount;


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
    unsigned BucketNumber;
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
    btreeData Data[MAXNODEDATA];
    btreeData dummy;
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

hashTableData* hashTable;
//Bucket** hashTable;
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
unsigned btreeSearch(float from,float to, unsigned till);


/*
 * hash functions 
 */
unsigned insertBucket(unsigned blockNumber,unsigned hashValue);
void packingBucket(BucketData*);
unsigned branchBucket(unsigned hashIdx);
Bucket* swapBucket(unsigned from, unsigned to);
unsigned newBucket(Bucket* b);
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
bool deleteRecord(unsigned ID);
bool deleteHashData(unsigned ID,BucketData* D);
float deleteBlockData(BucketData* D);
float deleteNodeData(float score);

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
         currentNode = (btreeNode*)malloc(BLOCKSIZE);
         memcpy(currentNode,rootTree,BLOCKSIZE);
         numOfScore = btreeSearch(lower,upper,0); 

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
    ifstream fin("sample3/1.inp", ios::in);
//    ifstream fin("Assignment3.inp",ios::in);
    // for ESPA
    ofstream fout("Assignment3.out", ios::out);
    
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
               InsertCount++;
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
        } else if(type == "d"){
               unsigned studentId = 0;
               
               fin >> studentId;
               // cout << type << "\t" << studentId << endl;
               
               // deleteRecord
               bool isThere = false;
               isThere = deleteRecord(studentId);
               
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
    hashTable = (hashTableData*)malloc(sizeof(hashTableData*));
//    hashTable = (Bucket**)malloc(sizeof(Bucket*));
    memset(hashTable,0x00,sizeof(Bucket*));
   
    b_New = (Bucket*)malloc(sizeof(Bucket)); 
    memset(b_New,0x00,sizeof(Bucket));
    b_NewAttr = (BucketAttr*)b_New;
    b_NewAttr->referenceCount++;
    hashTable[0].BucketNumber=0; 
   // hashTable[0]=b_New;

    currentBucket = (Bucket*)b_New;
    currentBucketNum=0;
    currentBucketCount=1;

    blockPtr=(unsigned*)currentBlock;
    memset(blockPtr+2,0xFF,sizeof(unsigned)*MAXBLOCKDATA); 

    rootTree = (btreeNode*)malloc(BLOCKSIZE);
    memset(rootTree,0x00,BLOCKSIZE);
    memset(rootTree->Data, 0xFF,MAXNODEDATA*sizeof(btreeData));

    memcpy(rootTree, &tempHeader, sizeof(btreeNodeHeader)); 
    currentNode = (btreeNode*)malloc(BLOCKSIZE);
    memset(currentNode,0x00,BLOCKSIZE);
    memcpy(currentNode, rootTree,BLOCKSIZE);
    currentNodeNum = rootTree->header.nodeNum;

    currentBlockNum=0;
    currentBlockCount=1;

//need to restore saved file

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
    
    free(file);
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
//    B = hashTable[DynamicHashIdx];
    B = swapBucket(currentBucketNum,DynamicHashIdx);

    blockNumber = findBlockNumFromBucket(B,hashValue);
    if(blockNumber == 0xFFFFFFFF){
       // printf("ERROR!! blocknumbr = -1!\n");
    }
    return blockNumber;
}
unsigned insertBucket(unsigned blockNumber,unsigned hashValue){
    unsigned hV = hashValue;
    unsigned hashIdx = getHashIndex(hV);
//    Bucket* B = hashTable[hashIdx];
    Bucket* B = swapBucket(currentBucketNum,hashIdx);
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
    hashTableData* tempHashTable;
//    Bucket** tempHashTable;
    Bucket* b_New;
    Bucket* b_Branch; 
    BucketAttr b_Attr, new_Attr;
    BucketData* b_Data, *new_Data;
     

#ifdef DEBUGHIGH
    printf("insert branchBucket -- : \n");
#endif
    /*
     * copy exist hash table and extend table if needed
     */ 
    isTableSizeChange=hashTableSize = 1<<hashPrefix; //need to chekc reference count of Bucket
    tempHashTable = hashTable;
//    if(((BucketAttr*)hashTable[hI])->referenceCount == 1){
    if(((BucketAttr*)swapBucket(currentBucketNum,hI))->referenceCount ==1){
        for (i = 0 ; i < hashTableSize ; i ++){
//            ((BucketAttr*)tempHashTable[i])->referenceCount=0;
           ((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount=0;
        }
        hashPrefix++; 
        hashTableSize = 1<<hashPrefix; //need to check reference count of Bucket
        tempHashTable = (hashTableData*)malloc(sizeof(hashTableData*)*hashTableSize);
        memset(tempHashTable,0x00,sizeof(hashTableData)*hashTableSize);
//        tempHashTable = (Bucket**)malloc(sizeof(hashTableData*)*hashTableSize);
        for (i = 0 ; i < hashTableSize ; i ++){
            memcpy(tempHashTable+i, hashTable+i/2, sizeof(hashTableData));
//            ((BucketAttr*)tempHashTable[i])->referenceCount=0;
        //    ((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount=0;
        }
    
    }
    else{
        for (i = 0 ; i < hashTableSize ; i ++){
//            ((BucketAttr*)tempHashTable[i])->referenceCount=0;
           ((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount=0;
        }
        tempHashTable = (hashTableData*)malloc(sizeof(hashTableData*)*hashTableSize);
        memset(tempHashTable,0x00,sizeof(hashTableData)*hashTableSize);
//        tempHashTable = (Bucket**)malloc(sizeof(Bucket*)*hashTableSize);
        memcpy(tempHashTable,hashTable,sizeof(hashTableData*)*hashTableSize);
    }


   // okay 2014-6-9 13:14; 
#ifdef DEBUG
    printf("debug1\n");
#endif
    /*
     * make new bucket and copy from exist old bucket
     *
     */
    if(isTableSizeChange != hashTableSize) 
        hashIdx = hI<<1; 
    else {
        hashIdx = hI&0xFFFFFFFE;
    }
    if (isTableSizeChange != hashTableSize)
        b_Branch = swapBucket(currentBucketNum,hashIdx/2);
    else
        b_Branch = swapBucket(currentBucketNum,hashIdx);

    b_Attr = b_Branch->Attr;
    b_Data = b_Branch->Data;



    free(hashTable);
 
    b_New = (Bucket*)malloc(sizeof(Bucket)); 
    memset(b_New, 0x00, sizeof(Bucket));
   

    new_Attr = b_New->Attr;
    new_Data = b_New->Data;
    
#ifdef DEBUG
    printf("debug2\n");
#endif 
    
//    b_Branch = tempHashTable[hashIdx]; 
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

    

    /*******apply attribute change*********/
    new_Attr.minIdx=hashIdx+1;
    new_Attr.maxIdx=maxhashIdx;
    b_Attr.minIdx = minhashIdx;
    b_Attr.maxIdx = hashIdx;

    memcpy(&b_New->Attr ,&new_Attr,sizeof(BucketAttr));
    memcpy(&b_Branch->Attr,& b_Attr,sizeof(BucketAttr));

    newBucket(b_New);
    
    // need to check duplicated
    for(i = hashIdx + 1 ; i < maxhashIdx+1 ; i++) {
//        tempHashTable[i]=b_New;
//        tempHashTable[i].BucketNumber=swapBucket(b_New)->Attr.;
        tempHashTable[i].BucketNumber = currentBucketCount-1;
    }
    hashTable = (hashTableData*)malloc(sizeof(hashTableData*)*hashTableSize);
//    hashTable = (Bucket**)malloc(sizeof(Bucket*)*hashTableSize);
    memcpy(hashTable,tempHashTable,sizeof(hashTableData*)*hashTableSize);

    for ( i =0 ; i< hashTableSize ; i++ ){
//        ((BucketAttr*)tempHashTable[i])->referenceCount++;
        ((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount++;
    }
    for (i = maxhashIdx+1; i < hashTableSize; i++){
//        ((BucketAttr*)tempHashTable[i])->minIdx=i;
//        i= ((BucketAttr*)tempHashTable[i])->maxIdx=i+((BucketAttr*)tempHashTable[i])->referenceCount-1;
       // ((BucketAttr*)swapBucket(currentBucketNum,i))->minIdx=i;
       // i= ((BucketAttr*)swapBucket(currentBucketNum,i))->maxIdx = i+((BucketAttr*)swapBucket(currentBucketNum,i))->referenceCount-1;
        swapBucket(currentBucketNum,i);
        currentBucket->Attr.minIdx=i;
        i= currentBucket->Attr.maxIdx = i + (currentBucket)->Attr.referenceCount-1;
        
    }
    swapBucket(currentBucketNum,currentBucketNum);
    free(tempHashTable);
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
                    for(k = j+1 ; k < MAXBUCKETDATA+1; k++){
#ifdef DEBUG
                        printf("i : %d j : %d k : %d hash : %x\n", i , j , k , (tempD+k)->hashValue);                    
#endif
                        movesize++;
                        if((tempD+k)->hashValue == 0)
                            break;
                    }
                    memcpy(tempD+emptyIndex,tempD+movestartIndex,sizeof(BucketData)*movesize);
                    if(emptyIndex+movesize > movestartIndex)
                        memset(tempD+emptyIndex + movesize ,0x00, sizeof(BucketData)*(movestartIndex-emptyIndex));
                    else{
                        memset(tempD+movestartIndex,0x00,sizeof(BucketData)*movesize);
                    }
                   // memmove(tempD+emptyIndex,tempD+movestartIndex,sizeof(BucketData)*movesize);
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
    unsigned to2hashTable=0;
    
    b_Attr=*((BucketAttr*)currentBucket);
    maxIdx = b_Attr.maxIdx;
    minIdx = b_Attr.minIdx;
    
    to2hashTable = hashTable[to].BucketNumber;
    if(to2hashTable == currentBucketNum){
        fseek(hashFile,from*BLOCKSIZE,SEEK_SET);
        fwrite(currentBucket, sizeof(Bucket),1,hashFile);

    }
/*
    if((to<=maxIdx) && (minIdx<=to) ){
        fseek(hashFile,from*BLOCKSIZE,SEEK_SET);
        fwrite(currentBucket, sizeof(Bucket),1,hashFile);
 //       printf("currentBucket\n");
        return currentBucket;
    }
    */
  //  else if(to2hashTable < currentBucketCount){
  
    else if(to2hashTable < currentBucketCount+1){
    //    printf("exist bucket\n");
        fseek(hashFile,from*BLOCKSIZE,SEEK_SET);
        fwrite(currentBucket, sizeof(Bucket),1,hashFile);
        currentBucketNum=to2hashTable;
        fseek(hashFile, to2hashTable* BLOCKSIZE ,SEEK_SET);
        memset(currentBucket,0x00,BLOCKSIZE);
        fread(currentBucket,sizeof(Bucket),1,hashFile);

    }
    else{
        //printf("new bucket\n");
        /*
        fseek(hashFile,from*BLOCKSIZE,SEEK_SET);
        fwrite(currentBucket, sizeof(Bucket),1,hashFile);
        currentBucketNum=to;
        free(currentBucket);
        currentBucket = NULL;
        currentBucket = (Bucket*)malloc(sizeof(Bucket));
        memset(currentBucket,0x00,sizeof(Bucket));

        currentBucketCount++;
        */
    }
    return currentBucket;

}
unsigned newBucket(Bucket* B){
//    unsigned bucketIdx = hashTable[B->Attr.minIdx].BucketNumber; 
//    fseek(hashFile, bucketIdx * BLOCKSIZE, SEEK_SET);
    fseek(hashFile, currentBucketCount * BLOCKSIZE, SEEK_SET);
    fwrite(B,sizeof(Bucket),1,hashFile);
    currentBucketCount++;
    return currentBucketNum;
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
           // printf("Error!!\n");
        }
#ifdef DEBUGHIGH
        printf("insert Record --- return b#: %u, currentBucket:%p \n",blockNumber,currentBucket);
#endif
//insert btreeNode

        memset(currentNode,0x00,BLOCKSIZE);
        free(currentNode);
        currentNode = (btreeNode*)malloc(BLOCKSIZE);
        memcpy(currentNode,rootTree,BLOCKSIZE);
        currentNodeNum = rootTree->header.nodeNum;
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
    unsigned currentDepth, currentNodeN,parentNum;
    btreeData* Data,tempData;
    elementC = currentNode->header.elementCount;
    currentDepth = currentNode->header.depth;
    Data=currentNode->Data;
    currentNodeN = currentNode->header.nodeNum;
    parentNum = currentNode->header.parentNum;
    for ( i = 0 ; i < elementC+1; i++ ){
        if( Data->score <= sc ){
           Data++; 
        }else{
            tempData = *Data;
            if(currentDepth == maxDepth){
                btreeData tempD = {blockNumber,sc};
                if(MAXNODEDATA <= elementC){
                    //branchNode
                    spliteNode(blockNumber,sc,currentD);
                    if(currentNode  != NULL){
                        nodeWrite(currentNode->header.nodeNum, currentNode);
                        memset(currentNode,0x00,BLOCKSIZE);
                        free(currentNode);
                    }
                    currentNode = (btreeNode*)nodeRead(rootTree->header.nodeNum);
                    currentNodeNum = rootTree->header.nodeNum;
                    //restart insert
                    insertNode(blockNumber,sc,0); 
                    return 0; 
                    
                }
                if( currentNodeN != currentNode->header.nodeNum){
                    nodeWrite(currentNode->header.nodeNum, currentNode);
                    if(currentNode != NULL){
                        memset(currentNode,0x00,BLOCKSIZE);
                        free(currentNode);
                    }
                    currentNode=NULL;
                    currentNode = (btreeNode*)nodeRead(tempData.blockNumber);
                    currentNodeNum = (currentNode)->header.nodeNum;
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
                nodeWrite(currentNode->header.nodeNum, currentNode);
                if(currentNode != NULL){
                    memset(currentNode,0x00,BLOCKSIZE);
                    free(currentNode);
                }
                currentNode=NULL;
                currentNode = (btreeNode*)nodeRead(tempData.blockNumber);
                currentNodeNum = (currentNode)->header.nodeNum;
                
                insertNode(blockNumber,sc,currentD+1);
                break;
            }
        }
    }
    //when need to move nextblock
    if(i >= elementC+1){
        if(currentDepth == maxDepth){
            unsigned nextNode = ((btreeLeaf*)currentNode)->nextLeafNum;
            nodeWrite(currentNodeNum,currentNode);
            if(currentNode != NULL){
                memset(currentNode,0x00,BLOCKSIZE);
                free(currentNode);
            }
            currentNode = (btreeNode*)nodeRead(nextNode);
            currentNodeNum = nextNode;
        }else{
            
        }
        insertNode(blockNumber,sc,currentD);
    } 
    //
    nodeWrite(currentNode->header.nodeNum,currentNode);
    memset(currentNode,0x00,BLOCKSIZE);
    free(currentNode);
    currentNode=NULL;
    currentNode = (btreeNode*)nodeRead(currentNodeN);
    if(currentNode->header.nodeNum == rootTree->header.nodeNum)
        memcpy(rootTree,currentNode,BLOCKSIZE);
}
//manage nodenum, nodecount, new node num, new node type, 
//if prev or next or parent == -1 => non 
//check depth 
//
unsigned spliteNode(unsigned blockNumber, float score,unsigned currentD){
    int i=0,isRoot=0; 
    float cmpScore = currentNode->Data[LEFTNODEMOVESIZE-1].score;
    unsigned leftNodeNum, rightNodeNum, parentNodeNum, tempNextNodeNum,parentElementCount;
    btreeLeaf *newLeftLeaf,*newRightLeaf;
    btreeNode *newParentNode,*newLeftNode,*newRightNode;
    btreeLeafHeader tempLeafHeader = {0,0,0,-1};
    btreeNodeHeader tempNodeHeader = {0,0,0,-1}; 
    btreeData tempNewNodeData, *tempCurrentNodeData,tempNodeData, tempNextNodeData={0xFFFFFFFF,0};
    parentNodeNum = currentNode->header.parentNum;
    leftNodeNum = currentNode->header.nodeNum;     
    parentElementCount = currentNode->header.elementCount;
    if(parentNodeNum == 0xFFFFFFFF) isRoot =1;
    memcpy(&tempNodeData,(currentNode->Data)+LEFTNODEMOVESIZE-1,sizeof(btreeData));
    nodeWrite(currentNode->header.nodeNum,currentNode);
#ifdef BTREEDEBUG

    printf("blockNumber : %u, score: %f, currentD: %u, currentNode: %u\n",blockNumber,score,currentD, currentNode->header.nodeNum);
#endif
    if(currentNode != NULL){
        memset(currentNode,0x00,BLOCKSIZE);
        free(currentNode);
    }
    currentNode=NULL;
    if(isRoot){
       newParentNode = (btreeNode*)malloc(sizeof(btreeNode));
        memset(newParentNode->Data,0xFF,MAXNODEDATA*sizeof(btreeData));

       tempNodeHeader.elementCount=1;
       parentNodeNum = tempNodeHeader.nodeNum = currentNodeCount+1;

       memcpy(newParentNode,&tempNodeHeader,sizeof(btreeNodeHeader));
        //insert middle of currentData;
       tempNewNodeData.score=cmpScore; 
       tempNewNodeData.blockNumber= leftNodeNum;
        memcpy(newParentNode->Data,
                &tempNewNodeData,
                sizeof(btreeData));
        tempNextNodeData.score = 0xFFFFFFFF;
        tempNextNodeData.blockNumber = tempNodeHeader.nodeNum+1;
        memcpy(newParentNode->Data+1,
                &tempNextNodeData,
                sizeof(btreeData));
        tempNextNodeData.score = 0xFFFFFFFF;
        tempNextNodeData.blockNumber = 0xFFFFFFFF;
        memcpy(newParentNode->Data+2,
                &tempNextNodeData,
                sizeof(btreeData));
       tempNextNodeNum = 0xFFFFFFFF; 
       //rootTree=newParentNode;
       rootTree = (btreeNode*)malloc(BLOCKSIZE);
       memcpy(rootTree,newParentNode,BLOCKSIZE);

    //TODO : update depth for all node
    }
    else{
        newParentNode = (btreeNode*)nodeRead(parentNodeNum);
        //check elementCount;
        currentNode = newParentNode;
        parentNodeNum = currentNodeNum = newParentNode->header.nodeNum;

        if(newParentNode->header.elementCount >= MAXNODEDATA-1){
            spliteNode(blockNumber, score, currentD-1); 
        }
        if(currentNodeNum != parentNodeNum){
            nodeWrite(currentNodeNum,currentNode);
            if(currentNode != NULL){
                memset(currentNode,0x00,BLOCKSIZE);
                free(currentNode);
            }
            currentNode=NULL;
            newParentNode = (btreeNode*)nodeRead(parentNodeNum);
            currentNode = newParentNode;
            parentNodeNum = currentNodeNum = newParentNode->header.nodeNum;
            currentNodeNum = parentNodeNum;
        } 
        tempCurrentNodeData = newParentNode->Data;
        for(i = 0 ; i < newParentNode->header.elementCount+1; i++ ){
            if((tempCurrentNodeData->score > cmpScore) ) break;
            tempCurrentNodeData++; 
        } 
        //concern about it
        tempNodeData.blockNumber = currentNodeCount+1; //new right node
        tempNodeData.score = tempCurrentNodeData->score; //splite max score
        memmove(tempCurrentNodeData+2,tempCurrentNodeData+1,sizeof(btreeData)*(MAXNODEDATA-i));
        memcpy(tempCurrentNodeData+1,&tempNodeData,sizeof(btreeData));
        tempCurrentNodeData->score = cmpScore;
        newParentNode->header.elementCount++;
        tempNextNodeNum = (tempCurrentNodeData+2)->blockNumber;

    }
    nodeWrite(newParentNode->header.nodeNum , newParentNode);
    parentNodeNum = newParentNode->header.nodeNum;

//when leaf split
    if(currentD == maxDepth){
        //make new next(right) Node    
        if(isRoot && parentElementCount >= MAXNODEDATA-1) {
           maxDepth++; 
        }
        newLeftLeaf = (btreeLeaf*)nodeRead(leftNodeNum);
        newRightLeaf = (btreeLeaf*)malloc(BLOCKSIZE);
        memset(newRightLeaf,0xFF,BLOCKSIZE);

        
        memcpy(newRightLeaf->Data,newLeftLeaf->Data+LEFTNODEMOVESIZE, sizeof(btreeData)*RIGHTNODEMOVESIZE);
        memset(newLeftLeaf->Data+LEFTNODEMOVESIZE,0xFF,sizeof(btreeData)*RIGHTNODEMOVESIZE);
        //get next leaf node number from parent
        newRightLeaf->nextLeafNum = tempNextNodeNum;

        tempLeafHeader.elementCount = RIGHTNODEMOVESIZE;
        tempLeafHeader.leafNum =currentNodeCount+1;
        tempLeafHeader.depth = maxDepth;
        tempLeafHeader.parentNum = parentNodeNum;
        memcpy(&newRightLeaf->header,&tempLeafHeader,sizeof(btreeLeafHeader));

        //write to the file (leftleaf)
        nodeWrite(currentNodeCount+1,newRightLeaf);

        //modify leftleaf
        newLeftLeaf->nextLeafNum =newRightLeaf->header.leafNum;
        newLeftLeaf->header.elementCount=LEFTNODEMOVESIZE;
        newLeftLeaf->header.depth=maxDepth; 
        newLeftLeaf->header.parentNum = parentNodeNum;
        nodeWrite(newLeftLeaf->header.leafNum,newLeftLeaf);
        free(newRightLeaf);
        currentNode = (btreeNode*)newLeftLeaf;
        currentNodeNum = leftNodeNum;
    }
    else{   //when node split 
        if(isRoot && parentElementCount >= MAXNODEDATA-1 ){
            maxDepth ++;
            currentD++;
        }
        newLeftNode = (btreeNode*)nodeRead(leftNodeNum);
        newRightNode = (btreeNode*)malloc(sizeof(btreeNode));
        memset(newRightNode,0xFF,sizeof(btreeNode));

        memcpy(newRightNode->Data,newLeftNode->Data+LEFTNODEMOVESIZE+1, sizeof(btreeData)*RIGHTNODEMOVESIZE);

        tempNodeHeader.elementCount = RIGHTNODEMOVESIZE;
        tempNodeHeader.nodeNum = currentNodeCount+1;
        tempNodeHeader.depth=currentD;
        tempNodeHeader.parentNum = parentNodeNum;
        memcpy(&newRightNode->header,&tempNodeHeader, sizeof(btreeNodeHeader));

        nodeWrite(currentNodeCount+1,newRightNode);
        
        memset(newLeftNode->Data+LEFTNODEMOVESIZE+1,0xFF, sizeof(btreeData)*RIGHTNODEMOVESIZE);

        newLeftNode->header.elementCount = LEFTNODEMOVESIZE;
        newLeftNode->header.depth = currentD;
        newLeftNode->header.parentNum = parentNodeNum;
        nodeWrite(newLeftNode->header.nodeNum,newLeftNode);
        free(newRightNode);
        currentNode = newLeftNode;
        currentNodeNum = newLeftLeaf->header.leafNum;
    }
    free(newParentNode);

}

void nodeWrite(unsigned nodeNumber ,void* Node ){
    if(nodeNumber > currentNodeCount)currentNodeCount++;

    fseek(idxFile,nodeNumber*BLOCKSIZE,SEEK_SET);
    fwrite(Node, BLOCKSIZE,1,idxFile);
}
void* nodeRead(unsigned nodeNumber){
    void* node;

    if(nodeNumber > currentNodeCount) return NULL;

    fseek(idxFile,nodeNumber * BLOCKSIZE, SEEK_SET);
    node = malloc(BLOCKSIZE);
    memset(node,0x00,BLOCKSIZE);
    fread(node,BLOCKSIZE,1,idxFile); 
    return node;
}
unsigned btreeSearch(float lower,float upper, unsigned till){
    int i = 0;
    unsigned tillFound = till;
    unsigned searchDepth ,loopRange;
    btreeData* currentData;
    loopRange = currentNode->header.elementCount;
    searchDepth = currentNode->header.depth;
    currentData = currentNode->Data;

#ifdef BTREEDEBUG
    printf("enter btreesearch : depthe: %u\n",searchDepth);
#endif
    if(searchDepth == maxDepth){
        for(i = 0 ; i < loopRange; i++){
            if(currentData->score >= lower && currentData->score <= upper){
                tillFound++;
                currentData++;
            }else if(currentData->score < lower){
                currentData++;
            }else{
                break;
            }
        } 
        if(i == loopRange && maxDepth != 0 ){
            unsigned next = ((btreeLeaf*)currentNode)->nextLeafNum;
            if(next == 0xFFFFFFFF || next == currentNode->header.nodeNum)return tillFound;
            free(currentNode);
            currentNode = NULL;
            currentNode=(btreeNode*)nodeRead(next);
            tillFound = btreeSearch(lower,upper,tillFound);
        }
    }
    else{
        for(i = 0; i < loopRange; i++){
            if(currentData->score >= lower){
                free(currentNode);
                currentNode=NULL;
                currentNode = (btreeNode*)nodeRead(currentData->blockNumber);
                tillFound = btreeSearch(lower,upper,tillFound);
                break;
            }
            else if(currentData->score < lower){
                currentData++; 
            }
        }
        if (i == loopRange){
            memset(currentNode,0x00,BLOCKSIZE);
            free(currentNode);
            currentNode = (btreeNode*)nodeRead(currentData->blockNumber);
            tillFound=btreeSearch(lower,upper,tillFound);
        }
    
    }
#ifdef  BTREEDEBUG
    printf("lower : %f, upper : %f, till : %u, currentNode :%u\n",lower,upper,tillFound,currentNode->header.nodeNum);
#endif
    return tillFound;
}
/*
 * 1. delete Block Data
 * 2. delete hash Table Data 
 * 3. delete b+tree Data
 */
bool deleteRecord(unsigned ID){
    // add code here 
    bool result = false;
    float score = 0.0;
    BucketData * Data=NULL;
    unsigned HashValue = RJhash(ID);

    Data=(BucketData*)malloc(sizeof(BucketData));
    memset(Data,0x00,sizeof(BucketData));

    result = deleteHashData(HashValue,Data);
    if(result){
        //swap Node to root
        if(currentNode != NULL){
            nodeWrite(currentNode->header.nodeNum,currentNode);
            memset(currentNode,0x00,BLOCKSIZE);
        } 
        memcpy(currentNode,rootTree,BLOCKSIZE);
        currentNodeNum = rootTree->header.nodeNum;

        //start to search
        score = deleteBlockData(Data );
        deleteNodeData(score);
    }
    return result;
}
bool deleteHashData(unsigned HashValue, BucketData* D){
    float result = false;
    int loopCount=0;
    BucketData * tempData;
    unsigned hashIdx = getHashIndex(HashValue);
    swapBucket(currentBucketNum,hashIdx);
    tempData = currentBucket->Data;    
    //get hash idx find Data

    while(tempData->hashValue != HashValue){
        loopCount++;
        tempData++;
    }

    // if found, get block# and erase data.
    if(loopCount < currentBucket->Attr.elementCount){
        result = true;
        memset(tempData,0x00,sizeof(BucketData));
        packingBucket(currentBucket->Data);
       // printf("delete found\n");
        memcpy(D,  tempData,sizeof(BucketData));
    }
    else{
        //printf("not found\n");
        free(D);
        D=NULL;
    }
    
    return result;
}
float deleteBlockData(BucketData *D){
    int i;
    unsigned *idxPtr;
    unsigned blockN;
    unsigned hashV;
    float score=0.0;
    Data tempD;

    blockN = D->blockNumber;
    hashV = D->hashValue; 
    
    //swap Block and delelte then restore
    fseek(datFile,currentBlockNum * BLOCKSIZE, SEEK_SET);
    fwrite(currentBlock, sizeof(char),BLOCKSIZE,datFile);
    memset(currentBlock,0x00,BLOCKSIZE);
    fseek(datFile,blockN * BLOCKSIZE, SEEK_SET);
    fread(currentBlock, sizeof(char),BLOCKSIZE,datFile);
    
    
    //check and erase
    idxPtr = ((unsigned*)currentBlock)+2; 
    for(i=0; i < MAXBLOCKDATA ; i++,idxPtr++){
        if(*idxPtr == hashV) {
           *idxPtr=0xFFFFFFFF;
           memcpy(&tempD, currentBlock+BLOCKSIZE-(i)*sizeof(Data),sizeof(Data));
           memset(currentBlock+BLOCKSIZE-(i)*sizeof(Data),0x00,sizeof(Data));
           break;
        }
    }
    //swap back
    fseek(datFile,blockN * BLOCKSIZE, SEEK_SET);
    fwrite(currentBlock, sizeof(char),BLOCKSIZE,datFile);
    memset(currentBlock, 0x00,BLOCKSIZE);
    fseek(datFile,currentBlockNum * BLOCKSIZE, SEEK_SET);
    fwrite(currentBlock, sizeof(char),BLOCKSIZE,datFile);
    
    return tempD.score;
}

float deleteNodeData(float sc){
    //goto leafnode and delete
    //if node contain no element then migrate
    float score = sc;
    float result = -1.0;
    int i = 0 ;
    unsigned elementC, currentDepth, currentNodeN, parentNum;
    btreeData* Data,tempData;
    
    
    elementC = currentNode->header.elementCount;
    currentDepth = currentNode->header.depth;
    Data = currentNode->Data;
    currentNodeN = currentNode->header.nodeNum;
    parentNum = currentNode->header.parentNum;

    //from root , goto certain score 
    for ( i = 0 ; i < elementC+1; i++ ){
        if (Data->score < score){
            Data++;
        }
        else{
            tempData = *Data;
           if(currentDepth == maxDepth){
                //delete score and packing 
                if ( i == elementC ){
                   result = (Data-1)->score;
                }
                else{
                   memmove(Data,Data+1,sizeof(btreeData)*(elementC-i-1));  
                }
               currentNode->header.elementCount-=1;
               memset(currentNode->Data+elementC,0xFF,sizeof(btreeData));
               nodeWrite(currentNode->header.nodeNum, currentNode);
               break;
           } 
           else{
                nodeWrite(currentNode->header.nodeNum,currentNode);
                if(currentNode != NULL){
                    memset(currentNode,0x00,BLOCKSIZE);
                    free(currentNode);
                }
                currentNode = NULL;
                currentNode = (btreeNode*)nodeRead(tempData.blockNumber);
                currentNodeNum = (currentNode)->header.nodeNum;
                result = deleteNodeData(score);
                break;
           }
        
        }
    
    } 
    if (currentNodeN != currentNode->header.nodeNum){
        nodeWrite(currentNode->header.nodeNum,currentNode);
        memset(currentNode,0x00,BLOCKSIZE);
        free(currentNode);
        currentNode =(btreeNode*) nodeRead(currentNodeN); 
    }

    if(result != -1.0){
         (currentNode->Data+i)->score= result;
    }
    // if score is last position it might be need to check parent node    
        
    return result;
}
