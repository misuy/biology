#!/bin/bash

sleep 5

fio load1.fio

sleep 3

dd if=/dev/urandom of=/dev/bpsda oflag=direct bs=4K count=1000

sleep 10

dd if=/dev/urandom of=/dev/bpsda oflag=direct bs=64K count=777
