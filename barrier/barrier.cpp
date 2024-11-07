#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <random>
#include <condition_variable>


const int n_sleepers = 100;
bool go_to_work=false;
pthread_mutex_t protect_go_to_work;
pthread_cond_t barrier = PTHREAD_COND_INITIALIZER;

void * sleeper(void * arg) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(10,20);
    std::cout<<*((int*)arg)<<" is going to bed"<<std::endl;
    pthread_mutex_lock(&protect_go_to_work);
    while(!go_to_work) {
        pthread_cond_wait(&barrier,&protect_go_to_work);
        //sleep(uni(rng));
    }
    pthread_mutex_unlock(&protect_go_to_work);
}

int main() {

    pthread_t p_threads[n_sleepers];
    pthread_cond_t wakeup;

    pthread_mutex_init(&protect_go_to_work,NULL);

    for (int i=0;i<n_sleepers;i++) {
        pthread_create(&p_threads[i], NULL,sleeper,(void*) & i);
    }

    std::cout<<"Main goes to bed"<<std::endl;
    sleep(5);
    std::cout<<"It's time for work! Get up!"<<std::endl;
    pthread_mutex_lock(&protect_go_to_work);
    go_to_work = true;
    pthread_cond_signal(&barrier);
    pthread_mutex_unlock(&protect_go_to_work);

    
    for(int i = 0; i<n_sleepers; i++) {
        pthread_join(p_threads[i], NULL);
    }
}