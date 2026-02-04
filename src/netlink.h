#ifndef NETLINK_H
#define NETLINK_H

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include <net/if.h>
#include <net/if_arp.h>

#include <limits.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ether.h>

#include <linux/rtnetlink.h>
#include <linux/neighbour.h>
#include <linux/sockios.h>
#include <linux/ip.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/if_vlan.h>
#include <linux/if_bridge.h>
#include <linux/if_tunnel.h>
#include <linux/ip6_tunnel.h>
#include <linux/ethtool.h>
#include <linux/fib_rules.h>
#include <linux/veth.h>
#include <linux/version.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  
#include <fcntl.h>
#include <glob.h>
#include <time.h>
#include <errno.h>


#include <netlink/msg.h>
#include <netlink/socket.h>
#include <netlink/attr.h>

#include <libubox/uloop.h>

#include "event_handler.h"
#include "utils.h"

extern struct nl_sock *listen_sock_rtnl;
extern struct nl_sock *sock_rtnl;
extern struct nl_cb *listen_cb;
struct nl_sock *create_socket(int protocol, int groups);
int init_netlink_sockets();

#endif // NETLINK_H