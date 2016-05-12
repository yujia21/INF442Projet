#ifndef JOIN_DIST_H
#define JOIN_DIST_H

#include "Relation.hpp"
#include <string>
#include <vector>

Relation::Atom joinDist(Relation *r1, Relation *r2, std::vector<std::string> list1, 
      std::vector<std::string> list2, int argc, char **argv, int root);
      
Relation::Atom joinDist(Relation::Atom *a1, Relation::Atom *a2, int argc, char **argv,
int root);

#endif