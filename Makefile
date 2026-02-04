#
# Copyright (C) 2025 Viettel Group
# 

include $(TOPDIR)/rules.mk

PKG_NAME:=tinybr
PKG_VERSION:=1.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=Lam The Bui <lambt9@mail.viettel.vn>

PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk

define Package/tinybr
  SECTION:=Viettel Properties
  CATEGORY:=Viettel
  TITLE:=Lightweight Bridge Utility
  DEPENDS:=+libubus +libubox +libblobmsg-json +libuci +libnl-tiny +ubusd
endef

define Package/tinybr/description
	Tinybr is a lightweight bridge utility for managing bridge interfaces
  and configurations in embedded systems, designed to be efficient and
  easy to use.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(STAGING_DIR)/usr/include \
	-flto

TARGET_LDFLAGS += -flto -fuse-linker-plugin

CMAKE_OPTIONS += \
	-DLIBNL_LIBS=-lnl-tiny \
	-DDEBUG=1


define Package/tinybr/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinybr $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,tinybr))