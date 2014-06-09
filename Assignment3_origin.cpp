#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string>
#include <string.h>

using namespace std;

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
bool openDB(char* filename){
        
        return false;
}


// Insert record into myDB
unsigned insertRecord(char* name, unsigned ID, float score, char* dept){
         unsigned blockNumber = 1;
         // add code here
         
         return blockNumber;
}


// Delete record 
bool deleteRecord(unsigned ID){
        // add code here
        
        return false;
}


// Search by ID
unsigned searchID(unsigned ID){
         unsigned blockNumber = 0;
         // add code here
         
         return blockNumber;
}


// Search by score
unsigned searchScore(float lower, float upper){
         unsigned numOfScore = 0;
         // add code here
         
         return numOfScore;
}
// =========================Your code here down!!(End)=========================


// =========================You don't need to touch=========================         
// main
int main()
{
    
    openDB("myDB");
    
    // Input data
    ifstream fin("Assignment3.inp", ios::in);
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
               fullName = firstName + " " + lastName;
               char name[20];
               strcpy(name, fullName.c_str());
               // for Visual Studio 2012
               // strcpy_s(name, fullName.c_str());
               char dept[10];
               strcpy(dept, deptStr.c_str());
               // for Visual Studio 2012
               // strcpy_s(dept, deptStr.c_str());
               
               //cout << type << "\t" << name << "\t"  << studentId << "\t"  << score << "\t"  << dept << endl;
               
               // insertRecord
               unsigned blockNumber = insertRecord(name, studentId, score, dept);
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
                   
        } else if(type == "c"){    // searchScore
               float lower = 0;
               float upper = 0;
               
               fin >> lower >> upper;
               //cout << type << "\t" << lower << "\t" << upper << endl;
               
               // searchScore
               unsigned result = searchScore(lower, upper);
               fout << result << endl;               
        } else if(type == "d"){
               unsigned studentId = 0;
               
               fin >> studentId;
               // cout << type << "\t" << studentId << endl;
               
               // deleteRecord
               bool isThere = false;
               isThere = deleteRecord(studentId);
               
        }
        
    }
    
    fout.close();
    fin.close();
    
    // for Dev C++
    /*
    system("PAUSE");
    return EXIT_SUCCESS;
    */
    return 0;
}

// ======================You don't need to touch (End)======================
