#include "netlink.h"

/*
* tinybr - Bridge utility base on NETLINK socket
* type: NETLINK_ROUTE
*
*/

struct nl_sock *listen_sock_rtnl = NULL;
struct nl_cb *listen_cb = NULL;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "monitor") == 0) {
        printf("tinybr start monitor...\n");
        do_listen_multicast();
        // Add code to start tinybr service
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}