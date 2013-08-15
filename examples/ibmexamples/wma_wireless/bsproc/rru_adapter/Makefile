#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

LIBRARY			= librruadapter.a

INSTALL_FILES           = librruadapter.a

INSTALL_DIR             = ../lib

INCLUDE			= -I ../include -I ../include/rru_adapter
CFLAGS			= -g -DLOG_LEVEL=LOG_LEVEL_WARN -O3

LDFLAGS			=

ENABLE_AGENT_CONNECT       = yes
ENABLE_RRH_CONNECT		= yes
ENABLE_BS_CONNECT		= yes
ENABLE_MS_CONNECT		= no
ENABLE_MONITOR_CONNECT	= yes
ENABLE_RRH_CONNECT_RAW_SOCKET    = yes

ifeq ($(ENABLE_AGENT_CONNECT), yes)
CFLAGS      += -DTRANS_AGENT_COMPILE
endif

ifeq ($(ENABLE_RRH_CONNECT), yes)
CFLAGS      += -DTRANS_RRH_COMPILE
endif

ifeq ($(ENABLE_BS_CONNECT), yes)
CFLAGS      += -DTRANS_BS_COMPILE
endif

ifeq ($(ENABLE_MS_CONNECT), yes)
CFLAGS      += -DTRANS_MS_COMPILE
endif

ifeq ($(ENABLE_MONITOR_CONNECT), yes)
CFLAGS      += -DTRANS_MONITOR_COMPILE
endif

ifeq ($(ENABLE_RRH_CONNECT_RAW_SOCKET), yes)
CFLAGS      += -DTRANS_RRH_RAW_SOCKET
endif

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../../make.inc
endif

