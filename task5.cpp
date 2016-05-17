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

#include <math> //floor

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
   return floor(m*(a*n-floor(a*n)));
}

int*** relationToArray(Relation r, int numtasks, int index){
   int ** array = 0;
   array = new int*[numtasks];
   for (int i = 0; i < this->size(); i++){
      std::vector<int> current = this->getindex(i);
      //int n = distributeTupleHash(current, index, numtasks); //decomment for task6
      //int n = distributeTuple(current, index, numtasks);      
      std::copy(this->getindex(i).begin(), this->getindex(i).end(), array[n]);
   }
   return array;
}

int main (int argc, char **argv) {
   
   /* TO DO
?? do squares and etc according to argc

int main(argv blabla)
argv[0] = file name of relations
*/
   //init mpi
   const int root = 0;
   
   int numtasks, taskid;
   MPI_Status status;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
   
   /*if root 
   read files, construct relations r1 and r2
   call toArray o distribute for r1 and r2
   */   
   if (taskid == root){
      //Read file, create relations
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
      
      char* infile = argv[0];
      Relation* relations1 = new Relation(inputFile);
      Relation* relations2 = new Relation(inputFile);   
      Relation* relations3 = new Relation(inputFile);      
      
      ofstream data("run_data_dist.txt", ios::out | ios::app);
      data<<"File: "<<infile<<endl
      <<"Number of initial relations : "<<relations1->size()<<endl;
      data.close();
   
      //Create lists, import variable names
      static const string arr1[] = {"x1","x2"};
      static const string arr2[] = {"x2","x3"};
      static const string arr3[] = {"x3","x1"};   
      vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
      vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
      vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );   

      //Get common vars
      vector<vector<int> > temp = commonOrder(list1,list2);
      vector<int> order1 = temp[0];
      vector<int> order2 = temp[1];
      int ncommonvar = temp[2][0];
      int index1 = order1[0]; //comparison column
      int index2 = order2[0];
      
      //Start clock
      clock_t tic, toc;
      struct timeval theTV;
      struct timezone theTZ;
      gettimeofday(&theTV, &theTZ);
      srandom(theTV.tv_usec);
      tic = clock();
      
      //construct array of array of array
      int*** array1 = relationToArray(relations1, numtasks, index1);
      int*** array2 = relationToArray(relations2, numtasks, index2);      
   }
   
   //scatter
   //how to know the number of relations in each array[n]
   int** arraylocal1;
   int** arraylocal2;   
   int loc_n_1;
   int loc_n_2;
   MPI_Scatter(&array1,loc_n_1,MPI_INT, &arraylocal1, loc_n_1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(&array2,loc_n_2,MPI_INT, &arraylocal2, loc_n_2, MPI_INT, root, MPI_COMM_WORLD);   

      
/*
on all
   construct relation from array<array<int>> received
   call join seq
   extract vector<vector<int>> from atom
*/
   Relation* relationlocal1 = new Relation(arraylocal1);
   Relation* relationlocal2 = new Relation(arraylocal2);
   //send order 1 and 2 via scatter? should all already have access?
   
//gather
   MPI_Gather(&finalarraylocal, loc_n, MPI_INT, &finalarray, loc_n, MPI_INT,
         root, MPI_COMM_WORLD);

/*
if root
   take vector<vector<vector<int>>> and put back in new relation
   change relation of atom that came from join seq
*/
   if (taskid == root){
      Relation* finalrelation = new Relation(finalarray);
   }
   
//return atom

}
