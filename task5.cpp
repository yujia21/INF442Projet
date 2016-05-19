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

   //For broadcast. relations created on all, to be able to be accessed later
   Relation* relations1;
   Relation* relations2;
   Relation* relations3;      
   
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
      ofstream data("run_data_dist.txt", ios::out | ios::app);
      data<<"File: "<<inputFile<<endl
      <<"Number of procs : "<<numtasks<<endl     
      <<"Number of initial relations : "<<relations1->size()<<" and "
            <<relations2->size()<<endl;
      data.close();
      
   }
   
   //Waits for all to reach this point
   MPI_Barrier(MPI_COMM_WORLD); 
   if (taskid==root){cout<<"First Join"<<endl;}
   //Everyone creates a_local, but only root has complete good one
   Relation::Atom a_local = joinDist(relations1,relations2,list1,list2,root);
   //Waits for all to exit join
   MPI_Barrier(MPI_COMM_WORLD);
   
   if (taskid == root){
      //Write intermediate runtime and relations data
      toc = clock();
      ofstream data("run_data_dist.txt", ios::out | ios::app);
      data<< "Elapsed CPU (Join 1)= "
         << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
      data<<"Number of intermediate relations :"
            <<a_local.relations->size()<<" and "<<relations3->size()<<endl;
      data.close();
   }
   
   //Waits for all to reach this point
   MPI_Barrier(MPI_COMM_WORLD);
   if (taskid==root){cout<<"Second Join"<<endl;}   
   //Everyone creates a_local_final, but only root has the complete one
   Relation::Atom a_local_final =
         joinDist(a_local.relations,relations3,a_local.variables,list3,root);
   //Waits for all to exit join
   MPI_Barrier(MPI_COMM_WORLD);
   
   if (taskid == root){
      //Write final runtime and relations data
      toc = clock();
      ofstream data("run_data_dist.txt", ios::out | ios::app);
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
