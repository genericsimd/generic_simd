#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

LIBRARY			= libbsadapter.a

INSTALL_FILES   	= libbsadapter.a

INSTALL_DIR		= ../lib

INCLUDE			= -I ../include -I ../include/bs_adapter -I ../include/mac -I ../include/phy

CFLAGS			= -g -DLOG_LEVEL=LOG_LEVEL_DEBUG -O3

LDFLAGS			=  -lpthread

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../../make.inc
endif

