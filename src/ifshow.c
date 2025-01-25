#include "interfaces.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>

/**
 * Affiche l'usage du programme.
 */
void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s -i <interface_name> | -a\n", program_name);
}

/**
 * Point d'entrée principal du programme.
 *
 * @param argc Nombre d'arguments.
 * @param argv Tableau des arguments.
 * @return 0 en cas de succès, autre chose en cas d'erreur.
 */
int main(int argc, char *argv[]) {
    if (argc == 3 && strcmp(argv[1], "-i") == 0) {
        char *info = get_interface_info(argv[2]);
        if (info) {
            printf("%s\n", info);
            free(info);
        } else {
            fprintf(stderr, "Erreur : Impossible de récupérer les informations pour l'interface %s\n", argv[2]);
        }
        return EXIT_SUCCESS;
    }

    if (argc == 2 && strcmp(argv[1], "-a") == 0) {
        char *info = get_all_interfaces_info();
        if (info) {
            printf("%s\n", info);
            free(info);
        } else {
            fprintf(stderr, "Erreur : Impossible de récupérer les informations sur les interfaces.\n");
        }
        return EXIT_SUCCESS;
    }

    print_usage(argv[0]);
    return EXIT_FAILURE;
}