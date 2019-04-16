#include <stdio.h>
#include <time.h>
#include "mpi.h"
#include <stdlib.h>
#define N 100000
#define NMSG 100000
int main(int argc, char *argv[])
{
    int rank, size, i;
    MPI_Status status;
    clock_t start_t, end_t;
    start_t = clock();
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
    	int* send_buffer = (int *)malloc(N * sizeof(int));
	for (i = 0; i < N; i++)
	    send_buffer[i] = i;
	for (i = 0; i < NMSG; i++){
	    MPI_Send(send_buffer, N, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }	
        printf("send conpleted\n");
    }
    else {
    	int* recv_buffer = (int *)malloc(N * sizeof(int));
	for(i = 0; i < NMSG; i++)
	    MPI_Recv(recv_buffer, N, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        end_t = clock();
        clock_t t = end_t - start_t;
        double time_taken = ((double)t)/CLOCKS_PER_SEC;

	printf("recv completed!\n");
        printf("Send/Recv took %f seconds to execute \n", time_taken);
    }
    MPI_Finalize();
    return 0;
}
