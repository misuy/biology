#!/bin/bash

rmmod biology_proxy ||:
rmmod biology_gen ||:

cd ../biology-proxy
make CC=gcc-12
insmod biology-proxy.ko

rm -rf /root/bp-test
mkdir /root/bp-test

echo -n "create /dev/sda" > /sys/module/biology_proxy/module/ctl
echo -n "enable /root/bp-test" > /sys/module/biology_proxy/bpsda/ctl
cd ../scripts
fio simple-test.fio
echo -n "disable" > /sys/module/biology_proxy/bpsda/ctl

rm -rf /root/bp-test-1
mkdir /root/bp-test-1

echo -n "enable /root/bp-test-1" > /sys/module/biology_proxy/bpsda/ctl

cd ../biology-gen
make CC=gcc-12
insmod biology-gen.ko

echo -n "start gen-bpsda /dev/bpsda /root/bp-test 8" > /sys/module/biology_gen/module/ctl
