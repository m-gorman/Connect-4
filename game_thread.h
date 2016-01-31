// function to handle an individual client's connect 4 game


void *c4_game(void *thread_args);
void write_to_log(char *str, pthread_mutex_t *lock);
void write_to_socket(int sockfd, char *str);
char *get_curr_time();

// structure to store arguments for the threads
// sockfd - socket file descriptor
// logfd - log file descriptor
// write_file - mutex file writing lock
typedef struct {
	int sockfd;
	pthread_mutex_t *write_mutex;
	struct sockaddr_in cli_address;
} args_t;
