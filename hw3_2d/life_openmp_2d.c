/* 
   Name:Md Kamal Hossain Chowdhury 
    
   Homework #: 3 Design and implementation of an efficient multithreaded version of the "Game of Life" program using OpenMP
  To compile: gcc -fopenmp -Wall -O life_openmp_2d.c -o life_openmp_2d
              icx -qopenmp -wall life_openmp_2d.c -o life_openmp_2d_intel
  To execute:    ./life_openmp_2d 5000 5000 4 4  /scratch/ualmkc001/<== for 4x 4 threads
                 ./life_openmp_2d_intel 5000 5000 8 2   <== for 8x2 threads


   Use -DDEBUG1 for output at the start and end.
   Use -DDEBUG2 for output at each iteration.
   Use -DDEBUGC for correctness checking
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <omp.h>
#include <string.h>

#define DIES   0
#define ALIVE  1

char* filePath;
double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

int** allocarray(int P, int Q) {
  int i, *p, **a;

  p = (int *)malloc(P*Q*sizeof(int));
  a = (int **)malloc(P*sizeof(int*));
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q]; 

  return a;
}

void freearray(int **a) {
  free(&a[0][0]);
  free(a);
}
void swap(int ***a, int ***b) {
  int ** tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

void file_write(char* path, int **A, int m, int n,int iterations, int P){
   // Specify the full path where you want to create the file
    //const char *path = "/scratch/ualmkc001/filename.txt";
    const char* name = path;
    const char* extension = ".txt";
    char size[5];
    snprintf(size, 5, "%d", n);
    char iter[5];
    snprintf(iter, 5, "%d", iterations);
    char p_char[5];
    snprintf(p_char, 5, "%d", P);
    

    char* name_with_extension;
    name_with_extension = malloc(strlen(name)+1+25); /* make space for the new string (should check the return value ...) */
    strcpy(name_with_extension, name); /* copy name into the new var */
    strcat(name_with_extension,"output_2d");
    strcat(name_with_extension,".");
    strcat(name_with_extension,size);
    strcat(name_with_extension,".");
    strcat(name_with_extension,iter);
  
    strcat(name_with_extension,".");
    strcat(name_with_extension,p_char);

    strcat(name_with_extension, extension); /* add the extension */
    
    
    // Open file in write mode
    FILE *file = fopen(name_with_extension, "w");

    // Check if file creation is successful
    if (file == NULL) {
        printf("Failed to create the file.\n");
        return ;
    }
   
    for(int i=1;i<m+1;i++){
       for(int j=1;j<n+1;j++){
          // Write something to the file
         fprintf(file, " %d ",A[i][j]);
       }
       fprintf(file, "\n");
    }
    

    // Close the file
    fclose(file);
    printf("File created successfully at %s\n", name_with_extension);
    free(name_with_extension);
}
void printarray(int **a, int N, int k) {
  int i, j;
  printf("Life after %d iterations:\n", k) ;
  for (i = 1; i < N+1; i++) {
    for (j = 1; j< N+1; j++)
      printf("%d ", a[i][j]);
    printf("\n");
  }
  printf("\n");
}


int compute_serial(int **life, int **temp, int N) {
  int i, j, value, flag=0;

  for (i = 1; i < N+1; i++) {
    for (j = 1; j < N+1; j++) {
      /* find out the value of the current cell */
      value = life[i-1][j-1] + life[i-1][j] + life[i-1][j+1]
	+ life[i][j-1] + life[i][j+1]
	+ life[i+1][j-1] + life[i+1][j] + life[i+1][j+1] ;
      
      /* check if the cell dies or life is born */
      if (life[i][j]) { // cell was alive in the earlier iteration
	if (value < 2 || value > 3) {
	  temp[i][j] = DIES ;
	  flag++; // value changed 
	}
	else // value must be 2 or 3, so no need to check explicitly
	  temp[i][j] = ALIVE ; // no change
      } 
      else { // cell was dead in the earlier iteration
	if (value == 3) {
	  temp[i][j] = ALIVE;
	  flag++; // value changed 
	}
	else
	  temp[i][j] = DIES; // no change
      }
    }
  }

  return flag;
}


int compute(int **life, int **temp, int N, int P, int Q, int NTIMES) {
  int i, j, value;
  int flag=0;
  int k=0, res;
  
  #pragma omp parallel default(none) shared(k,N,P,Q,life,temp,NTIMES,res,flag) private(value,i,j) num_threads(P*Q) 
  //reduction(+:k)
    { 
      
      for (k = 0; k < NTIMES; ) {
        int tid = omp_get_thread_num();
        int local_flag=0;
        flag=0;
        //printf("k=%d\n",k);
        int p = tid / Q;
        int q = tid % Q;
        int myM = N / P;
        int istart = p * myM;
        int iend = istart + myM;
        if (p == P-1) iend = N;
        for (i=istart+1; i<iend+1; i++) {
          int myN = N / Q;
          int jstart = q * myN;
          int jend = jstart + myN;
          if (q == Q-1) jend = N;
          
            #ifdef DEBUG0
            printf("tid=%d p,q=[%d,%d]: i:{%d,%d} j:{%d,%d}\n", tid, p, q, istart, iend, jstart, jend);
            #endif
    // #pragma omp parallel for default(none) shared(life,temp,N,P,Q) private(j,flag,value) num_threads(Q)
      //{
            
          for (j=jstart+1; j<jend+1; j++) {
             
          /* find out the value of the current cell */
          value = life[i-1][j-1] + life[i-1][j] + life[i-1][j+1]+ life[i][j-1] + life[i][j+1]+ life[i+1][j-1] + life[i+1][j] + life[i+1][j+1] ;
             
          /* check if the cell dies or life is born */
          if (life[i][j]) { // cell was alive in the earlier iteration
            if (value < 2 || value > 3) {
              temp[i][j] = DIES ;
              local_flag+=1; // value changed 
            }
            else // value must be 2 or 3, so no need to check explicitly
              temp[i][j] = ALIVE ; // no change
              } 
          else { // cell was dead in the earlier iteration
            if (value == 3) {
              temp[i][j] = ALIVE;
              local_flag+=1; // value changed 
            }
            else
              temp[i][j] = DIES; // no change
              }
        }
        
      }
       #pragma omp critical
      {
        flag+=local_flag;
      }
      #pragma omp barrier

       #pragma omp single
      {
        res=k+1;
        if(flag==0) 
        {
          # ifdef DEBUG2
          printf("tid=%d flag=%d k=%d\n",tid,flag,k);
          # endif
          k=NTIMES;
          //continue;     
        }
      }   
     // printf("tid=%d line=%d\n",tid,__LINE__);
      
      
     
      # ifdef DEBUG2
        //printf("tid=%d local_flag=%d flag=%d k=%d\n",tid,local_flag, flag,k);
        //if(tid==0){
           #pragma omp single
            printf("No. of cells whose value changed in iteration %d = %d\n",k+1,flag) ;
       // }

      # endif
       
      //printf("tid=%d local_flag=%d flag=%d\n",tid,local_flag, flag);

      
      
      
      #pragma omp single
      {
        swap(&life,&temp);
        k=k+1;
      }
        
     
       
      #ifdef DEBUG2
          #pragma omp single
          printarray(life, N, k+1);   
      #endif
      
      
   }

 }
    
    

    return res;
  
}


int main(int argc, char **argv) {
  int N, NTIMES, **life=NULL, **temp=NULL;
  int i, j, P,Q;
  //,flag;
  int k;
  double t1, t2;
  
  
  if (argc != 6) {
      printf("Usage: %s <N> <Generations> <P> <Q><filePath>\n", argv[0]);
      exit(-1);
    }
    
  
  N = atoi(argv[1]);
  NTIMES = atoi(argv[2]);
  P = atoi(argv[3]);
  Q = atoi(argv[4]); 
  filePath=argv[5];
  k=NTIMES;
  /* Allocate memory for both arrays */
  life = allocarray(N+2,N+2);
  temp = allocarray(N+2,N+2);

  
  /* Initialize the boundaries of the life matrix */
  for (i = 0; i < N+2; i++) {
    life[0][i] = life[i][0] = life[N+1][i] = life[i][N+1] = DIES ;
    temp[0][i] = temp[i][0] = temp[N+1][i] = temp[i][N+1] = DIES ;
  }

  /* Initialize the life matrix */
  for (i = 1; i < N+1; i++) {
    srand(54321|i);
    for (j = 1; j< N+1; j++)
      if (drand48() < 0.5) 
	life[i][j] = ALIVE ;
      else
	life[i][j] = DIES ;
  }

#ifdef DEBUG1
  /* Display the initialized life matrix */
  printarray(life, N, 0);
#endif

  t1 = gettime();
  k=compute(life, temp, N,P,Q,k);
  //printf("k=%d\n",k);
  
  t2 = gettime();
  if(k!=NTIMES){
    printf("Could not finish work\n");
  } 
  file_write(filePath,temp,N,N,NTIMES, P*Q);
  //Take the problem size, maximum number of iterations, number of threads, and
//the directory to write the output file as command-line arguments.
  printf("Time taken %f seconds for size=%d with %d iterations thread=%d\n", t2 - t1,N, k,P*Q);

#ifdef DEBUG1
  /* Display the life matrix after k iterations */
  printarray(life, N, k);
#endif

  freearray(life);
  freearray(temp);

  printf("Program terminates normally\n") ;

  return 0;
}

