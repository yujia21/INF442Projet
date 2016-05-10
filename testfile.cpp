#include "Relation.hpp"
#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

using namespace std;

struct Order {
   Order (std::vector <int> neworder){this->order = neworder;}
   bool operator() (const std::vector<int>& a, const std::vector<int>& b){
      using namespace std;
      for ( int i = 0;i<this->order.size();i++ ){
         if ( a[this->order[i]] != b[this->order[i]] ){
            return ( a[this->order[i]] < b[this->order[i]] );
         }
      }
      return true;
   }
   std::vector<int> order;
};

bool importorder(const vector<int>& a, const vector<int>& b){
   static const int arr[] = {1,0};
   vector<int> order (arr, arr + sizeof(arr) / sizeof(arr[0]) );   
   for ( int i = 0;i<2;i++ ){
      if ( a[order[i]] != b[order[i]] ){
         return ( a[order[i]] < b[order[i]] );
      }
   }
   return true;
}


int main () {

   static const int arr[] = {0,1};
   vector<int> vec1 (arr, arr + sizeof(arr) / sizeof(arr[0]) );
   
   static const int arr2[] = {1,0};
   vector<int> vec2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );

   static const int arr3[] = {0,2};
   vector<int> vec3 (arr3, arr3 + sizeof(arr) / sizeof(arr[0]) );
   
   static const int arr4[] = {2,0};
   vector<int> vec4 (arr4, arr4 + sizeof(arr2) / sizeof(arr2[0]) );
   
   static const int arr5[] = {0,3};
   vector<int> vec5 (arr5, arr5 + sizeof(arr) / sizeof(arr[0]) );
      
   vector< vector <int> > relations;
   relations.push_back(vec1);
   relations.push_back(vec2);
   relations.push_back(vec3);
   relations.push_back(vec4);
   relations.push_back(vec5);
   
   sort (relations.begin(), relations.end(), Order(vec2));
         
   for (int j = 0;j<5;j++){
      for (vector<int>::iterator it=relations[j].begin(); it!=relations[j].end(); ++it)
        {cout << ' ' << *it;}
      cout<<endl;
   }
}

void task3(){
      int j = 0;                  
      for (int i = 0;i<size1;i++){ //over set of relations of 1
            vector<int> v1 = this->relations[i];
            cout<< "in i loop : "<<i<<endl;

            vector<int> v2 = r->relations[j];
            do {
               v2 = r->relations[j];
               j++;
               cout<<j<<endl; //PROBLEM : j reaching size 2
            } while (!compare_vect(v1, v2, order1, order2, ncommonvar) && j<size2) ;
            cout<<"starting j : "<<j<<endl;
            
            //If same on X, add all combi that agree with t and t' on X to output
            cout<< "j : ";
            while (j < size2){
               v2 = r->relations[j]; //TO DO: optimize
               if(compare_vect(v1, v2, order1, order2, ncommonvar)){
                  cout<<j<<" ";
                   vector<int> newrel(this->relations[i]); //copy from list 1
   
                   //add unrepeated from list 2, insert into finalr
                   newrel.insert(newrel.end(),r->relations[j].begin()+ncommonvar,r->relations[j].end());
                   finalr->addrelation(newrel);

                }
                j++;
            }
            if (i<size1 && this->relations[i+1]==this->relations[i]){
               j = 0;
            }
            cout<<endl;
      }
   }
