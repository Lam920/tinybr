#include "netlink.h"

struct nl_sock *create_socket(int protocol, int groups) {
    struct nl_sock *sock;
    sock = nl_socket_alloc();
    if (!sock) {
        fprintf(stderr, "Failed to allocate netlink socket\n");
        return NULL;
    }

    printf("DEBUG: Created socket, protocol=%d, groups=0x%x\n", protocol, groups);

    if (groups) {
        /* 
         * Set sockaddr_nl group to join my groups mask 
         * So that we can receive multicast messages
         */
        nl_join_groups(sock, groups);
        printf("DEBUG: nl_join_groups returned\n");
        
        /* Disable sequence number checking for multicast sockets */
        nl_socket_disable_seq_check(sock);
        printf("DEBUG: Disabled sequence number checking\n");
    }

    /* Connect and bind to netlink socket type ROUTE to send/listen Network event */
    if (nl_connect(sock, protocol) < 0) {
        fprintf(stderr, "Failed to connect netlink socket\n");
        nl_socket_free(sock);
        return NULL;
    }
    
    printf("DEBUG: Socket connected successfully\n");
    return sock;
}

// int init_netlink_sockets() {
//     sock_rtnl = create_socket(NETLINK_ROUTE, 0);
//     if (!sock_rtnl) {
//         return -1;
//     }
//     return 0;
// }