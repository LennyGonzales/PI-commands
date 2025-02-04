// neighbor.h
#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#define NEIGHBOR_PORT 12346
#define MAX_HOSTNAME_LENGTH 256
#define MAX_NEIGHBORS 64

typedef struct {
    char hostname[MAX_HOSTNAME_LENGTH];
    char ip_address[64];
} Neighbor;

typedef struct {
    Neighbor neighbors[MAX_NEIGHBORS];
    int neighbor_count;
} NeighborList;

// Function prototypes for neighborshow and neighboragent
int discover_neighbors(NeighborList *list, int max_hops);
void start_neighbor_agent();
void send_neighbor_info(const char *server_ip);

#endif // NEIGHBOR_H