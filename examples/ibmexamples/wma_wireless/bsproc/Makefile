#
# Makefile for framework in Wireless Network Cloud (WNC) project
#

#DIRS = mac phy rru_adapter bs_adapter hash stube_ut

PROGRAM	    = bsproc

INCLUDE	    = -I ./include/bsl -I ./include -I ./include/mac -I ./include/bs_adapter \
		      -I ./include/phy -I ./include/hash -I ./include/stube_ut

CFLAGS	    = -g -DLOG_LEVEL=LOG_LEVEL_INFO -O3

LDFLAGS     =  ./lib/libmac.a ./lib/libphy.a ./lib/libstube_mac.a ./lib/libbsadapter.a \
               ./lib/openssl/libCMAInterface.a ./lib/openssl/libStoreInterface.a \
               ./lib/openssl/libcrypto.a ./lib/openssl/libssl.a

#for test only, temporary
ENABLE_IPP              = no
ENABLE_SSE              = no
ENABLE_SIM_RRH		= no
ENABLE_IP_LAYER		= yes
ENABLE_OLD_RRH		= no
ENABLE_UART_TRIGGER	= no
ENABLE_FAKE_ULMAP	= no
ENABLE_RX_FRAME_POOL    = yes
ENABLE_DUMP_UTIL        = yes
ENABLE_FAKE_DL_TX       = no
ENABLE_NEW_TRANS	= yes
ENABLE_PHY_STUB		= no
ENABLE_PROF		= no
ENABLE_NEW_TRANS_GREN	= yes
ENABLE_VSX      = yes
ENABLE_SYS_POWER    = yes

ifeq ($(ENABLE_SSE), yes)
CFLAGS      += -DSSE2OPT -m64 -msse2 -msse3
endif

ifeq ($(ENABLE_SYS_POWER), yes)
CFLAGS      += -DSYS_POWER_COMPILE
#CFLAGS      += -DRRH_ENDIAN_SWITCH  #remove comment if RRU not support endian switch
endif

ifeq ($(ENABLE_VSX), yes)
CFLAGS         += -DVSXOPT -m64 -maltivec -mabi=altivec -mcpu=power7 -mtune=power7 
endif

ifeq ($(ENABLE_IPP), yes)
IPPROOT     = /opt/intel/ipp/6.1.1.042/em64t
INCLUDE	    += -I $(IPPROOT)/include
LDFLAGS	    += -L $(IPPROOT)/sharedlib -lippsem64t -lippcoreem64t
CFLAGS	    += -DIPP_OPT_TEMP730
endif

ifeq ($(ENABLE_SIM_RRH), yes)
CFLAGS      += -D_SIM_RRH_
endif

ifeq ($(ENABLE_IP_LAYER), yes)
CFLAGS      += -D_IP_ENABEL_
endif

ifeq ($(ENABLE_UART_TRIGGER), yes)
CFLAGS	    += -D_UART_ENABLE_
endif

ifeq ($(ENABLE_FAKE_ULMAP), yes)
CFLAGS      += -D_FAKE_ULMAP_ENABLE_
endif

ifeq ($(ENABLE_OLD_RRH), yes)
CFLAGS      += -D_OLD_RRH_
endif

ifeq ($(ENABLE_RX_FRAME_POOL), yes)
CFLAGS      += -D_RX_FRAME_POOL_ENABLE_ -DRX_FRM_POOL_SIZE=12
endif

ifeq ($(ENABLE_FAKE_DL_TX), yes)
CFLAGS      += -D_DL_TX_FAKE_
endif

ifeq ($(ENABLE_TX_RAW_DUMP), yes)
CFLAGS      += -D_TX_RAW_DUMP_
endif

ifeq ($(ENABLE_RX_RAW_DUMP), yes)
CFLAGS      += -D_RX_RAW_DUMP_
endif

ifeq ($(ENABLE_DUMP_UTIL), yes)
CFLAGS      += -D_DUMP_UTIL_ENABLE_ -D_DUMP_BIN_RAW_DATA_
endif

ifeq ($(ENABLE_NEW_TRANS), yes)
CFLAGS      += -D_NEW_TRANS_ENABLE_
INCLUDE	    += -I ./include/trans
LDFLAGS	    += ./lib/libtrans.a
else
INCLUDE     += -I ./include/rru_adapter
LDFLAGS     += ./lib/librruadapter.a
endif

ifeq ($(ENABLE_PHY_STUB), yes)
CFLAGS      += -D_PHY_STUB_ENABLE_
endif

ifeq ($(ENABLE_NEW_TRANS_GREN), yes)
CFLAGS      += -D_NEW_TRANS_GREN_ENABLE_
endif

ifeq ($(ENABLE_PROF), yes)
CFLAGS      += -DENABLE_PROF
CFLAGS      += -DPROF_TIMER_ENABLE
#CFLAGS      += -DPROF_COUNTER_ENABLE
endif

LDFLAGS     +=  ./lib/bsl/libwmrt_64b.a    \
       #         ./lib/bsl/libwminfo_64b.a    \
                ./lib/bsl/libwmfunc_64b.a

LDFLAGS     += ./lib/libhash.a ./lib/json/libjansson.a \
               -lpthread -lm -lc -lrt -lstdc++ -ldl


ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../make.inc
endif

