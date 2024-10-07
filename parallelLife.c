/*
Name:  Ali Takallou
Email: atakallou@crimson.ua.edu
Course Section: CS 581
Homework #:3
Instructions to compile the program: gcc -Wall -O -fopenmp parallelLife.c -o parallelLife
Instructions to run the program: .\parallelLife.exe <Size> <maxmium number of Generations> <Number of Threads> <Output Directory>

   Use -DDEBUG1 for output at the start and end.
   Use -DDEBUG2 for output at each iteration.
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <omp.h>
#include <stdlib.h>

#define DIES   0
#define ALIVE  1


double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

int **allocarray(int P, int Q) {
  int i, *p, **a;

  p = (int *)malloc(P*Q*sizeof(int));
  a = (int **)malloc(P*sizeof(int*));
  /* for row major storage */
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q]; 

  return a;
}

void freearray(int **a) {
  free(&a[0][0]);
  free(a);
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

int compute(int **life, int **temp, int N, int NTHREADS) {
    int i, j, value, flag = 0;

    // Parallel region with OpenMP
    #pragma omp parallel for default(none) shared(life, temp, N) private(i, j, value) reduction(+:flag) num_threads(NTHREADS)
    for (i = 1; i < N+1; i++) {
        for (j = 1; j < N+1; j++) {
            // Calculate the sum of the 8 surrounding cells
            value = life[i-1][j-1] + life[i-1][j] + life[i-1][j+1]
                  + life[i][j-1]                + life[i][j+1]
                  + life[i+1][j-1] + life[i+1][j] + life[i+1][j+1];

            // Check if the current cell was alive
            if (life[i][j]) {
                // Cell dies if it's underpopulated or overpopulated
                if (value < 2 || value > 3) {
                    temp[i][j] = DIES;
                    flag++; // The state of the cell changed
                } else {
                    temp[i][j] = ALIVE; // The cell remains alive
                }
            } 
            // If the current cell was dead, check for reproduction
            else {
                if (value == 3) {
                    temp[i][j] = ALIVE; // New cell is born
                    flag++; // The state of the cell changed
                } else {
                    temp[i][j] = DIES; // The cell remains dead
                }
            }
        }
    }

    return flag; // Return the number of state changes
}


int main(int argc, char **argv) {
  int N, NTIMES, NTHREADS, **life=NULL, **temp=NULL, **ptr ;
  int i, j, k, flag=1;
  
  double t1, t2;
  char output_filepath[256];
    // Check for four command-line arguments
  if (argc != 5) {
        printf("Usage: %s <size> <max_iterations> <num_threads> <output_directory>\n", argv[0]);
        exit(-1);
  }

    // Parse command-line arguments
  N = atoi(argv[1]);
  NTIMES = atoi(argv[2]);
  NTHREADS = atoi(argv[3]);
  snprintf(output_filepath, sizeof(output_filepath), "%s/final_state.txt", argv[4]);
  /* Allocate memory for both arrays */
  life = allocarray(N+2,N+2);
  temp = allocarray(N+2,N+2);

  /* Initialize the boundaries of the life matrix */
  for (i = 0; i < N+2; i++) {
    life[0][i] = life[i][0] = life[N+1][i] = life[i][N+1] = DIES ;
    temp[0][i] = temp[i][0] = temp[N+1][i] = temp[i][N+1] = DIES ;
  }

  /* Initialize the life matrix */
    for (i = 1; i < N + 1; i++) {
        srand(42); // Initialize random seed uniquely for each row
        for (j = 1; j < N + 1; j++) {
            // Generate a random number between 0 and 1 and assign the cell state
            if ((double)rand() / RAND_MAX < 0.5) {
                life[i][j] = ALIVE; // The cell is alive
            } else {
                life[i][j] = DIES;  // The cell is dead
            }
        }
    }
    
#ifdef DEBUG1
  /* Display the initialized life matrix */
  printarray(life, N, 0);
#endif

  t1 = gettime();
  /* Play the game of life for given number of iterations */
  for (k = 0; k < NTIMES && flag != 0; k++) {
    flag = 0;
    flag = compute(life, temp, N, NTHREADS);

    /* copy the new values to the old array */
    ptr = life;
    life = temp;
    temp = ptr;

#ifdef DEBUG2
    /* Print no. of cells alive after the current iteration */
    printf("No. of cells whose value changed in iteration %d = %d\n",k+1,flag) ;

    /* Display the life matrix */
    printarray(life, N, k+1);
#endif
  }
  t2 = gettime();

  printf("Time taken %f seconds for %d iterations\n", t2 - t1, k);

#ifdef DEBUG1
  /* Display the life matrix after k iterations */
  printarray(life, N, k);
#endif



  
      // Write the final board state to an output file
  FILE *output_file = fopen(output_filepath, "w");
  if (output_file == NULL) {
        fprintf(stderr, "Error: Could not open output file %s\n", output_filepath);
        exit(-1);
    }
    for (i = 1; i < N + 1; i++) {
        for (j = 1; j < N + 1; j++) {
            fprintf(output_file, "%d ", life[i][j]);
        }
        fprintf(output_file, "\n");
    }
    fclose(output_file);
  
  freearray(life);
  freearray(temp);
  printf("Program terminates normally\n") ;
  printf("Final board state saved to %s\n", output_filepath);


  return 0;
}
