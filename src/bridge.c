#include "netlink.h"


static struct nl_msg *__system_ifinfo_msg(int af, int index, const char *ifname, uint16_t type, uint16_t flags)
{
	struct nl_msg *msg;
	struct nlattr *br_linkinfo, *br_data;

	struct ifinfomsg ifi = {
		.ifi_family = af,
		.ifi_index = index,
	};

	msg = nlmsg_alloc_simple(type, flags | NLM_F_REQUEST);
	if (!msg)
		return NULL;

	nlmsg_append(msg, &ifi, sizeof(ifi), 0);
	if (ifname)
		nla_put_string(msg, IFLA_IFNAME, ifname);

	/* Add link is "bridge" */
	if (!(br_linkinfo = nla_nest_start(msg, IFLA_LINKINFO)))
		goto nla_put_failure;

	nla_put_string(msg, IFLA_INFO_KIND, "bridge");

	/* Currently no data need to added, bypass , just dummy code */
	if (!(br_data = nla_nest_start(msg, IFLA_INFO_DATA)))
		goto nla_put_failure;
	
	nla_nest_end(msg, br_data);
	nla_nest_end(msg, br_linkinfo);

	return msg;

nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

static struct nl_msg *system_ifinfo_msg(const char *ifname, uint16_t type, uint16_t flags)
{
	return __system_ifinfo_msg(AF_UNSPEC, 0, ifname, type, flags);
}


static int bridge_modify(int cmd, unsigned int flags, int argc, char **argv)
{
    char *br_name = NULL;
    struct nl_msg *msg;
    int ret;
    
    if (matches(*argv, "dev") == 0)
        NEXT_ARG();
    
    br_name = *argv;
    printf("DEBUG: Bridge name is %s\n", br_name);

    msg = system_ifinfo_msg(br_name, cmd, flags);
    if (!msg) {
        fprintf(stderr, "Failed to allocate netlink message\n");
        return -ENOMEM;
    }

    printf("DEBUG: Sending netlink message...\n");
    ret = nl_send_auto_complete(sock_rtnl, msg);
    if (ret < 0) {
        fprintf(stderr, "Failed to send netlink message: %s\n", nl_geterror(ret));
        nlmsg_free(msg);
        return ret;
    }
    printf("DEBUG: Message sent, ret=%d\n", ret);

    nlmsg_free(msg);

    printf("DEBUG: Waiting for ACK...\n");
    ret = nl_wait_for_ack(sock_rtnl);
    if (ret < 0) {
        fprintf(stderr, "Failed to receive ACK: %s\n", nl_geterror(ret));
        return ret;
    }
    
    printf("Bridge %s created successfully!\n", br_name);
    return 0;
}

int do_bridge(int argc, char **argv) {
    if (argc < 1) {
        fprintf(stderr, "Usage: tinybr link add dev <bridge_name>\n");
        return -1;
    }
    
    /* Initialize socket if not already done */
    if (!sock_rtnl) {
        printf("DEBUG: Initializing netlink socket for bridge operations\n");
        sock_rtnl = create_socket(NETLINK_ROUTE, 0);
        if (!sock_rtnl) {
            fprintf(stderr, "Failed to create netlink socket\n");
            return -1;
        }
    }
    
    if (matches(*argv, "add") == 0)
		return bridge_modify(RTM_NEWLINK,
				     NLM_F_CREATE|NLM_F_EXCL,
				     argc-1, argv+1);
    return 0;
}