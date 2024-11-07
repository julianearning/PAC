#include <iostream>
#include <unistd.h>
#include <chrono>
#include <random>
#include <cmath>
#ifdef _OPENMP
#include <omp.h>
#endif

const int n_points=1000000;
const int n_threads=4;

bool point_in_circle() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<> uni(0, 1);
    return ((std::pow(uni(rng),2))+(std::pow(uni(rng),2))<=1);
}


int main(int argc, char * argv []) {
    // start;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;
    std::chrono::duration<double> elapsed_seconds;
    start = std::chrono::steady_clock::now();
    unsigned int sum=0;
    // first not parallel
    for(int i=0; i<n_points; i++) {
        if(point_in_circle()) {
            sum++;
        }
    }
    end = std::chrono::steady_clock::now();

    //elapsed_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout<<"Pi is: "<<(double)4*(double)sum/(double)n_points<<". "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms"<<std::endl;

    
    std::cout<<"With Directive and omp parallel"<<std::endl;
    // pragma parallel
    start = std::chrono::steady_clock::now();
    sum=0;
    #pragma omp parallel num_threads (n_threads) reduction(+:sum)
    {
    sum=0;
    int threads = omp_get_num_threads();
    int sample_points_per_thread=n_points/threads; 
        for(int i=0; i<sample_points_per_thread; i++) {
            if(point_in_circle()) {
                sum++;
            }
        }
    }
    end = std::chrono::steady_clock::now();
    elapsed_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout<<"Pi is: "<<(double)4*(double)sum/(double)n_points<<". "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms"<<std::endl;


    std::cout<<"With Directive and omp parallel for"<<std::endl;
    // pragma parallel
    start = std::chrono::steady_clock::now();
    sum=0;
    #pragma omp parallel for num_threads (n_threads) reduction(+:sum)
        for(int i=0; i<n_points; i++) {
    {
            if(point_in_circle()) {
                sum++;
            }
        }
    }
    end = std::chrono::steady_clock::now();
    elapsed_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout<<"Pi is: "<<(double)4*(double)sum/(double)n_points<<". "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms"<<std::endl;


    std::cout<<"With Critical"<<std::endl;
    // pragma parallel
    start = std::chrono::steady_clock::now();
    sum=0;
    #pragma omp parallel for num_threads (n_threads)
    for(int i=0; i<n_points; i++) {
    { 
            if(point_in_circle()) {
                #pragma omp critical(sum)
                {
                sum++;
                }
            }
        }
    }  
    end = std::chrono::steady_clock::now();
    elapsed_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout<<"Pi is: "<<(double)4*(double)sum/(double)n_points<<". "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms"<<std::endl;

    

    std::cout<<"With Atomic directive"<<std::endl;


    // pragma parallel
    start = std::chrono::steady_clock::now();
    sum=0;
    #pragma omp parallel for num_threads (n_threads) 
        for(int i=0; i<n_points; i++) {
    { 
            if(point_in_circle()) {
                #pragma omp atomic
                sum++;
                
        }
    }  
        }
    end = std::chrono::steady_clock::now();
    elapsed_seconds = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout<<"Pi is: "<<(double)4*(double)sum/(double)n_points<<". "<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms"<<std::endl;

    return 0;
    

}