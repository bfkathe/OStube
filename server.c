#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>

#define CONNMAX 1000
#define BYTES 7340032

char *ROOT;
int listenfd;
int static clients[CONNMAX];
void error(char *);
void startServer(char *);
void  respond(int);
time_t t;
struct tm *tm;
char fechayhora[100];
static sem_t semaforo;

char hora_de_inicio[100];
static int * cantidad_de_bytes_transferidos; 
static int * cantidad_de_clientes_diferentes;
static int * cantidad_de_solicitudes_atendidas;
static int * cantidad_de_threads_creados;

void mostrardatos(){
	printf("Hora de Inicio del program: %s\n",hora_de_inicio);
	printf("Cantidad de bytes transferidos: %d\n",*cantidad_de_bytes_transferidos);
	printf("Cantidad de clientes atendidos: %d\n",*cantidad_de_clientes_diferentes);
	printf("Cantidad de solicitudes atendidas: %d\n",*cantidad_de_solicitudes_atendidas);
	printf("Cantidad de hilos creados: %d\n",*cantidad_de_threads_creados);
}

void agregarvideo(){

}

void editarinformacion(){

}

void eliminarvideo(){

}
void menu(){
	int numero;
	while(1){
		printf("Manejo de datos\n 1 Mostrar datos\n 2 Agregar video\n 3 Editar informacion de video\n 4 Eliminar video\n Digite un número: ");
		scanf("%d", &numero);
		switch (numero)
		{
			case 1:
				mostrardatos();
				break;

		    	case 2:
				agregarvideo();
				break;
			
			case 3:
				editarinformacion();
				break;
			
			case 4:
				eliminarvideo();
				break;
		    	
			default:
				printf("Ingrese un número correcto\n");
		}

	}
}

int main(int argc, char* argv[])
{
	t=time(NULL);
	tm=localtime(&t);
	strftime(hora_de_inicio, 100, "%d/%m/%Y %H:%M:%S", tm);

	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;
	sem_init(&semaforo, 0, 1);    
	
	//Default Values PATH = ~/ and PORT=10000
	char PORT[6];
	ROOT = getenv("PWD");
	strcpy(PORT,"10000");

	int slot=0;

	cantidad_de_bytes_transferidos = mmap(NULL, sizeof *cantidad_de_bytes_transferidos, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*cantidad_de_bytes_transferidos = 0;
	cantidad_de_clientes_diferentes = mmap(NULL, sizeof *cantidad_de_clientes_diferentes, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*cantidad_de_clientes_diferentes = 0;
	cantidad_de_solicitudes_atendidas = mmap(NULL, sizeof *cantidad_de_solicitudes_atendidas, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*cantidad_de_solicitudes_atendidas = 0;
	cantidad_de_threads_creados = mmap(NULL, sizeof *cantidad_de_threads_creados, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*cantidad_de_threads_creados = 0;

	//Parsing the command line arguments
	while ((c = getopt (argc, argv, "p:r:")) != -1)
		switch (c)
		{
			case 'r':
				ROOT = malloc(strlen(optarg));
				strcpy(ROOT,optarg);
				break;
			case 'p':
				strcpy(PORT,optarg);
				break;
			case '?':
				fprintf(stderr,"Wrong arguments given!!!\n");
				exit(1);
			default:
				exit(1);
		}
	
	printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",PORT,"\033[0m","\033[92m",ROOT,"\033[0m");
	// Setting all elements to -1: signifies there is no client connected
	int i;
	for (i=0; i<CONNMAX; i++)
		clients[i]=-1;
	startServer(PORT);
	if ( fork()==0 )
	{
		menu();
	}
	// ACCEPT connections
	while (1)
	{
		addrlen = sizeof(clientaddr);
		clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);
		if (clients[slot]<0)
			error ("accept() error");
		else
		{
			struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&clientaddr;
			struct in_addr ipAddr = pV4Addr->sin_addr;
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
			////printf("cliente: %s\n", str);
			//contador = slot * 1;
			//pthread_t hilo;
			//pthread_create(&hilo, NULL, respond, NULL);
			if ( fork()==0 )
			{
				respond(slot);
				exit(0);
			}
		}

		while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
	}

	return 0;
}

//start server
void startServer(char *port)
{
	struct addrinfo hints, *res, *p;

	// getaddrinfo for host
	memset (&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo( NULL, port, &hints, &res) != 0)
	{
		//perror ("getaddrinfo() error");
		exit(1);
	}
	// socket and bind
	for (p = res; p!=NULL; p=p->ai_next)
	{
		listenfd = socket (p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1) continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
	}
	if (p==NULL)
	{
		//perror ("socket() or bind()");
		exit(1);
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if ( listen (listenfd, 10000) != 0 )
	{
		//perror("listen() error");
		exit(1);
	}
}

//client connection
void  respond(int n)
{
	char mesg[99999], *reqline[3], data_to_send[BYTES], path[99999];
	int rcvd, fd, bytes_read;
	memset( (void*)mesg, (int)'\0', 99999 );

	rcvd=recv(clients[n], mesg, 99999, 0);

	if (rcvd<0)    // receive error
		fprintf(stderr,("recv() error\n"));
	else if (rcvd==0)    // receive socket closed
		fprintf(stderr,"Client disconnected upexpectedly.\n");
	else    // message received
	{
		////printf("%s", mesg);
		reqline[0] = strtok (mesg, " \t\n");
		if ( strncmp(reqline[0], "GET\0", 4)==0 )
		{
			reqline[1] = strtok (NULL, " \t");
			reqline[2] = strtok (NULL, " \t\n");
			if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
			{
				write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
			}
			else
			{
				if ( strncmp(reqline[1], "/\0", 2)==0 )
					reqline[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], reqline[1]);
				////printf("file: %s\n", path);
				if(strcmp(reqline[1], "/index.html") == 0){
					////printf ("Usuario entrando al sistema\n");
					t=time(NULL);
					tm=localtime(&t);
					strftime(fechayhora, 100, "%d/%m/%Y %H:%M:%S", tm);
					////printf ("Fecha y Hora de entrada: %s\n\n", fechayhora);
					*cantidad_de_clientes_diferentes = *cantidad_de_clientes_diferentes+1;
				}
				else{
					////printf("Solicitud del navegador\n");
					t=time(NULL);
					tm=localtime(&t);
					strftime(fechayhora, 100, "%d/%m/%Y %H:%M:%S", tm);
					////printf ("Fecha y Hora de solicitud: %s\n\n", fechayhora);
				}
				*cantidad_de_solicitudes_atendidas = *cantidad_de_solicitudes_atendidas+1;
				*cantidad_de_threads_creados = *cantidad_de_threads_creados+1;
				if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
				{
					FILE *fich;

					fich=fopen(path,"r");

					fseek(fich, 0L, SEEK_END);
					*cantidad_de_bytes_transferidos = *cantidad_de_bytes_transferidos+ftell(fich);
					fclose(fich);
					send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
					while ( (bytes_read=read(fd, data_to_send, BYTES))>0 )
						write (clients[n], data_to_send, bytes_read);
				}
				else    write(clients[n], "HTTP/1.0 404 Not Found\n", 23); //FILE NOT F
			}
		}
	}

	//Closing SOCKET
	shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
	close(clients[n]);
	clients[n]=-1;
	pthread_exit(NULL);
}

