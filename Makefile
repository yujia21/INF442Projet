CC = g++
MPICC = mpic++

Relation.o: Relation.cpp Relation.hpp
	$(CC) -c Relation.cpp

Join_seq.o: Join_seq.cpp Join_seq.hpp
	$(CC) -c Join_seq.cpp
        
Join_dist.o: Join_dist.cpp Join_dist.hpp Join_seq.hpp Relation.hpp
	$(MPICC) -c Join_dist.cpp           
        
task1.o: task1.cpp Relation.hpp
	$(CC) -c task1.cpp

task2.o: task2.cpp Relation.hpp
	$(CC) -c task2.cpp

task3.o: task3.cpp Relation.hpp
	$(CC) -c task3.cpp

task4.o: task4.cpp Relation.hpp Join_seq.hpp
	$(CC) -c task4.cpp
        
task5.o: task5.cpp Relation.hpp Join_seq.hpp Join_dist.hpp
	$(MPICC) -c task5.cpp        

task7.o: task7.cpp Relation.hpp Join_seq.hpp Join_dist.hpp
	$(MPICC) -c task7.cpp        

task8.o: task8.cpp Relation.hpp Join_seq.hpp
	$(MPICC) -c task8.cpp
        
task1: Relation.o task1.o
	$(CC) Relation.o task1.o -o task1

task2: Relation.o task2.o
	$(CC) Relation.o task2.o -o task2

task3: Relation.o Join_seq.o task3.o
	$(CC) Relation.o Join_seq.o task3.o -o task3

task4: Relation.o Join_seq.o task4.o
	$(CC) Relation.o Join_seq.o task4.o -o task4
        
task5: Relation.o Join_seq.o Join_dist.o task5.o
	$(MPICC) Relation.o Join_seq.o Join_dist.o task5.o -o task5

task7: Relation.o Join_seq.o Join_dist.o task7.o
	$(MPICC) Relation.o Join_seq.o Join_dist.o task7.o -o task7
        
task8: Relation.o Join_seq.o task8.o
	$(MPICC) Relation.o Join_seq.o task8.o -o task8

clean: 
	rm -f task1
	rm -f task2
	rm -f task3
	rm -f task4
	rm -f task5                
	rm -f task7
	rm -f task8        
	rm -f *.o

cleantestoutput:
	rm -f testoutput*

cleanrundata:
	rm -f run_data*
