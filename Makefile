CC = g++

Relation.o: Relation.cpp Relation.hpp
	$(CC) -c Relation.cpp
        
task1.o: task1.cpp Relation.hpp
	$(CC) -c task1.cpp
        
task1: Relation.o task1.o
	$(CC) Relation.o task1.o -o task1

clean: 
	rm -f task1
	rm -f *.o
