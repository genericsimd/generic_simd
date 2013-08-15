#! /bin/sh

#57_75_ThinkCenter

export IPPROOT=/opt/intel/ipp/6.1.1.042/em64t
export INCLUDE=$IPPROOT/include:$INCLUDE
export LD_LIBRARY_PATH=$IPPROOT/sharedlib:$LD_LIBRARY_PATH
export LIB=$IPPROOT/lib:$LIB
export CPATH=$IPPROOT/include:$CPATH
export LIBRARY_PATH=$IPPROOT/lib:$LIBRARY_PATH
export NLSPATH=$IPPROOT/lib/locale/%l_%t/%N:$NLSPATH

