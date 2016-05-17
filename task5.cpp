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
   return (int)floor(m*(a*n-floor(a*n)));
}

int*** relationToDistArray(Relation* r, int numtasks, int index){
   std::vector<std::vector<std::vector<int> > > arranged; 
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
   const int maxsize = max;
   const int arity = r->arity();
   
   int*** array = 0;
   array = new int**[numtasks];

   for (int i = 0; i < numtasks; i++){
      array[i] = new int*[max];
      for (int j = 0; j < max; j++){
         array[i][j] = new int[r->arity()];
         if (! arranged[i][j].empty()){
            std::copy(arranged[i][j].begin(), arranged[i][j].end(), array[i][j]);
         }
      }
   }
   return &(array[0][0][0]); //TO DO : FIX THIS
   //http://stackoverflow.com/questions/8617683/return-a-2d-array-from-a-function
}

int main (int argc, char **argv) {
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
   
   // To send
   int sizes[4]; // 0 is for intermediate, i is for i
   int*** array1;
   int*** array2; 

   // To receive
   int** arraylocal1;
   int** arraylocal2;   
  
   //Create lists, import variable names : 
   //accessible for all to be able to call join_seq
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};   
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );  
   
   if (taskid == root){
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
      Relation* relations1 = new Relation(inputFile);
      Relation* relations2 = new Relation(inputFile);   
      Relation* relations3 = new Relation(inputFile); 
      sizes[0] = 0;
      sizes[1] = relations1->size();
      sizes[2] = relations2->size();
      sizes[3] = relations3->size(); 
      
      //Write initial relations
      ofstream data("run_data_dist.txt", ios::out | ios::app);
      data<<"File: "<<inputFile<<endl
      <<"Number of initial relations : "<<relations1->size()<<endl;
      data.close();

      //Get common vars
      vector<vector<int> > temp = commonOrder(list1,list2);
      int index1 = temp[0][0]; //Comparison column
      int index2 = temp[1][0];
      
      //Construct array of array of array
      array1 = relationToDistArray(relations1, numtasks, index1);
      array2 = relationToDistArray(relations2, numtasks, index2);      
   }
   
   // Send sizes of relations
   MPI_Bcast(&sizes, 4, MPI_INT, root, MPI_COMM_WORLD);
   
   // Scatter
   MPI_Scatter(&array1,sizes[1],MPI_INT, &arraylocal1, sizes[1], MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(&array2,sizes[2],MPI_INT, &arraylocal2, sizes[2], MPI_INT, root, MPI_COMM_WORLD);   

   //Construct relation from array<array<int>> received
   Relation* relationlocal1;
   relationlocal1->importArray(arraylocal1);
   Relation* relationlocal2;
   relationlocal2->importArray(arraylocal2);
   
   //Call join seq
   Relation::Atom a_local = join(relationlocal1,relationlocal2,list1,list2);
   
   //Create array to send back to root
   int** finalarraylocal;
   finalarraylocal = a_local.relations->toArray();
   int*** finalarray;
   
   //HOW TO DO LOC N
   int loc_n = 1;
   
   // Gather 
   MPI_Gather(&finalarraylocal, loc_n, MPI_INT, &finalarray, loc_n, MPI_INT,
         root, MPI_COMM_WORLD);

   if (taskid == root){
      // Take array of array of array of int and put back in new relation
      Relation* finalrelation;
      for (int i = 0; i < numtasks; i++){
         finalrelation->importArray(finalarray[i]);
      }

      //Write intermediate runtime and relations data
      //WHY NEED TO REINITIALIZE EVERYTHING?
      toc = clock();
      ofstream data("run_data_dist.txt", ios::out | ios::app);
      data<< "Elapsed CPU (Join 1)= "
         << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
      data<<"Number of intermediate relations : "<<finalrelation->size()<<endl;
      data.close();
      /*
      //Get column of comparison
      vector<vector<int> > temp = commonOrder(a_local.variables,list3);
      int index1 = temp[0][0]; //Comparison column
      int index2 = temp[1][0];

      //Construct array of array of array for r3 and for intermediate
      array1 = relationToDistArray(finalrelation, numtasks, index1);
      array2 = relationToDistArray(relations3, numtasks, index2);
      */
   }
}
