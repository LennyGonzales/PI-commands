#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define PORT 12345

/**
 * Affiche les interfaces réseau disponibles.
 *
 * @param response_buffer Tampon pour stocker la réponse.
 */
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

/**
 * Récupère les détails d'une interface réseau spécifique.
 *
 * @param interface_name Nom de l'interface.
 * @param response_buffer Tampon pour stocker les détails.
 */
void get_interface_details(const char *interface_name, char *response_buffer) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        snprintf(response_buffer, BUFFER_SIZE, "Erreur: Impossible de créer un socket\n");
        return;
    }

    struct ifreq interface_request;
    char ipv4_str[INET_ADDRSTRLEN];
    char ipv6_str[INET6_ADDRSTRLEN];

    strncpy(interface_request.ifr_name, interface_name, IFNAMSIZ - 1);
    interface_request.ifr_name[IFNAMSIZ - 1] = '\0';

    snprintf(response_buffer, BUFFER_SIZE, "Interface: %s\n", interface_name);

    // IPv4
    if (ioctl(socket_fd, SIOCGIFADDR, &interface_request) == 0) {
        struct sockaddr_in *ipv4_address = (struct sockaddr_in *)&interface_request.ifr_addr;
        inet_ntop(AF_INET, &ipv4_address->sin_addr, ipv4_str, sizeof(ipv4_str));
        strncat(response_buffer, "  IPv4: ", BUFFER_SIZE - strlen(response_buffer) - 1);
        strncat(response_buffer, ipv4_str, BUFFER_SIZE - strlen(response_buffer) - 1);
        strncat(response_buffer, "\n", BUFFER_SIZE - strlen(response_buffer) - 1);
    } else {
        strncat(response_buffer, "  IPv4: Non disponible\n", BUFFER_SIZE - strlen(response_buffer) - 1);
    }

    // IPv6
    struct sockaddr_in6 *ipv6_address = (struct sockaddr_in6 *)&interface_request.ifr_addr;
    inet_ntop(AF_INET6, &ipv6_address->sin6_addr, ipv6_str, sizeof(ipv6_str));
    strncat(response_buffer, "  IPv6: ", BUFFER_SIZE - strlen(response_buffer) - 1);
    strncat(response_buffer, ipv6_str, BUFFER_SIZE - strlen(response_buffer) - 1);
    strncat(response_buffer, "\n", BUFFER_SIZE - strlen(response_buffer) - 1);

    close(socket_fd);
}

/**
 * Démarre le serveur pour répondre aux requêtes réseau.
 */
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

/**
 * Démarre le client pour envoyer des requêtes réseau.
 *
 * @param server_ip Adresse IP du serveur.
 * @param command Commande à envoyer.
 */
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

/**
 * Point d'entrée principal du programme.
 *
 * @param argc Nombre d'arguments.
 * @param argv Arguments de la ligne de commande.
 * @return Code de sortie.
 */
int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--server") == 0) {
        start_server();
    } else if (argc == 3 && strcmp(argv[1], "-a") == 0) {
        char server_ip[16];
        strcpy(server_ip, argv[2]);
        start_client(server_ip, "-a");
    } else if (argc == 4 && strcmp(argv[1], "-i") == 0) {
        char server_ip[16];
        strcpy(server_ip, argv[3]);
        char command[BUFFER_SIZE];
        snprintf(command, sizeof(command), "-i %s", argv[2]);
        start_client(server_ip, command);
    } else {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s --server\n", argv[0]);
        fprintf(stderr, "  %s -a <adresse_serveur>\n", argv[0]);
        fprintf(stderr, "  %s -i <interface> <adresse_serveur>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}