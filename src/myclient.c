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


void clear_input(void){
	int input;
	do{
		input = getchar();
	}while(input != '\n' && input != EOF);
}

void snd_check(char* snd){
	while(1){
		printf("Sender: ");
		if( fgets( snd, 9, stdin) != NULL){
			if( strlen( snd) != 8){
				printf("Enter an username with 8 characters!\n");
			}else{
				break;
			}
		}else{
			printf("Error reading the Sender");
			break;
		}
	}
}
void rcv_check(char* rcv){
	while(1){
		printf("Receiver: ");
		if( fgets( rcv, 9, stdin) != NULL){
			if( strlen( rcv) != 8){
				printf("Enter an username with 8 characters!\n");
			}else{
				break;
			}
		}else{
			printf("Error reading the Reciever");
			break;
		}
	}
}

void sbj_check(char* sbj){
	while(1){
		printf("Subject: ");
		if( fgets( sbj, 81, stdin) != NULL){
			break;
		}else{
			printf("Error reading the Subject");
			break;
		}
	}
}

void msg_check( char* msg){
	//check the message
	char msg_temp[1024];
	while(1){
		printf("Message: ");
		if( fgets( msg_temp, 1024, stdin) != NULL){
			//find x
			strcat(msg, msg_temp);
			if( strstr( msg_temp, ".") != 0){
				break;
			}
		}
		else{
			printf("Error reading the snds username\n");
			break;
		}
	}
}





int main(int argc, char **argv)
{
	int create_socket;
	char buffer[BUF] = "", tosend[BUF] = "";
	char snd[9], rcv[9], sbj[81], msg[BUF]="";
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
      memset( tosend, 0, sizeof( tosend));
      if (fgets(buffer, BUF, stdin) != NULL)
      {
		//////////////////////////////////////////////////////////////////////
		// SEND DATA
		send(create_socket, buffer, strlen(buffer), 0);
		if( strcmp( buffer, "send\n") == 0){
			snd_check( snd);
			clear_input();
			rcv_check( rcv);
			clear_input();
			sbj_check( sbj);
			strtok( sbj, "\n");
			msg_check( msg);

			strcat( tosend, snd);
			strcat( tosend, ";");
			strcat( tosend, rcv);
			strcat( tosend, ";");
			strcat( tosend, sbj);
			strcat( tosend, ";");
			strcat( tosend, msg);
			send(create_socket, tosend, strlen(tosend), 0);
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
			memset( buffer, 0, sizeof( buffer));
			rcv_check( rcv);
			clear_input();
			send(create_socket, rcv, strlen(rcv), 0);
			size = recv(create_socket, buffer, BUF - 1, 0);
			if (size > 0)
			{
				if( strcmp( buffer, "0") == 0){
					printf("%s has %s Emails.\n", rcv, buffer);
				}else{
					printf("%s has %s Emails.\n", rcv, buffer);
					size = recv(create_socket, buffer, BUF - 1, 0);
					// print sbjs
					printf( "%s", buffer);
				}

			}else{
				printf("ERR\n");
			}

		}else if( strcmp( buffer, "read\n") == 0){
			memset( buffer, 0, sizeof( buffer));
			rcv_check( rcv);
			clear_input();
			send(create_socket, rcv, strlen(rcv), 0);
			size = recv(create_socket, buffer, BUF - 1, 0);
			if (size > 0)
			{
				if( strcmp( buffer, "0") == 0){
					printf("%s has %s Emails.\n", rcv, buffer);
				}else{
					printf( "Write email number:");

					if( fgets( email_nr, 4, stdin) != NULL){
						send(create_socket, email_nr, sizeof(email_nr), 0);
						size = recv(create_socket, buffer, BUF - 1, 0);
						printf("Message: \n\n");
						printf( "%s", buffer);
					}else{
						printf("Error reading the sbj number!\n");
					}

				}

			}else{
				printf("ERR\n");
			}

		}else if( strcmp( buffer, "del\n") == 0){
			memset( buffer, 0, sizeof( buffer));
			rcv_check( rcv);
			clear_input();
			send(create_socket, rcv, strlen(rcv), 0);
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
						printf("Error reading the sbj number!\n");
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
