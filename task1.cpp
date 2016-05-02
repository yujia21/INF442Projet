#include <iostream>
#include "Relation.hpp"

using namespace std;

int main () {
   //read file
   char* infile = "facebook.dat";
   cout << "File name is : " << infile << endl;
   Relation* relations = new Relation(infile);

   //check if read
   relations->printdata();
   
   //write file
   char* outfile = "testoutput1";
   relations->write(outfile);
   //relations->write(outfile, 0, 5);
   
   return 0;
}