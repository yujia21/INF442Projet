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


//TO DO : coder les fonctions de hachage !!

int hash1(int n){
   return n%2;
}

int hash2(int n){
   return n%2;
}

int hash3(int n){
   return n%2;
}


//Return the number corrersponding to a given triple
//We use here 27 machines

int machineNumber(int a, int b, int c)
{
   return (a + b*3 + c*9);
}


int maxSize(const std::vector<std::vector<std::vector<int> > > vect)
{
   int maxSize = 0;
   for(int i=0; i<vect.size(); i++)
   {
      if(vect[i].size()>maxSize)
         maxSize = vect[i].size();
   }
   return maxSize;
}


void completeBigArray(const std::vector<std::vector<std::vector<int> > > relationToDistribute, int* bigArray, int maxSize)
{
   //DEBUG
   std::cout<<"completeBigArray called"<<std::endl<<std::endl;
   
  for(int i=0; i<27; i++)
      {                 
         
         //DEBUG
         std::cout<<"Machine "<< i << " done"<<std::endl;
         
         for(int j=0; j<relationToDistribute[i].size(); j++)                  //On remplit en mettant les couples bout a bout
         {
            bigArray[i*2*maxSize+2*j] = relationToDistribute[i][j][0];
            std::cout<<"First affectation done"<<std::endl;
            bigArray[i*2*maxSize+2*j+1] = relationToDistribute[i][j][1];
         }
                        
         for(int j=2*relationToDistribute[i].size(); j<2*maxSize; j++)       //On complete avec des -1
         {
            bigArray[i*2*maxSize+j] = -1;
         }
      } 
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
      
   //Create lists, import variable names : 
   //Accessible for all to be able to call join_seq on each
   
   static const string arr1[] = {"x1","x2"};
   static const string arr2[] = {"x2","x3"};
   static const string arr3[] = {"x3","x1"};
      
   vector<string> list1 (arr1, arr1 + sizeof(arr1) / sizeof(arr1[0]) );
   vector<string> list2 (arr2, arr2 + sizeof(arr2) / sizeof(arr2[0]) );   
   vector<string> list3 (arr3, arr3 + sizeof(arr3) / sizeof(arr3[0]) );  

   //For broadcast. Relations initialized on all, to be able to be accessed later
   Relation* relations;
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
   
      //Create relations1
      relations = new Relation(inputFile);
      
            
      //Write initial file details
      ofstream data(outputFile, ios::out | ios::app);      
      data<<"File: "<<inputFile<<endl
      <<"Number of procs : " << numtasks << endl     
      <<"Number of initial relations : "<< relations1->size() << endl;
      data.close();
      
   }
   
   //Waits for all to reach this point
   MPI_Barrier(MPI_COMM_WORLD); 
      
   int * bigArrayToDistribute1;
   int * bigArrayToDistribute2;
   int * bigArrayToDistribute3;
   
   int maxSize1 = 0;
   int maxSize2 = 0;
   int maxSize3 = 0;
   
   int * localArray1;
   int * localArray2;
   int * localArray3;
   
      
   if (taskid==root){
      
      int indexToPush;
      
      vector<vector<vector<int> > > relationsToDistribute1 (numtasks);
      vector<vector<vector<int> > > relationsToDistribute2 (numtasks);
      vector<vector<vector<int> > > relationsToDistribute3 (numtasks);
      
      //Create vector to Distribute
      for(int i=0; i<relations->size(); i++)
      {                                                                                          
         vector<int> curentTuple = relations->getindex(i);
         
         for(int j=0; j<3; j++)
         {
         indexToPush = machineNumber(hash1(curentTuple[0]),hash2(curentTuple[1]),j);
         relationsToDistribute1[indexToPush].push_back(curentTuple);
         
         indexToPush = machineNumber(j, hash2(curentTuple[0]), hash3(curentTuple[1]));
         relationsToDistribute2[indexToPush].push_back(curentTuple);
         
         indexToPush = machineNumber(hash1(curentTuple[1]), j, hash3(curentTuple[0]));
         relationsToDistribute3[indexToPush].push_back(curentTuple);
         
         }
      }
      
      for(int k=0; k<27; k++)
      {
         cout<<"For machine " << k << " : " << relationsToDistribute2[k].size() << endl;
      }
            
      //On supprime les relations lues 
      relations->~Relation();
      
      //Calcul les tailles max
      int maxSize1 = maxSize(relationsToDistribute1);
      int maxSize2 = maxSize(relationsToDistribute2);
      int maxSize3 = maxSize(relationsToDistribute3);
      
      //DEBUG
      cout<<"MaxSizes Read"<<endl;
      cout<<"maxSize1 = " << maxSize1<<endl;
      cout<<"maxSize2 = " << maxSize2<<endl;
      cout<<"maxSize3 = " << maxSize3<<endl;
      cout<<endl;
      
      
      //Allocation de la taille des tableaux a distribuer
      bigArrayToDistribute1 = new int[2*numtasks*maxSize1];
      bigArrayToDistribute2 = new int[2*numtasks*maxSize2];
      bigArrayToDistribute3 = new int[2*numtasks*maxSize3];
      
      //On remplit les tableaux a partir des relations a distribuer      
      completeBigArray(relationsToDistribute1, bigArrayToDistribute1, maxSize1);
      
      //DEBUG
      cout<<"Big Arrays number1 Completed"<<endl<<endl;
      
      //DEBUG
      /*
      for(int k=0; k<numtasks*maxSize1; k++)
      {
         cout<< bigArrayToDistribute1[2*k]<< " "<< bigArrayToDistribute1[2*k+1];
         cout<<endl;
      }
      */
      cout<<endl; 
      
      completeBigArray(relationsToDistribute2, bigArrayToDistribute2, maxSize2);
      
            
      //DEBUG
      cout<<"Big Arrays number2 Completed"<<endl<<endl;
      
      completeBigArray(relationsToDistribute3, bigArrayToDistribute3, maxSize3);
      
      //DEBUG
      cout<<"Big Arrays number3 Completed"<<endl<<endl;
      
      //DEBUG
      cout<<"Big Arrays Completed"<<endl<<endl;
      
      vector<vector<vector<int> > > ().swap(relationsToDistribute1);
      vector<vector<vector<int> > > ().swap(relationsToDistribute2);
      vector<vector<vector<int> > > ().swap(relationsToDistribute3);  
      
      //DEBUG
      cout<<"Garbage done"<<endl<<endl;    
                
   }
   
   MPI_Bcast(&maxSize1, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&maxSize2, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&maxSize3, 1, MPI_INT, root, MPI_COMM_WORLD);
   
   //Allocation m2moire des tableaux locaux
   localArray1 = new int[2*maxSize1];
   localArray2 = new int[2*maxSize2];
   localArray3 = new int[2*maxSize3];
   
   MPI_Scatter(bigArrayToDistribute1, maxSize1, MPI_INT, localArray1, maxSize1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(bigArrayToDistribute2, maxSize2, MPI_INT, localArray2, maxSize2, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(bigArrayToDistribute3, maxSize3, MPI_INT, localArray3, maxSize3, MPI_INT, root, MPI_COMM_WORLD);
   
   delete[] bigArrayToDistribute1;
   delete[] bigArrayToDistribute2;
   delete[] bigArrayToDistribute3;
   
   
   relations1->importArray(localArray1, maxSize1, 2);
   relations2->importArray(localArray2, maxSize2, 2);
   relations3->importArray(localArray3, maxSize3, 2);
   
   delete[] localArray1;
   delete[] localArray2;
   delete[] localArray3;   
   
   Relation::Atom a1 = join(relations1,relations2,list1,list2);
   
   //Create third atom
   Relation::Atom* a2 = new Relation::Atom(relations3, list3);
   
   //Call join   
   Relation::Atom a3 = join(&(a1),a2); 
      
   //Creation du tableau de triangles local
   int localTrianglesArraySize = (a3.relations)->size();   
   int* localTrianglesArray = new int[localTrianglesArraySize];   
   (a3.relations)->toArray(localTrianglesArraySize, localTrianglesArray);
   
   //Creation du tableau des tailles des tableaux de triangles
   int* recvCount = new int[numtasks];          
   MPI_Gather(&localTrianglesArraySize, 1, MPI_INT, recvCount, 1, MPI_INT, root, MPI_COMM_WORLD);
   
   int* displs = new int[numtasks];
   displs[0] = 0;
      
   for (int j = 1; j < numtasks; j++){
      displs[j]=displs[j-1]+recvCount[j-1];         
   }
    
   //Creation du tableau de triangles global
   int* globalTrianglesArray;
   int globalTriangleArraySize =0;
   
   //Allocation memoire du tableau de triangles global (seulement pour le root)
   if(taskid == root){
      globalTriangleArraySize = displs[numtasks-1]+recvCount[numtasks-1];
      globalTrianglesArray = new int[globalTriangleArraySize];
   }
         
   MPI_Gatherv(localTrianglesArray, numtasks, MPI_INT, globalTrianglesArray, recvCount, displs, MPI_INT, root, MPI_COMM_WORLD);
   
   delete[] localTrianglesArray;
            
   Relation* triangles;
   
   if (taskid == root){
   triangles->importArray(globalTrianglesArray, globalTriangleArraySize, 3);
   triangles->toPrint();
   }
   
   MPI_Finalize();     
}


/*
yay
int main(argv blabla)
argv = file name of relations

init mpi

m = m1*m2*m3

define hash functions : h1 h2 h3 
(structure that takes in int)

function that converts from machine # (a, b, c) <-> n
(0 1 2) multiply by 1 3 9

if root:
   construct 3 atoms
   construct 3 <vector <vector <vector <int > > > for r1, r2, r3
   for r1
      for vector<int> (t,t') in relation
         put into machine n1/2/3 where n = (h1(t), h2(t'), 1/2/3)
   do similar for r2 and r3
         
on all
   from vector<vector<int>> make relations
   join_seq
   create <vector<vector<int>> to send back

gather

if root:
   insert all vector<vector<int>> back into finalrelation


part 1 : 
m1 = m2 = m3

part 2:
diff m1, m2, m3

*/
