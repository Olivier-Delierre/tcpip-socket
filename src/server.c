#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/// Longueur maximal du buffer.
#define BUFLEN 512
/// Nombre maximum de clients autorisés à se connecter.
#define MAX_CLIENTS 3

/// Typedef pour faciliter la compréhension du code.
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;

/// On déclare les sockets.
SOCKET server_socket;
SOCKET client_socket;

/// On déclare les adresses de socket client et serveur.
SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

int addr_length = sizeof(struct sockaddr_in);

int nb_client = 0;

/** 
 * Structure d'un client contenant un socket,
 * ainsi qu'une adresse de socket.
 */
typedef struct
{
	SOCKET socket;
	SOCKADDR_IN client_addr;
}Client;

Client clients[MAX_CLIENTS];

/**
 * Tue le programme en cas d'erreur
 * et spécifie l'erreur.
 *
 * @params s Le nom de la fonction ayant plantée.
 */
void die(char *s)
{
    perror(s);
    exit(1);
}

/**
 * Thread permettant de
 * recevoir et d'envoyer des messages 
 * individuellement aux clients.
 *
 * @params client_void_number Un pointeur comportant le numéro du client
 *                            à écouter.
 */
void *echo_message(void *client_void_number)
{
    int client_number = (int *)client_void_number;
    
    int bytes_read;
    char recv_data[BUFLEN];
   
    if ((bytes_read = recv(clients[client_number].socket, recv_data, 1024, 0)) < 0)
        die("recv");

    /** Tant qu'on reçoit des données, on continue */
    while (bytes_read > 0) 
    {     
        printf("\033[1;32m%i\033[0m : %s\n", client_number, recv_data);
        
        for (int i = 0; i < nb_client; i++)
        { 
            if (send(clients[i].socket, recv_data, bytes_read, 0) != bytes_read)
                die("send"); 
        }
       
        recv_data[bytes_read] = 0;
        
        if ((bytes_read = recv(clients[client_number].socket, recv_data, 1024, 0)) < 0)
            die("recv"); 
    }
    
    char quit_phrase[512] = "\n0 quitted.\n";
    printf("Connection lost to %i.\n", client_number);
    
    quit_phrase[1] = client_number + '0';
   
    for (int i = 0; i < nb_client; i++)
    {
        if (send(clients[i].socket, quit_phrase, sizeof(quit_phrase), 0) != sizeof(quit_phrase))
            die("send");
    }
}

/**
 * Thread permettant d'autoriser les clients
 * à se connecter.
 *
 * @params arg Un argument passé en pointeur.
 */
void *handle_client(void *arg)
{
    char join_phrase[512] = "\n0 join the server.\n";

    while(1)
	{
		if ((clients[nb_client].socket = accept(server_socket, (struct sockaddr *)&clients[nb_client].client_addr, &addr_length)) < 0)
	        die("accept");
          
        printf("Handling client %s called %i.\n", inet_ntoa(clients[nb_client].client_addr.sin_addr), nb_client);    

        join_phrase[1] = nb_client + '0';

        for (int i = 0; i <= nb_client; i++)
        {
            if (send(clients[i].socket, join_phrase, sizeof(join_phrase), 0) != sizeof(join_phrase))
                die("send");
        }

        nb_client++;
        
        pthread_t echo_message_thread;

        if (pthread_create(&echo_message_thread, NULL, echo_message, nb_client - 1) == -1)
            die ("thread_echo");
    }

	(void) arg;
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    /// Si l'utilisateur spécifie pas un numéro de port, on termine le programme.
    if (argc != 2)
    {
        fprintf(stderr, "Usage : %s <Server Port>\n", argv[0]);
        exit(1);
    }

    system("clear");
    printf("-------------------\n");
	printf("-- Server socket --\n");
	printf("-------------------\n");
	
	int port = atoi(argv[1]);

	/// On initialise les buffers pour réceptionner les datas, d'une taille de BUFSIZE octets.
	char recv_data[BUFLEN];
	
    /// On déclare la longueur de l'adresse.
	int addr_len; 
    int bytes_read;

	// Si l'initialisation du socket échoue, on le tue.
    if ((server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        die("socket");

	/// On initialise les paramètres de l'adresse du socket serveur.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
	
	/// Si le bind de l'adresse du socket serveur échoue, on le tue.
    if (bind(server_socket,(struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
        die("bind");

	/**
	 * Si l'écoute échoue, on la tue
	 * Celle-ci est limitée à MAX_CLIENTS connexion simultanée
	 */
	if (listen(server_socket, MAX_CLIENTS) < 0)
		die("listen");

    printf("Waiting for a client on port %d : \n", port);
    fflush(stdout);
    
    /** Création du thread de handling */
    pthread_t handle_client_thread;
		
	if (pthread_create(&handle_client_thread, NULL, handle_client, NULL) == -1)
	        die ("thread_handle");

    /** Boucle infinie */
	while(1)
	{}
}
