#ifndef RELATION_H
#define RELATION_H

#include <string>
#include <vector>

class Relation {

private:

   std::vector<std::vector<int> > relations;

public: 
   int size();
   int arity();
   
   //TASK 1
   Relation();
   Relation(char * infile);
   ~Relation();
   
   //prints number of relations and arity on console
   void printdata(); 
   
   //prints relations on console
   void toPrint(); 
   
   void write(char* outfile);   
   void write(char* outfile, int start, int end);
   
   void addrelation(std::vector<int> r);
   std::vector<int> getindex(int i);

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
      Relation* relations;
      std::vector<std::string> variables;
   
      Atom();
      ~Atom();
      Atom(Relation* relations, std::vector<std::string> variables);
   };
   
   //TASK 5
   //takes a 1d array of size,ignores negative. Needs arity to start construction
   void importArray(int* array, int size, int arity); 
   
   //makes a 1d array, feels everything from relations.size to maxsize with -1
   void toArray(int maxsize, int* array); 
};

#endif
