#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define min(x, y) ((x)<(y)?(x):(y))

int main(int argc, char* argv[]) {
	int nrows, ncols;
	double *aa, *b, *c;  //Pointer to an address that contains a double    
	double *buffer, ans;
	double *times;
	double total_times;
	int run_index;
	int nruns;
	int myid, master, numprocs;
	double starttime, endtime;
	MPI_Status status;  //Structure containing: MPI_Source - id of processor sending the message, MPI_Tag - the message tag, MPI_Error - error status
	int i, j, numsent, sender;
	int anstype, row;
	srand(time(0));  //Seed for a random number generator
	MPI_Init(&argc, &argv);  //Initialize MPI execution environment (argc - pointer to num of args, argv - pointer to arg vector)
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);  //Size of the group associated with a communicator (size - number of processes in the group of comm (int))    
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);  //The rank of the calling process (MPI_Comm, int *rank) comm communicator, rank of calling process

	if (argc > 1) {
		nrows = atoi(argv[1]); //Convert string to int
		ncols = nrows;
		aa = (double*)malloc(sizeof(double) * nrows * ncols);  //allocates a block of size bytes of memory, returning a pointer to the beginning of the block
		b = (double*)malloc(sizeof(double) * ncols);  //number of columns
		c = (double*)malloc(sizeof(double) * nrows);  //number of rows
		buffer = (double*)malloc(sizeof(double) * ncols);
		master = 0;

		if (myid == master) {
		// Master Code goes here
			for (i = 0; i < nrows; i++) {
				for (j = 0; j < ncols; j++) {
					aa[i*ncols + j] = (double)rand()/RAND_MAX;
				}
			}

			//Returns elapsed time on calling processor (returns time in seconds since arbitrary time in the past).
			starttime = MPI_Wtime();
			numsent = 0;
			//Broadcasts a message from the process with rank root to all other processes of the comm (void *buffer, int count, MPI_DOUBLE - Datatype datatype, int root, MPI_Comm comm)
			MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);

			for (i = 0; i < min(numprocs-1, nrows); i++) {
				for (j = 0; j < ncols; j++) {
					buffer[j] = aa[i * ncols + j];
			}
				//Performs a blocking send (const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) 
				MPI_Send(buffer, ncols, MPI_DOUBLE, i+1, i+1, MPI_COMM_WORLD);             
				numsent++;
			}

			for (i = 0; i < nrows; i++) {
				//Blocking receive for a message (count - max num of elements in the receive buffer (int), datatype - datatype of each reveive buffer element, source - rank of source, 
				//tag - message tag, comm - communicator)
				MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);   
				sender = status.MPI_SOURCE;
				anstype = status.MPI_TAG;
				c[anstype-1] = ans;

				if (numsent < nrows) {
					for (j = 0; j < ncols; j++) {
						buffer[j] = aa[numsent*ncols + j];
					}

					MPI_Send(buffer, ncols, MPI_DOUBLE, sender, numsent+1, MPI_COMM_WORLD);
					numsent++;
				} else {
					MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
				}
			}

				endtime = MPI_Wtime();
				printf("%f\n",(endtime - starttime));
		} else {
			// Slave Code goes here

			MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);

			if (myid <= nrows) {
				while(1) {
					MPI_Recv(buffer, ncols, MPI_DOUBLE, master, MPI_ANY_TAG,
					MPI_COMM_WORLD, &status);

					if (status.MPI_TAG == 0){
						break;
					}

					row = status.MPI_TAG;
					ans = 0.0;

					for (j = 0; j < ncols; j++) {
						ans += buffer[j] * b[j];
					}

					MPI_Send(&ans, 1, MPI_DOUBLE, master, row, MPI_COMM_WORLD);
				}
			}
		}
	} else {
		fprintf(stderr, "Usage matrix_times_vector <size>\n");
	}

	MPI_Finalize();  //Terminates MPI connection
	return 0;
}
