#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H
#include "netlink.h"

struct nlevt_pair {
    int event_type;
    const char *event_string;
};

extern struct nlevt_pair nlevt_map[];

int valid_handler(struct nl_msg *msg, void *arg);
#endif // EVENT_HANDLER_H