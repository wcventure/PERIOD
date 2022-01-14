#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define TRACE_SERVER_PORT 2014

int main(int argc, char **argv)
{
	int num, bytesWritten, nread, _serv_sockfd;
	struct sockaddr_in serv_addr = {0};
	char name[1024], buf[64];
	struct hostent *hostptr = NULL;

	/* create endpoint */
	if((_serv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror(NULL);
		return 1;
	}

	gethostname(name, 1024);

	hostptr = gethostbyname(name);

	printf("Connecting to %s\n", name);

	(void) memset(&serv_addr, 0, sizeof(serv_addr));
	(void) memcpy(&serv_addr.sin_addr, hostptr->h_addr, hostptr->h_length);

	/* connect to trace server */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(TRACE_SERVER_PORT);

	if(connect(_serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		perror(NULL);
		return 1;
	}	

	printf("Connected.\n");

	while(gets(buf, 64) && strcmp(buf, "quit")){
		num = 0;
		/*
		for(bytesWritten=0; bytesWritten<64; bytesWritten+=num)
			num = write(_serv_sockfd,
				(void*)((char *)buf+bytesWritten),
				64 - bytesWritten);
		*/
		for(bytesWritten=0; bytesWritten<64; bytesWritten+=num)
			num = send(_serv_sockfd,
				(void*)((char *)buf+bytesWritten),
				64 - bytesWritten, MSG_NOSIGNAL);
		
		if(bytesWritten != 64)
			break;

		nread = 0;
		while((nread += read(_serv_sockfd, buf, 64))< 64)
			;
		printf("%s\n", buf);
		if(!strcmp(buf, "Goodbye."))
			break;
	}	

	return 0;
}
