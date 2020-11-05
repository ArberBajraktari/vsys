#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "functions.h"

///////////////////////////////////////////////////////////////////////////////

#define BUF 1024
#define PORT 6543

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	int create_socket;
	char buffer[BUF] = "", to_send[BUF] = "";
	char sender[9], reciever[9], subject[81], msg[BUF]="";
	struct sockaddr_in address;
	int size;
	char email_nr[4];


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
   else if( argc == 2)
   {
		inet_aton(argv[1], &address.sin_addr);
   }else if( argc == 3){
		inet_aton(argv[1], &address.sin_addr);
		char *end;
		long prt = strtol( argv[2], &end, 10);
		address.sin_port = htons((uint16_t)prt);
   }

   ////////////////////////////////////////////////////////////////////////////
   // CREATE A CONNECTION
   if (connect(create_socket, 
               (struct sockaddr *)&address, 
               sizeof(address)) == 0)
   {
      printf("Connection with server (%s) established\n", 
             inet_ntoa(address.sin_addr));


      /////////////////////////////////////////////////////////////////////////
      // RECEIVE DATA
      size = recv(create_socket, buffer, BUF - 1, 0);
      if (size > 0)
      {
         buffer[size] = '\0';
         printf("%s", buffer);
      }
   }
   else
   {
      perror("Connect error - no server available");
      return EXIT_FAILURE;
   }

   do
   {
      printf("Send message: ");
      memset( buffer, 0, sizeof( buffer));
      memset( to_send, 0, sizeof( to_send));
      if (fgets(buffer, BUF, stdin) != NULL)
      {
		//////////////////////////////////////////////////////////////////////
		// SEND DATA
		send(create_socket, buffer, strlen(buffer), 0);
		if( strcmp( buffer, "send\n") == 0){
			check_username("Sender", sender);
			clean_stdin();
			check_username("Reciever", reciever);
			clean_stdin();
			check_subject( subject);
			strtok( subject, "\n");
			check_msg( msg);
			
			strcat( to_send, sender);
			strcat( to_send, ";");
			strcat( to_send, reciever);
			strcat( to_send, ";");
			strcat( to_send, subject);
			strcat( to_send, ";");
			strcat( to_send, msg);
			send(create_socket, to_send, strlen(to_send), 0);
			memset(buffer, 0, sizeof(buffer));
			size = recv(create_socket, buffer, BUF - 1, 0);
			if (size > 0)
			{
				printf("%s", buffer);
			}else{
				printf("ERR\n");
			}
			
			// list ---------------------------------------------
		}else if( strcmp( buffer, "list\n") == 0){
			printf("null\n");
			memset( buffer, 0, sizeof( buffer));
			check_username("Username", reciever);
			clean_stdin();
			send(create_socket, reciever, strlen(reciever), 0);
			size = recv(create_socket, buffer, BUF - 1, 0);
			
			if (size > 0)
			{
				if( strcmp( buffer, "0") == 0){
					printf("%s has %s Emails.\n", reciever, buffer);
				}else{
					printf("%s has %s Emails.\n", reciever, buffer);
					size = recv(create_socket, buffer, BUF - 1, 0);
					// print subjects
					printf( "%s", buffer);
				}
				
			}else{
				printf("ERR\n");
			}
			
		}else if( strcmp( buffer, "read\n") == 0){
			memset( buffer, 0, sizeof( buffer));
			check_username("Username", reciever);
			clean_stdin();
			send(create_socket, reciever, strlen(reciever), 0);
			size = recv(create_socket, buffer, BUF - 1, 0);
			if (size > 0)
			{
				if( strcmp( buffer, "0") == 0){
					printf("%s has %s Emails.\n", reciever, buffer);
				}else{
					printf( "Write email number:");
					
					if( fgets( email_nr, 4, stdin) != NULL){
						send(create_socket, email_nr, sizeof(email_nr), 0);
						size = recv(create_socket, buffer, BUF - 1, 0);
						printf("Message: \n\n");
						printf( "%s", buffer);
					}else{
						printf("Error reading the subject number!\n");
					}
					
				}
				
			}else{
				printf("ERR\n");
			}
			
		}else if( strcmp( buffer, "del\n") == 0){
			memset( buffer, 0, sizeof( buffer));
			check_username("Username", reciever);
			clean_stdin();
			send(create_socket, reciever, strlen(reciever), 0);
			size = recv(create_socket, buffer, BUF - 1, 0);
			if (size > 0)
			{
				if( strcmp( buffer, "ERR\n") == 0 ){
					printf( "%s", buffer);
					break;
				}else{
					printf( "Write email number:");
					if( fgets( email_nr, 4, stdin) != NULL){
						send(create_socket, email_nr, sizeof(email_nr), 0);
						size = recv(create_socket, buffer, BUF - 1, 0);
						printf( "%s", buffer);
					}else{
						printf("Error reading the subject number!\n");
					}
				}
			}else{
				printf("ERR\n");
			}
		}else if( strcmp( buffer, "quit\n") != 0){
			printf("Please select between:\nsend, list, read, del or quit\n");
		}
      }
	
   } while (strcmp(buffer, "quit\n") != 0);

   ////////////////////////////////////////////////////////////////////////////
   // CLOSES THE DESCRIPTOR
   close(create_socket);
   return EXIT_SUCCESS;
}
