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

