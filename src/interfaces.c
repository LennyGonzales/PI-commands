#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>

/**
 * Calcule le préfixe CIDR à partir du masque IPv4.
 */
int get_ipv4_prefix(struct sockaddr_in *netmask) {
    uint32_t mask = ntohl(netmask->sin_addr.s_addr);
    int prefix = 0;

    while (mask & 0x80000000) {
        prefix++;
        mask <<= 1;
    }

    return prefix;
}

/**
 * Calcule le préfixe CIDR à partir du masque IPv6.
 */
int get_ipv6_prefix(struct sockaddr_in6 *netmask) {
    int prefix = 0;
    uint8_t *bytes = (uint8_t *)&netmask->sin6_addr;

    for (int i = 0; i < 16; i++) {
        uint8_t byte = bytes[i];
        for (int j = 0; j < 8; j++) {
            if (!(byte & 0x80))
                return prefix;
            prefix++;
            byte <<= 1;
        }
    }

    return prefix;
}

/**
 * Récupère les adresses IPv4 et IPv6 associées à une interface réseau donnée.
 *
 * @param interface_name Nom de l'interface réseau (ex: eth0, wlan0).
 * @return Chaîne formatée contenant les informations ou NULL en cas d'erreur.
 */
char *get_interface_info(const char *interface_name) {
    struct ifaddrs *ifaddr, *ifa;
    char ipv4_str[INET_ADDRSTRLEN + 5] = "Non disponible";
    char ipv6_str[INET6_ADDRSTRLEN + 5] = "Non disponible";

    if (getifaddrs(&ifaddr) == -1) {
        perror("Erreur lors de l'obtention des interfaces");
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || strcmp(ifa->ifa_name, interface_name) != 0)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)ifa->ifa_addr;
            struct sockaddr_in *netmask = (struct sockaddr_in *)ifa->ifa_netmask;
            int prefix = get_ipv4_prefix(netmask);

            inet_ntop(AF_INET, &ipv4->sin_addr, ipv4_str, sizeof(ipv4_str));
            snprintf(ipv4_str + strlen(ipv4_str), 5, "/%d", prefix);
        } 
        else if (ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            struct sockaddr_in6 *netmask = (struct sockaddr_in6 *)ifa->ifa_netmask;
            int prefix = get_ipv6_prefix(netmask);

            inet_ntop(AF_INET6, &ipv6->sin6_addr, ipv6_str, sizeof(ipv6_str));
            snprintf(ipv6_str + strlen(ipv6_str), 5, "/%d", prefix);
        }
    }

    freeifaddrs(ifaddr);

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
    struct ifaddrs *ifaddr, *ifa;
    char *result = malloc(4096);
    result[0] = '\0';

    if (getifaddrs(&ifaddr) == -1) {
        perror("Erreur lors de l'obtention des interfaces");
        free(result);
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (!(ifa->ifa_flags & IFF_UP)) continue; // Filtrer uniquement les interfaces actives

        char *interface_info = get_interface_info(ifa->ifa_name);
        if (interface_info) {
            strncat(result, interface_info, 4096 - strlen(result) - 1);
            free(interface_info);
        }
    }

    freeifaddrs(ifaddr);
    return result;
}