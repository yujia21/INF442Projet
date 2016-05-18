#include "Relation.hpp"
#include "Join_seq.hpp"

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

#include <math.h> //floor

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
   return (int)floor(m*(a*n-floor(a*n)));
}

int* relationToDistArray(Relation* r, int numtasks, int index){
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

void joinDist(Relation* r1, Relation* r2, int root){
}

int main (int argc, char **argv) {
   using namespace std;
   //init mpi
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
   //accessible for all to be able to call join_seq
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};   
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );  

   //For broadcast   
   //int sizes[2]; 
   int arity[2];    
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
      <<"Number of initial relations : "<<relations1->size()<<endl;
      data.close();
      
      //To send
      //sizes[0] = relations1->size();
      //sizes[1] = relations2->size();
      arity[0] = relations1->arity();
      arity[1] = relations1->arity();      
   }
   // Send sizes and arity of relations
   //MPI_Bcast(&sizes, 2, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&arity, 2, MPI_INT, root, MPI_COMM_WORLD);
   
   // To scatter and gather
   int* array1;
   int* array2; 
   int sizetosend1;
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
   //Broadcast size to send
   MPI_Bcast(&sizetosend1, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&sizetosend2, 1, MPI_INT, root, MPI_COMM_WORLD);   
   
   //Initialize receiving arrays
   int arraylocal1[sizetosend1];
   int arraylocal2[sizetosend2];
   
   // Send
   MPI_Scatter(array1,sizetosend1,MPI_INT, &arraylocal1, sizetosend1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(array2,sizetosend2,MPI_INT, &arraylocal2, sizetosend2, MPI_INT, root, MPI_COMM_WORLD);   
   
   //cout<<"scattered"<<endl; 
   
   //for (int i = 0; i < sizetosend1 ; i++){
   //   cout<<arraylocal1[i]<<" ";
   //}
   //cout<<endl;
   
   //for (int i = 0; i < sizetosend2 ; i++){
   //   cout<<arraylocal2[i]<<" ";
   //}
   //cout<<endl;
   
   //Construct relation from array<array<int>> received
   Relation* relationlocal1 = new Relation();
   relationlocal1->importArray(arraylocal1,sizetosend1, arity[0]);
   Relation* relationlocal2 = new Relation();
   relationlocal2->importArray(arraylocal2,sizetosend2, arity[1]);
   cout<<"imported arrays"<<relationlocal1->arity()<<" "<<relationlocal2->arity()<<endl;
   
   //Call join seq
   Relation::Atom a_local = join(relationlocal1,relationlocal2,list1,list2);
   cout<<taskid<<" joined"<<endl;
   
   //Create array to send back to root
   int finalarraylocal[sizetosend1*sizetosend2];
   a_local.relations->toArray(sizetosend1*sizetosend2, finalarraylocal);

   //printing in ToArray and outside gives very diff results??
   
   if(taskid==1){
      for (int i = 0; i<sizetosend1*sizetosend2; i++){
         cout<<finalarraylocal[i]<<" ";
      }
      cout<<endl;
   }

   
   int finalarray[sizetosend1*sizetosend2*numtasks];
   
   // Gather 
   MPI_Gather(&finalarraylocal, sizetosend1*sizetosend2, MPI_INT, &finalarray, sizetosend1*sizetosend2, MPI_INT,
         root, MPI_COMM_WORLD);
   


   if (taskid == root){
      for (int i = 0; i<sizetosend1*sizetosend2*numtasks; i++){
         cout<<finalarray[i]<<" ";
      }
      cout<<endl;
      
      // Take array of array of array of int and put back in new relation
      cout<<"Size before : "<<a_local.relations->size()<<endl;
      a_local.relations->importArray(finalarray, sizetosend1*sizetosend2*numtasks,
            arity[0]+arity[1]-ncommonvar);
      cout<<"Size after : "<<a_local.relations->size()<<endl;
      //Write intermediate runtime and relations data
      toc = clock();
      ofstream data("run_data_dist.txt", ios::out | ios::app);
      data<< "Elapsed CPU (Join 1)= "
         << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
      data<<"Number of intermediate relations : "<<a_local.relations->size()<<endl;
      data.close();
      
   }
   MPI_Finalize();     
}
