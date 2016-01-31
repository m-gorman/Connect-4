
// client program for connect4 game
// based on client code on the LMS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void read_socket(int sockfd, char *buff);

int main(int argc, char**argv)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;


	if (argc < 3) 
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);

	
	/* Translate host name into peer's IP address ;
	 * This is name translation service by the operating system 
	 */
	server = gethostbyname(argv[1]);
	
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	
	/* Building data structures for socket */

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);

	serv_addr.sin_port = htons(portno);

	/* Create TCP socket -- active open 
	* Preliminary steps: Setup: creation of active open socket
	*/
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) 
	{
		perror("ERROR opening socket");
		exit(0);
	}
	
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	{
		perror("ERROR connecting");
		exit(0);
	}

	// buffer to store server messages in
	char buffer[1024];

	int gameFinished = 0;
	// loop until the game has been finished
	while (!gameFinished) {
		bzero(buffer, 1024);
		
		// get data from server
		read_socket(sockfd, buffer);

		
		// print data from server
		printf("%s", buffer);
		
		// if server is asking for a column, get an int from the keyboard and send it
		if (strcmp(buffer, "Enter a column number: ") == 0) {
			bzero(buffer,1024);
			fgets(buffer,1023,stdin);
			n = write(sockfd,buffer,strlen(buffer));
		}
		
		// if server reports AI win
		if (strcmp(buffer, "I guess I have your measure!\n") == 0) {
			gameFinished = 1;
		}
		
		// if server reports draw
		if (strcmp(buffer, "An honourable draw\n") == 0) {
			gameFinished = 1;
		}
		
		// if server reports player win
		if (strcmp(buffer, "Ok, you beat me, beginner's luck!\n") == 0) {
			gameFinished = 1;
		}
		
		// if server reports abnormal outcome
		if (strcmp(buffer, "Panic\n") == 0) {
			gameFinished = 1;
		}
	}
	
	
	return 0;
}

// read data into a socket
void read_socket(int sockfd, char *buff) {
	// read characters one at a time from socket to buffer, until
	// null byte is reached
	char curr[1];
	int n = read(sockfd, curr, 1);
	if (n < 0) {
		perror("Error reading from scoket");
		exit(0);
	}
	int len = 0;
	len += sprintf(buff + len, "%c", *curr);
	// read until null byte
	// every message sent by server is terminated by null byte
	while (*curr != '\0') {
		read(sockfd, curr, 1);
		len += sprintf(buff + len, "%c", *curr);
	}
}
