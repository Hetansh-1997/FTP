// We have took some of the code from our previous assignment 1,2,3 and 4

#include<stdio.h>
#include<netinet/in.h>
#include<sys/un.h>
#include <arpa/inet.h>
#include<sys/socket.h>
#include<stdlib.h>
#include <errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/signal.h>
#define buffSize 4096	// we have defined the maximum size of data is 4096.
int clientDescriptor,newSocketDescriptor=0,file;	// these are the global variable and used to close the connections when client is terminated. clientDescriptor is used for command channel socket, newSocketDescriptor is used for data port, file are random files open and use in store.
char* readServerResponse(int fd){	// this function is used to read input line by line from the stdin.
	char *buffer,character;	// buffer is a array where I will store the line and character variable will be use to read one by one character
	int i=0,length,n;	//	initalizing the i to 0 and it will be use to iterate buffer array.
	buffer=malloc(sizeof(char)*buffSize);	// assigning the memory so that it can store upto buffSize characters.
	while((n=read(fd,&character,1))>0){	// this will read one by one character from the stdin till end of file
		//printf("%c %d",character,character);
		if(character=='\0'){ // this will check if the line is read by checking current character if it is '\n', EOF or ';' and if condition satisfied means line is read and below will be executed.
			return buffer;	// this will return the whole line to the calling function
		}	
		buffer[i]=character;	// if it is not end of a line then it will assign one by one characters in buffer array.
		i++;	//this is incrementing the i variable by 1 everytime a character is assigned to ith(before incrementing i) index of buffer array.
	}
	buffer[i]='\0';	// this is to add a null character to indicate end of string
	return buffer;	// this will return the buffer
}
char* readRequest(int fd){	// this function is used to read input line by line from the stdin.
	char *buffer,character;	// buffer is a array where I will store the line and character variable will be use to read one by one character
	int i=0,length,n;	//	initalizing the i to 0 and it will be use to iterate buffer array.
	buffer=malloc(sizeof(char)*buffSize);	// assigning the memory so that it can store upto buffSize characters.
	while((n=read(fd,&character,1))>0){	// this will read one by one character from the stdin till end of file
		if(character=='\n' || character=='\0'){ // this will check if the line is read by checking current character if it is '\n', EOF or ';' and if condition satisfied means line is read and below will be executed.
			buffer[i]='\0';		// this is to add a null character to indicate end of string
			return buffer;	// this will return the whole line to the calling function
		}	
		buffer[i]=character;	// if it is not end of a line then it will assign one by one characters in buffer array.
		i++;	//this is incrementing the i variable by 1 everytime a character is assigned to ith(before incrementing i) index of buffer array.
	}
	buffer[i]='\0';	// this is to add a null character to indicate end of string
	return buffer;	// this will return the buffer
}
int retrCommand(int newSocketDescriptor,char *input){	// this function is used to RETR command and to handle response of the command
	int file;	// this will be used to store the descriptor of the file.
	if(newSocketDescriptor<0){	// this  will check if data port is open if data port is not open then it will fail and return -1 that port is not working
		printf("PORT is not working");	// this will print the PORT is not working
		close(newSocketDescriptor);	// this will close the data port
		return -1;	// this will return -1
	}
	char *r=strtok(input+5," ");	// This will take the first argument of the command which is the remoteFile
	char *l=malloc(sizeof(input));	// This help to store localFile
	l=strtok(NULL," ");		// this will strtok to store the second argument of the command which is localFile
	if(l==NULL){	// if localFile is null then below code will be executed.
		if(access(r,F_OK)!=-1){	// this will check if the file exists.
			remove(r);	// this will remove the existing file.
		}
		if((file=open(r,O_WRONLY|O_CREAT,0777))==-1){	// this will open the file
			printf("Issue");	// this will print the issue.
			close(newSocketDescriptor);	// this will close the data port.
			return -1;	// this will retun -1
		}
		char buffer;	// this will be used to store each byte of the data read and that same variable is used to write in file.
		while((read(newSocketDescriptor,&buffer,1))>0){	// this loop until the socket descriptor has data.
			if((write(file,&buffer,1))!=1){	// this will write each every by in the file.
				printf("Issue");	// this will print Issue if there is any issue.
				close(newSocketDescriptor);	// this will close the socket descriptor.
				return -1;	// this will return -1;
			}
		}
	}else{
		if(access(l,F_OK)!=-1){	// this will check if the file exists.
			remove(l);	// this will remove the existing file.
		}
		if((file=open(l,O_WRONLY|O_CREAT,0777))==-1){	// this will open the file
			printf("Issue");	// this will print Issue if there is any issue.
			close(newSocketDescriptor);	// this will close the socket descriptor.
			return -1;// this will return -1;
		}
		char buffer;	// this will be used to store each byte of the data read and that same variable is used to write in file.
		while((read(newSocketDescriptor,&buffer,1))>0){
			if((write(file,&buffer,1))!=1){	// this loop until the socket descriptor has data.
				printf("Issue");	// this will print Issue if there is any issue.
				close(newSocketDescriptor);	// this will close the socket descriptor.
				return -1;	// this will return -1;
			}
		}
	}
	printf("%s",readServerResponse(clientDescriptor));
	printf("\nFile is created\n");	// this will print file is created 
	close(newSocketDescriptor);	// this will close the data socket  
}
int append(int client,int clientDescriptor,char *input){	// this function will handle the response for the append from server.
	if(newSocketDescriptor<1){	// this will check if data port is open if no then it will print Port is not working
		printf("PORT is not working\n");	// printing port is not working
		return -1;	// this will continue;
	}
	char filename[strlen(input)];	// this will be used to store the filename of the local
	strcpy(filename,input);	// this will copy the input to filename.
	char *command=strtok(filename," ");	// this will extract the first argument of the command which is file
	char *temp=malloc(sizeof(filename));
	temp=strtok(NULL," ");
	if((file=open(temp,O_RDONLY))==-1){	// this will try to open the file in read only mode and if it returns -1 then if block will be printed
		printf("File is not present in client\n");	// this will print that there is an issue.
		close(file);
		return -1;	// this will continue;
	}
	int length=lseek(file,0,SEEK_END);	// this will get the length of the file
	lseek(file,0,SEEK_SET);		// this will again bring back the pointer to start of the file
	int n;
	char buffer[length];	// this will help to read the whole data from file and write it in data port.
	if((read(file,buffer,length))>0){	// this will read whole data from file
		if((n=write(client,buffer,length))>0){	// this will write whole data to data port.
			
		}
	}
	return 0;	// this will return zero states that the function is executed successfully.
}


int listCommand(int clientDescriptor,int newSocketDescriptor){	// this function will help to handle the response of the list command
	char *response;	// this will help to store the response from the client.
	response=readServerResponse(newSocketDescriptor);	// this will read the server's response and store it in the response variable.
	if(response!=NULL){	// if the response contains data then below block will be executed.
		printf("%s\n",response);	// response will be printed in the client.
	}
	response=readServerResponse(clientDescriptor);	// this will read the server's response and store it in the response variable.
	if(response!=NULL){	// if the response contains data then below block will be executed.
		printf("%s\n",response);	// response will be printed in the client.
	}
	return 0;	// this will return zero states that the function is executed successfully.
}
int writeRequestToServer(int clientDescriptor,char *input){	// this function will write each request to the server.
	int n;
	if((n=write(clientDescriptor,input,strlen(input)+1))<0){	// this will write the data to file descriptor which is send by calling function and accordingly the data request will be made on that port.
		return -1;
	}
	return 0;	// this will return zero states that the function is executed successfully.
}
void handle(int signo){	// this will handle the SIGTSTP and SIGINT signals so if user press Ctrl+C or Ctrl+Z then this function will be executed it will a message to server QUIT and connection of the serever is disconnected.
	writeRequestToServer(clientDescriptor,"QUIT");	// this will send the command QUIT to server so that server's connection gets disconnected
	if(file>0){		// if file is open then it will be closed by below block
		if(close(file)==-1){	// this will close the file
			//printf("Issue closing file");	// if there is an issue then print function is executed as Issue closing file.
		}
	}
	if(clientDescriptor>0){	// if connection port is open then it will be closed by below block
		if(close(clientDescriptor)==-1){	// this will close the connection port
			//printf("Issue closing client");	// if there is an issue then print function is executed as Issue closing client
		}
	}
	if(newSocketDescriptor>0){// if data port is open then it will be closed by below block
		if(close(newSocketDescriptor)==-1){	// this will close the connection port
			//printf("Issue closing socket");	// if there is an issue then print function is executed as Issue closing socket
		}
	}	
	exit(0);	// this will exit from client;
}

void main(){	// this is the main function
	char *cwd;	// this is used to store the current working directory of the client.
	char *temp;
	struct sockaddr_in address;	// this is used for opening the connection port
	struct sockaddr_in newaddress;	// this is used for opening the data port
	int flag=0;	// this will indicate if user had logged in
	signal(SIGINT,handle);	// this will handle the signals such that if Ctrl+c is pressed by user then instead of terminating it will call handle function
	signal(SIGTSTP,handle);	// this will handle the signals such that if Ctrl+Z is pressed by user then instead of terminating it will call handle function
	if((clientDescriptor= socket(AF_INET,SOCK_STREAM,0))<0){	// this will create endpoint for communication with byte-stream connection
		printf("Error, Could not create socket");
		exit(0);	// If it is an error then it will be get exited.
	}
	address.sin_family=AF_INET;	// this will assign the family which assigns to internet
	address.sin_port=htons(5000);	// assigning port 5000 to sin_port.
	if(connect(clientDescriptor,(struct sockaddr*) &address,sizeof(address))<0){ // this is trying to connect using the port. if failed below block is executed.
		printf("Failed to connect\n");	// this will print that failed to connect as it is unable to connect using port
		exit(0);	// this will exit program.
	}
	printf("%s\n",readServerResponse(clientDescriptor));	// this will read the first response from server when client gets connected.
	char pid[buffSize];	// this is used to store the client's pid ad string
	sprintf(pid,"%d",getpid());	// this will convert integer from string
	writeRequestToServer(clientDescriptor,pid);	// the Pid is then passed to the server.
	//int port;	// 
	char *input,*response;	// input is to store user input from client, response is used to store the reponse from the server.
	while(1){
		input=readRequest(STDIN_FILENO);	// this will execute and wait for the user to enter the command and once user enters command it will be store in input variable.
		if(input!=NULL){	// if input is not null then below block will be executed.
			if(strncmp(input,"USER",4)==0){	// this will check if input of user is USER and if yes then it will execute the if block
				flag=1;		// this will change the flag to 1 which is true which indicates that USER is logged in.
			}			
			if(strncmp(input,"REIN",4)==0 && flag==1){	// this line will check if user has types REIN as command, and user is logged in if yes then below block will be executed.
				flag=0;	// this will indicate user has be logged out.
				if(newSocketDescriptor>0){	// this will check if data port is open if yes then it will close
					close(newSocketDescriptor);	// close will close the data port
				}else{
					continue;	// if no port is open then no action is performed hence continue.
				}
			}
			if(strncmp(input,"STOR",4)==0 && flag==1){ // this line will check if user has types APPE as command, and user is logged in if yes then below block will be executed.
				if(newSocketDescriptor<1){	// this will check if data port is open if no then it will print Port is not working
						printf("PORT is not working\n");	// printing port is not working
						continue;	// this will continue;
					}
					char filename[strlen(input)];	// this will be used to store the filename of the local
					strcpy(filename,input);	// this will copy the input to filename.
					char *command=strtok(filename," ");	// this will extract the first argument of the command which is file
					temp=malloc(sizeof(filename));
					temp=strtok(NULL," ");
					if((file=open(temp,O_RDONLY))==-1){	// this will try to open the file in read only mode and if it returns -1 then if block will be printed
						printf("File is not present in client\n");	// this will print that there is an issue.
						close(file);	// this will close the file descriptor.
						continue;	// this will continue;
					}
			}
			if(writeRequestToServer(clientDescriptor,input)==-1){	// this will write the data to server in command port.
				printf("Error writing request to Server");	// if there is any issue then error writing request to Server.
			}
			if(strncmp(input,"PORT",4)==0 && flag){	// this line will check if user has types PORT as command, and user is logged in if yes then below block will be executed.
				char *str=strtok(input," ");
				char *port=strtok(NULL," ");
					if(port==NULL){
						
					}else{
						if((newSocketDescriptor=socket(AF_INET,SOCK_STREAM,0))<0){	// this will create endpoint for communication with byte-stream connection for data port
							printf("Error, Could not create socket");	// this will print error could not create socket
							exit(0);
						}
						newaddress.sin_family=AF_INET;	// this will assign the family which assigns to internet
						newaddress.sin_port=htons(atoi(input+5));	// assigning user defined port to sin_port.
						while(1){	// this will continue until the socket is connected.
							if(connect(newSocketDescriptor,(struct sockaddr*) &newaddress,sizeof(newaddress))==0){	// this is trying to connect using the port. if successful below block is executed.
								break; // once the connection is established the loop will break;
							}
						}
					}
				}	
			response=readServerResponse(clientDescriptor);	// this will wait reponse from the server
			if(response!=NULL){			// if response not NULL then it is getting printed in Client side.
				printf("%s\n",response);	// the response is printed in CLient side.
			}
			if(strcmp(response,"530 Not logged in.")==0){	// this will compare the response 530 Not logged in if user not logged in then while loop will be continue
				continue;
			}
			if(strncmp(response,"125",3)==0){	// if response starts with 125 then below code will be executed.
				if(strncmp(input,"LIST",4)==0){	// if the input is LIST then below block is executed.
					listCommand(clientDescriptor,newSocketDescriptor);	// the listcommand function will be called with data port and command port's client socket.
				}else if(strncmp(input,"STOR",4)==0){	// if the input is STOR then below block is executed.
					int length=lseek(file,0,SEEK_END);	// this will take to pointer to last character of the file and return the length and it will store in length variable
					lseek(file,0,SEEK_SET);	// again the pointer of the read will point to start.
					int n;	// this is to store the number of bytes written
					char buffer[length];	// this is to read and write whole content of file
					if((read(file,buffer,length))>0){	// this will read the whole content of file
						if((n=write(newSocketDescriptor,buffer,length))>0){	// this will write the whole content of file
							
						}else{
							printf("ISSUE");
						}
					}
					close(file);	// this will close the file descriptor
					close(newSocketDescriptor);	// this will close the connection of the data port
					response=readServerResponse(clientDescriptor);	// this will read the response of STOR from server.
					if(response!=NULL){	// if response not NULL then it is getting printed in Client side.
						printf("%s\n",response);	// the response is printed in CLient side.
					}
				}else if(strncmp(input,"APPE",4)==0){	// if the input is APPE then below block is executed.
					if(newSocketDescriptor<0){	// this will check if data port is open if no then it will print Port is not working
						printf("PORT is not working");	// printing port is not working
						continue;	// this will continue;
					}
					if(append(newSocketDescriptor,clientDescriptor,input)==-1){	// this will call append function passing the data socket and command if the function return -1 then  below block will be executed
						close(newSocketDescriptor);	// this will close the data socket from client
						continue;	// this will continue;
					}
					close(newSocketDescriptor);	// this will close the data socket from client
					response=readServerResponse(clientDescriptor);	// this will read the response of APPE from server.
					if(response!=NULL){	// if response not NULL then it is getting printed in Client side.
						printf("%s\n",response);	// the response is printed in CLient side.
					}
				}else if(strncmp(input,"RETR",4)==0){	// if the input is RETR then below block is executed.
					retrCommand(newSocketDescriptor,input);	// the listcommand function will be called with data port and command port's client socket.
				}
			}
			if(strncmp(input,"QUIT",4)==0){	// if the input is QUIT then below block is executed.
				if(newSocketDescriptor>0){	// if data port is open then data port will be closed.
					close(newSocketDescriptor);	// closing data port from client end.
				}
				close(clientDescriptor);	// closing the command port from client end.
				exit(0);	// exiting from the program.
			}
		}
	}
	close(clientDescriptor);	// closing the command port from client end.
	close(file);	// closing the file if any open.
	exit(0);
}