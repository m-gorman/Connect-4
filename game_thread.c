// COMP30023 Project 2
// Michael Gorman
// mgorman
// 641487

// function to handle an individual client's connect 4 game
// code based on connect4 implementation provided on LMS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "game_thread.h"
#include "c4.h"
#include "server.h"


// thread to be run every time a new connection is made
// processes a game with a client
void *c4_game(void *thread_args) 
{
	
	// for getting the current time
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char date_time[20];
	bzero(date_time, 20);
	
	// copy current date time into string
	sprintf(date_time, "%.2d %.2d %.4d %.2d:%.2d:%.2d",  tm.tm_mday, 
				tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	// get arguments
	args_t *args = (args_t*)thread_args;
	int newsockfd = args->sockfd;
	pthread_mutex_t *write_lock = args->write_mutex;
	struct sockaddr_in client_add = args->cli_address;

	// get client ip address
	int len;
	char * client_ip;
	struct sockaddr_in cli_add;
	len = sizeof(cli_add);
	if (getpeername(newsockfd, (struct sockaddr *) &cli_add, &len) < 0) {
			perror("Could not get client address");
	}
	client_ip = inet_ntoa(cli_add.sin_addr);
	
	// get thread id
	pthread_t id = pthread_self();
	
	// initialize the game
	c4_t board;
	srand(RSEED);
	init_empty(board, newsockfd);
	print_config(board, newsockfd);

	
	// to store client / server's moves
	int move;

	// initialize buffer for writing to log file
	char log_buff[100];
	bzero(log_buff, 100);
	sprintf(log_buff, "[%s](%s)(soc_id %d) client connected\n", date_time, client_ip, newsockfd);
	write_to_log(log_buff, write_lock);
	
	// buffer for writing to the socket
	char sock_buff[100];
	bzero(sock_buff, 100);
	
	
	while (1) {
		
		// get client's move and write to log
		bzero(log_buff, 100);
		move = get_move(board, newsockfd);
		sprintf(log_buff, "[%s](%s)(soc_id %d) client's move = %d\n", date_time, client_ip, newsockfd, move);
		write_to_log(log_buff, write_lock);
		bzero(log_buff, 100);

		
		// if move is invalid
		if (do_move(board, move, YELLOW)!=1) {
			sprintf(log_buff, "[%s](%s)(soc_id %d) game over, code = %d\n", date_time, client_ip, newsockfd, STATUS_ABNORMAL);
			write_to_log(log_buff, write_lock);
			write_to_socket(newsockfd, "Panic\n");
			return;
		}
		
		// print config (send to client)
		print_config(board, newsockfd);
		
		// if client wins
		if (winner_found(board) == YELLOW) {
			// rats, the person beat us! 
			sprintf(log_buff, "[%s](%s)(soc_id %d) game over, code = %d\n", date_time, client_ip, newsockfd, STATUS_USER_WON);
			write_to_log(log_buff, write_lock);
			write_to_socket(newsockfd, "Ok, you beat me, beginner's luck!\n");
			return;
		}
		
		// if draw
		if (!move_possible(board)) {
			// yes, looks like it was
			sprintf(log_buff, "[%s](%s)(soc_id %d) game over, code = %d\n", date_time, client_ip, newsockfd, STATUS_DRAW);
			write_to_log(log_buff, write_lock);
			write_to_socket(newsockfd, "An honourable draw\n");
			return;
		}
		
		// get server's move and write to log
		move = suggest_move(board, RED);
		sprintf(log_buff, "[%s](0.0.0.0)(soc_id %d) servers's move = %d\n", date_time, newsockfd, move);
		write_to_log(log_buff, write_lock);

		
		// pretend to be thinking hard 
		write_to_socket(newsockfd, "Ok, let's see now....");
		sleep(1);
		
		// then play the move 
		sprintf(sock_buff, "I play in column %d\n", move);
		write_to_socket(newsockfd, sock_buff);
		bzero(sock_buff, 100);
		
		// if invalid move
		if (do_move(board, move, RED)!=1) {
			sprintf(log_buff, "[%s](%s)(soc_id %d) game over, code = %d\n", date_time, client_ip, newsockfd, STATUS_ABNORMAL);
			write_to_log(log_buff, write_lock);
			write_to_socket(newsockfd, "Panic\n");
			return;
		}
		print_config(board, newsockfd);
		
		// if server won
		if (winner_found(board) == RED) {
			sprintf(log_buff, "[%s](%s)(soc_id %d) game over, code = %d\n", date_time, client_ip, newsockfd, STATUS_AI_WON);
			write_to_log(log_buff, write_lock);
			write_to_socket(newsockfd, "I guess I have your measure!\n");
			return;
		} 
		
	}
	
	// close socket and free buffers
	close(newsockfd);
	free(date_time);
	free(log_buff);
	return NULL;
}

// write a string to the log file, using a mutex to avoid any concurrency issues
void write_to_log(char *str, pthread_mutex_t *lock) {
	// lock the writing mutex so no other threads try to write at the same time
	pthread_mutex_lock(lock);
	FILE *log;
	log = fopen(LOG_FILE_NAME, "a");
	fwrite(str, sizeof(char), strlen(str), log);
	fclose(log);
	pthread_mutex_unlock(lock);
	// all finished, so unlock the mutex
}

// write a string to the designated socket, and conclude with a null byte
void write_to_socket(int sockfd, char *str) {
	int n = write(sockfd, str, strlen(str));
	if (n <= 0) {
		perror("ERROR Writing to client");
		pthread_exit(NULL);
	}
	n = write(sockfd, "\0", 1);
	if (n <= 0) {
		perror("ERROR Writing to client");
		pthread_exit(NULL);
	}
}

char *get_curr_time() {
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	printf ( "%s", asctime (timeinfo) );
}

