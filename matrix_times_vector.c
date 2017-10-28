#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//#define min(x, y) ((x)<(y)?(x):(y))

int main(int argc, char* argv[]) { //argc - number of args passed, argv[] - pointer array that points to each arg (argv[0] holds name of program
	int numberOfRows, numberOfColumns;
	double *matrixA, *b, *resultMatrix;  //Pointer to an address that contains a double
	double *buffer, multiplicationResult;
	double *times;
	double total_times;
	int run_index;
	int nruns;
	int myid, master, numberOfProcesses;
	double starttime, endtime;
	MPI_Status status;  //Structure containing: MPI_Source - id of processor sending the message, MPI_Tag - the message tag, MPI_Error - error status
	int i, j, numsent, sender;
	int anstype, row;
	srand(time(0));  //Seed for a random number generator
	MPI_Init(&argc, &argv);  //Initialize MPI execution environment (argc - pointer to num of args, argv - pointer to arg vector)
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);  //Total number of MPI processes running (size - number of processes in the group of comm (int))
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);  //The ID of the current MPI process running (MPI_Comm, int *rank) comm communicator, rank of calling process

	if (argc > 1) {
		numberOfRows = atoi(argv[1]); //Convert string to int
		numberOfColumns = numberOfRows; //Gives you n x n matrix based on user entry
		matrixA = (double*)malloc(sizeof(double) * numberOfRows * numberOfColumns);  //allocates a block of size bytes of memory, returning a pointer to the beginning of the block
		b = (double*)malloc(sizeof(double) * numberOfColumns);  //memory space for number of columns
		resultMatrix = (double*)malloc(sizeof(double) * numberOfRows * numberOfColumns);  //memory space for number of rows *****ADDED numberOfColumns
		buffer = (double*)malloc(sizeof(double) * numberOfColumns); //memory space for
		master = 0;

        //Create a matrix of size nrows / ncols and fill each index of aa(0.0, 0.1, 0.2, etc..) with random values
		if (myid == master) {
		// Master Code goes here
			for (i = 0; i < numberOfRows; i++) {
				for (j = 0; j < numberOfColumns; j++) {
					matrixA[i * numberOfColumns + j] = (double)rand() / RAND_MAX;
				}
			}

			//Returns elapsed time on calling processor (returns time in seconds since arbitrary time in the past).
			starttime = MPI_Wtime();
			numsent = 0;

			//Broadcasts a message from the process with rank root to all other processes of the comm
            //(void *buffer, int count, MPI_DOUBLE - Datatype datatype, int root, MPI_Comm comm)
            //buf (your data) - starting address of send buffer, count - number of elements to send, type - MPI data type of each send buffer element
            //dest - node rank id to send the buffer to, tag - message tag (label a message with a special number), comm - communicator.
			MPI_Bcast(b, numberOfColumns, MPI_DOUBLE, master, MPI_COMM_WORLD);

            //fmin - takes minimum of: number of running processes or the number of rows (because if we have more rows than running processes, we cannot
            //exceed the number of processes)
            //stores values in aa into buffer and sends each row to the next node rank id (0+1, 1+1, 2+1, etc., up to row amount or max num of processes avail)
			for (i = 0; i < fmin(numberOfProcesses - 1, numberOfRows); i++) {
				for (j = 0; j < numberOfColumns; j++) {
					buffer[j] = matrixA[i * numberOfColumns + j];
			    }

				//Performs a blocking send
                //MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
                //buf (your data) - starting address of send buffer, count - number of elements to send, type - MPI data type of each send buffer element
                //dest - node rank id to send the buffer to, tag - message tag (label a message with a special number), comm - communicator.
				MPI_Send(buffer, numberOfColumns, MPI_DOUBLE, i + 1, i + 1, MPI_COMM_WORLD);
				numsent++;
			}

			for (i = 0; i < numberOfRows; i++) {
				//Blocking receive for a message
                //MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
                //buf - starting address of receive buffer, count - number of elements in receive buffer, type - same as send,
                //src - node rank id to receive the buffer from, tag - same as send, comm - same as send, status -
				MPI_Recv(&multiplicationResult, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				sender = status.MPI_SOURCE;
				anstype = status.MPI_TAG;
				resultMatrix[anstype - 1] = multiplicationResult;

				if (numsent < numberOfRows) {
					for (j = 0; j < numberOfColumns; j++) {
						buffer[j] = matrixA[numsent*numberOfColumns + j];
					}

					MPI_Send(buffer, numberOfColumns, MPI_DOUBLE, sender, numsent+1, MPI_COMM_WORLD);
					numsent++;
				} else {
					MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
				}
			}

				endtime = MPI_Wtime();
				printf("%f\n",(endtime - starttime));
		} else {
			// Slave Code goes here
            //Receive data sent from master and calculate matrix multiplication
            //Send back each result to the master and display result

			MPI_Bcast(b, numberOfColumns, MPI_DOUBLE, master, MPI_COMM_WORLD);

			if (myid <= numberOfRows) {
				while(1) {
					MPI_Recv(buffer, numberOfColumns, MPI_DOUBLE, master, MPI_ANY_TAG,
					MPI_COMM_WORLD, &status);

					if (status.MPI_TAG == 0){
						break;
					}

					row = status.MPI_TAG;
					multiplicationResult = 0.0;

					for (j = 0; j < numberOfColumns; j++) {
						multiplicationResult += buffer[j] * b[j];
					}

					MPI_Send(&multiplicationResult, 1, MPI_DOUBLE, master, row, MPI_COMM_WORLD);
				}
			}
		}
	} else {
		fprintf(stderr, "Usage matrix_times_vector <size>\n");
	}

	MPI_Finalize();  //Terminates MPI connection
	return 0;
}
