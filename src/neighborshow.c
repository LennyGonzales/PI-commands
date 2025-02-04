
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "neighbor.h"

void start_neighbor_agent() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(NEIGHBOR_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Neighbor agent listening on port %d\n", NEIGHBOR_PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        // Send local hostname information
        char hostname[MAX_HOSTNAME_LENGTH];
        gethostname(hostname, sizeof(hostname));
        write(client_fd, hostname, strlen(hostname));
        close(client_fd);
    }
}

// neighborshow.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "neighbor.h"

int discover_neighbors(NeighborList *list, int max_hops) {
    // Discover neighbors on the local network
    // This is a simplified implementation and would need network scanning logic
    list->neighbor_count = 0;

    // Example local network scanning (would need more sophisticated implementation)
    char *local_network[] = {"192.168.1.1", "192.168.1.2", "192.168.1.3"};
    int network_size = sizeof(local_network) / sizeof(local_network[0]);

    for (int i = 0; i < network_size && list->neighbor_count < MAX_NEIGHBORS; i++) {
        int socket_fd;
        struct sockaddr_in server_addr;
        char buffer[MAX_HOSTNAME_LENGTH];

        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) continue;

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(NEIGHBOR_PORT);
        inet_pton(AF_INET, local_network[i], &server_addr.sin_addr);

        if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0) {
            int bytes_received = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                
                // Check for duplicates
                int is_duplicate = 0;
                for (int j = 0; j < list->neighbor_count; j++) {
                    if (strcmp(list->neighbors[j].hostname, buffer) == 0) {
                        is_duplicate = 1;
                        break;
                    }
                }

                if (!is_duplicate) {
                    strncpy(list->neighbors[list->neighbor_count].hostname, buffer, MAX_HOSTNAME_LENGTH);
                    strncpy(list->neighbors[list->neighbor_count].ip_address, local_network[i], 64);
                    list->neighbor_count++;
                }
            }
        }
        close(socket_fd);
    }

    return list->neighbor_count;
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [-hop n]\n", program_name);
    fprintf(stderr, "  -hop n: display neighbors within n network hops (default: 1)\n");
}

int main(int argc, char *argv[]) {
    int max_hops = 1;

    if (argc == 3 && strcmp(argv[1], "-hop") == 0) {
        max_hops = atoi(argv[2]);
    } else if (argc > 1) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    NeighborList neighbors;
    int discovered = discover_neighbors(&neighbors, max_hops);

    printf("Discovered %d neighbors:\n", discovered);
    for (int i = 0; i < discovered; i++) {
        printf("  %s (IP: %s)\n", 
               neighbors.neighbors[i].hostname, 
               neighbors.neighbors[i].ip_address);
    }

    return EXIT_SUCCESS;
}