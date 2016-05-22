#include <iostream>
#include "Relation.hpp"

using namespace std;

int main (int argc, char **argv) {
   //read file
   char* inputFile;
   if (argc == 1){
      cout << "Enter input file name : " <<endl;
      inputFile = new char[30];
      cin >> inputFile;
   } else if (argc == 2){
      inputFile = argv[1];
   } else {
      cout << "Wrong number of input variables" <<endl ;
      cout << "Enter input file name : " <<endl;
      inputFile = new char[30];
      cin >> inputFile;
   }
   cout << "File name is : " << inputFile << endl;
   Relation* relations = new Relation(inputFile);

   //check if read
   relations->printdata();
   
   //write file
   char* outfile = "testoutput_task1";
   relations->write(outfile);
   //relations->write(outfile, 0, 5);
   
   return 0;
}
