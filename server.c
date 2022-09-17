// We have took some of the code from our previous assignment 1,2,3 and 4
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <ftw.h>
#include <fcntl.h>
#define buffSize 4096
int client,sd,clientpid[500],i=0;	// client is used to store the data port connection, sd is used to store socket connection, clientpid is used to store the pid of client
int checkFile(char *filename){		// this is used to check the file exists if yes return 0 otherwise return 1
	if(access(filename,F_OK)!=-1){	// this will check if given filename is exists
		return 0;	// if exists return 0;
	}
	return -1;	// if file doesn't exists return -1
}
int writeResponse(char *response,int fd){	// this function is used to write the response to the client depending on the file descriptor passed it will be written in data port or connection port
	int n;	// this will store the number of bytes written
	if((n=write(fd,response,strlen(response)+1))<0){	// this will write whole response to the fd which is provided
		printf("Error user");	// if any error then it will print error user.
		return -1;	// this will return -1 stating there is an issue.
	}
	return 0;
}
char* readRequestClient(int client){	// this function is used to read input line by line from the client.
	char *buffer,character;	// buffer is a array where I will store the line and character variable will be use to read one by one character
	int i=0,length,n;	//	initalizing the i to 0 and it will be use to iterate buffer array.
	buffer=malloc(sizeof(char)*buffSize);	// assigning the memory so that it can store upto buffSize characters.
	while((n=read(client,&character,1))==1){	// this will read one by one character from the client till end of file	
		buffer[i]=character;	// if it is not end of request then it will assign one by one characters in buffer array.
		if(character=='\r'||character =='\0')	// this will read the characters if it is end of string then it will break.
			break;	// this will break.
		i++;	//this is incrementing the i variable by 1 everytime a character is assigned to ith(before incrementing i) index of buffer array.
	}
	buffer[i]='\0';	// this will add null byte to end of the string
	return buffer;	// this will return the buffer
}
int changeDirectory(char *dir){	// this function is used to change the directory
	return chdir(dir);	// the chdir is used to change the directory as per the argument given in dir
}
char* getCurrentDirectory(){	// this function is for ./ command if ./ is called with any command.
	char *path=malloc(buffSize*sizeof(char));	// this will allocate the maximum memory for the path pointer.
	getcwd(path, buffSize);		// this will get the current working directory
	return path;	// this will return the current working directory path to the calling function
}
int userCommand(int fd){	// this command will simply pass one response that user is logged in please procees.
	writeResponse("230 User logged in, Please proceed",fd);	// this will five response to the client that user is logged in and next command can be processed.
	return 0;	// this will return 0 which is successful execution of function.
}
int listCommand(int fd,int client,char *command){	//	this function will help to send the list of the files either in current working directory or directory specified. the data will be transfered by the port defined by the user but the reponse will be send by command channel.
	if(!client){	// this will check if data port is established if not then if will executed else, else block will be executed.
		writeResponse("425 Can't open data connection.",fd);	// this will send the client that data connection is not open please check
		return 0;	// this will return the successful execution of code.
	}else{
		writeResponse("125 Data connection already open; transfer starting.",fd);	// this will send to client that data port is established and data communication can take place.
	}
	char *path=strtok(command+5," ");	// this will take the path which is specified in the argument of the command
	if(path==NULL){	// this will check if argument was provided with LIST if not then below block will be executed.
		path=getCurrentDirectory();	// this will store the currentDirectory of the server in path variable.
	}
	DIR *directory;		// this will be usd to point the directory.
	struct dirent *dirp;	// this will use to get the file structure and display the filename with attribute d_name.
	if((directory=opendir(path))==NULL){	// this will try to open the path which is either current working directory or directory specified.
		writeResponse("450 Requested file action not taken.",fd);	// this will send the response to the client that there is some issue with opening the directory or it doesn't exists.
		return 0;	// this will return the successful execution of code.
	}
	char *str=malloc(sizeof(char)*buffSize);	// this will allocate maximum size which is buffSize to str pointer dynamically
	while((dirp=readdir(directory))!=NULL){	// this is used to loop each element of the directory.
		strcat(str,dirp->d_name);	// it will concatenate the directory name or filename to str
		strcat(str,"\n");	// it will concatenate '\n' to str
	}
	writeResponse(str,client);	// this will write the response(list of file) using the data port.
	writeResponse("250 Requested file action okay, completed.",fd);	//	this will provide response to the client that the action is completed.
	return 0;	// this will return the successful execution of code.
}

int portCommand(int fd,char* command){
	int sd,client;	// sd and client will store the descriptor's of socket
	struct sockaddr_in address;	//this will used to store the socket address
	char *command1=strtok(command," ");	// this will break the command and retrive the name of command
	char *port=malloc(sizeof(command));	// this will allocate memory
	port=strtok(NULL," ");	// this will get the argument of port command
	if(port==NULL){	// If no port is specified then syntax error will be done as reply
		writeResponse("501 Syntax error in parameters or arguments.",fd);	// this will write the response to client with the response message
		return 0;
	}
	if((sd=socket(AF_INET,SOCK_STREAM,0))<0){	// this will create endpoint for communication with byte-stream connection		
		writeResponse("550 Requested action not taken.",fd);	// this will write response to client.
	}
	address.sin_family=AF_INET;	// this will assign the family which assigns to internet
	address.sin_addr.s_addr=htonl(INADDR_ANY);	// htonl is used to host to network conversions for multiple bytes and it will assigned in s_addr
	address.sin_port=htons(atoi(port));		// assigning user defiedn port to sin_port.
	if(bind(sd, (struct sockaddr*)&address, sizeof(address)) >= 0){	// this will check if bind is done if yes the below block is executed
		if(listen(sd,5)==0){	// this will listen for the client in same socket if able to detect the client then below code will be executed.
			while(1){		// this will loop infinitely.
				if((client=accept(sd,(struct sockaddr *) NULL, NULL))>0){	// this will accept the connection of the socket and return the client socket descriptor.
					writeResponse("200 Command okay.\n",fd);	// this will send a response to client upon successful.
					return client;	 // this function will return the new socket's client descriptor which will be used in future to trander the data in data channel.
				}
			}
		}
    }	
}
int cdupCommand(int fd){	// this function is used to transfer the cwd to one level up that means from hetansh/home/het to hetansh/home
	char *str=malloc(buffSize*sizeof(char));	// this will allocate the dynamic array of buffSize.
	if(changeDirectory("..")==0){	// this is used to changeDirectory to one level up and if 0 then below block will be executed else, else block will be executed.
		writeResponse("200 Command okay.",fd);	// if directory change it will send a response to client that command is okay
		return 0;	// this will return 0 that execution of block is completed
	}else{
		writeResponse("550 Requested action not taken.",fd);	// if directory not change it will send a response to client that requested action not taken
		return 0;	// this will return 0 that execution of block is completed
	}
}
int cwdCommand(int fd,char *command){	// this function is used to change the current working directory to directory provided.
	char *str=malloc(buffSize*sizeof(char));	// this will provide the buffSize to str
	char *c=strtok(command," ");	// this will retrive the command 
	char *d=strtok(NULL," ");	// this will retrive first argument
	if(d==NULL){	// this will check if it has argument else it is an incorrect syntax
		writeResponse("501 Syntax error in parameters or arguments.",fd);
		return 0;
	}
	if(changeDirectory(d)==0){	//if change directory works then below block is executed
		strcat(str,"200 directory changed to ");// this will append the string to str
		strcat(str,getCurrentDirectory());	// this will append the current working directory with str
		writeResponse(str,fd);	// write response that directory is changed
		return 0;	// this will return 0 that execution of block is completed
	}else{
		return writeResponse("550 Requested action not taken.",fd); // write response requested action not taken
	}
	return -1;	// return -1
}
int pwdCommand(int fd){	// this is to check the present working directory of the server
	char *str=malloc(buffSize*sizeof(char));	// this will provide the buffSize to str
	char *reply=malloc(buffSize*sizeof(char));// this will provide the buffSize to reply
	if((str=getCurrentDirectory())==NULL){	// this will get the current working directory if Null then below block is executed.
		writeResponse("431 No such directory",fd);	// write response No such directory
	}else{	
		strcat(reply,"257 \"");	// this will concatenate reply with 257 "
		strcat(reply,getCurrentDirectory());	//this will concatenate reply with current working directory
		strcat(reply,"/");	// this will concatenate reply with /
		strcat(reply,"\" directory.");	// this will concatenate reply with "directory
		writeResponse(str,fd);	// this will write response to the client.
	}
	return 0;// this will return 0 that execution of block is completed
}
int mkdCommand(int fd,char *command){	// this command function is used to make directory according to the name given
	int r;	// r is used to store status of the mkdir function
	char *c=strtok(command," ");	// this will provide the buffSize to str
	char *d=strtok(NULL," ");	// this will retrive first argument
	if(d==NULL){	// this will check if it has argument else it is an incorrect syntax
		writeResponse("501 Syntax error in parameters or arguments.",fd);	
		return 0;
	}
	char *str=malloc(buffSize*sizeof(char));	// the buffSize is allocate to str
	if((r=mkdir(d,0777))==0){	// this will call mkdir function and make the directory and if success the below block is executed.
		strcat(str,"257 \"");	// this will concatenate  257 " with str
		strcat(str,getCurrentDirectory());	// this will concatenate current working directory with str 
		strcat(str,"/");	// this will concatenate / with str
		strcat(str,command+4);	// this will concatenate command argument with str
		strcat(str,"\" directory created");	// this will concatenate " directory created with str.
		writeResponse(str,fd);	// this will write response to the client.
	}else if(r==-1 && errno==EEXIST){	// this is will check if mkdir doesn't work and errno is directory exists then below block will be executed
		strcat(str,"421 \"");	// this will concatenate 421 " with str
		strcat(str,getCurrentDirectory());	// this will concatenate current working directory with str 
		strcat(str,"/");	// this will concatenate / with str
		strcat(str,command+4);	// this will concatenate command argument with str
		strcat(str,"\" directory already exists");	// this will concatenate " directory already exists with str.
		writeResponse(str,fd);	// this will write response to the client.
	}
	return 0;	// this will return 0 that execution of block is completed
}
int deleteCommand(int fd,char *command){	// this command function is used to delete the file
	if(remove(command)==-1){	// this will call remove function which is used to delete the file if not deleted then -1 will return
		writeResponse("450 Requested file action not taken.",fd); // this will write response to the client that File is not found hence requested action is not taken
	}else{	// if file is deleted
		writeResponse("250 Requested file action okay, completed.",fd);	// this will write response to the client that requested action is okay and completed.
	}
	return 0;	// this will return 0 that execution of block is completed
}
char *rntoCommand(int fd,char *command){	// this command is used to get the newname of the file which needs to be renamed
	char *c=strtok(command," ");
	char *d=strtok(NULL," ");
	if(d==NULL){
		writeResponse("501 Syntax error in parameters or arguments.",fd);
		return NULL;
	}if(d!=NULL){	// this will check if command is having the argument or not.
		writeResponse("350 Requested file action pending further information.",fd);	// if it has argument then Requested file action pending further information
		return command+5;	// this will return the argument which is the filename.
	}
	return NULL;	// else it will return null if nothing was passed.
}
int rnfrCommand(int fd,char *command,char *newFileName){	// this is to finally rename the file
	if(newFileName==NULL){	// this will check if newname is not null if it is null the below block is executed.
		writeResponse("503 Bad sequence of commands.",fd);// this will write response to the client that Bad sequence of commands.
		return 0; // this will return 0 that execution of block is completed
	}
	
	if(rename(command+5,newFileName)==-1){	// this will try to rename the file if unsuccessful then below block is executed
		writeResponse("501 Syntax error in parameters or arguments.",fd);	// this will write response to the client that Issue while changing.
		return 0;	// this will return 0 that execution of block is completed
	}
	writeResponse("250 renamed",fd);	// this will write response to the client that 250 renamed.
	return 0;	// this will return 0 that execution of block is completed
}
int removeDirectories(const char *filePath,const struct stat *stateBuffer,int tFlag,struct FTW *ftwBuff){  // this function is declared in the start of the program and it is defined here. In this there are 4 argument where first argument is constant and provides the path, the second argument contains the pointer to the stats information of the files, tFlag will provide additional information of the path for example is it a directory or it is a symbolic link and fourth argument contains the FTW structure pointer.
	return remove(filePath);	// this will remove the files or directory until it is able to remove and if it is completed then return will be non zero value.
}
int rmdCommand(int fd,char *command){	// this is used to remove the directory.
	if(command+4 == NULL){	// this will check if argument is provided if not then below code is executed.
		writeResponse("501 Syntax error in parameters or arguments.",fd);		// this will write response to the client that Syntax error in parameters or arguments.
		return 0;	// this will return 0 that execution of block is completed
	}
	if(nftw(command+4,removeDirectories,2,FTW_PHYS|FTW_DEPTH)==-1) // this if condition will check if nfw walk has any error if function returns an error then it will exit with failure. The nftw function will walk through every path hierarchy from the path given recursively. The nftw has 4 argument 1st argument is to pass the path in this code  we check if a sepcific directory is given to traverse if no then current working directory will be passed else the path given in the argument will be passed as first argument. second argument is the function which will be called for each path, the second argument provide the maximum number of file descriptors that will be used during file tree traversal in this case I have set to 10, the last argument is to provide flag and in this case I have passed FTW_PHYS as I only want to report the symbolic link and not traverse it so FTW_PHYS will ensure that I don't traverse the symbolic link as it might lead to infinite loop.  
	{	
		writeResponse("550 Requested action not taken.",fd);	// this will write response to the client that Requested action not taken.
		return 0;	// this will return 0 that execution of block is completed
	}
	writeResponse("250 Requested file action okay, completed.",fd);	// this will write response to the client that 250 Requested file action okay, completed
	return 0;	// this will return 0 that execution of block is completed
}
int storeCommand(int fd,int client,char *filename){	// this function will upload the file from client to server
	if(!client){	// this will check if data port is open if not then below block will be executed
		writeResponse("425 Can't open data connection.",fd);	// this will write response to the client that 425 Can't open data connection.
		return 0;	// this will return 0 that execution of block is completed
	}else{
		writeResponse("125 Data connection already open; transfer starting.",fd);	// this will write response to the client that 125 Data connection already open; transfer starting.
	}
	char *command=strtok(filename," ");// this will break the filename and will get the client's file name
	char *name=strtok(NULL," ");// this will break the filename and will get the client's file name
	char *newname=malloc(sizeof(filename));	// this is to store the name of the file that needs used while storing the file in server
	int file;	// this is used to store the file descriptor.
	if(name!=NULL){	// this will check if client's filename is provided if yes then below block will be executed.
		newname=strtok(NULL," ");	// this will break the string and retrive next argument.
		if(newname==NULL){	// this will check if name in which server needs to store is given if yes not then if block will be executed else ELSE block will be executed.
			if(checkFile(name)==0){	// this will check if file exists.
				remove(name);	// if file exists then it will remove existing file from server
			}
			if((file=open(name,O_WRONLY|O_CREAT,0777))==-1){	// this will create new file in server with same name as in client.
				writeResponse("450 Requested file action not taken.",fd);	// this will write response to the client that 450 Requested file action not taken.
				return 0;	// this will return 0 that execution of block is completed
			}
		}else{
			if(checkFile(newname)==0){	// this will check if file exists.
				remove(newname);	// if file exists then it will remove existing file from server
			}
			if((file=open(newname,O_WRONLY|O_CREAT,0777))==-1){	//	this will open the new file according to the new name given by the client.
				writeResponse("450 Requested file action not taken.",fd);	// this will write response to the client that 450 Requested file action not taken.
				return 0; // this will return 0 that execution of block is completed
			}
		}
		char c;	// this is used to read and write single character one by one from the socket and write in file.
		int i=0;	// initalizing i to check if read executed atleadt once.
		while(read(client,&c,1)>0){	// this will read 1 character from data socket and store in C
			if(write(file,&c,1)!=1){	// this will write the character to file which is open
				writeResponse("450 Requested file action not taken.",fd);	// this will write response to the client that 450 Requested file action not taken.
				close(file);	// this will close the file descriptor of file
				return 0;	// this will return 0 that execution of block is completed
			}
			i++;	// incrementing i to 1
		}
		if(i>0){	// if i>0 then below response is provided.
			writeResponse("250 Requested file action okay, completed.\n226 Closing data connection.",fd);	// this will write response to the client that 250 Requested file action okay, completed.\n226 Closing data connection.
		}
		else{
			writeResponse("450 Requested file action not taken.\n226 Closing data connection.",fd);	// this will write response to the client that 450 Requested file action not taken.
			return 0;
		}
		close(file);	// this will close the file descriptor
	}else{
		writeResponse("501 Syntax error in parameters or arguments.\n226 Closing data connection.",fd);	// this will write response to the client that 501 Syntax error in parameters or arguments.
	}
	close(client);	// this will close the data socket.
	return 0; // this will return 0 that execution of block is completed
}
int statCommand(char *command,int fd, int client){	// this will send the status of server and if its with argument then it will work as a list
	char *temp=strtok(command," ");	// this will get the command and store in temp
	char *temp1=strtok(NULL," ");	// this will get the first argument of command and store it in temp1
	if(temp1==NULL){	// this will check if argument is not given then below code is executed.
		if(fd>0 && client>0){	// this will check if data connection and command connection is open
			writeResponse("211 Data and control connection are open",fd);	// this will write response to the client that 211 Data and control connection are open
		}else if(fd>0){	// this will check if command connection is open
			writeResponse("211 Control connection is open",fd);	// this will write response to the client that 211 Control connection are open
		}
		return 0;	// this will return 0 that execution of block is completed
	}
	DIR *directory;	// this will be usd to point the directory.
	struct dirent *dirp;	// this will use to get the file structure and display the filename with attribute d_name.
	if((directory=opendir(temp1))==NULL){	// this will try to open the path which is either current working directory or directory specified.
		writeResponse("450 File not exists.",fd);	// this will send the response to the client that 450 File not exists.
		return 0;	// this will return 0 that execution of block is completed
	}
	char *str=malloc(sizeof(char)*buffSize);	// this will allocate maximum size which is buffSize to str pointer dynamically	
	while((dirp=readdir(directory))!=NULL){		// this is used to loop each element of the directory.
		strcat(str,dirp->d_name);		// it will concatenate the directory name or filename to str
		strcat(str,"\n");		// it will concatenate '\n' to str
	}
	strcat(str,"250 Requested file action okay, completed.");	// this will concatenate the string with str.
	writeResponse(str,fd);	// this will write the response(list of file) using the data port.
	return 0;	// this will return 0 that execution of block is completed
}
int appendCommand(int fd,int client,char *command){	// this is used to append the data in file if file not exists then it will create and write.
	if(!client){	// this will check if data port is open if not then below block will be executed
		writeResponse("425 Can't open data connection.",fd);	// this will write response to the client that 425 Can't open data connection.
		return 0;	// this will return 0 that execution of block is completed
	}else{
		writeResponse("125 Data connection already open; transfer starting.",fd);	// this will write response to the client that 125 Data connection already open; transfer starting.
		//return 0;	
	}
	char *cname=strtok(command," ");// this will break the filename and will get the client's file name
	char *name=strtok(NULL," ");// this will break the filename and will get the client's file name
	char *newname=malloc(sizeof(command));	// this is to store the name of the file that needs used while
	int file;	// this is used to store the file descriptor.
	if(name!=NULL){	// this will check if client's filename is provided if yes then below block will be executed.
		newname=strtok(NULL," ");	// this will break the string and retrive next argument.
		if(newname==NULL){	// this will check if name in which server needs to store is given if yes not then if block will be executed else ELSE block will be executed.
			if((file=open(name,O_APPEND|O_WRONLY|O_CREAT,0777))==-1){	//	this will open the new file according to the client's local file name.
				writeResponse("450 Requested file action not taken.",fd);	// this will write response to the client that 450 Requested file action not taken.	
				return 0;	// this will return 0 that execution of block is completed
			}
		}else{
			if((file=open(newname,O_APPEND|O_WRONLY|O_CREAT,0777))==-1){	//	this will open the new file according to the new name given by the client.
				writeResponse("450 Requested file action not taken.",fd);	// this will write response to the client that 450 Requested file action not taken.
				return 0;	// this will return 0 that execution of block is completed
			}
		}
		char c;	// this is used to read and write single character one by one from the socket and write in file.
		int i=0;	// initalizing i to check if read executed atleadt once.
		while(read(client,&c,1)>0){		// this will read 1 character from data socket and store in C
			if(write(file,&c,1)!=1){	// this will write the character to file which is open
				writeResponse("450 Requested file action not taken.",fd);	// this will write response to the client that 450 Requested file action not taken.
				close(file);	// this will close the file descriptor
				return 0;	// this will return 0 that execution of block is completed
			}
			i++;	// incrementing i to 1
		}
		if(i>0){	// if i>0 then below response is provided.
			writeResponse("250 Requested file action okay, completed.226 Closing data connection.",fd); // this will write response to the client that 250 Requested file action okay, completed.\n226 Closing data connection.
		}
		else{
			writeResponse("450 Requested file action not taken.226 Closing data connection.",fd);	// this will write response to the client that 450 Requested file action not taken.	
		}
	}else{
		writeResponse("501 Syntax error in parameters or arguments.226 Closing data connection.",fd);	// this will write response to the client that 501 Syntax error in parameters or arguments.
	}
	close(client);	// this will close the socket connection
	return 0; // this will return 0 that execution of block is completed
}
int retriveCommand(int fd,int client,char *filename){	// this command is to send the data from server to client
	if(!client){	// this will check if data port is open if not then below block will be executed
		writeResponse("425 Can't open data connection.",fd);	// this will write response to the client that 425 Can't open data connection.
		return 0;	// this will return 0 that execution of block is completed
	}else{
		writeResponse("125 Data connection already open; transfer starting.",fd);	// this will write response to the client that 125 Data connection already open; transfer starting.
	}
	int file;	// this is used to store the file descriptor.
	char *temp=strtok(filename," ");	// this is used to get the first argument which contains the name of file are server end
	if((file=open(temp,O_RDONLY))==-1){	// this will try to open the file in readonly mode if not able to open then below block is executed.
		writeResponse("450 Requested file action not taken.",fd);	// this will write response to the client that 450 Requested file action not taken.
		close(client);
		return 0;	// this will return 0 that execution of block is completed
	}
	int length=lseek(file,0,SEEK_END);	// this will get the length of the file
	lseek(file,0,SEEK_SET);	// this will again bring back the pointer to start of the file
	char buffer[length];	// this will help to read the whole data from file and write it in data port.
	if((read(file,buffer,length))>0){	// this will read whole data from file
		if((write(client,buffer,length))>0){	// this will write whole data to data port.
			writeResponse("250 Requested file action okay, completed.\n226 Closing data connection.",fd);	// this will write response to the client that 450 Requested file action not taken.
		}
	}
	close(client);	// this will close the socket connection
	return 0;	// this will return 0 that execution of block is completed
}
void noopCommand(int fd){	// this will send a signal to client that noop is handled
	writeResponse("200 NOOP command successful.",fd);	// this will write response to the client that 200 NOOP command successful.
}
int reinCommand(int fd,int client){	// this will close the data port if open 
	if(client>0){	// this will check if data port is open if yes then below is executed.
		close(client);	// this will close the data port.
		writeResponse("220 Service ready for new user.",fd);	// this will provide a response to client that 220 Service ready for new user.
		return 0;	// this will return 0 that execution of block is completed
	}
}

int checkCommand(char *command,int fd){	// this function will check the command and according to the commands respective functions are called.
	static int client=0;	// this is to keep track of data port is established or not.
	static char* newFileName=NULL;	// this is to keep track of RNFR is before RNTO.
	static int user=0;	// this is to track whether user command was passsed or not.
	if(strncmp(command,"USER",4)==0){	// if the command is User then below block will be executed.
		userCommand(fd);	// the userCommand function is called and fd is passed which is client socket descriptor.
		user=1;		// the flag will be true and user has logged in
		newFileName=NULL;	// this flag will be remain null.
		return 0;	// the function will return 0 which indicate successful execution of function.
	}else if(strncmp(command,"LIST",4)==0 && user==1){	//	this will check if user is logged in and command is LIST.
		newFileName=NULL;		// this flag will be remain null.
		if(listCommand(fd,client,command)==0)	// this will call list command with connection channel's client descriptor, data channel's client descriptor and command
			return 0;	// this will return 0 that execution of block is completed
		return -1;	// this will return -1 that execution of block is having error
	}else if(strcmp(command,"QUIT")==0){
		newFileName=NULL;	// this flag will be remain null.
		writeResponse("221 Service closing control connection.",fd);// this will provide reponse to client that the connection is closing
		if(client>0){	// this will check if data port is open
			close(client);	// this will close the data port
		}
		close(fd);	// this will close the command channel 
		exit(0);	// this will exit from the program.
	}else if(strncmp(command,"PORT",4)==0  && user==1){	// this will check first if user is logged in and then it will check if command is PORT
		newFileName=NULL;	// this will set newFileName flag to NULL
		client=portCommand(fd,command);	// this will call the port command and it will return the client socket descriptor of nre port.
		return 0;	// this will return 0 that execution of block is completed	
	}else if(strncmp(command,"STAT",4)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is STAT
		newFileName=NULL;	// this flag will be remain null.
		statCommand(command,fd,client);	//this will call the stat command function with socket descriptors and command
		return 0;	// this will return 0 that execution of block is completed
	}else if(strcmp(command,"CDUP")==0  && user ==1){	// this will check first if user is logged in and then it will check if command is CDUP
		newFileName=NULL;	// this flag will be remain null.
		return cdupCommand(fd);	//this will call the cdup command function with socket descriptor
	}else if(strncmp(command,"CWD",3)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is CWD
		newFileName=NULL;	// this flag will be remain null.
		return cwdCommand(fd,command);	// this will call cwd Command with socket descriptor and command as argument	
	}else if(strcmp(command,"PWD")==0  && user ==1){	// this will check first if user is logged in and then it will check if command is PWD
		newFileName=NULL;	// this flag will be remain null.
		return pwdCommand(fd);	//this will call the present working directory command function with socket descriptor
	}else if(strncmp(command,"MKD",3)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is MKD
		newFileName=NULL;	// this flag will be remain null.
		return mkdCommand(fd,command);	//this will call the make directory function command function with socket descriptors and the command
	}else if(strncmp(command,"RMD",3)==0  && user ==1){ // this will check first if user is logged in and then it will check if command is RMD
		newFileName=NULL;	// this flag will be remain null.
		return rmdCommand(fd,command);	//this will call the remove directory command function with socket descriptor and the command
	}else if(strncmp(command,"DELE",4)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is DELE
		newFileName=NULL;	// this flag will be remain null.
		return deleteCommand(fd,command+5);		//this will call the delete command function with socket descriptors and the command
	}else if(strncmp(command,"RNTO",4)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is RNTO
		newFileName=rntoCommand(fd,command);	//this will call the rnto command function with socket descriptors and the command
	}else if(strncmp(command,"RNFR",4)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is RNFR
		return rnfrCommand(fd,command,newFileName);		//this will call the rnfr command function with socket descriptors and the command
	}else if(strncmp(command,"REIN",4)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is REIN
		newFileName=NULL;	// this flag will be remain null.
		if(reinCommand(fd,client)==0){	// this will check if rein command is successful is completed.
			client=0;	// this flag will indicate that data port is closed
			user=0;	// this flag will indicate that the user has logged out
			return 0;	// this will return 0 that execution of block is completed	
		}
	}else if(strncmp(command,"NOOP",4)==0  && user==1){	// this will check first if user is logged in and then it will check if command is NOOP
		noopCommand(fd);	//this will call the noop command function with socket descriptor.
		return 0;	// this will return 0 that execution of block is completed
	}else if(strncmp(command,"STOR",4)==0  && user==1){	// this will check first if user is logged in and then it will check if command is STOR
		newFileName=NULL;	// this flag will be remain null.
		storeCommand(fd,client,command);//this will call the store command function with socket descriptors and the command
		client=0;	// this flag will indicate that data port is closed
		return 0;	// this will return 0 that execution of block is completed
	}else if(strncmp(command,"APPE",4)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is APPE
		newFileName=NULL;	// this flag will be remain null.
		appendCommand(fd,client,command);		//this will call the append command function with socket descriptors and the command
		client=0;	// this flag will indicate that data port is closed
		return 0;	// this will return 0 that execution of block is completed
	}else if(strncmp(command,"RETR",4)==0  && user ==1){	// this will check first if user is logged in and then it will check if command is RETR
		newFileName=NULL;	// this flag will be remain null.
		retriveCommand(fd,client,command+5);	//this will call the retrive command function with socket descriptors and the command
		client=0;	// this flag will indicate that data port is closed
		return 0;	// this will return 0 that execution of block is completed
	}else if(user == 0){	// this will check if user has already logged in if not then below block will be executed.
		writeResponse("530 Not logged in.",fd);	// this will provide a response to client that 530 Not logged in.
	}else{
		writeResponse("502 Command not implemented.",fd);	// this will provide a response to client that 502 Command not implemented.
	}
	return 0;	// this will return 0 that execution of block is completed
}
void child(int client){		// this function will handle all the requests of the child.
	char *buffer,*temp;		// these variable is used store the command
	while(1){		// this will execute infinitely.
		buffer=readRequestClient(client);	// this will call the readrequestClient function which will eventually wait for the request from the client and once request is read it will be  assigned to buffer.
		temp=malloc(sizeof(char)*strlen(buffer));// this will allocate the memory according to the size of the command to temp pointer
		strcpy(temp,buffer);	// this will copy the command and store it in temp.
		if(temp!=NULL){	// this will check if command has been request if temp is not null then below block will be executed.
			if(checkCommand(temp,client)==-1){	// this will call the checkCommand which will check the command and functions are called respectively.
				printf("Issue with the command");	// if the command is having any issue then Issue with the command will be printed in server.
			}
		}
	}
	free(temp);	// this will free the memory of temp variable.
}	
void handle(){			// this will handle the SIGTSTP and SIGINT signals so if user press Ctrl+C or Ctrl+Z then this function will be executed it will send a signal to all the clients and it will close the its descriptors and free the memory.
	for(int j=0;j<i;j++){	// this will loop through the number of clients connected
		if(clientpid[j]!=0){	// this will check if clientpid contains the pid or not and if yes then it will be non zero hence below block will be executed.
			kill(clientpid[j],SIGINT);	// kill will send a signal to each client SIGINT which is handle in client to terminate itself but before that it will clean it's memory.
			wait(NULL);	// this will wait to accept the death of the children.
		}
	}
	if(close(client)==-1){	// this will close the client socket descriptor.
	}
	if(close(sd)==-1){	// this will close the server socket descriptor.
	}
	exit(0);	// it will get terminate from the program.
}
void main(int arg,char *args[]){
	char *cwd;		// this is used to store the current working directory 
	struct sockaddr_in address;	//this will used to store the socket address
	signal(SIGINT,handle);	// this will change the handling of Crtl+C and whenever Crtl+C is pressed then handle function is called
	signal(SIGTSTP,handle);	// this will change the handling of Crtl+Z and whenever Crtl+Z is pressed then handle function is called
	if(arg==3){	// this will check the number of arguments and if it is 3 then it will go in if block
		if(strcmp(args[1],"-d")==0){		// this will compare the argument to -d
			cwd=malloc(sizeof(strlen(args[2])));	// this will allocate the memory according to the size of args[2]
			strcpy(cwd,args[2]);	// this will copy the args[2](current working directory of server) to cwd 
			if(changeDirectory(cwd)==-1){	// this will call change directory to change the current working directory to the directory passed in cwd
				printf("Error changing the directory");// this will print if currentDirectory function returns -1
			}
		}
	}
	if((sd=socket(AF_INET,SOCK_STREAM,0))<0){	// this will create endpoint for communication with byte-stream connection		
		printf("Error, Could not create socket");	// this will print that error could not create socket
		_exit(0);	// If it is an error then it will be get exited.
	}
	address.sin_family=AF_INET;		// this will assign the family which assigns to internet
	address.sin_addr.s_addr=htonl(INADDR_ANY);	// htonl is used to host to network conversions for multiple bytes and it will assigned in s_addr
	address.sin_port=htons(5000);		// assigning port 5000 to sin_port.
	bind(sd,(struct sockaddr*) &address,sizeof(address));	// this will bind ip address with the port to a socket.
	listen(sd,5);	// this will listen for the connection from FTP client socket.
	while(1){	// this will run infinitely
		client=accept(sd,(struct sockaddr *) NULL, NULL);	// this is used to accept the connection from client and client descriptor will be assigned to client.		
		if(client>0){		// this will check if socket descriptor is greater than 0 then the below block will be executed.
			writeResponse("220 Service ready for new user.",client);	// this will write a response to client that the server is connected with client.
			clientpid[i]=atoi(readRequestClient(client));	// this will wait for client to provide its process 
			i++;	// this will update the count of the clients that is connected.
			int pid=fork();	// this will fork for clients so that one client will have a child and parent will create child for number of client connected.	
			if(!pid){	// if it is child then below block will be executed.
				child(client);	// this will call the child function to handle each request of client.
			}	
		}
		
	}
	free(cwd);	// this will free the memory of cwd
	close(client);	// this will close the socket descriptor
	close(sd);	// this will close the socket descriptor
	exit(0);	// this will exit from command.
}