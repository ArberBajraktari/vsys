#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


///////////////////////////////////////////////////////////////////////////////

#define BUF 1024
#define PORT 6543

///////////////////////////////////////////////////////////////////////////////

void* client_cmn(void* sckt);

int main(int argc, char **argv)
{
	int create_socket, new_socket;
	char buffer[BUF];
	socklen_t addrlen;
	struct sockaddr_in address, cliaddress;

   ////////////////////////////////////////////////////////////////////////////
   // CREATE A SOCKET
   // https://man7.org/linux/man-pages/man2/socket.2.html
   // https://man7.org/linux/man-pages/man7/ip.7.html
   // https://man7.org/linux/man-pages/man7/tcp.7.html
   // IPv4, TCP (connection oriented), IP (same as client)
   if((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("Socket error"); // errno set by socket()
      return EXIT_FAILURE;
   }

   ////////////////////////////////////////////////////////////////////////////
   // INIT ADDRESS
   // Attention: network byte order => big endian
   memset(&address, 0, sizeof(address));
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(PORT);

   if( argc != 3){
	printf("Falsche Parametereingabe.\nBitte schreiben Sie die Mailpool und port\n");
      	return EXIT_FAILURE;
   }else{
	if( strcmp(argv[1], "inbox") != 0){
	    printf("Falsche Mailpool.\n");
	    return EXIT_FAILURE;
	}
	//convert to uint_16
	char *fin;
	long prt = strtol( argv[2], &fin, 10);
	address.sin_port = htons((uint16_t)prt);

   }


   ////////////////////////////////////////////////////////////////////////////
   // ASSIGN AN ADDRESS WITH PORT TO SOCKET
   if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) != 0)
   {
      perror("bind error");
      return EXIT_FAILURE;
   }

   ////////////////////////////////////////////////////////////////////////////
   // ALLOW CONNECTION ESTABLISHING
   // Socket, Backlog (= count of waiting connections allowed)
   listen(create_socket, 5);


   while (1)
   {
      printf("Waiting for connections...\n");

      /////////////////////////////////////////////////////////////////////////
      // ACCEPTS CONNECTION SETUP
      // blocking
      addrlen = sizeof(struct sockaddr_in);
      new_socket = accept(create_socket,
                          (struct sockaddr *)&cliaddress,
                          &addrlen);
      if (new_socket > 0)
      {
         printf("Client connected from %s:%d...\n",
                inet_ntoa(cliaddress.sin_addr),
                ntohs(cliaddress.sin_port));
         strcpy(buffer, "Welcome to myserver!\r\nPlease enter your commands...\r\n");
         send(new_socket, buffer, strlen(buffer), 0);
      }

      pthread_t t1;
      pthread_create(&t1, NULL, client_cmn, (void*) &new_socket);
      pthread_detach( t1);

   }

   // frees the descriptor
   close(create_socket);
   return EXIT_SUCCESS;
}

void* client_cmn(void* sckt){
	int new_socket = *(int *) sckt;
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	//int new_socket = *((int*) (&vargp));
	char tosend[BUF];
	char leer = ' ';
	int size;
	char buffer[BUF];
	char name_file[25];

	char filename[0x100];
	char * message;
	char * sndr;
	char * rcv;
	char * sbj;
	char count[5];
	int cnt = 0;
	int email_nmr = 0;
	int email_in = 0;
	bool msg_count = false;
	FILE *fp;
	FILE *fp_temp;
	do
      {


         //////////////////////////////////////////////////////////////////////
         // RECEIVE
         size = recv(new_socket, buffer, BUF - 1, 0);
         if (size > 0)
         {
		// remove ugly debug message, because of the sent newline
		if(buffer[size-1] == '\n') {
			--size;
		}

		buffer[size] = '\0';
		printf("User selected %s option.\n", buffer);
		//if send option is selected
		if( strcmp( buffer, "send") == 0){
			size = recv(new_socket, buffer, BUF - 1, 0);
			printf("Recieved message from user!\n");
			sndr = strtok( buffer, ";");
			rcv = strtok( NULL, ";");
			sbj = strtok( NULL, ";");
			message = strtok( NULL, ".");

			//lock the file, other threads are not allowed to open it
			pthread_mutex_lock( &lock);
			snprintf(filename, sizeof(filename), "inbox/%s.txt", rcv);
			fp = fopen(filename,"a");
			if( fp == NULL){
				send(new_socket, "ERR\n", 4, 0);
			}else{
				fputs("New Email", fp);
				fputs(";\nSender:", fp);
				fputs( sndr, fp);
				fputs(";\nSubject:", fp);
				fputs( sbj, fp);
				fputs(";\nMessage:", fp);
				fputs( message, fp);
				fputs(";\n", fp);
				fflush(fp);
				fclose(fp);
				send(new_socket, "OK\n", 3, 0);
			}
			pthread_mutex_unlock(&lock);

		}else if( strcmp( buffer, "list") == 0){
			memset(buffer, 0, sizeof(buffer));
			memset(tosend, 0, sizeof(tosend));
			memset(name_file, 0, sizeof(name_file));
			size = recv(new_socket, name_file, 25, 0);
			//lock the file
			pthread_mutex_lock( &lock);
			snprintf(filename, sizeof(filename), "inbox/%s.txt", name_file);
			printf("Recieved message from user!\n");
			fp = fopen(filename,"r");
			if(fp == NULL){
				send(new_socket, "0", 1, 0);
				pthread_mutex_unlock( &lock);
			}else{
				cnt = 0;
				//file is being read, other threads should not change it
				pthread_mutex_lock( &lock);
				while( fgets( buffer, BUF - 1, fp)){
					if( strcmp( buffer, "New Email;\n") == 0){
						cnt++;
					}else if( strstr( buffer, "Subject:") != NULL){
						sndr = strtok( buffer, ":");
						sbj = strtok( NULL, ";");
						strcat( tosend, sbj);
						strcat( tosend, "\n");
					}
				}
				pthread_mutex_unlock( &lock);
				if( cnt == 0){
					send(new_socket, "0", 1, 0);
				}else{
					snprintf( count, 5, "%d", cnt);
					send( new_socket, count, BUF - 1, 0);
					send( new_socket, tosend, BUF - 1, 0);
					printf("Emails are shown!\n");
				}

			}

		}else if( strcmp( buffer, "read") == 0){
			memset(buffer, 0, sizeof(buffer));
			memset(tosend, 0, sizeof(tosend));
			memset(name_file, 0, sizeof(name_file));
			size = recv(new_socket, name_file, 25, 0);
			printf("user is %s\n", buffer);
			memset(filename, 0, sizeof(filename));
			//lock the file
			pthread_mutex_lock( &lock);
			snprintf(filename, sizeof(filename), "inbox/%s.txt", name_file);
			printf("Recieved message from %s!\n", filename);
			fp = fopen(filename,"r");
			pthread_mutex_unlock( &lock);
			if(fp == NULL){
				send(new_socket, "0", 1, 0);
			}else{
				send(new_socket, "x", 1, 0);
				message = &leer;
				memset(buffer, 0, sizeof(buffer));
				size = recv(new_socket, buffer, BUF - 1, 0);
				printf("%s", buffer);
				email_nmr = atoi(buffer);
				printf("%d\n", email_nmr);
				cnt = 0;

				memset(buffer, 0, sizeof(buffer));
				//fille is being read
				pthread_mutex_lock( &lock);
				while( fgets( buffer, BUF - 1, fp)){
					if( strcmp( buffer, "New Email;\n") == 0){
						cnt++;
					}
				}
				pthread_mutex_unlock( &lock);

				if( email_nmr > cnt){
					send(new_socket, "ERR\n", 4, 0);
				}else{
					printf("%d\n", cnt);
					email_in = 1;
					fseek( fp, 0, SEEK_SET);
					memset(buffer, 0, sizeof(buffer));
					//file is being read
					pthread_mutex_lock( &lock);
					while( fgets( buffer, BUF - 1, fp)){
						if( strstr( buffer, "Message:") != NULL || msg_count){
							if( email_in == email_nmr){
								//clear what is not needed
								if( strstr( buffer, ":") != NULL){
									sndr = strtok( buffer, ":");
									strncat( tosend, strtok( NULL, ";"), sizeof(strtok( NULL, ";")));
									msg_count = true;
								}else{
									if( strstr( buffer, ";") != NULL){
										strncat( tosend, strtok( buffer, ";"), sizeof(strtok( buffer, ";")));
										msg_count = false;
										break;
									}else{
										strncat( tosend, buffer, strlen(buffer));
										msg_count = true;
									}
								}

								email_in--;
							}
							email_in++;
						}
					}
					pthread_mutex_unlock( &lock);

					if( message != NULL){
						printf("%s\n", tosend);
						send(new_socket, tosend, BUF - 1, 0);
					}else{
						send(new_socket, "ERR\n", 4, 0);
					}

				}

			}
		}else if( strcmp( buffer, "del") == 0){
			fp_temp = fopen( "inbox/del.txt", "w");
			memset(buffer, 0, sizeof(buffer));
			memset(name_file, 0, sizeof(name_file));
			size = recv(new_socket, name_file, 25, 0);
			printf("Recieved message from user!\n%s\n", buffer);
			memset(filename, 0, sizeof(filename));
			//lock file
			pthread_mutex_lock( &lock);
			snprintf(filename, sizeof(filename), "inbox/%s.txt", name_file);
			printf("Recieved message from user!\n");
			fp = fopen(filename,"r");
			pthread_mutex_unlock( &lock);
			if( fp_temp == NULL){
				printf("%s\n", strerror(errno));
			}
			if(fp == NULL){
				send(new_socket, "ERR\n", 4, 0);
			}else{
				send(new_socket, "x", 1, 0);
				memset(buffer, 0, sizeof(buffer));
				size = recv(new_socket, buffer, 25, 0);
				email_nmr = atoi(buffer);
				cnt = 0;
				//file being read
				pthread_mutex_lock( &lock);
				while( fgets( buffer, BUF - 1, fp)){
					if( strcmp( buffer, "New Email;\n") == 0){
						cnt++;
					}
				}
				pthread_mutex_unlock( &lock);

				if( email_nmr > cnt){
					send(new_socket, "ERR\n", 4, 0);
				}else{
					email_in = 0;
					fseek( fp, 0, SEEK_SET);
					//file being read
					pthread_mutex_lock( &lock);
					while( fgets( buffer, BUF - 1, fp)){
						if( strstr( buffer, "New Email;") != NULL){
							email_in++;
						}
						if( email_in != email_nmr){
							fputs( buffer, fp_temp);
							printf("%s\n", buffer);
						}
					}
					fflush(fp_temp);
					fclose(fp_temp);
					remove( filename);
					//deleting an message from a user
					rename( "inbox/del.txt", filename);
					pthread_mutex_unlock( &lock);
					send(new_socket, "OK\n", BUF - 1, 0);
				}
			}
		}

         }
         else if (size == 0)
         {
            printf("Client closed remote socket\n");
            break;
         }
         else
         {
            perror("recv error");
         }

      } while (strcmp(buffer, "quit") != 0);

      close(new_socket);
      pthread_exit(0);
}
