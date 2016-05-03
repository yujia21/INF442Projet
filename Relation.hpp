#pragma once

#include <string>
#include <vector>

class Relation {

private:

   std::vector<std::vector<int> > relations;

public: 
   //TASK 1
   Relation(char * infile);
   ~Relation();
   
   void printdata();
   
   void write(char* outfile);   
   void write(char* outfile, int start, int end);

   //TASK 2
   void sortrelations(std::vector<int> order);
   bool importorder(const std::vector<int>& a, const std::vector<int>& b);
   
   struct Order{
     Order(std::vector<int> order);
     bool operator() (const std::vector<int>& a, const std::vector<int>& b);
     std::vector<int> order;
   };
   
   //TASK 3
   Relation join (Relation *r, std::vector<std::string> list1,
   std::vector<std::string> list2);
      
};
