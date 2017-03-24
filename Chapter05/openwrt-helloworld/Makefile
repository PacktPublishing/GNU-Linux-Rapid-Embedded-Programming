include $(TOPDIR)/rules.mk

# Define package's name, version, release and the default package's
# build directory.
PKG_NAME:=helloworld
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

# Define package's section and category inside the OpenWRT system.
# These information are used to manage the package and to display
# it inside the comfiguration menu
define Package/$(PKG_NAME)
	SECTION:=apps
	CATEGORY:=Applications
	TITLE:=The Hello World program
	MAINTAINER:=Rodolfo Giometti <giometti@hce-engineering.com>
endef

# Define package's description (long version)
define Package/$(PKG_NAME)/description
 This package holds a program that display the "hello world" message
endef

# Set up the build directory in order to be use by the compilation
# stage.
# Our data are not downloaded from a remote site by we have them
# already into the "src" directory, so let's copy them accordingly
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

# Define the package's installation steps after the compilation
# stage has done
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin
endef

# The OpenWRT's main entry
$(eval $(call BuildPackage,$(PKG_NAME)))
