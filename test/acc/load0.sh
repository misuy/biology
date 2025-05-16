#!/bin/bash

dd if=/dev/urandom of=/dev/bpsda oflag=direct bs=16K count=7777
