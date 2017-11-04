#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define ROWS 5
#define COLUMNS 5

void printResultMatrix();

int nRowsA, nRowsB, nColsA, nColsB;
int myid, numberOfProcs;
int i, j, k;
int offset, workers;
int divideRows, leftOverRows, rows;
MPI_Status status;
double matrixA[ROWS][COLUMNS] = {
        {1.0, 2.0, 3.0, 4.0, 5.0},
        {6.0, 7.0, 8.0, 9.0, 10.0},
        {11.0, 12.0, 13.0, 14.0, 15.0},
        {16.0, 17.0, 18.0, 19.0, 20.0},
        {21.0, 22.0, 23.0, 24.0, 25.0}
};
double matrixB[ROWS][COLUMNS] = {
        {1.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 1.0}
};
double resultMatrix[ROWS][COLUMNS];
int low;
int high;
int divideRows;


int main(int argc, char *argv[]){
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if (myid == 0){
        for (i = 1; i < numberOfProcs; i++){
            //Try to divide up rows evenly among available processors
            divideRows = (ROWS / (numberOfProcs - 1));
            low = (i - 1) * divideRows;
            if (((i + 1) == numberOfProcs) && ((ROWS % (numberOfProcs - 1)) != 0)) {
                high = ROWS;
            } else {
                high = low + divideRows;
            }

            MPI_Send(&low, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            MPI_Send(&high, 1, MPI_INT, i, 1 + 1, MPI_COMM_WORLD);
            MPI_Send(&matrixA[low][0], (high - low) * COLUMNS, MPI_DOUBLE, i, 1 + 2, MPI_COMM_WORLD);
        }
    }

    MPI_Bcast(&matrixB, ROWS * COLUMNS, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (myid > 0){
        MPI_Recv(&low, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&high, 1, MPI_INT, 0, 1 + 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&matrixA[low][0], (high - low) * COLUMNS, MPI_DOUBLE, 0, 1 + 2, MPI_COMM_WORLD, &status);

        for (i = low; i < high; i++) {
            for (j = 0; j < COLUMNS; j++) {
                for (k = 0; k < ROWS; k++) {
                    resultMatrix[i][j] += (matrixA[i][k] * matrixB[k][j]);
                }
            }
        }

        MPI_Send(&low, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(&high, 1, MPI_INT, 0, 2 + 1, MPI_COMM_WORLD);
        MPI_Send(&resultMatrix[low][0], (high - low) * COLUMNS, MPI_DOUBLE, 0, 2 + 2, MPI_COMM_WORLD);
    }

    if (myid == 0){
        for (i = 1; i < numberOfProcs; i++) {
            MPI_Recv(&low, 1, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&high, 1, MPI_INT, i, 2 + 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&resultMatrix[low][0], (high - low) * COLUMNS, MPI_DOUBLE, i, 2 + 2, MPI_COMM_WORLD, &status);
        }

        printResultMatrix();
    }
    MPI_Finalize();
    return 0;
}

void printResultMatrix(){
    printf("\n");
    for (i = 0; i < ROWS; i++) {
        printf("\n");
        for (j = 0; j < COLUMNS; j++)
            printf("%8.2f  ", resultMatrix[i][j]);
    }
    printf("\n\n");
}