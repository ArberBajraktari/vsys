void clean_stdin(void){
	int c;
	do{
		c = getchar();
	}while(c != '\n' && c != EOF);
}

void check_username(char* reference, char* uname){
	while(1){
		printf("%s: ", reference);
		if( fgets( uname, 9, stdin) != NULL){
			if( strlen( uname) != 8){
				printf("Enter an username with 8 characters!\n");
			}else{
				break;
			}
		}else{
			printf("Error reading the username");
			break;
		}
	}
}

void check_subject(char* sub){
	while(1){
		printf("Subject: ");
		if( fgets( sub, 81, stdin) != NULL){
			break;
		}else{
			printf("Error reading the username");
			break;
		}
	}
}

void check_msg( char* msg){
	char msg_temp[1024];
	//reset variables if the last msg was longer than the current one
	while(1){
		printf("Message: ");
		if( fgets( msg_temp, 1024, stdin) != NULL){
			strcat(msg, msg_temp);
			if( strstr( msg_temp, ".") != 0){
				break;
			}
		}
		else{
			printf("Error reading the senders username\n");
			break;
		}
	}
}



