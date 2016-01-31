// server implementation to handle client connections for connect4 game
// based on server code on LMS

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



pthread_mutex_t write_lock;

int main(int argc, char **argv)
{
	int counter = 0;
	// initialize
	// sock_addr_in is a structure containing an internet address
	int sockfd, newsockfd, portno, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	if (argc < 2) 
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	 /* Create TCP socket */
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) 
	{
		perror("ERROR opening socket");
		exit(1);
	}

	// clear the server address just in case
	bzero((char *) &serv_addr, sizeof(serv_addr));

	portno = atoi(argv[1]);
	
	/* Create address we're going to listen on (given port number)
	 - converted to network byte order & any IP address for 
	 this machine */
	
	serv_addr.sin_family = AF_INET;
	// s_addr is the ip address of the host. INADDR_ANY gets this address.
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	// htons() convers a port number in host byte order to a port number in network byte order
	serv_addr.sin_port = htons(portno);  // store in machine-neutral format

	 /* Bind address to the socket */
	// bind the socket to the address in this case the address is the address of the current host and port number
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0) 
	{
		perror("ERROR on binding");
		exit(1);
	}
	
	/* Listen on socket - means we're ready to accept connections - 
	 incoming connection requests will be queued */
	// first argument is the socket file descriptor, second is the size of the backlog queue
	// this is the number of connections taht can be waiting while the process is handling a particular connection
	listen(sockfd,5);
	
	clilen = sizeof(cli_addr);

	/* Accept a connection - block until a connection is ready to
	 be accepted. Get back a new file descriptor to communicate on. */
	pthread_t tid; /* thread identifier */
	
	// open log file for appending


	args_t *thread_args;

	// reset log file
	FILE *fd;
	fd = fopen(LOG_FILE_NAME, "w+");
	if (fd < 0) {
		perror("Log init failed\n");
		exit(EXIT_FAILURE);
	}
	if (fclose(fd) < 0) {
		perror("Log init failed\n");
		exit(EXIT_FAILURE);
	}
	
	// initialize mutex
	if (pthread_mutex_init(&write_lock, NULL) != 0)
    {
        perror("\n mutex init failed\n");
        exit(0);
    }
	
	// process will sleep until a connection with a client is successfully made
	// loop so we keep getting new connections
	while (newsockfd = accept(	sockfd, (struct sockaddr *) &cli_addr, &clilen)) {
		// set up arguments for the thread
		thread_args = malloc(sizeof(args_t));
		thread_args->sockfd = newsockfd;
		thread_args->write_mutex = &write_lock;
		thread_args->cli_address = cli_addr;
	
		pthread_create(&tid, NULL, c4_game, (void*)thread_args);

		if (newsockfd < 0) 
		{
			perror("ERROR on accept");
			exit(1);
		}
	}
	
	return 0; 
}


