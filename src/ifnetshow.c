#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "interfaces.h"

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

// Envoie une commande au serveur et affiche la réponse
void send_command_to_server(const char *server_ip, const char *command)
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        perror("Erreur lors de la création du socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Adresse IP invalide");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Échec de la connexion au serveur");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    // Envoyer la commande
    send(socket_fd, command, strlen(command), 0);

    // Recevoir la réponse
    char buffer[4096];
    int bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
    {
        perror("Erreur lors de la réception des données");
    }
    else
    {
        buffer[bytes_received] = '\0'; // Assurer une chaîne terminée
        printf("%s\n", buffer);
    }

    close(socket_fd);
}

// Fonction principale du serveur réseau
void start_server()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Création du socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Erreur lors de la création du socket serveur");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Écoute sur toutes les interfaces
    server_addr.sin_port = htons(SERVER_PORT);

    // Liaison du socket au port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Erreur lors du bind du socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Mise en mode écoute
    if (listen(server_fd, 5) < 0)
    {
        perror("Erreur lors de l'écoute");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de connexions sur le port %d...\n", SERVER_PORT);

    // Boucle principale d'acceptation des connexions
    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0)
        {
            perror("Erreur lors de l'acceptation d'une connexion");
            continue;
        }

        printf("Client connecté: %s\n", inet_ntoa(client_addr.sin_addr));

        // Lecture de la commande du client
        memset(buffer, 0, BUFFER_SIZE);
        read(client_fd, buffer, BUFFER_SIZE - 1);

        // Traitement de la commande
        char response[BUFFER_SIZE] = "";
        if (strncmp(buffer, "GET_ALL", 7) == 0)
        { // Liste de toutes les interfaces
            char *interfaces_info = get_all_interfaces_info();
            if (interfaces_info)
            {
                snprintf(response, sizeof(response), "%s", interfaces_info);
                free(interfaces_info);
            }
            else
            {
                snprintf(response, sizeof(response), "Erreur : Impossible de récupérer les interfaces.\n");
            }
        }
        else if (strncmp(buffer, "GET_IF", 6) == 0)
        { // Détails d'une interface
            char *interface_name = buffer + 7;
            char *interface_info = get_interface_info(interface_name);
            if (interface_info)
            {
                snprintf(response, sizeof(response), "%s", interface_info);
                free(interface_info);
            }
            else
            {
                snprintf(response, sizeof(response), "Erreur : Impossible de récupérer les informations pour l'interface %s\n", interface_name);
            }
        }
        else
        {
            snprintf(response, sizeof(response), "Commande invalide\n");
        }

        // Envoi de la réponse au client
        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
}

// Affiche l'usage du programme
void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s -n <addr> -i <ifname> | -a | --server\n", program_name);
}

// Point d'entrée principal du programme
int main(int argc, char *argv[])
{
    if (argc == 5 && strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-i") == 0)
    {
        char command[256];
        snprintf(command, sizeof(command), "GET_IF %s", argv[4]);
        send_command_to_server(argv[2], command);
        return EXIT_SUCCESS;
    }

    if (argc == 4 && strcmp(argv[1], "-n") == 0 && strcmp(argv[3], "-a") == 0)
    {
        send_command_to_server(argv[2], "GET_ALL");
        return EXIT_SUCCESS;
    }

    if (argc == 2 && strcmp(argv[1], "--server") == 0)
    {
        // Lancer le serveur pour écouter les commandes
        printf("Démarrage du serveur d'écoute sur le port %d...\n", SERVER_PORT);
        start_server();
        return EXIT_SUCCESS;
    }

    print_usage(argv[0]);
    return EXIT_FAILURE;
}
