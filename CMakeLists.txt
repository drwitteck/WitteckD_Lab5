cmake_minimum_required(VERSION 3.8)
project(WitteckD_Lab5)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        witteck_matrix_times_matrix.c
        mmult_mpi_omp.c)

add_executable(WitteckD_Lab5 ${SOURCE_FILES})