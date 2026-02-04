#include "netlink.h"

/* Callback to handle sequence number errors - just ignore them for multicast */
static int seq_check(struct nl_msg *msg, void *arg) {
    return NL_OK;
}

int do_listen_multicast() {
    int ret;
    listen_sock_rtnl = create_socket(NETLINK_ROUTE, RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE);
    if (!listen_sock_rtnl) {
        fprintf(stderr, "Failed to create netlink socket\n");
        return EXIT_FAILURE;
    }

    /* Setup callbacks to handle */
    listen_cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!listen_cb) {
        fprintf(stderr, "Failed to allocate netlink callbacks\n");
        nl_socket_free(listen_sock_rtnl);
        return EXIT_FAILURE;
    }

    /* Set callback to ignore sequence number errors */
    nl_cb_set(listen_cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, seq_check, NULL);
    nl_cb_set(listen_cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, NULL);

    while (1) {
        ret = nl_recvmsgs(listen_sock_rtnl, listen_cb);
        if (ret < 0 ) {
            fprintf(stderr, "Error receiving netlink messages: %s (ret=%d)\n", nl_geterror(ret), ret);
            continue;
        }
    }

    nl_cb_put(listen_cb);
    nl_socket_free(listen_sock_rtnl);
}