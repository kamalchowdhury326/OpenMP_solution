#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn
module load intel

icx -fopenmp -Wall -O life_openmp_2d.c -o life_openmp_2d 

# gcc -fopenmp -Wall -O life_openmp_2d.c -o life_openmp_2d


./life_openmp_2d 5000 5000 1 1 /scratch/ualmkc001/
./life_openmp_2d 5000 5000 2 1 /scratch/ualmkc001/
./life_openmp_2d 5000 5000 2 2 /scratch/ualmkc001/
./life_openmp_2d 5000 5000 4 2 /scratch/ualmkc001/
./life_openmp_2d 5000 5000 5 2 /scratch/ualmkc001/
./life_openmp_2d 5000 5000 4 4 /scratch/ualmkc001/
./life_openmp_2d 5000 5000 10 2 /scratch/ualmkc001/

cd /scratch/ualmkc001

diff output_2d.5000.5000.1.txt output_2d.5000.5000.2.txt 
diff output_2d.5000.5000.2.txt output_2d.5000.5000.4.txt 
diff output_2d.5000.5000.2.txt output_2d.5000.5000.8.txt 
diff output_2d.5000.5000.2.txt output_2d.5000.5000.10.txt 
diff output_2d.5000.5000.2.txt output_2d.5000.5000.16.txt 
diff output_2d.5000.5000.16.txt output_2d.5000.5000.20.txt 
