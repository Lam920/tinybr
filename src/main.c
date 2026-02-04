#include "netlink.h"

/*
* tinybr - Bridge utility base on NETLINK socket
* type: NETLINK_ROUTE
*
*/

static void usage(void)
{
    fprintf(stderr,
        "Usage: tinybr <command>\n"
        "Commands:\n"
        "  help                Show this help message\n"
        "  monitor             Start monitoring network events\n"
        " add dev ...          Add bridge interface\n"
    );

}

static int do_help(int argc, char **argv)
{
	usage();
	return 0;
}



struct nl_sock *listen_sock_rtnl = NULL;
struct nl_sock *sock_rtnl = NULL;

struct nl_cb *listen_cb = NULL;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return EXIT_FAILURE;
    }


    if (strcmp(argv[1], "help") == 0)
		do_help(argc, argv);
    else if (strcmp(argv[1], "monitor") == 0) {
        printf("tinybr start monitor...\n");
        do_listen_multicast();
        // Add code to start tinybr service
    } 
    else if (strcmp(argv[1], "link") == 0)
    {
        // Add code to create bridge interface, affect to layer 2 only
        do_bridge(argc - 2, argv + 2);
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}