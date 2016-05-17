#ifndef RELATION_H
#define RELATION_H

#include <string>
#include <vector>

class Relation {

private:

   std::vector<std::vector<int> > relations;

public: 
   //TASK 1
   Relation();
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
   
   //TASK 3 and 4
   
   struct Atom{
      Atom(Relation* relations, std::vector<std::string> variables);
      Relation* relations;
      std::vector<std::string> variables;
   };
   
   void addrelation(std::vector<int> r);
   int size();
   int arity();
   std::vector<int> getindex(int i);
      
   //TASK 5
   void importArray(int** array);
   int** toArray();
};

#endif
