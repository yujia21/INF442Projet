#include <iostream>
#include <time.h>
#include "Relation.hpp"

using namespace std;

int main () {
   //read file
   char* infile = "facebook.dat";
   Relation* relations = new Relation(infile);

   //create lists, import variable names
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};   
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );   
   
   clock_t t;
   t = clock();
   
   Relation::Atom* a3 = new Relation::Atom(relations, list3);
   
   Relation::Atom result1 = relations->join(relations,list1,list2);
   Relation::Atom result2 = result1.join(a3); 

   t = clock() - t;
   cout<<((float)t)/CLOCKS_PER_SEC<<" seconds to run"<<endl;
         
   Relation* relationsfinal = result2.relations;
   
   //write new 
   char* outfile = "testoutput";
   relationsfinal->write(outfile);
   relationsfinal->printdata();
   return 0;
}
