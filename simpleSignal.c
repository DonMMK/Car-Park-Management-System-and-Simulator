#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
char value[7];

void *signalThread(void *args) {
    usleep(5000000);
    printf("Changing value to goodby...\n");  

    pthread_mutex_lock(&mutex);
    strcpy(value, "goodby");
    pthread_mutex_unlock(&mutex);

    pthread_cond_signal(&cond);
}

void *waitThread(void *args) {
    pthread_mutex_lock(&mutex);
    while (strcmp(value, "goodby")){
        printf("in loop\n");
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);  

    printf("wait has been executed and value is now %s\n", value); 
      
}


int main(void){
    pthread_t threadSignal;
    pthread_t threadWait;
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    
    strcpy(value, "helloo");
    printf("Initialising value to: %s...\n", value);

    pthread_create(&threadSignal, NULL, &signalThread, NULL);
    pthread_create(&threadWait, NULL, &waitThread, NULL);

    pthread_join(threadSignal,NULL);
    pthread_join(threadWait,NULL);

  
}   