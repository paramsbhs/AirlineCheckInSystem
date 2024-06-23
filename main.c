#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define NQUEUE 2
#define NCLERKS 4
#define NCUSTOMERS 10
#define TRUE 1
#define FALSE 0
#define IDLE 0

/* Function Declarations */
void *customer_entry(void *cus_info);
void *clerk_entry(void *clerkNum);

/* Global Variables */
struct customer_info {
    int user_id;
    int class_type;
    int service_time;
    int arrival_time;
};

struct timeval init_time;
double overall_waiting_time = 0;
int queue_length[NQUEUE] = {0, 0};
int queue_status[NQUEUE] = {IDLE, IDLE};
int winner_selected[NQUEUE] = {FALSE, FALSE};

pthread_mutex_t queue_mutex[NQUEUE];
pthread_cond_t queue_cond[NQUEUE];
pthread_mutex_t clerk_mutex[NCLERKS];
pthread_cond_t clerk_cond[NCLERKS];

struct customer_info customer_list[NCUSTOMERS];
pthread_t customer_threads[NCUSTOMERS];
pthread_t clerk_threads[NCLERKS];

double get_current_time() {
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    return (curr_time.tv_sec - init_time.tv_sec) + (curr_time.tv_usec - init_time.tv_usec) / 1000000.0;
}

void enQueue(int queue_id) {
    queue_length[queue_id]++;
}

void deQueue(int queue_id) {
    queue_length[queue_id]--;
}

int main() {
    FILE *file;
    char line[256];
    int i;

    // Initialize mutexes and condition variables
    for (i = 0; i < NQUEUE; i++) {
        pthread_mutex_init(&queue_mutex[i], NULL);
        pthread_cond_init(&queue_cond[i], NULL);
    }
    for (i = 0; i < NCLERKS; i++) {
        pthread_mutex_init(&clerk_mutex[i], NULL);
        pthread_cond_init(&clerk_cond[i], NULL);
    }

    // Initialize the initial time
    gettimeofday(&init_time, NULL);

    // Read customer information from txt file and store them in customer_list
    file = fopen("Sample_input.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    i = 0;
    while (fgets(line, sizeof(line), file) != NULL && i < NCUSTOMERS) {
        sscanf(line, "%d %d %d %d", &customer_list[i].user_id, &customer_list[i].class_type, &customer_list[i].service_time, &customer_list[i].arrival_time);
        i++;
    }
    fclose(file);

    // Create clerk threads
    for (i = 0; i < NCLERKS; i++) {
        pthread_create(&clerk_threads[i], NULL, clerk_entry, (void *)(long)i);
    }

    // Create customer threads
    for (i = 0; i < NCUSTOMERS; i++) {
        pthread_create(&customer_threads[i], NULL, customer_entry, (void *)&customer_list[i]);
    }

    // Wait for all customer threads to terminate
    for (i = 0; i < NCUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Destroy mutexes and condition variables
    for (i = 0; i < NQUEUE; i++) {
        pthread_mutex_destroy(&queue_mutex[i]);
        pthread_cond_destroy(&queue_cond[i]);
    }
    for (i = 0; i < NCLERKS; i++) {
        pthread_mutex_destroy(&clerk_mutex[i]);
        pthread_cond_destroy(&clerk_cond[i]);
    }

    // Calculate and print the average waiting time of all customers
    printf("Average waiting time: %.2f seconds\n", overall_waiting_time / NCUSTOMERS);

    return 0;
}

// Function entry for customer threads
void *customer_entry(void *cus_info) {
    struct customer_info *p_myInfo = (struct customer_info *)cus_info;
    usleep(p_myInfo->arrival_time * 1000);
    int cur_queue = p_myInfo->class_type;
	double queue_enter_time;
    printf("A customer arrives: customer ID %2d. \n", p_myInfo->user_id);

    pthread_mutex_lock(&queue_mutex[cur_queue]);
    {
        enQueue(cur_queue);
        printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", cur_queue, queue_length[cur_queue]);
        double queue_enter_time = get_current_time();

        while (TRUE) {
            pthread_cond_wait(&queue_cond[cur_queue], &queue_mutex[cur_queue]);
            if (queue_length[cur_queue] > 0 && !winner_selected[cur_queue]) {
                deQueue(cur_queue);
                winner_selected[cur_queue] = TRUE;
                break;
            }
        }
    }
    pthread_mutex_unlock(&queue_mutex[cur_queue]);

    int clerk_woke_me_up = queue_status[cur_queue];
    queue_status[cur_queue] = IDLE;
	
    double start_time = get_current_time();
    overall_waiting_time += (start_time - queue_enter_time);
    printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", start_time, p_myInfo->user_id, clerk_woke_me_up);

    usleep(p_myInfo->service_time * 1000);

    double end_time = get_current_time();
    printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", end_time, p_myInfo->user_id, clerk_woke_me_up);

    pthread_cond_signal(&clerk_cond[clerk_woke_me_up]);
    pthread_exit(NULL);
    return NULL;
}

// Function entry for clerk threads
void *clerk_entry(void *clerkNum) {
    int clerkID = (long)clerkNum;

    while (TRUE) {
        int selected_queue_ID = 1;
        pthread_mutex_lock(&queue_mutex[selected_queue_ID]);
        if (queue_length[0] > 0) {
            selected_queue_ID = 0;
        }

        queue_status[selected_queue_ID] = clerkID;
        pthread_cond_broadcast(&queue_cond[selected_queue_ID]);
        winner_selected[selected_queue_ID] = FALSE;
        pthread_mutex_unlock(&queue_mutex[selected_queue_ID]);

        pthread_cond_wait(&clerk_cond[clerkID], &clerk_mutex[clerkID]);
    }

    pthread_exit(NULL);
    return NULL;
}
