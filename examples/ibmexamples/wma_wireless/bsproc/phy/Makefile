#
## Makefile for framework in Wireless Network Cloud (WNC) project
#

LIBRARY		= libphy.a

INSTALL_FILES   = libphy.a

INSTALL_DIR     = ../lib

INCLUDE         = -I ../include/bsl -I ../include -I ../include/bs_adapter -I ../include/phy -I ../include/mac

CFLAGS          =-g -D_GNU_SOURCE -DLOG_LEVEL=LOG_LEVEL_WARN -O3
#-DDEBUG_LEVEL=4

LDFLAGS         = -lpthread -lm -lc 

ENABLE_IPP      = no
ENABLE_SSE      = no 
ENABLE_PHY_HOOK = yes
ENABLE_PHY_HOOK_DUMP = no
ENABLE_PHY_DUMP = yes
ENABLE_PHY_UT   = no
ENABLE_BER_TEST = no
ENABLE_MEAN_NOISE = no
ENABLE_RANGING_DUMP = no
ENABLE_AMC_INTERLEAVE = yes
ENABLE_SOFTBIT_TWO = yes
ENABLE_DUMP_UTIL        = yes 
ENABLE_PROF		= no
ENABLE_AMC              = yes
ENABLE_VSX      = yes
ENABLE_MULTIBAND = no
ENABLE_MBAND_SINGLE = no
ENABLE_SYS_POWER    = yes

ifeq ($(ENABLE_SSE), yes)
CFLAGS         += -DSSE2OPT -m64 -msse2 -msse3 -mssse3
endif

ifeq ($(ENABLE_VSX), yes)
CFLAGS         += -DVSXOPT -m64 -maltivec -mabi=altivec -mcpu=power7 -mtune=power7 
endif

ifeq ($(ENABLE_PHY_UT), yes)
CFLAGS         += -D__RANGING_UT__
endif

ifeq ($(ENABLE_PHY_HOOK), yes)
CFLAGS         += -D_BS_MONITOR_
endif

ifeq ($(ENABLE_BER_TEST), yes)
CFLAGS         += -D_BER_TEST_
endif

ifeq ($(ENABLE_PHY_HOOK_DUMP), yes)
CFLAGS         += -D_HOOK_BS_DUMP_
endif

ifeq ($(ENABLE_MEAN_NOISE), yes)
CFLAGS         += -D_MEAN_NOISE_EST_
endif

ifeq ($(ENABLE_RANGING_DUMP), yes)
CFLAGS         += -D_RANGING_DUMP_
endif

ifeq ($(ENABLE_AMC_INTERLEAVE), yes)
CFLAGS         += -D_AMC_INTERLEAVER_
endif

ifeq ($(ENABLE_SOFTBIT_TWO), yes)
CFLAGS         += -D_SOFTBIT_TWO_
else
CFLAGS         += -D_SOFTBIT_ONE_
endif

ifeq ($(ENABLE_DUMP_UTIL), yes)
CFLAGS      += -D_DUMP_UTIL_ENABLE_ 
CFLAGS      += -DDUMP_PHY_DL_PREAMPLE
CFLAGS      += -DDUMP_PHY_DL_TX
CFLAGS      += -DDUMP_PHY_UL_RX
CFLAGS      += -DDUMP_PHY_UL_FEC
CFLAGS      += -DDUMP_PHY_DL_FEC
CFLAGS      += -DDUMP_PHY_UL_OFDM_DEMOD
CFLAGS      += -DDUMP_PHY_DL_OFDM_MOD
CFLAGS      += -DDUMP_PHY_UL_ZONE_PERMUTATION
CFLAGS      += -DDUMP_PHY_DL_ZONE_PERMUTATION
CFLAGS      += -DDUMP_PHY_UL_RX_RANGING
endif

ifeq ($(ENABLE_PROF), yes)
CFLAGS      += -DENABLE_PROF
CFLAGS      += -DPROF_TIMER_ENABLE
#CFLAGS      += -DPROF_COUNTER_ENABLE
endif

ifeq ($(ENABLE_AMC), yes)
CFLAGS         += -D_AMC_
endif

ifeq ($(ENABLE_MULTIBAND), yes)
CFLAGS         += -D_MULTIBAND_
endif

ifeq ($(ENABLE_SYS_POWER), yes)
CFLAGS      += -DSYS_POWER_COMPILE
endif



#ENABLE_FRAME_NUM = yes

#ifeq ($(ENABLE_FRAME_NUM), yes)
#CFLAGS         += -D_REAL_FRAME_NUM_
#endif



ifeq ($(ENABLE_IPP), yes)
#IPPROOT         = /opt/intel/ipp/5.2/em64t
IPPROOT         = /opt/intel/ipp/6.1.2.051/em64t
#IPPROOT         = /opt/intel/ipp/6.1.1.042/em64t
INCLUDE         += -I $(IPPROOT)/include
LDFLAGS         += -L $(IPPROOT)/sharedlib -lippsem64t -lippcoreem64t
CFLAGS          += -DIPP_OPT_TEMP730
endif

ifeq ($(ENABLE_PHY_DUMP), yes)
CFLAGS          += -D_PHY_DUMP_
endif

ifdef SRC_ROOT
include $(SRC_ROOT)/make.inc
else
include ../../make.inc
endif

