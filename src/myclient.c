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
	char buffer[BUF], to_send[BUF];
	char sender[8], reciever[8], subject[80], msg[BUF], msg_temp[BUF];
	struct sockaddr_in address;
	int size;
	int email_nr;


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
      memset( buffer, 0, sizeof( buffer));
      memset( to_send, 0, sizeof( to_send));
      if (fgets(buffer, BUF, stdin) != NULL)
      {
		//////////////////////////////////////////////////////////////////////
		// SEND DATA
		// https://man7.org/linux/man-pages/man2/send.2.html
		send(create_socket, buffer, strlen(buffer), 0);
		if( strcmp( buffer, "send\n") == 0){
			memset( sender, 0, sizeof( sender));
			while(1){
				printf("Sender: ");
				if( fgets( sender, BUF, stdin) != NULL){
					if( strlen( sender) != 9){
						printf("Enter an username with 8 characters!\n");
					}else{
						sender[8] = '\0';
						strcat( to_send, sender);
						strcat( to_send, ";");
						break;
					}
				}
			}
			memset( reciever, 0, sizeof( reciever));
			while(1){
				printf("Reciever: ");
				if( fgets( reciever, BUF, stdin) != NULL){
					if( strlen( reciever) != 9){
						printf("Enter an username with 8 characters!\n");
					}else{
						reciever[8] = '\0';
						strcat( to_send, reciever);
						strcat( to_send, ";");
						break;
					}
				}
			}
			memset( subject, 0, sizeof( subject));
			while(1){
				printf("Subject: ");
				if( fgets( subject, BUF, stdin) != NULL){
					if( strlen( subject) > 80){
						printf("Enter an Subject with less than 80 characters!\n");
					}else{
						subject[strlen(subject) - 1] = '\0';
						strcat( to_send, subject);
						strcat( to_send, ";");
						break;
					}
				}
			}
			memset( msg_temp, 0, sizeof( msg_temp));
			memset( msg, 0, sizeof( msg));
			while(1){
				printf("Message: ");
				if( fgets( msg_temp, BUF, stdin) != NULL){
					strcat(msg, msg_temp);
					if( strstr( msg_temp, ".") != 0){
						strcat( to_send, msg);
						send(create_socket, to_send, strlen(to_send), 0);
						break;
					}
				}
			}
			
			size = recv(create_socket, buffer, BUF - 1, 0);
			if (size > 0)
			{
				buffer[size] = '\0';
				printf("%s", buffer);
			}else{
				printf("ERR\n");
			}
			
			// list ---------------------------------------------
		}else if( strcmp( buffer, "list\n") == 0){
			memset( reciever, 0, sizeof( reciever));
			memset( buffer, 0, sizeof( buffer));
			while(1){
				printf("Username: ");
				if( fgets( reciever, BUF, stdin) != NULL){
					if( strlen( reciever) != 9){
						printf("Enter an username with 8 characters!\n");
					}else{
						reciever[8] = '\0';
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
						break;
					}
				}
			}
		}else if( strcmp( buffer, "read\n") == 0){
			memset( reciever, 0, sizeof( reciever));
			while(1){
				printf("Username: ");
				if( fgets( reciever, BUF, stdin) != NULL){
					if( strlen( reciever) != 9){
						printf("Enter an username with 8 characters!\n");
					}else{
						reciever[8] = '\0';
						send(create_socket, reciever, strlen(reciever), 0);
						memset( buffer, 0, sizeof( buffer));
						size = recv(create_socket, buffer, BUF - 1, 0);
						if( strcmp( buffer, "0") == 0 ){
							printf( "User has %s Emails.\n", buffer);
							break;
						}else{
							memset( buffer, 0, sizeof( buffer));
							printf( "Write email number:");
							char email_nr[4];
							fgets( email_nr, 4, stdin);
							send(create_socket, email_nr, sizeof(email_nr), 0);
							size = recv(create_socket, buffer, BUF - 1, 0);
							printf("%s", buffer);
							break;
							
							
						}
						// print subjects
					}
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
