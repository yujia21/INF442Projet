#include <iostream>
#include <time.h>
#include <sys/time.h>
#include "Relation.hpp"
#include "Join_dist.hpp"
#include <fstream>

using namespace std;

int main (int argc, char **argv) {
   int root = 0;
   
   //Read file, create relations
   char* infile = "triangles.dat";
   Relation* relations1 = new Relation(infile);
   Relation* relations2 = new Relation(infile);   
   Relation* relations3 = new Relation(infile);      

   ofstream data("run_data_dist.txt", ios::out | ios::app);
   data<<"File: "<<infile<<endl
      <<"Number of initial relations : "<<relations1->size()<<endl;
   data.close();
   
   //Create lists, import variable names
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};   
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );   
   
   //Start clock
   clock_t tic, toc;
   struct timeval theTV;
   struct timezone theTZ;
   gettimeofday(&theTV, &theTZ);
   srandom(theTV.tv_usec);
   tic = clock();
   
   //Join 1
   cout<< "Starting first join..."<<endl;
   Relation::Atom a1 = joinDist(relations1,relations2,list1,list2, root);
   
   //Print intermediate data
   cout << "Intermediate Variables : ";
   vector<string>::iterator it;
   for (it = a1.variables.begin(); it != a1.variables.end(); ++it){
      cout<<*it<<" ";
   }
   cout << endl;

   Relation* relationsInt = a1.relations;
   relationsInt->printdata();
   
   //Write intermediate runtime and relations data
   toc = clock();
   data<< "Elapsed CPU (Join 1)= "
      << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl<<endl; 
   data<<"Number of intermediate relations : "<<relationsInt->size()<<endl;
   data.close();

   
   //Join 2   
   cout<< "Starting second join..."<<endl;  
   //Create third atom
   Relation::Atom* a2 = new Relation::Atom(relations3, list3);    
   //Call join
   Relation::Atom a3 = joinDist(&(a1),a2, root); 
   
   //Print final data
   cout << "Final Variables : ";
   for (it = a3.variables.begin(); it != a3.variables.end(); ++it){
      cout<<*it<<" ";
   }
   cout << endl;   

   Relation* relationsfinal = a3.relations;
   relationsfinal->printdata();   
   
   //Write new relations to file
   char* outfile = "testoutput";
   relationsfinal->write(outfile);
   
   //Get final time   
   toc = clock();
   cout << "Elapsed CPU = "
       << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
   
   //Write final runtime and relations data
   data<<"Number of final relations : "<<relationsfinal->size()<<endl
      << "Elapsed CPU (Join 2)= "
      << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl<<endl; 
   data.close();

   return 0;   
}
