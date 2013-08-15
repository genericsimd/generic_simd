#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

LIBRARY                 = libhash.a

INSTALL_FILES           = libhash.a

INSTALL_DIR             = ../lib

INCLUDE                 = -I ../include -I ../include/hash
CFLAGS                  = -g -DLOG_LEVEL=LOG_LEVEL_WARNING -O3

LDFLAGS                 = -lpthread

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../../make.inc
endif


