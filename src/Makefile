CUR_DIR = $(shell pwd)
PROGNAME = tinybr
INSTALL_DIR = $(CUR_DIR)
BUILD_DIR = build

SRCS := $(wildcard *.c)

OBJS := $(patsubst %.c, %.o, $(SRCS))

LDLIBS += -L$(STAGING_DIR)/usr/lib -lubus -lubox -lblobmsg_json -luci -lnl-tiny -lpthread
LDFLAGS +=

CFLAGS += -I. -D_GNU_SOURCE 

all: $(PROGNAME)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(PROGNAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJS) $(PROGNAME)

.PHONY: all clean