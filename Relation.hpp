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
   
   void sortrelations(std::vector<int> order);
   bool importorder(const std::vector<int>& a, const std::vector<int>& b);
   
   struct Order{
     Order(std::vector<int> order);
     bool operator() (const std::vector<int>& a, const std::vector<int>& b);
     std::vector<int> order;
   };
   
   static join (Relation r, std::vector<T> list1, vector<T> list2);
      
private:

   std::vector<std::vector<int> > relations;

};