CC = g++

Relation.o: Relation.cpp Relation.hpp
	$(CC) -c Relation.cpp
        
task1.o: task1.cpp Relation.hpp
	$(CC) -c task1.cpp

task2.o: task2.cpp Relation.hpp
	$(CC) -c task2.cpp
        
task1: Relation.o task1.o
	$(CC) Relation.o task1.o -o task1

task2: Relation.o task2.o
	$(CC) Relation.o task2.o -o task2

clean: 
	rm -f task1
	rm -f task2
	rm -f *.o

cleantestoutput:
	rm -f testoutput*
