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