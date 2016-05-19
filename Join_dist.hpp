#ifndef JOIN_DEF_H
#define JOIN_DEF_H

#include "Relation.hpp"
#include "Join_seq.hpp"

//TASK 5 
//given : m = total number of machines, index = column to compare, v = vector to send
//returns machine to send to
int distributeTuple(std::vector<int> v, int index, int m);

//Root calls this function to create return a 1D array
//max = max number of relations that a proc has
//assumes relations have no negative numbers
//if a proc doesn't have max relations, fills the rest with -1
//array[0] is total size = numtasks*max*arity
//processus i goes from 1+i*max*arity to 1 + (i+1)*max*arity
//where the jth relation is from 1+i*max*arity + j*arity to 1+i*max*arity + (j+1)*arity
int* relationToDistArray(Relation* r, int numtasks, int index);

//Join Dist
Relation::Atom joinDist(Relation* relations1, Relation* relations2,
      std::vector<std::string> list1, std::vector<std::string> list2, 
      int root);

//TASK 6
//given : m = total number of machines, index = column to compare, v = vector to send
//returns machine to send to
int distributeTupleHash(std::vector<int> v, int index, int m, double a);


#endif
