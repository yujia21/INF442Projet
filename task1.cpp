#include <iostream>
#include "Relation.hpp"

using namespace std;

int main () {
   char* infile = "facebook.dat";
   
   cout << "File name is : " << infile << endl;
   
   Relation* relations = new Relation(infile);
      
   relations->printdata();
   
   char* outfile = "testoutput1";
   
   relations->write(outfile);
   
   return 0;
}
