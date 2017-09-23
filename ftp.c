#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <dirent.h>
#include <regex.h>
#include <openssl/md5.h>


char* toString(long int n){
	char *num = (char*) malloc(sizeof(char) * 200);
	long int i, j;

	for(i = 0; n > 0; i++)
	{
		num[i] = (n % 10) + '0';
		n /= 10;
	}
	
	num[i] = '\0';

	i--;

	for(j = 0; i > j; i--, j++)
	{
		char tmp = num[j];
		num[j] = num[i];
		num[i] = tmp;
	}

	return num;
}

int server(int serverPort, DIR *dir, char f[256])
{
	int listenSocket = 0, connectionSocket = 0;
	struct sockaddr_in serv_addr;
	
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(listenSocket < 0)
	{
		printf("Error while creating socket. Aborting!!\n");
		return 0;
	}
	else
	{
		printf("[Server] Socket created successfully!!\n");
	}

	bzero((char*) &serv_addr, sizeof(serv_addr));

	int portno = serverPort;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);

	if(bind(listenSocket, (struct sockaddr * ) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR WHILE BINDING THE SOCKET\n");
	}
	else
	{	
		printf("[SERVER] SOCKET BINDED SUCCESSFULLY\n");
	}

	if(listen(listenSocket, 10) == -1)
	{
		printf("[SERVER] FAILED TO ESTABLISH LISTENING \n\n");
	}
	else
	{
		printf("[SERVER] Waiting for client to connect....\n" );
	}

	while((connectionSocket = accept(listenSocket, (struct sockaddr*) NULL, NULL)) < 0);

	printf("[Connected]\n");

	while(1)
	{
		char buffer[1024], send_data[1024], recv_data[1024];
		bzero(recv_data, 1024);
		bzero(buffer,1024);
		bzero(send_data,1024);

		if(recv(connectionSocket, recv_data, 1023, 0) < 0)
		{
			printf("Error while reading from client.\n");
		}

		else
		{
			if(recv_data[0] == 'I' && recv_data[1] == 'S')
			{	
				char *t;
				long int min = 0, max = 0, i;
				
				t = strtok(recv_data, " ");
				t = strtok(NULL, " ");

				if(t != NULL)
				{
					for(i = 0; t[i] != '\0'; i++)
					{
						min = min * 10 + (t[i] - '0');
					}
				}

				t = strtok(NULL, " ");

				if(t != NULL)
				{
					for(i = 0; t[i] != '\0'; i++)
					{
						max = max * 10 + (t[i] - '0');
					}
				}

				struct dirent *d;

				while((d = readdir(dir)) != NULL)
				{
					struct stat s;
					char file[1024];

					strcpy(file, f);
					strcat(file, "/");
					strcat(file, d->d_name);

					int r = lstat(file, &s);
					if(r < 0)
					{
						printf("Error!\n");
						continue;
					}

					if((s.st_mtime >= min) && (s.st_mtime <= max))
					{
						bzero(send_data, 1024);

						strcpy(send_data, d->d_name);
						
						int sz = s.st_size;

						char *size = toString(sz);
						strcat(send_data, "\t");
						strcat(send_data, size);

						char *t = toString(s.st_mtime);
						strcat(send_data, "\t");
						strcat(send_data, t);

						strcat(send_data, "\t");
						send_data[strlen(send_data)] = d->d_type;
						send_data[strlen(send_data)] = '\n';
						send_data[strlen(send_data)] = '\0';

						if(send(connectionSocket, send_data, strlen(send_data), 0) < 0)
						{
							printf("Error while sending data.");
						}
					}
				}
			}

			else if(recv_data[0] == 'I' && recv_data[1] == 'L')
			{
				struct dirent *d;

				while((d = readdir(dir)) != NULL)
				{
					struct stat s;
					char file[1024];

					strcpy(file, f);
					strcat(file, "/");
					strcat(file, d->d_name);

					int r = lstat(file, &s);
					if(r < 0)
					{
						printf("Error!\n");
						continue;
					}

					bzero(send_data, 1024);

					strcpy(send_data, d->d_name);
					
					int sz = s.st_size;

					char *size = toString(sz);
					strcat(send_data, "\t");
					strcat(send_data, size);

					char *t = toString(s.st_mtime);
					strcat(send_data, "\t");
					strcat(send_data, t);

					strcat(send_data, "\t");
					send_data[strlen(send_data)] = d->d_type;
					send_data[strlen(send_data)] = '\n';
					send_data[strlen(send_data)] = '\0';

					if(send(connectionSocket, send_data, strlen(send_data), 0) < 0)
					{
						printf("Error while sending data.");
					}
				}
			}

			else if(recv_data[0] == 'I' && recv_data[1] == 'R')
			{
				char* exp;

				exp = strtok(recv_data, " ");
				exp = strtok(NULL, " ");

				if(exp != NULL)
				{
					regex_t regex;

					int reti = regcomp(&regex, exp, 0);

					if(reti)
					{
						printf("Error compiling regex.\n");
						continue;
					}

					struct dirent *d;

					while((d = readdir(dir)) != NULL)
					{
						struct stat s;
						char file[1024];

						strcpy(file, f);
						strcat(file, "/");
						strcat(file, d->d_name);

						int r = lstat(file, &s);
						if(r < 0)
						{
							printf("Error!\n");
							continue;
						}
						reti = regexec(&regex, d->d_name, 0, NULL, 0);
						if(!reti)
						{
							bzero(send_data, 1024);

							strcpy(send_data, d->d_name);
							
							int sz = s.st_size;

							char *size = toString(sz);
							strcat(send_data, "\t");
							strcat(send_data, size);

							char *t = toString(s.st_mtime);
							strcat(send_data, "\t");
							strcat(send_data, t);

							strcat(send_data, "\t");
							send_data[strlen(send_data)] = d->d_type;
							send_data[strlen(send_data)] = '\n';
							send_data[strlen(send_data)] = '\0';

							if(send(connectionSocket, send_data, strlen(send_data), 0) < 0)
							{
								printf("Error while sending data.\n");
							}
						}
					}

					regfree(&regex);
				}
			}

			else if(recv_data[0] == 'D')
			{
				char* file = strtok(recv_data, " ");
				file = strtok(NULL, " ");

				if(file != NULL)
				{
					char file1[500];
					strcpy(file1, "./share/");
					strcat(file1, file);

					FILE *f = fopen(file1, "r");

					if(f == NULL)
					{
						printf("Could not open file.\n");
						continue;
					}

					bzero(send_data, 1024);

					while(!feof(f))
					{
						int sz = fread(send_data, 1, 1024, f);
						send_data[sz] = '\0';

						if(send(connectionSocket, send_data, strlen(send_data), 0) < 0)
						{
							printf("Error while sending data.\n");
						}
					}

					bzero(send_data, 1024);

					/*char end[] = "End of File";
					strcpy(send_data, end);
					if(send(connectionSocket, send_data, strlen(send_data), 0) < 0)
					{
						printf("Error while sending data.\n");
					}*/

					fclose(f);

					printf("Data sent.\n");
				}
			}

			else if(recv_data[0] == 'U')
			{
				printf("​FileUploadDeny or FileUploadAllow? \n");
				bzero(send_data, 1024);

				fgets(send_data, 1023, stdin);

				if(send(connectionSocket, send_data, strlen(send_data), 0) < 0)
				{
					printf("Error while sending data.\n");
				}

				if(strcmp(send_data, "FileUploadDeny") == 0)
				{
					close(connectionSocket);
					close(listenSocket);
					printf("Socket Closed.\n");	
					return 0;
				}
				else if(strcmp(send_data, "FileUploadAllow") == 0)
				{
					bzero(recv_data, 1024);

					int sz = read(connectionSocket, recv_data, 1024);

					bzero(buffer, 0);

					strcpy(buffer, recv_data);

					char file[500];
					strcpy(file, "./share/");
					strcat(file, recv_data);
					FILE *f = fopen(file, "w");

					bzero(recv_data, 1024);

					while(1)
					{
						int sz = read(connectionSocket, recv_data, 1024);

						if(sz <= 0)
						{
							printf("file ended.\n");
							break;
						}

						fwrite(recv_data, 1, sz, f);
					}

					fclose(f);

					

					struct stat s;
					int r = lstat(file, &s);
					if(r < 0)
					{
						printf("Error!\n");
						continue;
					}

					char *size = toString(s.st_size);
					strcat(buffer, "\t");
					strcat(buffer, size);

					char *t = toString(s.st_mtime);
					strcat(buffer, "\t");
					strcat(buffer, t);

					printf("%s\n", buffer);

					/*unsigned char c[MD5_DIGEST_LENGTH];
					int i;
					FILE *fi = fopen(file, "rb");

					MD5_CTX mdContext;

					int bytes;
					unsigned char data[1024];

					if(fi == NULL)
					{
						printf("Error opening file.\n");
						continue;
					}

					MD5_Init(&mdContext);
					while((bytes = fread(data, 1, 1024, fi)) != 0)
        				MD5_Update(&mdContext, data, bytes);
        			MD5_Final (c,&mdContext);

        			for(i = 0; i < MD5_DIGEST_LENGTH; i++) 
        				printf("%02x", c[i]);

        			fclose(fi);*/
				}
			}

			close(connectionSocket);
			close(listenSocket);
			printf("Socket Closed.\n");	
			return 0;
		}
	}
	return 0;
}


int client(int clientPort, DIR *dir)
{
	int clientSocket = 0;
	struct sockaddr_in serv_addr;


	//Create socket
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(clientSocket < 0)
	{
		printf("Error while creating socket. Aborting!!\n");
		return 0;
	}
	else
	{
		printf("[Client] Socket created successfully!!\n");
	}

	int portno = clientPort;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");


	//Establish connection
	while(connect(clientSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0);


	//Send requests
	while(1)
	{
		char send_data[1024], recv_data[1024], buffer[1024], *cmd;
		bzero(buffer, 1024);
		bzero(recv_data,1024);
		bzero(send_data,1024);

		fgets(buffer, 1023, stdin);
		if(buffer[strlen(buffer) - 1] == '\n')
			buffer[strlen(buffer) - 1] = ' ';

		cmd = strtok(buffer, " ");

		if(cmd != NULL)
		{
			if(strcmp(cmd, "IndexGet") == 0)
			{
				cmd = strtok(NULL, " ");
			
				if(cmd != NULL)
				{
					if(strcmp(cmd, "--shortlist") == 0)
					{
						strcpy(send_data, "IS ");

						cmd = strtok(NULL, " ");

						if(cmd != NULL)
						{
							strcat(send_data, cmd);

							cmd = strtok(NULL, " ");

							if(cmd != NULL)
							{
								strcat(send_data, " ");
								strcat(send_data, cmd);

								//send buffer
								if(send(clientSocket, send_data, strlen(send_data), 0) < 0)
								{
									printf("Error while writing to the socket.\n");
								}

								bzero(recv_data, 1024);

								while(1)
								{
									int sz = recv(clientSocket, recv_data, 1023, 0);

									if(sz < 0)
									{
										printf("Error while reading from socket.\n");
										break;
									}

									if(sz == 0)
									{
										break;
									}

									printf("%s\n", recv_data);
								}
							}
						}
						else
						{
							printf("Usage: IndexGet ­­shortlist <start­time­stamp> <end­time­stamp>\n");
						}
					}

					else if(strcmp(cmd, "--longlist") == 0)
					{
						strcpy(send_data, "IL");

						if(send(clientSocket, send_data, strlen(send_data), 0) < 0)
						{
							printf("Error while writing to the socket.\n");
						}

						bzero(recv_data, 1024);

						while(1)
						{
							int sz = recv(clientSocket, recv_data, 1023, 0);

							if(sz < 0)
							{
								printf("Error while reading from socket.\n");
								break;
							}

							if(sz == 0)
							{
								printf("no more data\n");
								break;
							}

							printf("%s\n", recv_data);
						}
					}

					else if(strcmp(cmd, "--regex") == 0)
					{
						strcpy(send_data, "IR ");

						cmd = strtok(NULL, " ");

						if(cmd != NULL)
						{
							strcat(send_data, cmd);

							if(send(clientSocket, send_data, strlen(send_data), 0) < 0)
							{
								printf("Error while writing to the socket.\n");
							}

							bzero(recv_data, 1024);

							while(1)
							{
								int sz = recv(clientSocket, recv_data, 1023, 0);

								if(sz < 0)
								{
									printf("Error while reading from socket.\n");
									break;
								}

								if(sz == 0)
								{
									break;
								}

								printf("%s\n", recv_data);
							}
						}
						else
						{
							printf("Usage: IndexGet ­­regex <regex­argument>\n");
						}
					}
				}
				else
				{
					printf("Usage: IndexGet --flag\n");
				}
			}
			
			else if(strcmp(cmd, "FileDownload") == 0)
			{
				cmd = strtok(NULL, " ");

				if(cmd != NULL)
				{
					strcpy(send_data, "D ");
					strcat(send_data, cmd);

					write(clientSocket, send_data, 1024);

					char file[500];
					strcpy(file, "./share/");
					strcat(file, cmd);
					FILE *f = fopen(file, "w");

					bzero(recv_data, 1024);

					while(1)
					{
						int sz = read(clientSocket, recv_data, 1024);

						if(sz <= 0)
						{
							printf("file ended.\n");
							break;
						}

						fwrite(recv_data, 1, sz, f);
					}

					fclose(f);

					bzero(buffer, 0);

					strcpy(buffer, cmd);

					struct stat s;
					int r = lstat(file, &s);
					if(r < 0)
					{
						printf("Error!\n");
						continue;
					}

					char *size = toString(s.st_size);
					strcat(buffer, "\t");
					strcat(buffer, size);

					char *t = toString(s.st_mtime);
					strcat(buffer, "\t");
					strcat(buffer, t);

					printf("%s\n", buffer);

					/*unsigned char c[MD5_DIGEST_LENGTH];
					int i;
					FILE *fi = fopen(file, "rb");

					MD5_CTX mdContext;

					int bytes;
					unsigned char data[1024];

					if(fi == NULL)
					{
						printf("Error opening file.\n");
						continue;
					}

					MD5_Init(&mdContext);
					while((bytes = fread(data, 1, 1024, fi)) != 0)
        				MD5_Update(&mdContext, data, bytes);
        			MD5_Final (c,&mdContext);

        			for(i = 0; i < MD5_DIGEST_LENGTH; i++) 
        				printf("%02x", c[i]);

        			fclose(fi);*/
				}
			}

			else if(strcmp(cmd, "FileUpload") == 0)
			{
				cmd = strtok(NULL, " ");

				if(cmd != NULL)
				{
					fprintf(stderr, "a\n");
					bzero(send_data, 1024);
					bzero(recv_data, 1024);

					strcpy(send_data, "U");
					send(clientSocket, send_data, strlen(send_data), 0);
					fprintf(stderr, "b\n");
					int sz = read(clientSocket, recv_data, 1024);
					fprintf(stderr, "%s\n", recv_data);
					if(strcmp(recv_data, "FileUploadDeny") == 0)
					{
						close(clientSocket);
						printf("client socket closed.\n");
						return 0;
					}

					else if(strcmp(recv_data, "FileUploadAllow") == 0)
					{
						char file[500];
						strcpy(file, "./share/");
						strcat(file, cmd);

						bzero(send_data, 1024);

						strcpy(send_data, file);
						write(clientSocket, send_data, 1024);

						FILE *f = fopen(file, "r");

						if(f == NULL)
						{
							printf("Could not open file.\n");
							continue;
						}

						bzero(send_data, 1024);

						while(!feof(f))
						{
							int sz = fread(send_data, 1, 1024, f);
							send_data[sz] = '\0';

							if(send(clientSocket, send_data, strlen(send_data), 0) < 0)
							{
								printf("Error while sending data.\n");
							}
						}

						printf("Data sent.\n");
						fclose(f);
					}
				}
			}

			close(clientSocket);
			printf("client socket closed.\n");
			return 0;
		}
	}

	return 0;
}


int main(int argc, char **argv)
{
	int clientPort = 7000, serverPort = 5000;

	/*printf("Enter Client Port: ");
	scanf("%d", &clientPort);

	printf("Enter Server Port: ");
	scanf("%d", &serverPort);*/

	DIR *dir = opendir("./share");

	int pid = fork();

	if(!pid)
	{
		client(clientPort, dir);
	}
	else
	{
		server(serverPort, dir, "./share");
	}

	return 0;
}
