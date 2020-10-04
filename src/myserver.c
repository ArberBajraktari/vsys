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
	char buffer[BUF];
	int size, sen_size, rec_size, sub_size, text_size;
	socklen_t addrlen;
	struct sockaddr_in address, cliaddress;

	char sen_uname[8], rec_uname[8];
	char subject[80];
	char filename[0x100];
	char text[BUF];
	FILE *fp;
	

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
	printf("%s", inet_ntoa(cliaddress.sin_addr));
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
		//if send option is selected
		if( strcmp( buffer, "send") == 0){
			//read senders username
			sen_size = recv(new_socket, sen_uname, BUF - 1, 0);
			if( sen_size > 0){
				if(sen_uname[sen_size-1] == '\n') {
				--sen_size;
				}
				sen_uname[sen_size] = '\0';
				//senders username is read 					correctly			
			}else{
				send(new_socket, "ERR\n", 4, 0);
				printf("error sending\n");
			}


			//read recievers username
			rec_size = recv(new_socket, rec_uname, BUF - 1, 0);
			if( rec_size > 0){
				if(rec_uname[rec_size-1] == '\n') {
				--rec_size;
				}
				rec_uname[rec_size] = '\0';
				//ready to save to inbox

				snprintf(filename, sizeof(filename), "inbox/%s.txt", 						rec_uname);
				fp=fopen(filename,"a");
				if( fp == NULL){
					printf("s");
				}
				fputs( sen_uname, fp);
				fputs("\n", fp);
				fflush(fp);

				printf("%s\n", rec_uname);		
			}else{
				send(new_socket, "ERR\n", 4, 0);
				printf("error sending\n");
			}

			//read subject
			sub_size = recv(new_socket, subject, BUF - 1, 0);
			if( sub_size > 0){
				if(subject[sub_size-1] == '\n') {
				--sub_size;
				}
				subject[sub_size] = '\0';
				//ready to save to inbox

				fp=fopen(filename,"a");
				if( fp == NULL){
					printf("s");
				}
				fputs( subject, fp);
				fputs("\n", fp);
				fflush(fp);

				printf("%s\n", subject);		
			}else{
				send(new_socket, "ERR\n", 4, 0);
				printf("error sending\n");
			}
				
			//read text
			text_size = recv(new_socket, text, BUF - 1, 0);
			if( text_size > 0){
				if(text[text_size-1] == '\n') {
				--text_size;
				}
				text[text_size] = '\0';
				//ready to save to inbox

				fp=fopen(filename,"a");
				if( fp == NULL){
					printf("s");
				}
				fputs( text, fp);
				fputs( "\n.\n", fp);
				fflush(fp);
				printf("%s\n", subject);		
			}else{
				send(new_socket, "ERR\n", 4, 0);
				printf("error sending\n");
			}
			send(new_socket, "OK\n", 3, 0);
			fclose(fp);
		}else if( strcmp( buffer, "list") == 0){
			printf("list...\n");
		}else if( strcmp( buffer, "read") == 0){
			printf("read...\n");
		}else if( strcmp( buffer, "del") == 0){
			printf("del...\n");
		}else{
			printf("cmd not known\n");
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
