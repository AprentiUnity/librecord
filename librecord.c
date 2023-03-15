// Attention, il faut mettre plus de messages d'erreur dans le programme même
// Attention, il reste à faire la gestion d'érreurs dans les fonction en dehors de main
// Programme destiné à tester l'échange en peer to peer en C

//////////////////////////////////////////////////////////////////////LIBRAIRIES////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

///////////////////////////////////////////////////////////////DEFINITION DE CONSTANTES/////////////////////////////////////////////////////////////////////////

#define PORT 19348
#define MAXCLIENTS 1
#define LONGUEURPSEUDO 17

//////////////////////////////////////////////////////////////////////FONCTIONS/////////////////////////////////////////////////////////////////////////////////

//Fonction d'envoi d'un message

void sending(char pseudo[])
{

    char buffer[2000] = {0};
    //Fetching port number
    short unsigned int PORT_server;
    char ipv4[16] = {0};

    //IN PEER WE TRUST
    printf("Entrez l'adresse ipV4 à laquelle vous souhaitez envoyer le message : \n");
    scanf(" %15[0-9.]^\n", ipv4);
    printf("Entrez le port sur lequel vous allez envoyer le message : \n"); //Considering each peer will enter different port
    scanf(" %5hd[0-9]^\n", &PORT_server);

    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char hello[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Erreur pendant la création du Socket \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = ipv4;
    serv_addr.sin_port = htons(PORT_server);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Echec de la Connection \n");
        return;
    }

    char dummy;
    printf("Entrez votre message:");
    scanf("%c", &dummy); //The buffer is our enemy
    scanf("%[^\n]s", hello);
    sprintf(buffer, "%s[PORT:%d] vous écrit : %s", pseudo, PORT, hello);
    send(sock, buffer, sizeof(buffer), 0);
    printf("\nMessage envoyé\n");
    close(sock);
}

//Fonction de reception des messages

void receiving(int socketServeur)
{
    struct sockaddr_in addresse;
    int valread;
    char buffer[2000] = {0};
    int addrlen = sizeof(addresse);
    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(socketServeur, &current_sockets);
    int k = 0;
    while (1)
    {
        k++;
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {

                if (i == socketServeur)
                {
                    int client_socket;

                    if ((client_socket = accept(socketServeur, (struct sockaddr *)&addresse,
                                                (socklen_t *)&addrlen)) < 0)
                    {
                        exit(6);
                    }
                    FD_SET(client_socket, &current_sockets);
                }
                else
                {
                    valread = recv(i, buffer, sizeof(buffer), 0);
                    printf("\n%s\n", buffer);
                    FD_CLR(i, &current_sockets);
                }
            }
        }

        if (k == (FD_SETSIZE * 2))
            break;
    }
}

//Processus de vérification des receptions

void *receive_thread(void *socketServeur)
{
    int s_fd = *((int *)socketServeur);
    while (1)
    {
        sleep(2);
        receiving(s_fd);
    }
}

//////////////////////////////////////////////////////////////////PROGRAMME PRINCIPAL////////////////////////////////////////////////////////////////////

int main(int argc, unsigned char *argv[])
{
    char pseudo[LONGUEURPSEUDO]; 
    printf("Entrez un nom d'utilisateur (16 caratères maximum avec uniquement des lettres et les chiffres) : ");
    while (scanf("%16[a-zA-Z0-9]^\n", pseudo) != 1)
    {
        printf("Nom d'utilisateur incorrect, réessayez\n");
        printf("Entrez un nom d'utilisateur (16 caratères maximum avec uniquement des lettres et les chiffres) : ");
    }
    int socketServeur;
    struct sockaddr_in addresse;

    if ((socketServeur = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        exit(3);
    }

    addresse.sin_family = AF_INET;
    addresse.sin_addr.s_addr = argv[1];
    addresse.sin_port = htons(PORT);

    printf("Le port de communication est : %d\n", PORT);

    if (bind(socketServeur, (struct sockaddr *)&addresse, sizeof(addresse)) < 0)
    {
        exit(4);
    }

    if (listen(socketServeur, MAXCLIENTS) < 0)
    {
        exit(5);
    }

    int ch;
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &socketServeur); //Creating thread to keep receiving message in real time
    printf("\n*****Choisissez :*****\n1.Envoyer un message\n0.Quitter\n");
    printf("\nEntrez votre choix : ");
    do
    {

        scanf("%d", &ch);
        switch (ch)
        {
        case 1:
            sending(pseudo);
            break;
        case 0:
            printf("\nAu revoir !\n");
            break;
        default:
            printf("\nChoix incorrect\n");
        }
    } while (ch);

    close(socketServeur);

    return 0;
}
