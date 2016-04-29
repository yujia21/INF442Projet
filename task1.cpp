#include <iostream>
#include "Relation.hpp"

using namespace std;

int main () {
   char* infile = "facebook.dat";
   
   cout << "File name is : " << infile << endl;
   
   Relation* relations = new Relation(infile);
      
   relations->printdata();
   
   char* outfile = "testoutput.txt";
   
   relations->write(outfile,0,2);
   
   return 0;
}
