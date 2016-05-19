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
   std::cout<<"infunc "<<taskid<<std::endl;
   r->toPrint();
   std::vector<std::vector<std::vector<int> > > arranged (numtasks); 
   for (int i = 0; i < r->size(); i++){
      std::vector<int> current = r->getindex(i);

      int n = distributeTuple(current, index, numtasks);      
      //int n = distributeTupleHash(current, index, numtasks); //decomment for task6
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
      
      //Construct array of array of array
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
   
   //Initialize receiving arrays
   int arraylocal1[sizetosend1];
   int arraylocal2[sizetosend2];

   cout<<"bcasted and arrays init"<<endl;
      
   // Scatter
   MPI_Barrier(MPI_COMM_WORLD); //wait for arraylocal to be constructed in root
   MPI_Scatter(array1,sizetosend1,MPI_INT, &arraylocal1, sizetosend1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(array2,sizetosend2,MPI_INT, &arraylocal2, sizetosend2, MPI_INT, root, MPI_COMM_WORLD);   
   
   //Construct relation from array<array<int>> received
   Relation* relationlocal1 = new Relation();
   relationlocal1->importArray(arraylocal1,sizetosend1, arity[0]);
   Relation* relationlocal2 = new Relation();
   relationlocal2->importArray(arraylocal2,sizetosend2, arity[1]);
   
   cout<<"scattered and relations constructed"<<endl;   
   
   //Call join seq
   Relation::Atom a_local = join(relationlocal1,relationlocal2,list1,list2);
   
   cout<<"joined"<<endl;
   
   //Create array to send back to root
   int finalarraylocal[sizetosend1*sizetosend2];
   a_local.relations->toArray(sizetosend1*sizetosend2, finalarraylocal);

   int finalarray[sizetosend1*sizetosend2*numtasks];
   
   // Gather 
   MPI_Gather(&finalarraylocal, sizetosend1*sizetosend2, MPI_INT, &finalarray, sizetosend1*sizetosend2, MPI_INT,
         root, MPI_COMM_WORLD);
   cout<<"gathered"<<endl;
   
   cout<<"proc "<<taskid<<endl;
   for (int i = 0;i<sizetosend1*sizetosend2*numtasks;i++){
      cout<<finalarray[i]<<" ";
   }
   cout<<endl;
      
   MPI_Barrier(MPI_COMM_WORLD); //wait for gather to be done
   if (taskid == root){
      // Take array of array of array of int and put back in new relation
      cout<<"Size before : "<<a_local.relations->size()<<endl;
      a_local.relations = new Relation();
      a_local.relations->importArray(finalarray, sizetosend1*sizetosend2*numtasks,
            arity[0]+arity[1]-ncommonvar);
      cout<<"Size after : "<<a_local.relations->size()<<endl;
      
      cout<<"After join"<<endl;
      a_local.relations->toPrint();
      return a_local;
   }
}
