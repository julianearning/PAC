#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    int ierr, myrank, numprocs;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int n_p=4;
    struct timeval stop, start;
    MPI_Request request;
    MPI_Status status;

    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    ierr = MPI_Get_processor_name(processor_name,&namelen);

    if(myrank==0) { 
        int my_num = rand() % 10 + 1;
        int number;
        int sum=my_num;
        for(int i = 1; i<n_p; i++) {
            number = rand() % 10 + 1;
            sum+=number;
            ierr = MPI_Ssend(&number, 1, MPI_INT, i, 101, MPI_COMM_WORLD);
        }
        printf("sum is: %d\n", sum);
        printf("my_num at rank 0: %d\n", my_num);
        MPI_Barrier(MPI_COMM_WORLD);

        int new_num=my_num;
        sum=my_num;
        for(int i = 0; i<(n_p-1); i++) {
            ierr = MPI_Ssend(&new_num, 1, MPI_INT, 1, 101, MPI_COMM_WORLD);
            MPI_Recv(&new_num, 1, MPI_INT, 3, 101, MPI_COMM_WORLD, &status);
            sum += new_num;
        }
        printf("sum at rank 0: %d\n", sum);
    } else if(myrank==1) {
        int my_num=0;
        MPI_Recv(&my_num, 1, MPI_INT, 0, 101, MPI_COMM_WORLD, &status);
        printf("my_num at rank 1: %d\n", my_num);
        MPI_Barrier(MPI_COMM_WORLD);

        int new_num=my_num;
        int sum=my_num;
        int old_num=my_num;
        for(int i = 0; i<(n_p-1); i++) {
            MPI_Recv(&new_num, 1, MPI_INT, 0, 101, MPI_COMM_WORLD, &status);
            sum+=new_num;
            ierr = MPI_Ssend(&old_num, 1, MPI_INT, 2, 101, MPI_COMM_WORLD);
            old_num=new_num;
        }
        printf("sum at rank 1: %d\n", sum);
    } else if(myrank==2) {
        int my_num=0;
        MPI_Recv(&my_num, 1, MPI_INT, 0, 101, MPI_COMM_WORLD, &status);
        printf("my_num at rank 2: %d\n", my_num);
        MPI_Barrier(MPI_COMM_WORLD);
        int new_num=my_num;
        int sum=my_num;
        for(int i = 0; i<(n_p-1); i++) {
            ierr = MPI_Ssend(&new_num, 1, MPI_INT, 3, 101, MPI_COMM_WORLD);
            MPI_Recv(&new_num, 1, MPI_INT, 1, 101, MPI_COMM_WORLD, &status);
            sum += new_num;
        }
        printf("sum at rank 2: %d\n", sum);

    } else if(myrank==3) {
        int my_num=0;
        MPI_Recv(&my_num, 1, MPI_INT, 0, 101, MPI_COMM_WORLD, &status);
        printf("my_num at rank 3: %d\n", my_num);
        MPI_Barrier(MPI_COMM_WORLD);

        int new_num=my_num;
        int sum=my_num;
        int old_num=my_num;
        for(int i = 0; i<(n_p-1); i++) {
            MPI_Recv(&new_num, 1, MPI_INT, 2, 101, MPI_COMM_WORLD, &status);
            sum+=new_num;
            ierr = MPI_Ssend(&old_num, 1, MPI_INT, 0, 101, MPI_COMM_WORLD);
            old_num=new_num;
        }
        printf("sum at rank 3: %d\n", sum);

    } 

    MPI_Finalize();
    return 0;
}