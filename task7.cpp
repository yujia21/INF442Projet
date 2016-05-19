#include "Relation.hpp"
#include "Join_seq.hpp"
#include "Join_dist.hpp"

#include "mpi.h" //mpi

#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector

#include <time.h> //track runtime
#include <sys/time.h>  //track runtime

int main (int argc, char **argv) {
   using namespace std;
   //Initialize MPI
   const int root = 0;
   
   int numtasks, taskid;
   MPI_Status status;
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
   
   //Start clock
   clock_t tic, toc;
   struct timeval theTV;
   struct timezone theTZ;
   gettimeofday(&theTV, &theTZ);
   srandom(theTV.tv_usec);
   tic = clock();
   
   //Create lists, import variable names : 
   //Accessible for all to be able to call join_seq on each
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};   
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );  

   //For broadcast. Relations initialized on all, to be able to be accessed later
   Relation* relations1;
   Relation* relations2;
   Relation* relations3;      
   
   //To write
   char* outputFile;
   outputFile = "run_data_dist_6.txt";
   
   if (taskid==root){      
      //Read file
      char* inputFile;
      if (argc == 1){
         cout << "Enter input file name : " <<endl;
         inputFile = new char[30];
         cin >> inputFile;
      } else if (argc == 2){
         inputFile = argv[1];
      } else {
         cout << "Wrong number of input variables" <<endl ;
         cout << "Enter input file name : " <<endl;
         inputFile = new char[30];
         cin >> inputFile;
      }
   
      //Create relations
      relations1 = new Relation(inputFile);
      relations2 = new Relation(inputFile);   
      relations3 = new Relation(inputFile); 
      
      //Write initial file details
      ofstream data(outputFile, ios::out | ios::app);      
      data<<"File: "<<inputFile<<endl
      <<"Number of procs : "<<numtasks<<endl     
      <<"Number of initial relations : "<<relations1->size()<<" and "
            <<relations2->size()<<endl;
      data.close();
      
   }
   
   //Waits for all to reach this point
   MPI_Barrier(MPI_COMM_WORLD); 
   
   if (taskid==root){cout<<"First Join"<<endl;}
   
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
   
   //Reset indexes for r1r2 and r3 on each proc
   //If a_local is empty ?
   vector<vector<int> > temp = commonOrder(a_local->variables,list3);
   index1 = temp[0][0]; //Comparison column
   index2 = temp[1][0];
   ncommonvar = temp[2][0];
   
   //Reset array1 for local arrays on each proc
   array1 = relationToDistArray(a_local->relations, numtasks, index1);
   
   //Reset array2 to list3 on root, sizetosend
   if (taskid==root){
      array2 = relationToDistArray(relations3, numtasks, index2);
      sizetosend2 = array2[0]/numtasks;      
   }
   
   MPI_Scatter(&array2[1],sizetosend2,MPI_INT, arraylocal2, sizetosend2, MPI_INT, root, MPI_COMM_WORLD);
   
   /* HOW SHOULD JOIN DIST OPTIMIZED WORK
if root 
   using distributeTuple. Take note of index to use

scatter

on all
   construct relation from array received
   call join seq
   extract array from atom

   construct array to distribute by distributeTuple
   
   receive (r1r2)
   join seq on (r1r2) and r3
   
gather

if root
   take array and put back in new relation
   change relation of atom that came from join seq

return atom*/





   
   
   
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

   
   
   
   
   
   
   //Waits for all to reach this point
   MPI_Barrier(MPI_COMM_WORLD);
   if (taskid==root){cout<<"Second Join"<<endl;}   
   //Everyone creates a_local_final, but only root has the complete one
   Relation::Atom a_local_final =
         joinDist(a_local.relations,relations3,a_local.variables,list3,root);
   //Waits for all to exit join
   MPI_Barrier(MPI_COMM_WORLD);
   
   //Clear memory of a_local and relations3. TO DO : this causes segmentation fault?
   //(&a_local)->~Atom();
   //relations3->~Relation();
   
   if (taskid == root){
      //Write final runtime and relations data
      toc = clock();
      ofstream data(outputFile, ios::out | ios::app);
      data<< "Elapsed CPU (Join 2)= "
         << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
      data<<"Number of final relations :"
            <<a_local_final.relations->size()<<endl<<endl;
      data.close();
      
      char* outfile = "testoutput_dist";
      a_local_final.relations->write(outfile);
   }
   
   MPI_Finalize();     
}

