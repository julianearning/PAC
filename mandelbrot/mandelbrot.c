#include <mpi.h>
#include <mpe.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <unistd.h>

#include <complex.h>
#include <math.h>

const int image_width=1000;
const int image_height=1000;
const double spacex_begin=-2;
const double spacex_end=2;
const double spacey_begin=-2;
const double spacey_end=2;
const int task_size=200;
const int NEXT_TAG=33;
const int STOP_TAG=160;

// returns a complex number as z0 for Coordinates (x,y) based on Graphics window thing
double complex coordinates_to_complex(int x, int y) {
    double real=(((double)x/(double)image_width)*(abs((double)spacex_begin)+abs((double)spacex_end))) + (double)spacex_begin;
    double imaginary=((((double)image_height-(double)y)/(double)image_height)*(abs((double)spacey_begin)+abs((double)spacey_end))) + (double)spacey_begin;
    return real + I * imaginary;
}


double mandelbrot(double complex zi, double complex c, int iter, int iter_max, double z_max) {
    if(iter<iter_max) {
        if(sqrt(creal(zi)*creal(zi)+cimag(zi)*cimag(zi))>z_max) {
            return iter;
        }
        return mandelbrot(zi*zi+c, c, ++iter, iter_max, z_max);
    } else {
        return iter_max;
    }
}


double worker(MPE_XGraph window, int myrank) {  // y reversed because computer graphics are dumb
    double result;
    double complex curr_num;
    int x;
    int y;
    int size;
    bool boo=true;
    MPI_Status status;


    int next_block[3];
    // get initial task from master
    MPI_Recv(next_block, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);   // next_block: array of 3 ints: x, y, size
    x = next_block[0];
    y = next_block[1];
    size = next_block[2];    

    while(true) { // there's work to do

        for(int i = x; i<(x+size); i++) {
            for(int j = y; j<(y+size); j++) {
                curr_num = coordinates_to_complex(i,j);
                result = mandelbrot(0 + 0 * I, curr_num, 0, 100, 10.0);
                if(result==100) {
                    MPE_Draw_point(window,i,j,MPE_BLACK);  // TODO: make custom color palette!
                } else {
                    MPE_Draw_point(window,i,j,255 * result / 100);  // TODO: make custom color palette!
                }
                MPE_Update(window);
            }
        }

        // ask for new task 
        MPI_Ssend(&boo,1, MPI_C_BOOL, 0, 101, MPI_COMM_WORLD);

        // receive new task
        MPI_Recv(next_block, 3, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);   // next_block: array of 3 ints: x, y, size
        if (status.MPI_TAG == STOP_TAG){ return 0; }
        x = next_block[0];
        y = next_block[1];
        size = next_block[2];  
    }
}

int main(int argc, char *argv[]) {
    if(((image_height%task_size)!=0) || ((image_width%task_size)!=0)) {
        printf("size must be divisible by image_height and image_width\n");
        return -1;
    }
    int ierr, myrank, numprocs, myWindowOpened, allWindowsOpened;
    int namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    ierr = MPI_Init(&argc, &argv);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    ierr = MPI_Get_processor_name(processor_name,&namelen);

    MPI_Status status;
    MPE_XGraph window;
    myWindowOpened = MPE_Open_graphics(&window,MPI_COMM_WORLD,NULL,-1,-1,image_width,image_height,0);
    printf("rank %d : myWindowOpened=%d\n", myrank, myWindowOpened);
    MPI_Allreduce(&myWindowOpened, &allWindowsOpened, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
    MPE_Color colorArray[256];
    MPE_Make_color_array(window, 256, colorArray);

    //if (allWindowsOpened) {

        if(myrank==0) { // master

            int last_x=0;
            int last_y=0;
            int n_tasks=(int)((float)(image_width*image_height) / (float)(task_size*task_size));
            int task[3];    // x,y,size
            task[2] = task_size;   // size is constant
            int n_busy=0;

            printf("Should be %d tasks\n", n_tasks);

            // initial work
            for(int i = 0; (i<image_width) && (n_busy < (numprocs-1)); i+=task_size) {
                for(int j=0; (j<image_height) && (n_busy < (numprocs-1)); j+=task_size) {   // task: litte task_size*task_size square 
                    task[0]=i; 
                    task[1]=j;
                    MPI_Ssend(task, 3, MPI_INT, n_busy+1, NEXT_TAG, MPI_COMM_WORLD);
                    n_busy+=1;
                    n_tasks-=1;
                    last_y=j;    
                }    
                last_x=i;
            }

            bool free=false;
            int free_worker; 
            // loopty loop
            while(n_tasks>0) {
                MPI_Recv(&free,1,MPI_C_BOOL,MPI_ANY_SOURCE, 101, MPI_COMM_WORLD, &status);
                n_tasks--;
                free_worker = status.MPI_SOURCE;
                if((last_y+task_size)<image_height) {
                    task[0]=last_x;
                    task[1]=last_y+task_size;
                    last_y=last_y+task_size;
                } else {    
                    task[0]=last_x+task_size;
                    task[1]=0;
                    last_x=last_x+task_size; 
                    last_y=0;
                }
                MPI_Ssend(task, 3, MPI_INT, free_worker, NEXT_TAG, MPI_COMM_WORLD);
            }
            printf("Work is over at last_x=%d and last_y=%d\n", last_x, last_y);


            // send all workers the stop signal
            for(int p=1;p<numprocs;p++) {
                MPI_Recv(&free,1,MPI_C_BOOL,MPI_ANY_SOURCE, 101, MPI_COMM_WORLD, &status);
                free_worker = status.MPI_SOURCE;
                //printf("Giving stop signal to %d\n", free_worker);
                MPI_Ssend(task, 3, MPI_INT, free_worker, STOP_TAG, MPI_COMM_WORLD);
            }


            int i, j, k;
            MPE_Get_mouse_press(window, &i, &j, &k);
        } else {
            worker(window, myrank);
            printf("worker %d exited\n", myrank);
        }

        MPI_Barrier(MPI_COMM_WORLD); 
        MPE_Close_graphics(&window);
        
    /*} else {
        if (!myrank) {
            fprintf(stderr, "One or more processes could not connect\n");
            fprintf(stderr, "to the display. Exiting.\n\n");
        }
        if (myWindowOpened)
            MPE_Close_graphics(&window);
        else 
            printf("I could not connect: %d\n", myrank);
    }*/
    MPI_Finalize();

}