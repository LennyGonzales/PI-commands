// Includes communs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>

// Constantes
#define PORT 12345 // Port du serveur
#define BUFFER_SIZE 1024

// Fonction pour afficher les interfaces (réutilisation de ifshow)
void list_interfaces(char *response_buffer) {
    struct if_nameindex *interfaces = if_nameindex();
    if (!interfaces) {
        snprintf(response_buffer, BUFFER_SIZE, "Erreur: Impossible de récupérer les interfaces\n");
        return;
    }

    char temp_buffer[BUFFER_SIZE] = "";
    for (struct if_nameindex *current = interfaces; current && current->if_name; ++current) {
        snprintf(temp_buffer, sizeof(temp_buffer), "Interface: %s\n", current->if_name);
        strncat(response_buffer, temp_buffer, BUFFER_SIZE - strlen(response_buffer) - 1);
    }

    if_freenameindex(interfaces);
}

void get_interface_details(const char *interface_name, char *response_buffer) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        snprintf(response_buffer, BUFFER_SIZE, "Erreur: Impossible de créer un socket\n");
        return;
    }

    struct ifreq interface_request;
    strncpy(interface_request.ifr_name, interface_name, IFNAMSIZ - 1);
    interface_request.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(socket_fd, SIOCGIFADDR, &interface_request) == 0) {
        struct sockaddr_in *ipv4_address = (struct sockaddr_in *)&interface_request.ifr_addr;
        char temp_buffer[BUFFER_SIZE];
        snprintf(temp_buffer, sizeof(temp_buffer), "  IPv4: %s\n", inet_ntoa(ipv4_address->sin_addr));
        strncat(response_buffer, temp_buffer, BUFFER_SIZE - strlen(response_buffer) - 1);
    } else {
        strncat(response_buffer, "  IPv4: Non disponible\n", BUFFER_SIZE - strlen(response_buffer) - 1);
    }

    close(socket_fd);
}

// --- Serveur ---
void start_server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Créer le socket serveur
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Erreur lors de la création du socket serveur");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Liaison du socket au port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors du bind du socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Attente de connexions
    if (listen(server_fd, 5) < 0) {
        perror("Erreur lors de l'écoute");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en attente de connexions sur le port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Erreur lors de l'acceptation d'une connexion");
            continue;
        }

        printf("Client connecté: %s\n", inet_ntoa(client_addr.sin_addr));

        memset(buffer, 0, BUFFER_SIZE);
        read(client_fd, buffer, BUFFER_SIZE - 1);

        char response[BUFFER_SIZE] = "";
        if (strncmp(buffer, "-a", 2) == 0) {
            list_interfaces(response);
        } else if (strncmp(buffer, "-i", 2) == 0) {
            char *interface_name = buffer + 3; // Après "-i "
            get_interface_details(interface_name, response);
        } else {
            snprintf(response, sizeof(response), "Commande invalide\n");
        }

        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
}

// --- Client ---
void start_client(const char *server_ip, const char *command) {
    int socket_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Erreur lors de la création du socket client");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Adresse IP invalide");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    write(socket_fd, command, strlen(command));

    memset(buffer, 0, BUFFER_SIZE);
    read(socket_fd, buffer, BUFFER_SIZE - 1);
    printf("Réponse du serveur:\n%s", buffer);

    close(socket_fd);
}

// --- Point d'entrée ---
int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--server") == 0) {
        start_server();
    } else if (argc == 4 && strcmp(argv[1], "-n") == 0) {
        start_client(argv[2], argv[3]);
    } else {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s --server\n", argv[0]);
        fprintf(stderr, "  %s -n <adresse_serveur> <-a | -i <interface>>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
