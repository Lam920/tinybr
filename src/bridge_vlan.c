#include "netlink.h"

static void do_vlan_on_bridge(int argc, char **argv, int do_add) {
    
    struct nl_msg *nlm;
    struct nlattr *afspec;
    char *dev_name = NULL;
    int vid = 0;
    unsigned short flags = 0;
    int ret;

    struct ifinfomsg ifi = { .ifi_family = PF_BRIDGE, };
    struct bridge_vlan_info vinfo = { .vid = vid, };
    
    while (argc > 0) {
        if (matches(*argv, "dev") == 0) {
            NEXT_ARG();
            dev_name = *argv;
            printf("DEBUG: do_vlan on dev name: %s\n", dev_name);
            ifi.ifi_index = if_nametoindex(dev_name);
            if (!ifi.ifi_index) {
                fprintf(stderr, "Invalid bridge name to get index: %s", dev_name);
            }
        } else if (matches(*argv, "vid") == 0) {
            NEXT_ARG();
            vinfo.vid = atoi(*argv);
            if (vinfo.vid < 0) {
                fprintf(stderr, "Invalid VID: %d", vinfo.vid);
                return -EINVAL;
            }
        } else if (matches(*argv, "self") == 0) {
            flags |= BRIDGE_FLAGS_SELF;
        } else if (matches(*argv, "master") == 0) {
            flags |= BRIDGE_FLAGS_MASTER;
        } else if (strcmp(*argv, "pvid") == 0) {
			vinfo.flags |= BRIDGE_VLAN_INFO_PVID;
		} else if (strcmp(*argv, "untagged") == 0) {
			vinfo.flags |= BRIDGE_VLAN_INFO_UNTAGGED;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", *argv);
            return -EINVAL;
        }
        argc--; argv++;
    }

    nlm = nlmsg_alloc_simple(do_add ? RTM_SETLINK : RTM_DELLINK, NLM_F_REQUEST);
	if (!nlm)
		return -1;

	nlmsg_append(nlm, &ifi, sizeof(ifi), 0);
    afspec = nla_nest_start(nlm, IFLA_AF_SPEC);
	if (!afspec) {
		ret = -ENOMEM;
		goto failure;
	}
    
	if (flags)
		nla_put_u16(nlm, IFLA_BRIDGE_FLAGS, flags);

    nla_put(nlm, IFLA_BRIDGE_VLAN_INFO, sizeof(vinfo), &vinfo);
	nla_nest_end(nlm, afspec);

    printf("DEBUG: Sending netlink message for VLAN ops...\n");
    ret = nl_send_auto_complete(sock_rtnl, nlm);
    nlmsg_free(nlm);
    if (ret < 0) {
        fprintf(stderr, "Failed to send netlink message: %s\n", nl_geterror(ret));
        return ret;
    }
    printf("DEBUG: NETLINK bridge Message sent, ret=%d\n", ret);

    printf("DEBUG: Waiting for ACK of bridge_vlan...\n");
    ret = nl_wait_for_ack(sock_rtnl);
    if (ret < 0) {
        fprintf(stderr, "Failed to receive ACK: %s\n", nl_geterror(ret));
        return ret;
    }
    
    printf("Device %s created successfully!\n", dev_name);
    return 0;
failure:
	nlmsg_free(nlm);
	return ret;
}

int bridge_vlan(int argc, char **argv) {
    int do_add = -1;
    if (argc < 1) {
        fprintf(stderr, "Usage: tinybr link vlan <add|del|set> dev <device> vid <vlan_id> [options]\n");
        return -EINVAL;
    }
    if (matches(*argv, "add") == 0) {
        // Handle adding VLAN to bridge
        printf("Adding VLAN to bridge\n");
        do_add = 1;
    }
    else if (matches(*argv, "del") == 0) {
        // Handle deleting VLAN from bridge
        printf("Deleting VLAN from bridge\n");
        do_add = 0;
    }
    else {
        fprintf(stderr, "Unknown vlan command: %s\n", *argv);
        return -EINVAL;
    } 

    if (do_add >= 0)
        do_vlan_on_bridge(argc - 1, argv + 1, do_add);
    
    return 0;
}
