#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h> //add library  pthread

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);

//total cost for all threads
double cost_of_writers;
double cost_of_readers;

//locks to lock-unlock the total costs addition
pthread_mutex_t writer_lock;
pthread_mutex_t reader_lock;


