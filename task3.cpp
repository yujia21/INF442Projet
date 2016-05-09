#include <iostream>
#include "Relation.hpp"

using namespace std;

int main () {
   //read file
   char* infile1 = "facebookshort.dat";
   char* infile2 = "facebookshort2.dat";
   Relation* relations1 = new Relation(infile1);
   Relation* relations2 = new Relation(infile2);
   Relation* relationsfinal;   
   
   //create lists, import variable names
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   

   //join
   pair<Relation,vector<string> > final = relations1->join(relations2,list1,list2);
   *(relationsfinal) = final.first;
   vector<string>::iterator it;
   for (it = final.second.begin(); it != final.second.end(); ++it){
      cout<<*it<<" ";
   }
   cout << "\n";

   //write new 
   char* outfile = "testoutput";
   relationsfinal->write(outfile);
   relationsfinal->printdata();
   return 0;
}
