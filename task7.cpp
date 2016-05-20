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

void relationToDistArrayOptimized(Relation* r, int recv, int index, int m, int* array){
   std::vector<std::vector<int> > temp;
   for (int i = 0; i < r->size(); i++){
      if (r->getindex(i)[index] % m == recv){
         temp.push_back(r->getindex(i));
      }
   }
   int arity = 0;
   if (r->size()!=0){
      arity = r->arity();
   }
   array = new int[temp.size()*arity+1];
   array[0] = temp.size()*arity;

   for (int i = 0; i<temp.size();i++){
      for (int j = 0; j < arity; j++){
         array[1+i*arity+j] = temp[i][j];
      }
   }

   std::vector<std::vector<int> >().swap(temp); 
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
   
   //To write
   char* outputFile;
   outputFile = "run_data_dist_7.txt";
   
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
   
   // Send arity of relations 1 to 3
   int arity[3];    
   if (taskid==root){
      arity[0] = relations1->arity();
      arity[1] = relations2->arity();      
      arity[2] = relations3->arity();            
   }
   MPI_Bcast(&arity, 3, MPI_INT, root, MPI_COMM_WORLD);
   
   // To scatter and gather arrays, sizetosend, ncommonvar
   int* array1;
   int* array2; 
   int sizetosend1; //max*arity
   int sizetosend2;
   int sizetosend3;
   int sizetosend4;
   int ncommonvar1;
   int ncommonvar2;   
   int index1;
   int index2;
   int index3;
   int index4;   
   
   if (taskid==root){         
      //Get common vars for r1 r2
      vector<vector<int> > temp = commonOrder(list1,list2);
      index1 = temp[0][0]; //Comparison column
      index2 = temp[1][0];
      ncommonvar1 = temp[2][0];
      
      //Create list for (r1r2)
      list4.insert(list4.end(),list1.begin(),list1.end());
      for (it = temp[1].begin()+ncommonvar;it != temp[1].end();++it){
         list4.push_back(list2[*it]);
      }
      
      //Get common vars for r3 r4
      vector<vector<int> > temp2 = commonOrder(list3,list4);
      index3 = temp2[0][0];
      index4 = temp2[0][0];      
      ncommonvar2 = temp2[2][0]
      
      //Construct array of relation 1 to 3
      array1 = relationToDistArray(relations1, numtasks, index1);
      array2 = relationToDistArray(relations2, numtasks, index2);      
      array3 = relationToDistArray(relations3, numtasks, index3);
      
      //Get size to send
      sizetosend1 = array1[0]/numtasks;
      sizetosend2 = array2[0]/numtasks;
      sizetosend3 = array3[0]/numtasks;
   } 
   //Broadcast sizetosend
   //Don't want to calculate direct on each processus
   //because relationToDistArray is only called by root
   MPI_Bcast(&sizetosend1, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&sizetosend2, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&sizetosend3, 1, MPI_INT, root, MPI_COMM_WORLD);   
   MPI_Bcast(&ncommonvar1, 1, MPI_INT, root, MPI_COMM_WORLD);      
   MPI_Bcast(&ncommonvar2, 1, MPI_INT, root, MPI_COMM_WORLD);         
   
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
   
   cout<<taskid<<" called join"<<endl;

   //Reset indexes for r1r2 and r3 on each proc
   //If a_local is empty ?
   vector<string> list4 = a_local.variables;   
   vector<vector<int> > temp = commonOrder(list4,list3);
   index1 = temp[0][0]; //Comparison column
   index2 = temp[1][0];
   ncommonvar = temp[2][0];
   
   //Reset arity
   arity[0] = list4.size();
   arity[1] = list3.size();      
   
   //Reset array2 to r3 on root, sizetosend
   if (taskid==root){
      array2 = relationToDistArray(relations3, numtasks, index2);

      sizetosend2 = array2[0]/numtasks;
      for (int i = 0; i<sizetosend2;i++){
         cout<<array2[i+1]<<" ";
      }
      cout<<endl;
   }
   MPI_Bcast(&sizetosend2, 1, MPI_INT, root, MPI_COMM_WORLD);
   
   cout<<taskid<<" all reset"<<endl;   
   
   MPI_Barrier(MPI_COMM_WORLD); //Make sure all reset before scatter. Necessary?
   //Each proc receives relation3 relations
   MPI_Scatter(&array2[1],sizetosend2,MPI_INT, arraylocal2, sizetosend2, MPI_INT, root, MPI_COMM_WORLD);
   cout<<taskid<<" r3 scattered"<<endl;
   if (taskid==root){
      for (int i = 0; i < sizetosend2;i++){
         cout<<arraylocal2[i]<<" ";
      }
   }
   cout<<endl;
   
   relationlocal2->importArray(arraylocal2,sizetosend2, arity[1]);

   cout<<taskid<<" r3 imported"<<endl;   
   
   
   
   
   
   
   
   //EACH PROC I SENDS TO PROC J WHAT IS NEEDED FOR J
   
   //sendarray1[i] stocks array to send to ith processor
   int** sendarray1;
   sendarray1=new int*[numtasks];
   int* recvcount;
   recvcount = new int[numtasks];
   int* displs;
   displs = new int[numtasks];   
   
   //Each proc stocks to array1[i] what it will send to i
   for (int i = 0; i < numtasks; i++){
      relationToDistArrayOptimized(a_local.relations, i, index1, numtasks, sendarray1[i]);
   }
   MPI_Barrier(MPI_COMM_WORLD);
   
   //Each proc i gathers all meant for proc i (array1[i] from each)
   for (int i = 0; i < numtasks; i++){      
      //puts in recvcount[j] the size of what is received from j
      MPI_Gather(&(sendarray1[i][0]), 1, MPI_INT, recvcount, 1, MPI_INT, i, MPI_COMM_WORLD);
      displs[0] = 0;
      for (int j = 1; j < numtasks; j++){
         displs[j]=displs[j-1]+recvcount[j];
      }

      //puts in arraylocal1 everything      
      sizetosend1=displs[numtasks-1]+recvcount[numtasks-1];
      arraylocal1 = new int[sizetosend1];
      
      MPI_Gatherv(&(sendarray1[i][1]), sendarray1[i][0],
            MPI_INT, arraylocal1, recvcount, displs, MPI_INT, i, MPI_COMM_WORLD);
   }
   

   
   
      

   
   
   


   relationlocal1->importArray(arraylocal1,sizetosend1, arity[0]);

   //Each join seq on (r1r2) and  r3
   a_local = join(relationlocal1,relationlocal2,list4,list3);
   
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
      //Write final runtime and relations data
      toc = clock();
      ofstream data(outputFile, ios::out | ios::app);
      data<< "Elapsed CPU (Join 2)= "
         << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
      data<<"Number of final relations :"
            <<a_local.relations->size()<<endl<<endl;
      data.close();
      
      char* outfile = "testoutput_dist";
      a_local.relations->write(outfile);
   }
   
   MPI_Finalize();     
}

