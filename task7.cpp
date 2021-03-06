#include "Relation.hpp"
#include "Join_seq.hpp"
#include "Join_dist.hpp"

#include "mpi.h" //mpi

#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector
#include <sstream>  //std::sort
#include <iterator> //std::istringstream
#include <algorithm> //std::sort

#include <time.h> //track runtime
#include <sys/time.h>  //track runtime

// Converts vector of vector of int to 1Darray (to use alltoallv).
// Vector of vector of int contains all relations to send to all processus, in order
// first n1 vectors to proc 1, next n2 to proc 2, ni to proc i etc.
int* vectToArray(std::vector<std::vector<int> > vect){
   int* array;
   if (vect.size() > 0){
      array = new int [vect.size()*vect[0].size()];
      for (int i = 0; i < vect.size(); i++){
         for (int j = 0; j < vect[0].size(); j++){
            array[i*vect[0].size()+j] = vect[i][j];
         }
      }
      return array;
   }
}

// Takes relation, machine to send to, column of comparison, total number of machines
// Returns a vector of relations to send to machine "recv"
std::vector<std::vector<int> > relationToDistArrayOptimized(Relation* r, int recv, int index, int m){
   std::vector<std::vector<int> > temp;
   for (int i = 0; i < r->size(); i++){
      if (r->getindex(i)[index] % m == recv){
         temp.push_back(r->getindex(i));
      }
   }
   return temp;
}

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
   vector<string> list4;
   
   //For broadcast. Relations initialized on all, to be able to be accessed later
   Relation* relations1;
   Relation* relations2;
   Relation* relations3;      
   
   //To write data
   char* outputFile;
   outputFile = "run_data_task7.txt";
   
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
   
      //Create relations r1 to 3
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
   
   // To scatter and gather arrays, sizetosend, ncommonvar
   int arity[4];   
   int* array1;
   int* array2; 
   int* array3;    
   int sizetosend[3]; //max*arity
   int ncommonvar[2];
   int index[4];
   
   if (taskid==root){
      //Get common vars for r1 r2
      vector<vector<int> > temp = commonOrder(list1,list2);
      index[0] = temp[0][0]; //Comparison column
      index[1] = temp[1][0];
      ncommonvar[0] = temp[2][0];
      
      //Create list for (r1r2)
      list4.insert(list4.end(),list1.begin(),list1.end());
      vector<int>::iterator it;
      for (it = temp[1].begin()+ncommonvar[0]; it != temp[1].end();++it){
         list4.push_back(list2[*it]);
      }
      
      //Get common vars for r3 r4
      vector<vector<int> > temp2 = commonOrder(list3,list4);
      index[2] = temp2[0][0];
      index[3] = temp2[1][0];
      ncommonvar[1] = temp2[2][0];
      
      //Construct array of relation 1 to 3
      array1 = relationToDistArray(relations1, numtasks, index[0], 0);
      array2 = relationToDistArray(relations2, numtasks, index[1], 0);      
      array3 = relationToDistArray(relations3, numtasks, index[2], 0);
      
      //Get size to send
      sizetosend[0] = array1[0]/numtasks;
      sizetosend[1] = array2[0]/numtasks;
      sizetosend[2] = array3[0]/numtasks;
      
      //Calculates all arity
      arity[0] = relations1->arity();
      arity[1] = relations2->arity();      
      arity[2] = relations3->arity();            
      arity[3] = list4.size();
      
   } 
   
   //Broadcast arity, sizetosend, index, ncommonvar
   //Don't want to calculate direct on each processus
   //because relationToDistArray is only called by root
   MPI_Bcast(&arity, 4, MPI_INT, root, MPI_COMM_WORLD);   
   MPI_Bcast(&sizetosend, 3, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&index, 4, MPI_INT, root, MPI_COMM_WORLD);   
   MPI_Bcast(&ncommonvar, 2, MPI_INT, root, MPI_COMM_WORLD);      
   
   //Initialize receiving arrays
   int* arraylocal1;
   arraylocal1 = new int [sizetosend[0]];
   int* arraylocal2;
   arraylocal2 = new int [sizetosend[1]];
   int* arraylocal3;
   arraylocal3 = new int [sizetosend[2]];
   int* arraylocal4;
   
   if (taskid==root){cout<<"Scattering for first join"<<endl;}
   // Scatter
   MPI_Scatter(&array1[1],sizetosend[0],MPI_INT, arraylocal1, sizetosend[0], MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(&array2[1],sizetosend[1],MPI_INT, arraylocal2, sizetosend[1], MPI_INT, root, MPI_COMM_WORLD);   
   
   //Construct relation from array received
   Relation* relationlocal1 = new Relation();
   relationlocal1->importArray(arraylocal1,sizetosend[0], arity[0]);
   Relation* relationlocal2 = new Relation();
   relationlocal2->importArray(arraylocal2,sizetosend[1], arity[1]);
   
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

   if (taskid==root){cout<<"Starting first join"<<endl;}      
   //Call join seq
   Relation::Atom a_local = join(relationlocal1,relationlocal2,list1,list2);
         
   //Free local relation memory
   relationlocal1->~Relation();
   relationlocal2->~Relation();   
   
   //DEBUG attendre pour afficher
   MPI_Barrier(MPI_COMM_WORLD);
   

   if (taskid==root){cout<<"Scattering for second join"<<endl;}
   //Reset or initialize list4 on each proc
   list4 = a_local.variables;   
   
   MPI_Barrier(MPI_COMM_WORLD); //Make sure all reset before scatter. Necessary?

   //Each proc receives relation3 relations
   MPI_Scatter(&array3[1],sizetosend[2],MPI_INT, arraylocal3, sizetosend[2], MPI_INT, root, MPI_COMM_WORLD);
   Relation* relationlocal3 = new Relation();
   relationlocal3->importArray(arraylocal3,sizetosend[2], arity[2]);


   //cout<<taskid<<" Calculating (r1r2)"<<endl;      
   
   //EACH PROC I SENDS TO PROC J WHAT IS NEEDED FOR J
   
   //sendarray[i] stocks array from (r1r2) to send to ith processor
   int size4; //total size of vectors taskid will send
   
   //table of sizes, sizetosend4[i] is the size of data sent to ith proc by proc(taskid)
   int* sizetosend4; 
   sizetosend4 = new int[numtasks]; 
   
   vector<vector<int> > sendvect; //data to send
   int* sendarray; //data to send after conversion
   
   //Variables for Alltoallv
   int* recvcount;
   recvcount = new int[numtasks];
   int* sdispls;
   sdispls = new int[numtasks];   
   int* rdispls;
   rdispls = new int[numtasks];   

   //Each proc concatenates in sendvect what it will send to 1, 2, ... numtasks
   
   for (int i = 0; i < numtasks; i++){
      vector<vector<int> > temp = relationToDistArrayOptimized(a_local.relations, i, index[3], numtasks);
      sendvect.insert(sendvect.end(),temp.begin(),temp.end()); //concatenate vectors in order
      sizetosend4[i] = 3*temp.size(); //stock size to be sent to each indiv proc //FAULT 3 pour l'arit� !!!!
   }
   
   sendarray = vectToArray(sendvect); //converts to array to send
   
   MPI_Barrier(MPI_COMM_WORLD); //waits for all to finish sendarray
   
   //puts in recvcount[j] the size of what is received from proc j
   MPI_Alltoall(sizetosend4, 1, MPI_INT, recvcount, 1, MPI_INT, MPI_COMM_WORLD);
   
   //Calculates displacement for sending and receiving on each
   rdispls[0] = 0;
   sdispls[0] = 0;
   for (int j = 1; j < numtasks; j++){
      rdispls[j]=rdispls[j-1]+recvcount[j-1];
      sdispls[j]=sdispls[j-1]+sizetosend4[j-1];
   } 

   //Puts in arraylocal4 everything 
   size4 =rdispls[numtasks-1]+recvcount[numtasks-1];
   arraylocal4 = new int[size4];
   
   
   MPI_Barrier(MPI_COMM_WORLD); //necessary?
   MPI_Alltoallv(sendarray, sizetosend4, sdispls,
            MPI_INT, arraylocal4, recvcount, rdispls, MPI_INT, MPI_COMM_WORLD);

   MPI_Barrier(MPI_COMM_WORLD);
   
   //Create (r1r2) relation
   Relation* relationlocal4 = new Relation();
   relationlocal4->importArray(arraylocal4, size4, arity[3]);

   if (taskid==root){cout<<"Starting second join"<<endl;}         
   //Each join seq on (r1r2) and  r3
   a_local = join(relationlocal3,relationlocal4,list3,list4);
   
   MPI_Barrier(MPI_COMM_WORLD);//wait for all to join
   
   //Find max of a_local relation size, and Bcast to all
   int finalarity = arity[2]+arity[3]-ncommonvar[1];
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
      
      //Write final runtime and relations data
      toc = clock();
      ofstream data(outputFile, ios::out | ios::app);
      data<< "Elapsed CPU = "
         << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
      data<<"Number of final relations :"
            <<a_local.relations->size()<<endl<<endl;
      data.close();
      
      char* outfile = "testoutput_task7";
      a_local.relations->write(outfile);
   }
   
   MPI_Finalize();     
}
