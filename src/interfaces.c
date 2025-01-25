#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>

/**
 * Récupère les adresses IPv4 et IPv6 associées à une interface réseau donnée.
 *
 * @param interface_name Nom de l'interface réseau (ex: eth0, wlan0).
 * @return Chaîne formatée contenant les informations ou NULL en cas d'erreur.
 */
char *get_interface_info(const char *interface_name) {
    struct ifreq interface_request;
    char ipv4_str[INET_ADDRSTRLEN] = "Non disponible";
    char ipv6_str[INET6_ADDRSTRLEN] = "Non disponible";
    int socket_fd;

    // Créer un socket pour interagir avec les informations réseau
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("Erreur lors de la création du socket");
        return NULL;
    }

    strncpy(interface_request.ifr_name, interface_name, IFNAMSIZ - 1);
    interface_request.ifr_name[IFNAMSIZ - 1] = '\0';

    // Récupérer l'adresse IPv4
    if (ioctl(socket_fd, SIOCGIFADDR, &interface_request) == 0) {
        struct sockaddr_in *ipv4_address = (struct sockaddr_in *)&interface_request.ifr_addr;
        inet_ntop(AF_INET, &ipv4_address->sin_addr, ipv4_str, sizeof(ipv4_str));
    }

    // Récupérer l'adresse IPv6 (optionnelle, exemple simplifié)
    if (ioctl(socket_fd, SIOCGIFADDR, &interface_request) == 0) {
        struct sockaddr_in6 *ipv6_address = (struct sockaddr_in6 *)&interface_request.ifr_addr;
        inet_ntop(AF_INET6, &ipv6_address->sin6_addr, ipv6_str, sizeof(ipv6_str));
    }

    close(socket_fd); // Fermer le socket

    // Allouer une chaîne pour le résultat
    char *result = malloc(256);
    snprintf(result, 256, "Interface: %s\n  IPv4: %s\n  IPv6: %s\n", interface_name, ipv4_str, ipv6_str);
    return result;
}

/**
 * Récupère les informations sur toutes les interfaces réseau disponibles.
 *
 * @return Chaîne formatée contenant les informations ou NULL en cas d'erreur.
 */
char *get_all_interfaces_info() {
    struct if_nameindex *interfaces = if_nameindex();
    if (!interfaces) {
        perror("Erreur lors de l'obtention des interfaces");
        return NULL;
    }

    // Concaténer les informations sur toutes les interfaces
    char *result = malloc(4096);
    result[0] = '\0';

    for (struct if_nameindex *current_interface = interfaces; current_interface && current_interface->if_name; ++current_interface) {
        char *interface_info = get_interface_info(current_interface->if_name);
        if (interface_info) {
            strncat(result, interface_info, 4096 - strlen(result) - 1);
            free(interface_info);
        }
    }

    if_freenameindex(interfaces); // Libérer la mémoire allouée pour la liste des interfaces
    return result;
}