#include "netlink.h"


struct nlevt_pair nlevt_map[] = {
    {RTM_NEWLINK, "RTM_NEWLINK"},
    {RTM_DELLINK, "RTM_DELLINK"},
    {RTM_NEWADDR, "RTM_NEWADDR"},
    {RTM_DELADDR, "RTM_DELADDR"},
    {RTM_NEWROUTE, "RTM_NEWROUTE"},
    {RTM_DELROUTE, "RTM_DELROUTE"},
    {RTM_NEWNEIGH, "RTM_NEWNEIGH"},
    {RTM_DELNEIGH, "RTM_DELNEIGH"},
    {RTM_NEWNSID, "RTM_NEWNSID"},
    {RTM_DELNSID, "RTM_DELNSID"},
    {0, NULL}
};

static int link_event_handler(struct nl_msg *msg, void *arg) {
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    struct ifinfomsg *ifi = nlmsg_data(nlh);
    struct rtattr *attr;
    int len;
    char *if_name = "unknown";
    char *if_state = "unknown";

    if (ifi->ifi_flags & IFF_UP)
        if_state = "UP";
    else
       if_state = "DOWN";

    attr = IFLA_RTA(ifi);
    len = IFLA_PAYLOAD(nlh);

    while (RTA_OK(attr, len)) {
        switch (attr->rta_type) {
            case IFLA_IFNAME:
                if_name = (char *)RTA_DATA(attr);
                break;
            default:
                break;
        }
        attr = RTA_NEXT(attr, len);
    }

    printf("Link event: Interface %s is %s\n", if_name, if_state);
    return NL_OK;
}

static int addr_event_handler(struct nl_msg *msg, void *arg) {
    return NL_OK;
}

static int route_event_handler(struct nl_msg *msg, void *arg) {
    return NL_OK;
}

static const char* get_event_name(int type) {
    for (int i = 0; nlevt_map[i].event_string != NULL; i++) {
        if (nlevt_map[i].event_type == type) {
            return nlevt_map[i].event_string;
        }
    }
    return "UNKNOWN";
}

int valid_handler(struct nl_msg *msg, void *arg) {
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    
    if (!nlh) {
        printf("DEBUG: NULL nlmsghdr\n");
        return NL_SKIP;
    }
    
    printf("DEBUG: Received message type: %d (%s)\n", nlh->nlmsg_type, get_event_name(nlh->nlmsg_type));
    
    switch (nlh->nlmsg_type) {
        case RTM_NEWLINK:
        case RTM_DELLINK:
            return link_event_handler(msg, arg);
            
        case RTM_NEWADDR:
        case RTM_DELADDR:
            return addr_event_handler(msg, arg);
            
        case RTM_NEWROUTE:
        case RTM_DELROUTE:
            return route_event_handler(msg, arg);
            
        default:
            printf("DEBUG: Unknown message type: %d\n", nlh->nlmsg_type);
            return NL_OK;
    }
}