#include "Relation.hpp"
#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

using namespace std;
/*
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
*/

int main () {

   static const int arr[] = {0,1};
   vector<int> vec1 (arr, arr + sizeof(arr) / sizeof(arr[0]) );
   
   static const int arr2[] = {1,0};
   vector<int> vec2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );
   
   vector< vector <int> > relations;
   relations.push_back(vec1);
   relations.push_back(vec2);
   
   sort(relations.begin(), relations.end(),
         [] (const vector<int>& a, const vector<int>& b) {
     return a[1] < b[1];
   }); //how to insert fn directly? [] is a problem

/*
   sort (relations.begin(), relations.end(), 
         [](const vector<int>& a, const vector<int>& b){return a[1]<b[1];
      
         static const int arr[] = {1,0};
         vector<int> order (arr, arr + sizeof(arr) / sizeof(arr[0]) );   
         for ( int i = 0;i<2;i++ ){
            if ( a[order[i]] != b[order[i]] ){
               return ( a[order[i]] < b[order[i]] );
            }
         }
         return true;
      });
   */
         
   for (int j = 0;j<2;j++){
      for (vector<int>::iterator it=relations[j].begin(); it!=relations[j].end(); ++it)
        {cout << ' ' << *it;}
      cout<<endl;
   }
}
