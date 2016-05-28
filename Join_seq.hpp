#ifndef JOIN_SEQ_H
#define JOIN_SEQ_H

#include "Relation.hpp"
#include <string>
#include <vector>

/*
Compares two vectors with given order and number of common variables
order 1 and order 2 are full size, ncommonvar is # of common vars
-1 if 1 is smaller than 2
*/
int compare_vect(std::vector<int> v1, std::vector<int> v2, std::vector<int>
      order1, std::vector<int> order2, int ncommonvar);


/*
Modularizes order finding :
return order1, order2, ncommonvar (which is a vector, int stocked in ncommonvar[0])
notcommon2 is equal to order2.begin()+ncommonvar
*/
std::vector<std::vector<int> > commonOrder(std::vector<std::string> list1, 
      std::vector<std::string> list2);
      
      
// Joins two relations, two lists of variables
Relation::Atom join(Relation *r1, Relation *r2, std::vector<std::string> list1, 
      std::vector<std::string> list2);


//Joins an atom to an atom
Relation::Atom join(Relation::Atom *a1, Relation::Atom *a2);

#endif
