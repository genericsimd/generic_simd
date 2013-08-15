#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

#DIRS = traffic_gen bsproc msproc

PROGRAM	    = rru_data_sim

INCLUDE	    = -I./ -I./bsl

CFLAGS	    =-g -Wall -DLOG_LEVEL=LOG_LEVEL_DEBUG

LDFLAGS     =  ./lib/libwmrt_64b.a    \
               ./lib/libwminfo_64b.a    \
               ./lib/libwmfunc_64b.a    \
               ./lib/libadapters_64b.a

LDFLAGS     += -lpthread -lm -lc -lrt

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../make.inc
endif

