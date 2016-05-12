#include "Relation.hpp"
#include "Join_seq.hpp"
#include "Join_dist.hpp"

#include <boost/mpi.hpp> //mpi
#include <boost/serialization/vector.hpp>  //mpi vector

#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

int distributeTuple(std::vector<int> v, int index, int m){ 
   //given : m = total number of machines, index = column to compare, v = vector to send
   //returns machine to send to
   return v[index] % m;
}

Relation::Atom joinDist(Relation *r1, Relation *r2, std::vector<std::string> list1, 
      std::vector<std::string> list2, int argc, char **argv, int root){
   int size1 = r1->size();   
   int size2 = r2->size();
   Relation* finalr = new Relation();   
   std::vector<std::string> commonvar;
   
   if (size1 == 0 || size2 == 0){ //TO DO use assert or raise exceptions?
      std::cerr << "No relations yet!" << std::endl;
   } else if (list1.size()!=r1->arity() || list2.size() != r2->arity()){
      std::cerr << "Number of variables don't match arity!" <<std::endl;
   } else {   
      using namespace std;
      //Initialize MPI
      int numtasks, taskid;
      boost::mpi::environment env{argc, argv};
      boost::mpi::communicator world;
      taskid = world.rank();
      numtasks = world.size();
   

      // IF ROOT: read relations and sort   
      if (taskid = root){
         vector<int>::iterator it;      
         vector<vector<int> > temp = commonOrder(list1,list2);
         vector<int> order1 = temp[0];
         vector<int> order2 = temp[1];
         int ncommonvar = temp[2][0];
         // TO DO ? OPTIMISE : should sorting be done on multiple proc?
         // if so, need root to broadcast : ncommonvar, order1, order2, notcommon2 
         // for each to be able to join later
   
         // vector of vectors of vectors of int. vector[i] to machine [i]
         // each machine gets vector<vector <int> > for r1, and for r2
         // should have numtasks columns
         vector<vector<vector<int> > > r1ToScatter(numtasks); 
         vector<vector<vector<int> > > r2ToScatter(numtasks);    
   
         // run through r1->relations then r2->relations
         // push_back each vector<int> into r1toscatter[distributeTuple(...)]
      }
      
      
      vector<vector<int> > r1Local;
      vector<vector<int> > r2Local;   
      // scatter1 : for r1
      // scatter2 : for r2
      boost::mpi::scatter(world, r1ToScatter, r1Local, root);
      boost::mpi::scatter(world, r2ToScatter, r2Local, root);   

      
      // ALL PROC:
      // make two Relations from vector<vector<int> > for r1 and for r2
      // call join seq on these two and on list1 list2 ? but this will recalculate order
      // and sort again ? unless we make this modular ? but complix, lots of in/output
      // OR: 
      // direct copy code from join seq
      vector<vector<int > > localResult;
   
   

      // Reconstitute by gather instead of reduce ?
      vector<vector<vector<int > > > result;
      boost::mpi::gather(world,localResult,result,root);
      //insert into finalr contents of result[i] for all i

   }
   // return Atom
   return (Relation::Atom(finalr,commonvar));
}

Relation::Atom joinDist(Relation::Atom *a1, Relation::Atom *a2, int argc, char **argv, int root){
   Relation* r1 = a1->relations;
   std::vector<std::string> l1 = a1->variables;
   Relation* r2 = a2->relations;
   std::vector<std::string> l2 = a2->variables;   
   return joinDist(r1,r2,l1,l2, argc, argv, root);
}
