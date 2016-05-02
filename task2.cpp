#include <iostream>
#include "Relation.hpp"

using namespace std;

int main () {
   //read file
   char* infile = "facebookshort.dat";
   cout << "File name is : " << infile << endl;
   Relation* relations = new Relation(infile);
   
   //write original
   char* outfile1 = "testoutput1";
   relations->write(outfile1);
   
   //sort in opposite order
   static const int arr2[] = {1,0};
   vector<int> neworder (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );

   relations->sortrelations(neworder);

   //write new 
   char* outfile2 = "testoutput2";
   relations->write(outfile2);
   
   return 0;
}
