#include "Relation.hpp"
#include "Join_seq.hpp"

#include "mpi.h" //mpi

#include <iostream>  //std::cout
#include <fstream>  //fstream
#include <string>  //std::string
#include <vector>  //std::vector

#include <time.h> //track runtime
#include <sys/time.h>  //track runtime

#include <math.h> //floor

//hash function
/*
   return an integer between 0 and M-1
   A is a hashing parameter
*/
int hash(const int M, const double A, const int n){
   return (int)floor(M*(A*n-floor(A*n)));
}

//Return the machine number corrersponding to a given triple
int machineNumber(const int m1,const int m2,const int m3, const int M1_max, const int M2_max)
{
   return (m1 + m2*M1_max + m3*M1_max*M2_max);
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


void completeBigArray(const std::vector<std::vector<std::vector<int> > > relationToDistribute, int* bigArray, int maxSize, int numtasks)
{
   
  for(int i=0; i<numtasks; i++)
      {                               
         
         for(int j=0; j<relationToDistribute[i].size(); j++)                  //On remplit en mettant les couples bout a bout
         {
            bigArray[i*2*maxSize+2*j] = relationToDistribute[i][j][0];
            
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
   int M1=0,   M2=0,    M3=0;
   
   MPI_Status status;
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
   
   //choose M1 M2 and M3 such that M1*M2*M3 = numtasks
   if(taskid==root)
   {
   
      while(numtasks != M1*M2*M3)
         {
            cout << "Enter M1 : ";
            cin >> M1;
         
            cout << "Enter M2 : ";
            cin >> M2;
         
            cout << "Enter M3 : ";
            cin >> M3;
         }   
   }
   
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
   Relation* relation;
   Relation* localRelation1 = new Relation();
   Relation* localRelation2 = new Relation();
   Relation* localRelation3 = new Relation();      
   
   //To write
   char* outputFile;
   outputFile = "run_data_task8.txt";
   
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
      relation = new Relation(inputFile);
      
            
      //Write initial file details
      ofstream data(outputFile, ios::out | ios::app);      
      data<<"File: "<<inputFile<<endl
      <<"Number of procs : " << numtasks << endl     
      <<"Number of initial relations : "<< relation->size() << endl;
      data.close();
      
   }
   
   //Waits for all to reach this point
   MPI_Barrier(MPI_COMM_WORLD); 
      
   int * bigArrayToDistribute1;
   int * bigArrayToDistribute2;
   int * bigArrayToDistribute3;
   
   int maxSize1;
   int maxSize2;
   int maxSize3;
   
   int * localArray1 = 0;
   int * localArray2 = 0;
   int * localArray3 = 0;
   
   
   if (taskid==root){
               
      int indexToPush;
      
      vector<vector<vector<int> > > relationToDistribute1 (numtasks);
      vector<vector<vector<int> > > relationToDistribute2 (numtasks);
      vector<vector<vector<int> > > relationToDistribute3 (numtasks);
      
      
      double a1=0.3, a2=0.4,  a3=0.5;
      
      //Create vector to Distribute
      for(int i=0; i<relation->size(); i++)
      {                                                                                          
         vector<int> curentTuple = relation->getindex(i);
         
         for(int j=0; j<M3; j++){
         
            indexToPush = machineNumber(hash(M1,a1,curentTuple[0]),hash(M2,a2,curentTuple[1]),j,M1,M2);         
            relationToDistribute1[indexToPush].push_back(curentTuple);
         }
         
         for(int j=0; j<M1; j++){  
                
            indexToPush = machineNumber(j, hash(M2,a2,curentTuple[0]), hash(M3,a3,curentTuple[1]),M1,M2);         
            relationToDistribute2[indexToPush].push_back(curentTuple);
         }
         
         for(int j=0; j<M2; j++){
            
            indexToPush = machineNumber(hash(M1,a1,curentTuple[1]), j, hash(M3,a3,curentTuple[0]),M1,M2);         
            relationToDistribute3[indexToPush].push_back(curentTuple);         
         }
      }
      
      //DEBUG
      //Affichage des donnees recues par chaque machine
      for(int k=0; k<numtasks; k++)
      {
         if(k<10)
         cout<<"ID  ";
         else
         cout<<"ID ";
         
         cout  << k << " : " 
               <<relationToDistribute1[k].size() <<"  " 
               <<relationToDistribute2[k].size() <<"  " 
               <<relationToDistribute3[k].size() 
               <<endl;
      }
            
      //On supprime les relations lues 
      relation->~Relation();
      
      //Calcul les tailles max
      maxSize1 = maxSize(relationToDistribute1);
      maxSize2 = maxSize(relationToDistribute2);
      maxSize3 = maxSize(relationToDistribute3);
      
      //DEBUG
      cout<<endl;
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
      completeBigArray(relationToDistribute1, bigArrayToDistribute1, maxSize1, numtasks);                 
      completeBigArray(relationToDistribute2, bigArrayToDistribute2, maxSize2, numtasks);      
      completeBigArray(relationToDistribute3, bigArrayToDistribute3, maxSize3, numtasks);
      
      //Liberation de la mémoire      
      vector<vector<vector<int> > > ().swap(relationToDistribute1);
      vector<vector<vector<int> > > ().swap(relationToDistribute2);
      vector<vector<vector<int> > > ().swap(relationToDistribute3);                 
                
   }
            
   MPI_Bcast(&maxSize1, 1, MPI_INT, root, MPI_COMM_WORLD); 
   MPI_Bcast(&maxSize2, 1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Bcast(&maxSize3, 1, MPI_INT, root, MPI_COMM_WORLD);
               
   //Allocation m2moire des tableaux locaux
   localArray1 = new int[2*maxSize1];
   localArray2 = new int[2*maxSize2];
   localArray3 = new int[2*maxSize3];
   
   MPI_Scatter(bigArrayToDistribute1, 2*maxSize1, MPI_INT, localArray1, 2*maxSize1, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(bigArrayToDistribute2, 2*maxSize2, MPI_INT, localArray2, 2*maxSize2, MPI_INT, root, MPI_COMM_WORLD);
   MPI_Scatter(bigArrayToDistribute3, 2*maxSize3, MPI_INT, localArray3, 2*maxSize3, MPI_INT, root, MPI_COMM_WORLD);
   
   MPI_Barrier(MPI_COMM_WORLD);
   
   if(taskid == root)
   {      
      delete[] bigArrayToDistribute1;
      delete[] bigArrayToDistribute2;
      delete[] bigArrayToDistribute3;
   }
   
   localRelation1->importArray(localArray1, 2*maxSize1, 2);     
   localRelation2->importArray(localArray2, 2*maxSize2, 2);
   localRelation3->importArray(localArray3, 2*maxSize3, 2);
       
   delete[] localArray1;
   delete[] localArray2;
   delete[] localArray3;
   
   Relation::Atom localAtome1 = join(localRelation1,localRelation2,list1,list2);
      
   //Create third atom
   Relation::Atom* localAtome2 = new Relation::Atom(localRelation3, list3);
   
   //Call join   
   Relation::Atom localAtome3 = join(&(localAtome1),localAtome2); 
      
   //Creation du tableau de triangles local
   int localTrianglesArraySize = 3*((localAtome3.relations)->size());
       
   int* localTrianglesArray = new int[localTrianglesArraySize];   
   (localAtome3.relations)->toArray(localTrianglesArraySize, localTrianglesArray);
   
   MPI_Barrier(MPI_COMM_WORLD);
            
   //Creation du tableau des tailles des tableaux de triangles
   int* recvCount = new int[numtasks];
             
   MPI_Gather(&localTrianglesArraySize, 1, MPI_INT, recvCount, 1, MPI_INT, root, MPI_COMM_WORLD);
         
   int* displs = new int[numtasks];
   
   if(taskid == root)
   {
      displs[0] = 0;
      
      for (int j = 1; j < numtasks; j++)
         displs[j]=displs[j-1]+recvCount[j-1];         
       
   }
    
   //Creation du tableau de triangles global
   int* globalTrianglesArray;
   int globalTriangleArraySize;
   
   //Allocation memoire du tableau de triangles global (seulement pour le root)
   if(taskid == root){
      globalTriangleArraySize = displs[numtasks-1]+recvCount[numtasks-1];
      globalTrianglesArray = new int[globalTriangleArraySize];
   }
   
   MPI_Barrier(MPI_COMM_WORLD);    
         
   MPI_Gatherv(localTrianglesArray, localTrianglesArraySize, MPI_INT, globalTrianglesArray, recvCount, displs, MPI_INT, root, MPI_COMM_WORLD);
       
   MPI_Barrier(MPI_COMM_WORLD);
   
   delete[] localTrianglesArray;
            
   Relation* triangles = new Relation();
   
   if (taskid == root){    
   triangles->importArray(globalTrianglesArray, globalTriangleArraySize, 3);
   delete[] globalTrianglesArray;
   }
   
   
   //Return atom of root
   if (taskid == root){
      //Write final runtime and relations data
      toc = clock();
      ofstream data(outputFile, ios::out | ios::app);
      data<< "Elapsed CPU = "
         << (toc - tic) / ((float)(CLOCKS_PER_SEC)) << "s" << endl; 
      data<<"Number of final relations :"
            <<triangles->size()<<endl<<endl;
      data.close();
      
      char* outfile = "testoutput_task8";
      triangles->write(outfile);
   }
   
   
   MPI_Finalize();     
}
