#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    int ierr, myrank, numprocs;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    struct timeval stop, start;
    MPI_Status status;
    const char filename [] = "pingpong_results04.csv";
    const int n_rep=10;

    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    ierr = MPI_Get_processor_name(processor_name,&namelen);
    if (myrank == 0) {
        FILE * log = fopen(filename, "a");
        fprintf(log, "n, t\n");
        MPI_Request request;
        long int time;
        double mean=0.0;
        for(int n = 2; n <= 1048576; n+=1024) {
            int size = n;
            for(int i = 0; i<n_rep; i++) {
                char * message = (char*) malloc(size*sizeof(char));
                gettimeofday(&start, NULL);
                MPI_Ssend(message, size, MPI_CHARACTER, 4, 101, MPI_COMM_WORLD);
                MPI_Recv(message, size, MPI_CHARACTER, 4, 101, MPI_COMM_WORLD, &status);
                gettimeofday(&stop, NULL);
                time = size/(((stop.tv_sec - start.tv_sec))/2);
                mean+=(double)time;
                //printf("%d Bytes: %ld mys\n", size, time);
                free(message);
            }
            fprintf(log, "%d, %f\n", n, mean/10.0);
        }
        fclose(log);
    } else if(myrank == 1) {
        printf(" hello from %d\n",myrank);
    } else if(myrank==2) {
        printf(" hello from %d\n",myrank);
    } else if(myrank==3) {
        printf(" hello from %d\n",myrank);
    } else if(myrank==4) {
        for(int n = 2; n <= 1048576; n+=1024) {
            for(int i = 0; i<n_rep; i++) {
                int size = n;
                char * message = (char*) malloc(size*sizeof(char)); 
                MPI_Recv(message, size, MPI_CHARACTER, 0, 101, MPI_COMM_WORLD, &status);
                MPI_Ssend(message, size, MPI_CHARACTER, 0, 101, MPI_COMM_WORLD);
                free(message);
            }
        }
    } 
    
    MPI_Finalize();
    return 0;
}