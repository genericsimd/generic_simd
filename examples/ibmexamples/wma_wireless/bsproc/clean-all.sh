#!/bin/sh

rm -f ./lib/*.a

for d in bs_adapter hash phy mac rru_adapter stube_ut trans
do
	make -C $d clean
done

make clean

