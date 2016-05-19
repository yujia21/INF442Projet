#include "Relation.hpp"
#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

// TASK 1
Relation::Relation(){
   std::vector<std::vector<int> > relations;   
}

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
         std::sort (this->relations.begin(),
               this->relations.end(),Order(neworder)); 
      }
   }
}

// TASK 3 : JOIN 
void Relation::addrelation(std::vector<int> r){
   this->relations.push_back(r);
}

Relation::Atom::Atom(){
   this->relations = new Relation();
   std::vector<std::string > variables;
}

Relation::Atom::Atom(Relation* relations, std::vector<std::string> variables){
   this->relations = relations;
   this->variables = variables;
}

int Relation::size(){
   return this->relations.size();
}
int Relation::arity(){
   if (this->relations.size()!=0){
      return this->relations[0].size();
   } else {
      std::cerr<<"Not initialized!"<<std::endl;
      return -1;
   }
}

std::vector<int> Relation::getindex(int i){
   if (i<0 || i>this->size()){ 
      std::cerr<<"Bad i : "<<i<<std::endl; 
      std::cerr<<"size : "<<this->size()<<std::endl;
   }
   return this->relations[i];
}

// TASK 5 : DISTRIBUTION
void Relation::importArray(int* array, int size, int arity){
   for (int i = 0; i<size/arity; i++){
      if (array[i*arity]>=0){
         std::vector<int> temp(arity);
         for (int j = 0; j < arity; j++){
            temp[j] = array[i*arity+j];
         }
         this->relations.push_back(temp);
      }
   }
}

void Relation::toArray(int maxsize, int* array){
   //fills the array
   for (int i = 0; i < this->size(); i++){
      for (int j = 0; j < this->arity(); j++){
         array[i*this->arity()+j]=this->getindex(i)[j];
      }
   }
   for (int i = this->size()*this->arity() ; i < maxsize ; i++){
      array[i] = -1;
   }
}


void Relation::toPrint(){
   std::cout<<"Printing Relation"<<std::endl;
   for (int i = 0; i<this->size();i++){
      for (int j = 0; j<this->arity();j++){
         std::cout<<this->getindex(i)[j]<<" ";
      }
      std::cout<<std::endl;
   }
}
