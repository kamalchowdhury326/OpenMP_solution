/* 
   Name:Md Kamal Hossain Chowdhury 
   BlazerId:kamalhc 
   Homework #: 3 Design and implementation of an efficient multithreaded version of the "Game of Life" program using OpenMP
  To compile: gcc -fopenmp -Wall -g -O hw3.c -o hw3
              icx -qopenmp -wall hw3.c -o hw3_intel
  To execute:    ./hw3 5000 5000 4  /scratch/$USER <== for 4 threads
                 ./hw3_intel 5000 5000 8  /scratch/$USER <== for 8 threads


   Use -DDEBUG1 for output at the start and end.
   Use -DDEBUG2 for output at each iteration.
   Use -DDEBUGC for correctness checking
*/

#include <stdlib.h>
#include<stdbool.h>
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

void file_write(char* path, int **A, int n,int iterations, int P){
   // Specify the full path where you want to create the file
    //const char *path = "/scratch/ualmkc001/filename.txt";
    const char* name = path;
    const char* extension = ".txt";
    char size[10];
    snprintf(size, 10, "%d", n);
    char iter[10];
    snprintf(iter, 10, "%d", iterations);
    char p_char[10];
    snprintf(p_char, 10, "%d", P);
    

    char* name_with_extension;
    name_with_extension = malloc(strlen(name)+1+10); /* make space for the new string (should check the return value ...) */
    strcpy(name_with_extension, name); /* copy name into the new var */
    
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
   
    for(int i=1;i<n+1;i++){
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


int main(int argc, char **argv) {
  int N, NTIMES;
  int** life=NULL, **temp=NULL;
  int i, j, P;
  //,flag;
  int k;
  double t1, t2;
  
  
  if (argc != 5) {
      printf("Usage: %s <N> <Generations> <P><filePath>\n", argv[0]);
      exit(-1);
    }
    
  
  N = atoi(argv[1]);
  NTIMES = atoi(argv[2]);
  P = atoi(argv[3]);
  filePath=argv[4];
  
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
  
  bool flag;
  int res=0;
  k=0;
  #pragma omp parallel default(none) shared(k,N,P,life,temp,NTIMES,flag,res) num_threads(P) 
  //reduction(+:k)
    { 
      
      for (k = 0; k < NTIMES; ) {
        
        int tid = omp_get_thread_num();
        bool local_flag=false;
        flag=false;
        # ifdef DEBUG2
          printf("beginning  tid=%d  flag=%d k=%d\n",tid,flag,k);
      # endif
        int myM = N / P;
        int istart = tid * myM;
        int iend = istart + myM;
        if (tid == P-1) iend = N;
        
        for (int i=istart+1; i<iend+1; i++) {
            #ifdef DEBUG0
            printf("tid=%d P=%d: i=%d i:{%d,%d} \n", tid, P, i, istart, iend);
            #endif
            
          for (int j=1; j< N+1; j++) {
             
            /* find out the value of the current cell */
            int value = life[i-1][j-1] + life[i-1][j] + life[i-1][j+1]+ life[i][j-1] + life[i][j+1]+ life[i+1][j-1] + life[i+1][j] + life[i+1][j+1] ;
            
            /* check if the cell dies or life is born */
            if (life[i][j]) { // cell was alive in the earlier iteration
              if (value < 2 || value > 3) {
                temp[i][j] = DIES ;
                local_flag=true; // value changed 
              }
              else // value must be 2 or 3, so no need to check explicitly
                temp[i][j] = ALIVE ; // no change
                } 
            else { // cell was dead in the earlier iteration
              if (value == 3) {
                temp[i][j] = ALIVE;
                local_flag=true; // value changed 
              }
              else
                temp[i][j] = DIES; // no change
                }
        }
        
      }
       #pragma omp critical
      {
        //flag+=local_flag;
        flag=flag || local_flag;
      }
      
     #pragma omp barrier
      
# ifdef DEBUG2
        #pragma omp single
            printf("No. of cells whose value changed in iteration %d = %d\n",k+1,flag) ;
# endif
       
      #pragma omp single
      {
        if(!flag) 
        {
          res=k;
          # ifdef DEBUG2
          printf("tid=%d flag=%d k=%d\n",tid,flag,k);
          # endif
          k=NTIMES;
          //continue;     
        } 
        swap(&life,&temp);
        #ifdef DEBUG2
          
          if(k!=NTIMES){
            printf("Generation %d:\n", k+1);
            printarray(life, N, k+1);
          }
          
        #endif
      }
      //#pragma omp barrier 
      # ifdef DEBUG2
          printf("outof single tid=%d  flag=%d k=%d\n",tid,flag,k);
      # endif
      #pragma omp single
      k=k+1;
   }

 }
  
  t2 = gettime();
  if(res>0){
    printf("res=%d NTIMES=%d\n",res,NTIMES);
    printf("Could not finish work\n");
    k=res;
  } 
 // 
  
  //Take the problem size, maximum number of iterations, number of threads, and
//the directory to write the output file as command-line arguments.
  
  printf("Time taken %f seconds for size=%d with %d iterations thread=%d\n", t2 - t1,N, k,P);
  //file_write(filePath,life,N,k, P);

  char outputFileDirectory[150];
    strcpy(outputFileDirectory, argv[4]);

    strcat(outputFileDirectory, "/output.");
    for (int k = 1; k <= 3; k++) {
        strcat(outputFileDirectory, argv[k]);
        if (k <= 2) {
            strcat(outputFileDirectory, ".");
        }
        else {
            strcat(outputFileDirectory, ".txt");
        }
    }

    FILE* output = fopen(outputFileDirectory, "w");
    for (int k = 1; k <= N; k++) {
        for (int l = 1; l <= N; l++) {
            fprintf(output, "%d ", life[k][l]);
        }
        fprintf(output, "\n");
    }
    fclose(output);
  
#ifdef DEBUG1
  /* Display the life matrix after k iterations */
  printarray(life, N, k);
#endif

  freearray(life);
  freearray(temp);

  printf("Program terminates normally\n") ;

  return 0;
}

