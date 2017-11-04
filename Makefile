PGMS=witteck_matrix_times_matrix

all:    ${PGMS}

witteck_matrix_times_matrix:    witteck_matrix_times_matrix.c
        mpicc -O3 -o witteck_matrix_times_matrix witteck_matrix_times_matrix.c

clean:
        rm -f *.o
        rm -f ${PGMS}
