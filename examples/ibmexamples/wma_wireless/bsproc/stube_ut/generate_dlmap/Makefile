#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

PROGRAM	    = build_dlmap_data

INCLUDE	    = -I ../../include/mac
CFLAGS	    =-g  -DDEBUG_LEVEL=DEBUG_LEVEL_SYSTEM  

#LDFLAGS     =  ./lib/libphy.a ./lib/libmac.a ./lib/libdladapter.a

LDFLAGS     += ../../lib/libmac.a -lpthread -lm -lc

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../../../make.inc
endif
