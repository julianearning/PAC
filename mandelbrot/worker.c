#include <complex.h>
#include <math.h>

const int image_width=1000;
const int image_height=1000;
const double spacex_begin=-2;
const double spacex_end=2;
const double spacey_begin=-2;
const double spacey_end=2;
const int task_size=100;
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
            for(int j = y; j>(y-size); j--) {
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
