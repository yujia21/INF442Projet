#pragma once

#include <string>
#include <vector>

class Relation {

public: 

   Relation(char * infile);
   ~Relation();
   
   void printdata();
   
   void write(char* outfile);   
   void write(char* outfile, int start, int end);
   
private:

   std::vector<std::vector<int> > relations;
   
};
