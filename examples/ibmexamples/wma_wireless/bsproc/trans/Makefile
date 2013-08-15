#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

LIBRARY			= libtrans.a

INSTALL_FILES           = libtrans.a

INSTALL_DIR             = ../lib

INCLUDE			= -I ../include -I ../include/trans
CFLAGS			= -g -DLOG_LEVEL=LOG_LEVEL_WARN -O3 -DTRANS_MONITOR_COMPILE

LDFLAGS			=

ENABLE_RRH_CONNECT              = yes
ENABLE_AGENT_CONNECT       	= no
ENABLE_BS_CONNECT               = yes
ENABLE_MS_CONNECT               = no
ENABLE_RRH_CONNECT_RAW_SOCKET   = no
ENABLE_MONITOR_TEST             = no
ENABLE_RRH_NEW_CONNECT          = yes
ENABLE_UI_CONNECT               = no
ENABLE_SYS_POWER                = yes

ifeq ($(ENABLE_RRH_CONNECT), yes)
CFLAGS      += -DTRANS_RRH_COMPILE 
endif

ifeq ($(ENABLE_AGENT_CONNECT), yes)
CFLAGS      += -DTRANS_AGENT_COMPILE
endif

ifeq ($(ENABLE_BS_CONNECT), yes)
CFLAGS      += -DTRANS_BS_COMPILE
endif

ifeq ($(ENABLE_MS_CONNECT), yes)
CFLAGS      += -DTRANS_MS_COMPILE
endif

ifeq ($(ENABLE_RRH_CONNECT_RAW_SOCKET), yes)
CFLAGS      += -DTRANS_RRH_RAW_SOCKET
endif

ifeq ($(ENABLE_MONITOR_TEST), yes)
CFLAGS      += -DTRANS_MONITOR_TEST_COMPILE
endif

ifeq ($(ENABLE_RRH_NEW_CONNECT), yes)
CFLAGS      += -DTRANS_RRH_NEW_CONNECT
endif

ifeq ($(ENABLE_UI_CONNECT), yes)
CFLAGS      += -DTRANS_UI_COMPILE
endif

ifeq ($(ENABLE_SYS_POWER), yes)
CFLAGS      += -DTRANS_SYS_POWER_COMPILE
endif

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../../make.inc
endif

