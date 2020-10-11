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
	int create_socket, new_socket;
	int size;
	char buffer[BUF], temp[BUF], to_send[BUF];
	socklen_t addrlen;
	struct sockaddr_in address, cliaddress;

	char filename[0x100];
	char * msg;
	char * sender;
	char * reciever;
	char * subject;
	int cnt = 0;
	int e_nr = 0;
	int in_email = 0;
	FILE *fp;
	int waiting = 0;
	

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

   if( argc < 2){
	printf("Falsche Parametereingabe.\nBitte schreiben Sie die Mailsverzsichniss\n");
      	return EXIT_FAILURE;
   }else{
	if( strcmp(argv[1], "inbox") != 0){
	    printf("Falsche Verzeichnisseingabe.\n");	
	    return EXIT_FAILURE;
	}
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

      // think here about thread-handling or forking ...

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
			
			memset(buffer, 0, sizeof(buffer));
			size = recv(new_socket, buffer, BUF - 1, 0);
			printf("Recieved message from user!\n");
			sender = strtok( buffer, ";");
			reciever = strtok( NULL, ";");
			subject = strtok( NULL, ";");
			msg = strtok( NULL, ".");
			
			snprintf(filename, sizeof(filename), "inbox/%s.txt", reciever);
			fp = fopen(filename,"a");
			if( fp == NULL){
				send(new_socket, "ERR\n", 4, 0);
			}else{
				fputs("New Email", fp);
				fputs(";\nSender:", fp);
				fputs( sender, fp);
				fputs(";\nSubject:", fp);
				fputs( subject, fp);
				fputs(";\nMsg:", fp);
				fputs( msg, fp);
				fputs(";\n", fp);
				fflush(fp);
				fclose(fp);
				send(new_socket, "OK\n", 3, 0);
			}
			
		}else if( strcmp( buffer, "list") == 0){
			memset(buffer, 0, sizeof(buffer));
			size = recv(new_socket, buffer, BUF - 1, 0);
			snprintf(filename, sizeof(filename), "inbox/%s.txt", buffer);
			printf("Recieved message from user!\n");
			fp = fopen(filename,"r");
			if(fp == NULL){
				send(new_socket, "0", 1, 0);
			}else{
				cnt = 0;
				memset(buffer, 0, sizeof(buffer));
				char * temp = (char *)malloc(512);
				memset(temp, 0, sizeof(temp));
				while( fgets( buffer, BUF - 1, fp)){
					if( strcmp( buffer, "New Email;\n") == 0){
						cnt++;
					}else if( strstr( buffer, "Subject:") != NULL){
						sender = strtok( buffer, ":");
						subject = strtok( NULL, ";");
						strcat( temp, subject);
						strcat( temp, "\n");
					}
				}
				char *count;
				snprintf( count, 10, "%d", cnt);
				send(new_socket, count, BUF - 1, 0);
				send( new_socket, temp, BUF - 1, 0);
				printf("Emails are shown!\n");
			}

		}else if( strcmp( buffer, "read") == 0){
			memset(buffer, 0, sizeof(buffer));
			size = recv(new_socket, buffer, BUF - 1, 0);
			printf("Recieved message from user!\n%s\n", buffer);
			memset(filename, 0, sizeof(filename));
			snprintf(filename, sizeof(filename), "inbox/%s.txt", buffer);
			printf("Recieved message from user!\n");
			fp = fopen(filename,"r");
			if(fp == NULL){
				send(new_socket, "0", 1, 0);
			}else{
				send(new_socket, "x", 1, 0);
				memset(buffer, 0, sizeof(buffer));
				size = recv(new_socket, buffer, BUF - 1, 0);
				printf("%s", buffer);
				e_nr = atoi(buffer);
				printf("%d\n", e_nr);
				cnt = 0;
				while( fgets( buffer, BUF - 1, fp)){
					if( strcmp( buffer, "New Email;\n") == 0){
						cnt++;
					}
				}
				
				if( e_nr > cnt){
					send(new_socket, "ERR\n", 4, 0);
				}else{
					printf("%d\n", cnt);
					in_email = 1;
					fseek( fp, 0, SEEK_SET);
					while( fgets( buffer, BUF - 1, fp)){
						if( strstr( buffer, "Msg:") != NULL){
							if( in_email == e_nr || waiting){	
								printf("%s", buffer);
								sender = strtok( buffer, ":");
								msg = strtok( NULL, ";");
								printf("%s", msg);
							}
							in_email++;
						}
					}
					send(new_socket, "ok\n", BUF - 1, 0);
				}
				
				
				
			}
			
		}else if( strcmp( buffer, "del") == 0){
			printf("del...\n");
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
            return EXIT_FAILURE;
         }

      } while (strcmp(buffer, "quit") != 0);

      // frees the descriptor
      close(new_socket);
   }

   // frees the descriptor
   close(create_socket);
   return EXIT_SUCCESS;
}
