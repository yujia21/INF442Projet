#include <iostream>
#include "Relation.hpp"

using namespace std;

int main () {
   //read file
   char* infile = "facebook.dat";
   cout << "File name is : " << infile << endl;
   Relation* relations = new Relation(infile);
   
   //write original
   char* outfile1 = "testoutput1";
   relations->write(outfile1, 0, 5);
   
   //sort in opposite order
   vector<int> neworder[] = {2,1};
   relation->sortrelations(neworder);

   //write new 
   char* outfile2 = "testoutput2";
   relations->write(outfile2, 0, 5);
   
   return 0;
}
