#include "Relation.hpp"
#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

// TASK 1
Relation::Relation(char* infile){

   using namespace std;
   
   cout << "Reading from file : " << infile << endl;
   
   fstream file;
   file.open(infile);
   string line;
   
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
   if (!this->variables.empty()){
      cout << "Variables : ";
      for (int i = 0; i<this->variables.size();i++){
         cout << this->variables[i] <<" " ;
      }
      cout << endl;
   }
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

// TASK 2 : ORDER STRUCT
Relation::Order::Order (std::vector <int> neworder){this->order = neworder;}
bool Relation::Order::operator() (const std::vector<int>& a, const std::vector<int>& b){
   using namespace std;
   for ( int i = 0;i<this->order.size();i++ ){
      if ( a[this->order[i]] != b[this->order[i]] ){
         return ( a[this->order[i]] < b[this->order[i]] );
      }
   }
   return true;
}   

// TASK 2 : MAIN FUNCTION
void Relation::sortrelations(std::vector<int> neworder){
   //order [a b c] compares int at a, then b, then c
   // TO DO : change to assert errors cerr
   using namespace std;
   int size = this->relations.size();
   if (size == 0){ //use assert or raise exceptions?
      cerr << "No relations to sort!" << endl;
   } else {
      int arity = this->relations[0].size();
      if (neworder.size()!=arity){
        cout << "Order has wrong arity!" << endl;
      } else {
         cout << "Sorting..." <<endl;
         std::sort (this->relations.begin(),
               this->relations.end(),Order(neworder)); 
         cout << "Sorted!" <<endl;
      }
   }
}

// TASK 3 : JOIN 
void Relation::getVariables(std::vector<std::string> newvariables){
   using namespace std;
   int size = this->relations.size();   
   if (size == 0){ //TO DO use assert or raise exceptions?
      cerr << "No relations yet!" << endl;
   } else if (newvariables.size()!=this->relations[0].size()){
      cerr << "Wrong arity size!" << endl;
   } else {
      this->variables = newvariables;
      cout << "New variables initialized!" << endl;
   }
}
   

Relation Relation::join (Relation r){
   
   using namespace std;
   vector<string> commonvar;
   vector<int> order1; //use this to stock common vars, and to order later on
   vector<int> order2; 

   //Find common variables
   
   
   //Order R and R', such that restrictions to X have same order

   //Iterate over R and R', call t and t'
   //If same on X, add all combi that agree with t and t' on X to output

   //Jump to first tuples that disag w t and t'
   //projX t =/= projXt' -> if go to t or t' depending if projX t or projXt' is smaller
   
   //print relations
   r->getVariables(commonvar);
   return r;
}
