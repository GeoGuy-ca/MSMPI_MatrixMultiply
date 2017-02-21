// MPI_MatrixMultiply.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include <iostream>

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv );

	int rank;
	int nodes;
	MPI_Comm_size(MPI_COMM_WORLD, &nodes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (rank == 0) {
		char helloStr[] = "Hello World";
		MPI_Bcast(helloStr, _countof(helloStr), MPI_CHAR, 0, MPI_COMM_WORLD);
	}
	else {
		char helloStr[12];
		MPI_Recv(helloStr, _countof(helloStr), MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Rank %d received string %s from Rank 0\n", rank, helloStr);
	}
	
	MPI_Finalize();
	return 0;
}

