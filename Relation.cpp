#include "Relation.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>

Relation::Relation(char* infile){

   using namespace std;
   
   cout << "Reading from file : " << infile << endl;
   
   fstream file;
   file.open(infile);
   string line;
   //vector< vector<int> > relations;
   
   while (getline(file, line))
    {
       istringstream is( line );
       this->relations.push_back(vector<int>(istream_iterator<int>(is),istream_iterator<int>()));
    }
    
   cout << "Read " << this->relations.size() << " lines " << endl;  
   file.close();
} 

Relation::~Relation(){}

void Relation::printdata(){
   using namespace std;
   cout << "Number of relations : " << this->relations.size() << endl;
   cout << "Arity : " << this->relations[0].size() << endl;  
}

void Relation::write(char* outfile){
   using namespace std; 
   int size = this->relations.size();
   if (size == 0){ //use assert or raise exceptions?
      cout << "No relations to write!" << endl;
   } else {
     int arity = this->relations[0].size();
     ofstream file;
     file.open(outfile);
     for (int j = 0; j < size; j++) {
        for (int i = 0; i < arity; i++) {
           file << this->relations[j][i]<<" ";
        }
        file<<"\n";
     }
     file.close();
   }
}


void Relation::write(char* outfile, int start, int end){
   using namespace std; 
   int size = this->relations.size();
   if (size == 0){ //use assert or raise exceptions?
      cout << "No relations to write!" << endl;
   } else {
      if (start>=0 & end < size){
         int arity = this->relations[0].size();
         ofstream file;
         file.open(outfile);
         for (int j = start; j < end; j++) {
            for (int i = 0; i < arity; i++) {
               file << this->relations[j][i]<<" ";
            }
            file<<"\n";
         }
         file.close();
      } else {
         cout << "Bad start and/or end values!" << endl;
      }
   }
}
