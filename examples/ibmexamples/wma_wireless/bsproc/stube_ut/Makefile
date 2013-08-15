#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

LIBRARY			= libstube_mac.a

INSTALL_FILES   	= libstube_mac.a

INSTALL_DIR		= ../lib

INCLUDE			= -I ../include -I ../include/bs_adapter -I ../include/mac -I ../include/phy \
			  -I ../include/stube_ut

CFLAGS			= -g -DLOG_LEVEL=LOG_LEVEL_DEBUG -O3 -DSTUBE_ULMAP_DURATION=100 -DSTUBE_ULMAP_UIUC=1

LDFLAGS			=  -lpthread

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../../make.inc
endif

