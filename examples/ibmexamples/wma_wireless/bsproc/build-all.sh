#!/bin/sh

rm -f ./lib/*.a

. ./ippvarsem64t.sh

for d in bs_adapter hash phy mac rru_adapter stube_ut trans
do
	make -C $d clean
	make -C $d 
done

make clean
make

