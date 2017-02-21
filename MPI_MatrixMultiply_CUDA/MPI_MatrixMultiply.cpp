// MPI_MatrixMultiply.cpp : Defines the entry point for the console application.
//

#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include "cuda_kernel.h"

//For Testing
void printMatrix(unsigned long long *c, int m, int n) {
	printf("\n");
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++){
			printf("%d,", c[m*j + i]);
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char* argv[]){
	MPI_Init(&argc, &argv );
	int rank;
	int nodes;
	MPI_Comm_size(MPI_COMM_WORLD, &nodes);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int begin_process;
	int m = atoi(argv[1]);
	int n = atoi(argv[2]);
	int o = atoi(argv[3]);

	int myWork = (m * o) / nodes;
	
	int mystartAddress = myWork * rank;
	if ((m*o) % nodes > rank) {
		myWork++;
		mystartAddress += rank;
	}
	else {
		mystartAddress += (m*o) % nodes;
	}

	if (m*o < rank) {
		myWork = 0;
		mystartAddress = 0;
	}
	

	unsigned long long *a = new unsigned long long[m * n]; //Allocate array A to heap
	unsigned long long *b = new unsigned long long[n * o]; //Allocate array A to heap
	unsigned long long *working = new unsigned long long[myWork]; //Allocate working array to heap
	memset(working, 0, myWork * sizeof(long long));
	unsigned long long *c;

	//printf("I'm node %d, and my workload is %d, my address is %d\n", rank, myWork, mystartAddress);
	
	if (rank == 0) {
		
		MatrixRandomFill(a, m, n); //Place random Data in Array A
		MatrixRandomFill(b, n, o); //Place random Data in Array B
		begin_process = GetTickCount();

		for (int i = 1; i < min(nodes, m*o); i++) {
			MPI_Send(a, m*n, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD);
			MPI_Send(b, n*o, MPI_UNSIGNED_LONG_LONG, i, 1, MPI_COMM_WORLD);
		}
		c = new unsigned long long[m*o]; //Allocate working array to heap
		memset(c, 0, m * o * sizeof(long long));
		for (int element = 0; element < myWork; element++) {
			int row = element / m;
			int col = element % m;
			for (int part = 0; part < n; part++) {
				c[element] += a[part*m + col] * b[row*n + part];
			}
			
		}

	}

	//Only workers do this. 
	else if (myWork > 0 ) {
		MPI_Recv(a, m*n, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
		MPI_Recv(b, n*o, MPI_UNSIGNED_LONG_LONG, 0, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
		//At this point everyone should have a copy of A and B, as well as a working copy.
		for (int element = 0; element < myWork; element++) {
			int row = (mystartAddress + element) / m;
			int col = (mystartAddress + element) % m;
			for (int part = 0; part < n; part++) {
				working[element] += a[part*m + col] * b[row*n + part];
			}

		}
		
		MPI_Send(working, myWork, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD);

	}


	//Host collects and cleans up
	if (rank == 0) {
		for (int i = 1; i < min(nodes, m*o); i++) {
			MPI_Recv(working, myWork, MPI_UNSIGNED_LONG_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);//myWork is sufficent buffer as rank 0 workload >= to any other node.
			int copyLength = (m * o) / nodes;
			int copyAddress = copyLength * rank;
			if ((m*o) % nodes > i) {
				copyLength++;
				copyAddress += i;
			}
			else {
				copyAddress += (m*o) % nodes;
			}
			memcpy(c + copyAddress, working, copyLength * sizeof(long long));
		}
		int end_process = GetTickCount();
		std::ofstream output;
		output.open("results.csv", std::ofstream::out | std::ofstream::app);
		output << "MPI, ";
		output << nodes;
		output << ", ";
		output << argv[1];
		output << ", ";
		output << argv[2];
		output << ", ";
		output << argv[3];
		output << ",";
		output << end_process - begin_process;
		output << "\n";
	}
	
	MPI_Finalize();
	return 0;
}

