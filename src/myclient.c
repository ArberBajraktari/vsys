#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////

#define BUF 1024
#define PORT 6543

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	int create_socket;
	char buffer[BUF];
	char sender[8];
	struct sockaddr_in address;
	int size;


   ////////////////////////////////////////////////////////////////////////////
   // CREATE A SOCKET
   // https://man7.org/linux/man-pages/man2/socket.2.html
   // https://man7.org/linux/man-pages/man7/ip.7.html
   // https://man7.org/linux/man-pages/man7/tcp.7.html
   // IPv4, TCP (connection oriented), IP (same as server)
   if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("Socket error");
      return EXIT_FAILURE;
   }

   ////////////////////////////////////////////////////////////////////////////
   // INIT ADDRESS
   // Attention: network byte order => big endian
   memset(&address, 0, sizeof(address)); // init storage with 0
   address.sin_family = AF_INET;         // IPv4
   // https://man7.org/linux/man-pages/man3/htons.3.html
   address.sin_port = htons(PORT);       
   // https://man7.org/linux/man-pages/man3/inet_aton.3.html
   if (argc < 2)
   {
      inet_aton("127.0.0.1", &address.sin_addr); 
   }
   else
   {
      inet_aton(argv[1], &address.sin_addr);
   }

   ////////////////////////////////////////////////////////////////////////////
   // CREATE A CONNECTION
   // https://man7.org/linux/man-pages/man2/connect.2.html
   if (connect(create_socket, 
               (struct sockaddr *)&address, 
               sizeof(address)) == 0)
   {
      printf("Connection with server (%s) established\n", 
             inet_ntoa(address.sin_addr));


      /////////////////////////////////////////////////////////////////////////
      // RECEIVE DATA
      // https://man7.org/linux/man-pages/man2/recv.2.html
      size = recv(create_socket, buffer, BUF - 1, 0);
      if (size > 0)
      {
         buffer[size] = '\0';
         printf("%s", buffer);
      }
   }
   else
   {
      // https://man7.org/linux/man-pages/man3/perror.3.html
      perror("Connect error - no server available");
      return EXIT_FAILURE;
   }

   do
   {
      printf("Send message: ");
      if (fgets(buffer, BUF, stdin) != NULL)
      {
		//////////////////////////////////////////////////////////////////////
		// SEND DATA
		// https://man7.org/linux/man-pages/man2/send.2.html
		send(create_socket, buffer, strlen(buffer), 0);
		printf("%s", buffer);
		if( strcmp( buffer, "send\n") == 0){
			do{
				printf(" 8 char pls\nSender: ");
				scanf( "%s", sender);
				size = strlen(sender);
			}while( size != 8);
		}else if( strcmp( buffer, "list\n") == 0){
			if( fgets( buffer, BUF, stdin) != NULL){
				if( strlen( buffer) != 9){
					printf("Enter an username with 8 characters!\n");
				}else{
					printf("ok");
				}
			}
		}else if( strcmp( buffer, "read\n") == 0){
			if( fgets( buffer, BUF, stdin) != NULL){
				if( strlen( buffer) != 9){
					printf("Enter an username with 8 characters!\n");
				}else{
					printf("ok");
				}
			}
		}else if( strcmp( buffer, "del\n") == 0){
			if( fgets( buffer, BUF, stdin) != NULL){
				if( strlen( buffer) != 9){
					printf("Enter an username with 8 characters!\n");
				}else{
					printf("ok");
				}
			}
		}
		/*
		//////////////////
		// send
		//////////////////
		
			fgets(buffer, BUF, stdin);
			send(create_socket, buffer, strlen(buffer), 0);
			printf("Reciever username: ");
			fgets(buffer, BUF, stdin);
			send(create_socket, buffer, strlen(buffer), 0);
			printf("Subject: ");
			fgets(buffer, BUF, stdin);
			send(create_socket, buffer, strlen(buffer), 0);
			printf("Message: ");
			fgets(buffer, BUF, stdin);
			send(create_socket, buffer, strlen(buffer), 0);
			size = recv(create_socket, buffer, BUF - 1, 0);
			if (size > 0)
			{
				buffer[size] = '\0';
				printf("%s", buffer);
			}else{
				printf("cant read");
			}

		}
		//////////////////
		// list
		//////////////////		
		else if( strcmp( buffer, "list\n") == 0){
			printf("Username: ");
			scanf(" %s", buffer);
			send(create_socket, buffer, strlen(buffer), 0);
		}
		//////////////////
		// read
		//////////////////
		else if( strcmp( buffer, "read\n") == 0){
			printf("read...\n");
		}
		//////////////////
		// del
		//////////////////
		else if( strcmp( buffer, "del\n") == 0){
			printf("del...\n");
		}
*/
      }
	
   } while (strcmp(buffer, "quit\n") != 0);

   ////////////////////////////////////////////////////////////////////////////
   // CLOSES THE DESCRIPTOR
   close(create_socket);
   return EXIT_SUCCESS;
}
