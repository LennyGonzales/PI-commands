#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>

/**
 * Affiche les adresses IPv4 et IPv6 associées à une interface réseau donnée.
 *
 * @param interface_name Nom de l'interface réseau (ex: eth0, wlan0).
 */
void show_ip_address(const char *interface_name)
{
    struct ifreq interface_request;
    char ipv4_str[INET_ADDRSTRLEN];
    char ipv6_str[INET6_ADDRSTRLEN];
    int socket_fd;

    // Créer un socket pour interagir avec les informations réseau
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        perror("Erreur lors de la création du socket");
        return;
    }

    strncpy(interface_request.ifr_name, interface_name, IFNAMSIZ - 1);
    interface_request.ifr_name[IFNAMSIZ - 1] = '\0';

    printf("Interface: %s\n", interface_name);

    // Récupérer l'adresse IPv4
    if (ioctl(socket_fd, SIOCGIFADDR, &interface_request) != 0)
    {
        printf("  IPv4: Non disponible\n");
        printf("  IPv6: Non disponible\n");
        return;
    }
    struct sockaddr_in *ipv4_address = (struct sockaddr_in *)&interface_request.ifr_addr;
    inet_ntop(AF_INET, &ipv4_address->sin_addr, ipv4_str, sizeof(ipv4_str));
    printf("  IPv4: %s\n", ipv4_str);

    struct sockaddr_in6 *ipv6_address = (struct sockaddr_in6 *)&interface_request.ifr_addr;
    inet_ntop(AF_INET6, &ipv6_address->sin6_addr, ipv6_str, sizeof(ipv6_str));
    printf("  IPv6: %s\n", ipv6_str);

    close(socket_fd); // Fermer le socket
}

/**
 * Affiche toutes les interfaces réseau disponibles et leurs adresses IP.
 */
void show_all_interfaces()
{
    struct if_nameindex *interfaces = if_nameindex();
    if (!interfaces)
    {
        perror("Erreur lors de l'obtention des interfaces");
        return;
    }

    // Parcourir chaque interface et afficher ses adresses IP
    for (struct if_nameindex *current_interface = interfaces; current_interface && current_interface->if_name; ++current_interface)
    {
        show_ip_address(current_interface->if_name);
    }

    if_freenameindex(interfaces); // Libérer la mémoire allouée pour la liste des interfaces
}

/**
 * Affiche l'usage du programme.
 */
void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s -i <interface_name> | -a\n", program_name);
}

/**
 * Point d'entrée principal du programme.
 *
 * @param argc Nombre d'arguments.
 * @param argv Tableau des arguments.
 * @return 0 en cas de succès, autre chose en cas d'erreur.
 */
int main(int argc, char *argv[])
{
    if (argc == 3 && strcmp(argv[1], "-i") == 0)
    {
        show_ip_address(argv[2]);
        return EXIT_SUCCESS;
    }

    if (argc == 2 && strcmp(argv[1], "-a") == 0)
    {
        show_all_interfaces();
        return EXIT_SUCCESS;
    }

    print_usage(argv[0]);
    return EXIT_FAILURE;
}
