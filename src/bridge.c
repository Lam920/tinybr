#include "netlink.h"


static struct nl_msg *__system_ifinfo_msg(int af, int index, const char *ifname, const char *master_ifname, uint16_t type, uint16_t flags, struct ifinfomsg *ifi, int vlan_filtering)
{
	struct nl_msg *msg;
	struct nlattr *br_linkinfo, *br_data;

    ifi->ifi_family = af;
    ifi->ifi_index = index;

	msg = nlmsg_alloc_simple(type, flags | NLM_F_REQUEST);
	if (!msg)
		return NULL;

	nlmsg_append(msg, ifi, sizeof(struct ifinfomsg), 0);
	if (ifname)
		nla_put_string(msg, IFLA_IFNAME, ifname);

    if (ifi->ifi_flags & IFF_UP) {
        ifi->ifi_index = if_nametoindex(ifname);
    }
    if (ifi->ifi_index == 0 || (vlan_filtering != -1)) {
        /* Add link is "bridge" */
        if (master_ifname) {
            /* IFLA_MASTER is not a nested attribute. Put it directly as u32. */
            if (nla_put_u32(msg, IFLA_MASTER, if_nametoindex(master_ifname)) < 0)
                goto nla_put_failure;
        }
        else {
            if (!(br_linkinfo = nla_nest_start(msg, IFLA_LINKINFO)))
                goto nla_put_failure;

            nla_put_string(msg, IFLA_INFO_KIND, "bridge");

            /* Currently no data need to added, bypass , just dummy code */
            if (!(br_data = nla_nest_start(msg, IFLA_INFO_DATA)))
                goto nla_put_failure;
            nla_put_u8(msg, IFLA_BR_VLAN_FILTERING, vlan_filtering == 1 ? 1 : 0);
            nla_nest_end(msg, br_data);
            nla_nest_end(msg, br_linkinfo);
        }
    }


	return msg;

nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

static struct nl_msg *system_ifinfo_msg(const char *ifname, const char *master_ifname, uint16_t type, uint16_t flags, struct ifinfomsg *ifi, int vlan_filtering)
{
	return __system_ifinfo_msg(AF_UNSPEC, 0, ifname, master_ifname, type, flags, ifi, vlan_filtering);
}

/*
When we created a bridge interface, the netlink message structure looks like this:
+----------------------------------+
| struct nlmsghdr                  |
|   nlmsg_len = total_length       |
|   nlmsg_type = RTM_NEWLINK (16)  |
|   nlmsg_flags = NLM_F_REQUEST |  |
|                 NLM_F_CREATE |   |
|                 NLM_F_EXCL       |
|   nlmsg_seq = sequence_number    |
|   nlmsg_pid = process_id         |
+----------------------------------+
| struct ifinfomsg                 |
|   ifi_family = AF_UNSPEC         |
|   ifi_type = 0                   |
|   ifi_index = 0                  |
|   ifi_flags = 0                  |
|   ifi_change = 0                 |
+----------------------------------+
| IFLA_IFNAME (rtattr)             |
|   rta_len = 12                   |
|   rta_type = IFLA_IFNAME (3)     |
|   data = "br-lan\0"              |
+----------------------------------+
| IFLA_LINKINFO (rtattr - nested)  |
|   rta_len = 24                   |
|   rta_type = IFLA_LINKINFO (18)  |
|   +----------------------------+ |
|   | IFLA_INFO_KIND (rtattr)    | |
|   |   rta_len = 12             | |
|   |   rta_type = 1             | |
|   |   data = "bridge\0"        | |
|   +----------------------------+ |
+----------------------------------+
This message will request the kernel to create a new link (bridge) with the specified name.
The kernel will respond with an ACK message if the bridge is created successfully.
If the bridge already exists, it will return an error.
- Only requests are handled by the kernel 
- For link create:
    + ifi_family = AF_UNSPEC
- For bridge ops:
    + ifi_family = AF_BRIDGE
- The ifi_index is 0 when creating a new bridge
- The ifi_flags and ifi_change are set to 0
- The IFLA_IFNAME attribute specifies the name of the bridge to be created
- The IFLA_LINKINFO attribute contains nested attributes that specify the type of link (bridge)
  and any additional data (none in this case)

For up bridge "br-lan", the message looks like this:
+----------------------------------+
| struct nlmsghdr (16 bytes)       |
|   nlmsg_len = 32                 |
|   nlmsg_type = RTM_NEWLINK (16)  |
|   nlmsg_flags = NLM_F_REQUEST(1) |
|   nlmsg_seq = <seq>              |
|   nlmsg_pid = <pid>              |
+----------------------------------+
| struct ifinfomsg (16 bytes)      |
|   ifi_family = AF_UNSPEC (0)     |
|   ifi_type = 0                   |
|   ifi_index = 3 (br-lan)         |
|   ifi_flags = IFF_UP (0x1)       |
|   ifi_change = IFF_UP (0x1)      |
+----------------------------------+
Total: 32 bytes (no attributes)



For add new port member to bridge "br-lan", the message looks like this:
+----------------------------------+
| struct nlmsghdr (16 bytes)       |
|   nlmsg_len = 44                 |
|   nlmsg_type = RTM_NEWLINK (16)  |
|   nlmsg_flags = NLM_F_REQUEST(1) |
|   nlmsg_seq = <seq>              |
|   nlmsg_pid = <pid>              |
+----------------------------------+
| struct ifinfomsg (16 bytes)      |
|   ifi_family = AF_UNSPEC (0)     |
|   ifi_type = 0                   |
|   ifi_index = 5 (eth1)           |
|   ifi_flags = 0                  |
|   ifi_change = 0                 |
+----------------------------------+
| IFLA_MASTER attribute (8 bytes)  |
|   rta_len = 8                    |
|   rta_type = IFLA_MASTER (10)    |
|   rta_data = 3 (br-wan's index)  |
+----------------------------------+
| Padding (4 bytes)                |
+----------------------------------+
Total: 44 bytes 
*/
static int bridge_modify(int cmd, unsigned int flags, int argc, char **argv)
{
    char *dev_name = NULL;
    char *master_dev_name = NULL;
    char *type = NULL;
    struct nl_msg *msg;
    int ret;
    int vlan_filtering = -1;

    struct ifinfomsg ifi = {
        .ifi_family = 0,
        .ifi_index = 0,
        .ifi_change = 0,
        .ifi_flags = 0
	};

    // ip link set dev eth1 master br-wan

    while (argc > 0) {
        if (matches(*argv, "dev") == 0) {
            NEXT_ARG();
            dev_name = *argv;
            printf("DEBUG: Device name is %s\n", dev_name);
        }
        else if (matches(*argv, "master") == 0) {
            NEXT_ARG();
            master_dev_name = *argv;
            printf("DEBUG: Master device name is %s\n", master_dev_name);
        }
        else if (matches(*argv, "up") == 0) {
            ifi.ifi_flags |= IFF_UP;
            ifi.ifi_change |= IFF_UP;
        }
        else if (matches(*argv, "down") == 0) {
            ifi.ifi_flags &= ~IFF_UP;
            ifi.ifi_change |= IFF_UP;
        }
        else if (matches(*argv, "type") == 0) {
            NEXT_ARG();
            type = *argv;
            printf("DEBUG: Type is %s\n", type);
        }
        else if (matches(*argv, "vlan_filtering") == 0) {
            NEXT_ARG();
            vlan_filtering = atoi(*argv);
            printf("DEBUG: vlan_filtering is %d\n", vlan_filtering);
        }
        else {
            fprintf(stderr, "Unknown argument: %s\n", *argv);
            return -EINVAL;
        }
        argc--; argv++;
    }

    msg = system_ifinfo_msg(dev_name, master_dev_name, cmd, flags, &ifi, vlan_filtering);
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
    
    printf("Device %s created successfully!\n", dev_name);
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

    /* ip link set dev eth1 master br-wan */
    
    if (matches(*argv, "add") == 0)
		return bridge_modify(RTM_NEWLINK,
				     NLM_F_CREATE|NLM_F_EXCL,
				     argc-1, argv+1);
    else if (matches(*argv, "set") == 0)
        return bridge_modify(RTM_NEWLINK,
				     0, argc-1, argv+1);
    else if (matches(*argv, "vlan") == 0)
        return bridge_vlan(argc-1, argv+1);
    else {
        fprintf(stderr, "Unknown bridge command: %s\n", *argv);
        return -EINVAL;
    }
    return 0;
}