#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn
module load intel

icx -fopenmp -Wall -O hw3.c -o hw3 

# gcc -fopenmp -Wall -O hw3.c -o hw3


./hw3 5000 5000 1 /scratch/ualmkc001/
./hw3 5000 5000 2 /scratch/ualmkc001/
./hw3 5000 5000 4 /scratch/ualmkc001/
./hw3 5000 5000 8 /scratch/ualmkc001/
./hw3 5000 5000 10 /scratch/ualmkc001/
./hw3 5000 5000 16 /scratch/ualmkc001/
./hw3 5000 5000 20 /scratch/ualmkc001/

cd /scratch/ualmkc001

diff output.5000.5000.1.txt output.5000.5000.2.txt 
diff output.5000.5000.2.txt output.5000.5000.4.txt 
diff output.5000.5000.2.txt output.5000.5000.8.txt 
diff output.5000.5000.2.txt output.5000.5000.10.txt 
diff output.5000.5000.2.txt output.5000.5000.16.txt 
diff output.5000.5000.2.txt output.5000.5000.20.txt 
