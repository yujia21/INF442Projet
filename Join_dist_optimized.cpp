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

int* vectToArray(std::vector<std::vector<std::vector<int> > > vect, int size){
   int* array;
   if (size > 0){
      array = new int [size];
      int m = 0;
      for (int i = 0; i < vect.size(); i++){ //proc
         for (int j = 0; j < vect[i].size(); j++){ //relation
            for (int k = 0; k <vect[i][j].size(); k++){
               array[i*vect[0].size()+j] = vect[i][j];
            }
         }
      }
      return array;
   }
}

std::vector<std::vector<std::vector<int> > > relationToDistArray(Relation* r, int numtasks, int index, double
      hashParameter){
   std::vector<std::vector<std::vector<int> > > arranged (numtasks); 
   for (int i = 0; i < r->size(); i++){
      std::vector<int> current = r->getindex(i);
           
      int n = distributeTupleHash(current, index, numtasks, hashParameter); 
      arranged[n].push_back(current);
   }
   return arranged; 
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
   int* sendcount1;
   int* sendcount2;   
   int* sdispls1;
   int* sdispls2;    
   int recvcount1;
   int recvcount2;
   int ncommonvar;   
   
   if (taskid==root){         
      //Get common vars
      vector<vector<int> > temp = commonOrder(list1,list2);
      int index1 = temp[0][0]; //Comparison column
      int index2 = temp[1][0];
      ncommonvar = temp[2][0];
      
      //Construct vectors of relation 1 and 2
      //vector [i] is for ith proc
      vector<vector<vector<int> > > vect1 = 
            relationToDistArray(relations1, numtasks, index1, hashParameter);
      vector<vector<vector<int> > > vect2 = 
            relationToDistArray(relations2, numtasks, index2, hashParameter);      
      
      //Stock each size, calculate sdisp, in arrays
      sendcount1 = new int[numtasks];
      sendcount2 = new int[numtasks]; 
      
      for (int i = 0;i<numtasks;i++){
         sendcount1[i] = vect1.size()*arity[0];
         sendcount2[i] = vect2.size()*arity[1];
      }

      sdispls1[0] = 0;
      sdispls2[0] = 0;      
      for (int j = 1; j < numtasks; j++){
         sdispls1[j]=sdispls1[j-1]+sendcount1[j-1];
         sdispls2[j]=sdispls2[j-1]+sendcount2[j-1];
      }      

      //Convert vect1 and vect2 to 1D array
      array1 = vectToArray(vect1, sdispls1[numtasks-1]+sendcount1[numtasks-1]);
      array2 = vectToArray(vect2, sdispls2[numtasks-1]+sendcount2[numtasks-1]);      
   } 
   //Broadcast commonvars
   MPI_Bcast(&ncommonvar, 1, MPI_INT, root, MPI_COMM_WORLD);      
   
   //Scatter recvcount
   MPI_Scatterv(sendcount1,1,MPI_INT,recvcount1,1,MPI_INT,root,MPI_COMM_WORLD);
   MPI_Scatterv(sdispls2,1,MPI_INT,recvcount2,1,MPI_INT,root,MPI_COMM_WORLD);   
   
   //Initialize receiving arrays
   int* arraylocal1;
   int* arraylocal2;

   // Scatterv
   MPI_Scatterv(array1,sendcount1,sdispls1,MPI_INT,
         arraylocal1, recvcount1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatterv(array2,sendcount2,sdispls2,MPI_INT, 
         arraylocal2, recvcount2, MPI_INT, root, MPI_COMM_WORLD);   
   
   //Construct relation from array received
   Relation* relationlocal1 = new Relation();
   relationlocal1->importArray(arraylocal1,recvcount1, arity[0]);
   Relation* relationlocal2 = new Relation();
   relationlocal2->importArray(arraylocal2,recvcount2, arity[1]);
   
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
   
   //convert a_local.relation to array
   //stock size
   
   // Gather sizes
   MPI_Gather(finalarraylocal, maxlocalsize, MPI_INT, finalarray, maxlocalsize, MPI_INT,
         root, MPI_COMM_WORLD);
   
   // Gatherv
   MPI_Gatherv(finalarraylocal, maxlocalsize, MPI_INT, finalarray, maxlocalsize, MPI_INT,
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
