#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define NQUEUE 2
#define NClerks 4
#define NCustomers 10
#define TRUE 1
#define FALSE 0

/* Function Declarations */
void *customer_entry(void *cus_info);
void *clerk_entry(void *clerkNum);

/* Global Variables */
struct customer_info{
	int user_id;
	int class_type;
	int service_time;
	int arrival_time;
};

struct timeval init_time;
double overall_waiting_time = 0;
int queue_length[NQUEUE] = {0, 0};
int queue_status[NQUEUE] = {0, 0};
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
	// Initialize all the condition variables and thread locks that will be used
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

	gettimeofday(&init_time, NULL);
	/** Read customer information from txt file and store them in the structure you created 
		1. Allocate memory(array, link list etc.) to store the customer information.
		2. File operation: fopen fread getline/gets/fread ..., store information in data structure you created
	*/
	file = fopen("customer_info.txt", "r");
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
	// Create clerk threads (optional)
	pthread_t clerkId[NClerks];
	for(int i = 0; i < NClerks; i++){
		pthread_create(&clerkId[i], NULL, clerk_entry, (void *)&clerk_info[i]);
	}

	// Create customer threads
	pthread_t customId[NCustomers];
	for(int i = 0; i < NCustomers; i++){
		pthread_create(&customId[i], NULL, customer_entry, (void *)&custom_info[i]);
	}

	// Wait for all customer threads to terminate
	for(int i = 0; i < NCustomers; i++){
		pthread_join(customId[i], NULL);
	}

	// Destroy mutex and condition variables (optional)

	// Calculate the average waiting time of all customers
	return 0;
}

// function entry for customer threads

void * customer_entry(void * cus_info){
	
	struct customer_info * p_myInfo = (struct info_node *) cus_info;
	
	usleep(/* the arrival time of this customer */);
	
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id);
	
	/* Enqueue operation: get into either business queue or economy queue by using p_myInfo->class_type*/
	
	
	pthread_mutex_lock(/* mutexLock of selected queue */); 
	{
		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", /*...*/);

		enQueue();
		queue_enter_time = getCurSystemTime();
		queue_length[cur_queue]++;
		
		while (TRUE) {
			pthread_cond_wait(/* cond_var of selected queue */, /* mutexLock of selected queue */);
			if (I_am_Head_of_the_Queue && !winner_selected[cur_queue]) {
				deQueue();
				queue_length[cur_queue]--;
				winner_selected[cur_queue] = TRUE; // update the winner_selected variable to indicate that the first customer has been selected from the queue
				break;
			}
		}
			
	}
	pthread_mutex_unlock(/*mutexLock of selected queue*/); //unlock mutex_lock such that other customers can enter into the queue
	
	/* Try to figure out which clerk awoken me, because you need to print the clerk Id information */
	usleep(10); // Add a usleep here to make sure that all the other waiting threads have already got back to call pthread_cond_wait. 10 us will not harm your simulation time.
	clerk_woke_me_up = queue_status[cur_queue];
	queue_status[cur_queue] = IDLE;
	
	/* get the current machine time; updates the overall_waiting_time*/
	
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", /*...*/);
	
	usleep(/* as long as the service time of this customer */);
	
	/* get the current machine time; */
	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", /* ... */);\
	
	pthread_cond_signal(/* convar of the clerk signaled me */); // Notify the clerk that service is finished, it can serve another customer
	
	pthread_exit(NULL);
	
	return NULL;
}

// function entry for clerk threads
void *clerk_entry(void * clerkNum){
	
	while(TRUE){
		
		/* selected_queue_ID = Select the queue based on the priority and current customers number */
		
		pthread_mutex_lock(/* mutexLock of the selected queue */);
		
		queue_status[selected_queue_ID] = clerkID; // The current clerk (clerkID) is signaling this queue
		
		pthread_cond_broadcast(/* cond_var of the selected queue */); // Awake the customer (the one enter into the queue first) from the selected queue
		
		winner_selected[selected_queue_ID] = FALSE; // set the initial value as the customer has not selected from the queue.
		
		pthread_mutex_unlock(/* mutexLock of the selected queue */);
		
		pthread_cond_wait(/* convar of the current clerk */); // wait for the customer to finish its service
	}
	
	pthread_exit(NULL);
	
	return NULL;
}
