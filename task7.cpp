/*
int main(argv blabla)
argv = file name of relations

init mpi
if root 
   read files, construct relations r1 r2 r3
   search commonvar for r1, r2  (for example call orderTuple and take first var) 
   search commonvar for r3 and (r1 or r2)
   construct vector<vector<vector<int>>> to distribute r1, r2, r3 :
   using distributeTuple. Take note of index to use

scatter

on all
   construct relation from vector<vector<int>> received
   call join seq
   extract vector<vector<int>> from atom

   construct vector<vector<vector<int>>> to distribute by distributeTuple
   
   receive (r1r2)
   join seq on (r1r2) and r3
   
gather

if root
   take vector<vector<vector<int>>> and put back in new relation
   change relation of atom that came from join seq

return atom*/