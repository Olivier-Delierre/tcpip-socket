#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

/// Longueur maximale du buffer
#define BUFLEN 1024

/// Typedef pour faciliter la compréhension du code.
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;

int port;

/** Création du socket, et des adresses de socket */
SOCKADDR_IN serv_addr;
SOCKET sock;

int addr_len = sizeof(struct sockaddr_in);

void die(char *s)
{
	perror(s);
	exit(1);
}

void *receive_message(void *arg)
{
    /** Initialisation du message à recevoir */
    char recv_data[BUFLEN];
    int recv_len;
	
    while(1)
	{
		if ((recv_len = recv(sock, recv_data, 1024, 0)) < 0)
            die("recv");

		if (recv_len > 0)
		{
			recv_data[recv_len] = 0;
			printf("Message from server : %s\n", recv_data);
		}
	}

	(void) arg;
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	/** On UNIX system */
	system("clear");
    
    printf("-------------------\n");
	printf("----- tcpCHAT -----\n");
	printf("-------------------\n");
		
	/** On demande à l'utilisateur de choisir le port d'envoi */
	printf("Choose the port : ");
	scanf("%d", &port);

	/** On UNIX system */
	system("clear");

	/** Initialisation du message à envoyer */
	char send_data[1024];

	/** Si le socket ne se créer pas, on le tue */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die("socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
    /** Si la connexion échoue, on la tue */
	if ((connect(sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr))) < 0)
		die("connect");

	int send_len = 0;
	
	/** 
	 * 	On créer un thread pour la réception des messages
	 *  pour qu'on puisse envoyer des messages tout en en recevant
	 */
	pthread_t receiveMessage;
		
	if (pthread_create(&receiveMessage, NULL, receive_message, NULL) == -1)
		die ("err thread receive");
	
	while(1)
	{
		printf("\rSend a message : ");
		fgets(send_data, 512, stdin);
        if (strlen(send_data) > 0 && (send_data[strlen(send_data) - 1] == '\n')) send_data[strlen(send_data) - 1] = '\0';
	
        if (send(sock, send_data, strlen(send_data), 0) != strlen(send_data))
			   die("send");	
	}
}

