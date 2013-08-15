#!/bin/sh

cd ./lib/
rm -rf libbsadapter.a  libhash.a  librruadapter.a  libstube_mac.a  libtrans.a
cd ../

. ./ippvarsem64t.sh

for d in bs_adapter hash rru_adapter stube_ut trans
do
	make -C $d clean
	make -C $d 
done

make clean;make

