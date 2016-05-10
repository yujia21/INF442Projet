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
         //cout << "Sorting..." <<endl;
         std::sort (this->relations.begin(),
               this->relations.end(),Order(neworder)); 
         //cout << "Sorted!" <<endl;
      }
   }
}

// TASK 3 : JOIN 
void Relation::addrelation(std::vector<int> r){
   this->relations.push_back(r);
}

int Relation::compare_vect(std::vector<int> v1, std::vector<int> v2, std::vector<int>
      order1, std::vector<int> order2, int ncommonvar){ 
   //order 1 and order 2 are full size, ncommonvar is # of common vars
   //-1 if 1 is smaller than 2
   for (int i = 0; i<ncommonvar; i++){
      if (v1[order1[i]] < v2[order2[i]]){
         return -1;
      } else if (v1[order1[i]] > v2[order2[i]]){
         return 1;
      }
   }
   return 0;
}

Relation::Atom::Atom(Relation* relations, std::vector<std::string> variables){
   this->relations = relations;
   this->variables = variables;
}
      
Relation::Atom Relation::join(Relation *r, std::vector<std::string> list1, 
      std::vector<std::string> list2){
   
//Relation* Relation::join(Relation *r, std::vector<std::string> list1, 
//      std::vector<std::string> list2){   
   using namespace std;
   int size1 = this->relations.size();   
   int size2 = r->relations.size();
   Relation* finalr = new Relation();   
   vector<string> commonvar;
   
   if (size1 == 0 || size2 == 0){ //TO DO use assert or raise exceptions?
      cerr << "No relations yet!" << endl;
   } else if (list1.size()!=this->relations[0].size() || list2.size() != r->relations[0].size()){
      cerr << "Number of variables don't match arity!" <<endl;
   } else {
      int arity1 = this->relations[0].size();
      int arity2 = r->relations[0].size();
            
      //1. Find common variables
      vector<int> order1; //order for this
      vector<int> order2; //order for r
      vector<int> notcommon1; //list of not common var for list1
      vector<int> notcommon2;      
      vector<int>::iterator it;
      int ncommonvar = 0;
     
      for (int i = 0;i<arity1;i++){
         bool icommon = false;
         for (int j = 0;j<arity2;j++){
            if (list1[i]==list2[j]){
               order1.push_back(i); //add index to order to sort
               order2.push_back(j);
               ncommonvar ++;
               icommon = true;
               break;
            }
         }
         if (!icommon){
            notcommon1.push_back(i);
         }
      }
      
      cout << "Number of common vars: " <<ncommonvar <<endl;
      for (int j = 0;j<arity2;j++){
         if (find(order2.begin(),order2.end(),j)==order2.end()){
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
      //this->write("testoutput1");
      //r->write("testoutput2");      
      
      //3. Iterate over R and R', call t and t'
      int istart = 0;
      int jstart = 0;
      int iend=0;
      int jend = 0;
      while (istart<size1 && jstart < size2){
         //load t and t'
         vector<int> v1 = this->relations[istart];
         vector<int> v2 = r->relations[jstart];
         
         //if same
         int flag = compare_vect(v1, v2, order1, order2, ncommonvar);
         switch(flag){
            case 0:
               //find all i equal v1
               iend = istart+1;
               while(compare_vect(v1,
                  this->relations[iend],order1,order1,ncommonvar)==0){
                  iend++;
               }
                     
               //find all j equals v2
               jend = jstart+1;
               while(compare_vect(v2,
                  r->relations[jend],order2,order2,ncommonvar)==0){
                  jend++;
               }
              
               //for loop over istart, iend, jstart, jend
               for (int i = istart; i<iend; i++){
                  for (int j = jstart; j<jend; j++){
                     //copy from list 1
                     vector<int> newrel(this->relations[i]); 
               
                     //add unrepeated from list 2
                     newrel.insert(newrel.end(),r->relations[j].begin()+ncommonvar,r->relations[j].end());
   
                     //insert into finalr
                     finalr->addrelation(newrel);
                  }
               }
               
               //Jump to first tuples that disag w t and t'
               istart=iend;
               jstart=jend;
               break;
      
            //projX t =/= projXt' -> if go to t or t' depending if projX t or projXt' is smaller            
            case 1: //v2 smaller
               jstart++;
               break;
               
            case -1: //v1 smaller
               istart++;
               break;
               
            default:
               cerr<<"Wrong comparison"<<endl;
         }
      }
      //Return variables and relation

      //fill in rest of variables to final string vector to return

      commonvar.insert(commonvar.end(),list1.begin(),list1.end());
      for (it = notcommon2.begin();it != notcommon2.end();++it){
         commonvar.push_back(list2[*it]);
      }

      vector<string>::iterator itstring;
   }
   return Atom(finalr,commonvar);

   //return finalr;
}

// how to call a function without instancing an atom?
Relation::Atom Relation::Atom::join(Relation::Atom *a){
   Relation* r1 = this->relations;
   std::vector<std::string> l1 = this->variables;
   Relation* r2 = a->relations;
   std::vector<std::string> l2 = a->variables;   
   return r1->join(r2,l1,l2);
}
