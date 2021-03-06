#include "Relation.hpp"
#include "Join_seq.hpp"

#include "mpi.h" //mpi

#include <iostream>  //std::cout
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

#include <time.h> //track runtime
#include <sys/time.h>  //track runtime

#include <math.h> //floor

int distributeTupleHash(std::vector<int> v, int index, int m, double a){ 
   int n = v[index];
   if(a == 0)
      return v[index] % m;
   else
      return (int)floor(m*(a*n-floor(a*n)));
}

int* relationToDistArray(Relation* r, int numtasks, int index, double
      hashParameter){
   std::vector<std::vector<std::vector<int> > > arranged (numtasks); 
   for (int i = 0; i < r->size(); i++){
      std::vector<int> current = r->getindex(i);
           
      int n = distributeTupleHash(current, index, numtasks, hashParameter); 
      arranged[n].push_back(current);
   }
   
   int max = 0;

   for (int i = 0; i < numtasks; i++){
      if (arranged[i].size() > max){max = arranged[i].size();}
   }
   
   int* array = new int[numtasks * max * r->arity()+1];
   array[0] = numtasks * max * r->arity(); //stock total size
   
   for (int i = 0; i < numtasks; i++){
      for (int j = 0; j < arranged[i].size(); j++){
         for (int k = 0; k < r->arity(); k++){
            array[1+i*max*r->arity()+j*r->arity()+k]=arranged[i][j][k];
         }
      }
      for (int j = arranged[i].size();j<max;j++){
         for (int k = 0; k < r->arity(); k++){
            array[1+i*max*r->arity()+j*r->arity()+k] = -1; //assume no negative in relations
         }
      }
   }
   //Free memory : arranged
   std::vector<std::vector<std::vector<int> > >().swap(arranged); 
   
   return array; 
}

Relation::Atom joinDist(Relation* relations1, Relation* relations2,
      std::vector<std::string> list1, std::vector<std::string> list2, 
      int root, double hashParameter){
   using namespace std;
   int numtasks, taskid;
   MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

   // Send arity of relations
   int arity[2];    
   if (taskid==root){
      arity[0] = relations1->arity();
      arity[1] = relations2->arity();      
   }
   MPI_Bcast(&arity, 2, MPI_INT, root, MPI_COMM_WORLD);
   
   // To scatter and gather arrays, sizetosend, ncommonvar
   int* array1;
   int* array2; 
   int sizetosend1; //max*arity
   int sizetosend2;
   int ncommonvar;   
   
   if (taskid==root){         
      //Get common vars
      vector<vector<int> > temp = commonOrder(list1,list2);
      int index1 = temp[0][0]; //Comparison column
      int index2 = temp[1][0];
      ncommonvar = temp[2][0];
      
      //Construct array of relation 1 and 2
      array1 = relationToDistArray(relations1, numtasks, index1, hashParameter);
      array2 = relationToDistArray(relations2, numtasks, index2, hashParameter);      
      
      //Get size to send
      sizetosend1 = array1[0]/numtasks;
      sizetosend2 = array2[0]/numtasks;
   } 
   //Broadcast sizetosend
   //Don't want to calculate direct on each processus
   //because relationToDistArray is only called by root
   MPI_Bcast(&sizetosend1, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&sizetosend2, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&ncommonvar, 1, MPI_INT, root, MPI_COMM_WORLD);      
   
   //Initialize receiving arrays
   int* arraylocal1;
   arraylocal1 = new int [sizetosend1];
   int* arraylocal2;
   arraylocal2 = new int [sizetosend2];

   // Scatter
   MPI_Scatter(&array1[1],sizetosend1,MPI_INT, arraylocal1, sizetosend1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(&array2[1],sizetosend2,MPI_INT, arraylocal2, sizetosend2, MPI_INT, root, MPI_COMM_WORLD);   
   
   //Construct relation from array received
   Relation* relationlocal1 = new Relation();
   relationlocal1->importArray(arraylocal1,sizetosend1, arity[0]);
   Relation* relationlocal2 = new Relation();
   relationlocal2->importArray(arraylocal2,sizetosend2, arity[1]);
   
   //Free memory from arraylocal and array
   MPI_Barrier(MPI_COMM_WORLD);    
   delete[] arraylocal1;
   delete[] arraylocal2;
   if(taskid==root){
      delete[] array1;
      int* array1;
      delete[] array2;
      int* array2;
   }
   MPI_Barrier(MPI_COMM_WORLD); //Make sure all cleared before join
   
   //Call join seq
   Relation::Atom a_local = join(relationlocal1,relationlocal2,list1,list2);
   
   //Free local relation memory
   relationlocal1->~Relation();
   relationlocal2->~Relation();   
   
   //Find max of a_local relation size, and Bcast to all
   int finalarity = arity[0]+arity[1]-ncommonvar;
   int localsize = finalarity*a_local.relations->size();
   int maxlocalsize;
   MPI_Reduce(&localsize, &maxlocalsize, 1, MPI_INT, MPI_MAX, root, MPI_COMM_WORLD);
   MPI_Bcast(&maxlocalsize, 1, MPI_INT, root, MPI_COMM_WORLD);
   
   //Create array to send back to root
   int* finalarraylocal;
   finalarraylocal=new int[maxlocalsize]; 
   
   if (localsize==0){
      for (int i = 0; i<maxlocalsize;i++){
         finalarraylocal[i]=-1;
      }
   } else {
      a_local.relations->toArray(maxlocalsize, finalarraylocal); 
   }
   
   int* finalarray;
   finalarray = new int[maxlocalsize*numtasks];

   // Gather 
   MPI_Gather(finalarraylocal, maxlocalsize, MPI_INT, finalarray, maxlocalsize, MPI_INT,
         root, MPI_COMM_WORLD);
   
   // Free finalarraylocal. Barrier before importArray, because importArray takes memory
   delete[] finalarraylocal;
   MPI_Barrier(MPI_COMM_WORLD);
   
   if (taskid == root){
      // Take array and put back in relation
      // Assumption : a_local.variables isn't empty?
      a_local.relations = new Relation();
      a_local.relations->importArray(finalarray, maxlocalsize*numtasks,finalarity); //memory can leak here
      
      cout<<"Post join size of relations : "<<a_local.relations->size()<<endl;
   }
   
   // Free finalarray
   delete[] finalarray;

   //Return atom of root
   if (taskid == root){
      return a_local;
   }
}

//TO DO : Create Atomized version of join_dist
//TO DO : Count variance amongst number of relations that each proc receives ?
