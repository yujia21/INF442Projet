#include <iostream>
#include "Relation.hpp"

using namespace std;

int main () {
   //read file
   char* infile1 = "facebookshort.dat";
   char* infile2 = "facebookshort2.dat";
   char* infile3 = "facebookshort3.dat";   
   Relation* relations1 = new Relation(infile1);
   Relation* relations2 = new Relation(infile2);
   Relation* relations3 = new Relation(infile3);   

   //create lists, import variable names
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};   
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );   
   
   Relation::Atom* a3 = new Relation::Atom(relations3, list3);
   
   //Task 3 : join first two
   Relation::Atom result1 = relations1->join(relations2,list1,list2);
   cout << "Task 3 Variables : ";
   vector<string>::iterator it;
   for (it = result1.variables.begin(); it != result1.variables.end(); ++it){
      cout<<*it<<" ";
   }
   cout << endl;
   
   //Task 4 : join third relation
   Relation::Atom result2 = result1.join(a3); //second order is wrong : why?
   
   Relation* relationsfinal = result2.relations;
   
   cout << "Task 4 Variables : ";
   for (it = result2.variables.begin(); it != result2.variables.end(); ++it){
      cout<<*it<<" ";
   }
   cout << endl;

   
   //write new 
   char* outfile = "testoutput";
   relationsfinal->write(outfile);
   relationsfinal->printdata();
   return 0;
}
