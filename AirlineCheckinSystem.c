#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "linked_list.h"

#define NQUEUE 2
#define NCLERKS 4
#define NCUSTOMERS 10
#define TRUE 1
#define FALSE 0
#define IDLE 0


struct customer_info {
    int user_id;
    int class_type;
    int service_time;
    int arrival_time;
};
