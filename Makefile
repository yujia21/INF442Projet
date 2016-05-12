CC = g++
MPICC = mpic++
MPIRUN = mpiboost
NP = -np 8

Relation.o: Relation.cpp Relation.hpp
	$(CC) -c Relation.cpp

Join_seq.o: Join_seq.cpp Join_seq.hpp
	$(CC) -c Join_seq.cpp
        
task1.o: task1.cpp Relation.hpp
	$(CC) -c task1.cpp

task2.o: task2.cpp Relation.hpp
	$(CC) -c task2.cpp

task3.o: task3.cpp Relation.hpp
	$(CC) -c task3.cpp

task4.o: task4.cpp Relation.hpp
	$(CC) -c task4.cpp
        
task5.exe: task5.cpp Relation.hpp
	$(MPICC) -c task5.cpp        

task1: Relation.o task1.o
	$(CC) Relation.o task1.o -o task1

task2: Relation.o task2.o
	$(CC) Relation.o task2.o -o task2

task3: Relation.o Join_seq.o task3.o
	$(CC) Relation.o Join_seq.o task3.o -o task3

task4: Relation.o Join_seq.o task4.o
	$(CC) Relation.o Join_seq.o task4.o -o task4
        
task5: Relation.o Join_dist.exe task4.o
	$(MPIRUN) $(NP) Join_dist.exe Relation.o task5.o -o task5

clean: 
	rm -f task1
	rm -f task2
	rm -f task3
	rm -f task4        
	rm -f *.o

cleantestoutput:
	rm -f testoutput*
