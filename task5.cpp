#include "Relation.hpp"
#include "Join_seq.hpp"

#include <boost/mpi.hpp> //mpi
#include <boost/serialization/vector.hpp>  //mpi vector

#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

#include <time.h>
#include <sys/time.h>

#include <math> //floor

using namespace std;

int distributeTuple(std::vector<int> v, int index, int m){ 
   //given : m = total number of machines, index = column to compare, v = vector to send
   //returns machine to send to
   return v[index] % m;
}

//TASK 6
int distributeTupleHash(std::vector<int> v, int index, int m, double a){ 
   //given : m = total number of machines, index = column to compare, v = vector to send
   //returns machine to send to
   int n = v[index];
   return floor(m*(a*n-floor(a*n)));
}

int main (int argc, char **argv) {
   int root = 0;
   
   //Read file, create relations
   char* infile = "triangles.dat";
   Relation* relations1 = new Relation(infile);
   Relation* relations2 = new Relation(infile);   
   Relation* relations3 = new Relation(infile);      

   ofstream data("run_data_dist.txt", ios::out | ios::app);
   data<<"File: "<<infile<<endl
      <<"Number of initial relations : "<<relations1->size()<<endl;
   data.close();
   
   //Create lists, import variable names
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};   
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );   
   
   //Start clock
   clock_t tic, toc;
   struct timeval theTV;
   struct timezone theTZ;
   gettimeofday(&theTV, &theTZ);
   srandom(theTV.tv_usec);
   tic = clock();
   
   //Join 1
   cout<< "Starting first join..."<<endl;
   Relation::Atom a1 = joinDist(relations1,relations2,list1,list2, root);
   
   //Print intermediate data
   cout << "Intermediate Variables : ";
   vector<string>::iterator it;
   for (it = a1.variables.begin(); it != a1.variables.end(); ++it){
      cout<<*it<<" ";
   }
   cout << endl;

   Relation* relationsInt = a1.relations;
   relationsInt->printdata();
   
   //Write intermediate runtime and relations data
   toc = clock();
   data<< "Elapsed CPU (Join 1)= "
      << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl<<endl; 
   data<<"Number of intermediate relations : "<<relationsInt->size()<<endl;
   data.close();

   
   //Join 2   
   cout<< "Starting second join..."<<endl;  
   //Create third atom
   Relation::Atom* a2 = new Relation::Atom(relations3, list3);    
   //Call join
   Relation::Atom a3 = joinDist(&(a1),a2, root); 
   
   //Print final data
   cout << "Final Variables : ";
   for (it = a3.variables.begin(); it != a3.variables.end(); ++it){
      cout<<*it<<" ";
   }
   cout << endl;   

   Relation* relationsfinal = a3.relations;
   relationsfinal->printdata();   
   
   //Write new relations to file
   char* outfile = "testoutput";
   relationsfinal->write(outfile);
   
   //Get final time   
   toc = clock();
   cout << "Elapsed CPU = "
       << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
   
   //Write final runtime and relations data
   data<<"Number of final relations : "<<relationsfinal->size()<<endl
      << "Elapsed CPU (Join 2)= "
      << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl<<endl; 
   data.close();

   return 0;   

}

/* TO DO
?? do squares and etc according to argc

int main(argv blabla)
argv = file name of relations

init mpi
if root 
   read files, construct relations r1 and r2
   construct 2 vector<vector<vector<int>>> to distribute for r1 and r2

scatter

on all
   construct relation from vector<vector<int>> received
   call join seq
   extract vector<vector<int>> from atom

gather

if root
   take vector<vector<vector<int>>> and put back in new relation
   change relation of atom that came from join seq

return atom





FROM OLD JOIN DIST FILE:

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

   */
