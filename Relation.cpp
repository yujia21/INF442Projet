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
std::pair<Relation,std::vector<std::string> > Relation::join(Relation *r, std::vector<std::string> list1, 
      std::vector<std::string> list2){
   using namespace std;
   int size1 = this->relations.size();   
   int size2 = r->relations.size();
   if (size1 == 0 || size2 == 0){ //TO DO use assert or raise exceptions?
      cerr << "No relations yet!" << endl;
   } else if (list1.size()!=this->relations[0].size() || list2.size() == size2){
      cerr << "Number of variables don't match arity!" <<endl;
   } else {
      int arity = this->relations[0].size();
      //1. Find common variables
      vector<string> commonvar;
      vector<int> order1;
      vector<int> order2;
      vector<int> notcommon1;      
      vector<int> notcommon2;      
      vector<int>::iterator it;
     
      for (int i = 0;i<arity;i++){
         bool icommon = false;
         for (int j = 0;j<arity;j++){
            if (list1[i]==list2[j]){
               order1.push_back(i); //add index to order to sort
               order2.push_back(j); 
               commonvar.push_back(list1[i]); //add string to return in the end
               icommon = true;
               break;
            }
         }
         if (!icommon){
            notcommon1.push_back(i);
         }
      }
      //fill up notcommon2
      for (int j = 0;j<arity;j++){
         if (! binary_search(order2.begin(),order2.end(),j)){
            notcommon2.push_back(j);
         }
      }
      
      //fill in non-common indexes for orders
      order1.insert( order1.end(), notcommon1.begin(), notcommon1.end() );
      order2.insert( order2.end(), notcommon2.begin(), notcommon2.end() );
      
      cout<<"Order 1: ";
      for (it = order1.begin(); it != order1.end(); ++it){
         cout<<*it<<" ";
      }
      cout <<"\n";
      
      cout<<"Order 2: ";
      for (it = order2.begin(); it != order2.end(); ++it){
         cout<<*it<<" ";
      }
      cout <<"\n";
      
      //2. Order R and R', such that restrictions to X have same order      
      this->sortrelations(order1);
      r->sortrelations(order2);      
   
      //3. Iterate over R and R', call t and t'
      for (int i = 0;i<this->relations.size();i++){ //over set of relations
          for (int j = 0;j<commonvar.size();j++){ //for all common vars
             cout << "i : " <<i<<", ";
             cout << "j : " <<j<<"\n";
             if (this->relations[i][j] = r->relations[i][j]){ //need all to be the same?
                cout << "yay same \n";
             }
           }
      }
            
      //If same on X, add all combi that agree with t and t' on X to output
   
      //Jump to first tuples that disag w t and t'
      //projX t =/= projXt' -> if go to t or t' depending if projX t or projXt' is smaller
      
      //Return variables and relation
      
      //fill in rest of variables to final string vector to return
      for (it = notcommon1.begin();it != notcommon1.end();++it){
         commonvar.push_back(list1[*it]);
      }
      for (it = notcommon2.begin();it != notcommon2.end();++it){
         commonvar.push_back(list2[*it]);
      }
      vector<string>::iterator itstring;
      for (itstring = commonvar.begin(); itstring != commonvar.end();++itstring){
         cout<< *itstring <<" ";
      }
      return make_pair(*r,commonvar);
}
}
