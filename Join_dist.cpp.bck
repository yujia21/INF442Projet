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

int distributeTuple(std::vector<int> v, int index, int m){ 
   return v[index] % m;
}

int distributeTupleHash(std::vector<int> v, int index, int m, double a){ 
   int n = v[index];
   return (int)floor(m*(a*n-floor(a*n)));
}

int* relationToDistArray(Relation* r, int numtasks, int index){
   int taskid;
   MPI_Comm_rank(MPI_COMM_WORLD, &taskid);         
   //std::cout<<"infunc "<<taskid<<std::endl;
   //r->toPrint();
   std::vector<std::vector<std::vector<int> > > arranged (numtasks); 
   for (int i = 0; i < r->size(); i++){
      std::vector<int> current = r->getindex(i);

      int n = distributeTuple(current, index, numtasks);      
      //int n = distributeTupleHash(current, index, numtasks, 2.3); //decomment for task6
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
   //delete &arranged;
   return array; 
}

Relation::Atom joinDist(Relation* relations1, Relation* relations2,
      std::vector<std::string> list1, std::vector<std::string> list2, 
      int root){
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
      array1 = relationToDistArray(relations1, numtasks, index1);
      array2 = relationToDistArray(relations2, numtasks, index2);      
      
      //Get size to send, shift arrays
      sizetosend1 = array1[0]/numtasks;
      sizetosend2 = array2[0]/numtasks;
      array1 = &array1[1];
      array2 = &array2[1]; 
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
   MPI_Barrier(MPI_COMM_WORLD); //wait for arraylocal to be constructed in root
   MPI_Scatter(array1,sizetosend1,MPI_INT, arraylocal1, sizetosend1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(array2,sizetosend2,MPI_INT, arraylocal2, sizetosend2, MPI_INT, root, MPI_COMM_WORLD);   
   
   //Construct relation from array received
   Relation* relationlocal1 = new Relation();
   relationlocal1->importArray(arraylocal1,sizetosend1, arity[0]);
   Relation* relationlocal2 = new Relation();
   relationlocal2->importArray(arraylocal2,sizetosend2, arity[1]);
   //cout<<"Proc " <<taskid<<" received and constructed relations"<<endl;
   
   //MPI_Barrier(MPI_COMM_WORLD);    
   //array1 = 0;
   //delete array2;
   //cout<<"deleted first arrays"<<endl;
   //delete arraylocal1;
   //delete arraylocal2;   
   //cout<<"deleted arraylocals"<<endl;
   
   //Call join seq
   Relation::Atom a_local = join(relationlocal1,relationlocal2,list1,list2);

   //Find max of a_local relation size, and Bcast to all
   int localsize = (arity[0]+arity[1]-ncommonvar)*a_local.relations->size();
   //cout<<taskid<<" "<<"Local Size : "<<localsize<<endl;

   int maxlocalsize;
   MPI_Reduce(&localsize, &maxlocalsize, 1, MPI_INT, MPI_MAX, root, MPI_COMM_WORLD);
   MPI_Bcast(&maxlocalsize, 1, MPI_INT, root, MPI_COMM_WORLD);
   if (taskid==0){cout<<"maxlocalsize: "<<maxlocalsize<<endl;}
   
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
   //cout<<"finalarray created on "<<taskid<<endl;
   
   // Gather 
   MPI_Gather(finalarraylocal, maxlocalsize, MPI_INT, finalarray, maxlocalsize, MPI_INT,
         root, MPI_COMM_WORLD);
   
   MPI_Barrier(MPI_COMM_WORLD); //wait for gather to be done
   if (taskid == root){
      //cout<<"Post gather"<<endl;
      // Take array and put back in relation
      a_local.relations = new Relation();
      a_local.relations->importArray(finalarray, maxlocalsize*numtasks,
            arity[0]+arity[1]-ncommonvar);
      cout<<"Post join size of relations : "<<a_local.relations->size()<<endl;
      return a_local;
   }
      
   //MPI_Barrier(MPI_COMM_WORLD);    
   //delete finalarraylocal;
   //cout<<"deleted finalarraylocals"<<endl;   
   //delete finalarray;
   //cout<<"deleted finalarrays"<<endl;  
}
