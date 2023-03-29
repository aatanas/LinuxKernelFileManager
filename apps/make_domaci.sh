#!/bin/bash
if gcc -m32 -o domaci.bin main.c scan.c -nostdlib -nostdinc -e main -Ttext=100 -static -fno-builtin ../../lib/lib.a -I ../../include; then
cd ../..
mkdir tmp_hd
sudo mount -o loop,offset=10240 hd_oldlinux.img tmp_hd
sudo cp apps/domaci/domaci.bin tmp_hd/root
sudo cp apps/domaci/scancodes.tbl tmp_hd/root
sudo cp apps/domaci/test1.tst tmp_hd/root
sudo cp apps/domaci/ctrl.map tmp_hd/root
sleep .5
sudo umount tmp_hd
rmdir tmp_hd
else
echo 'GCC Compile failed!'
fi
