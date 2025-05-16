#!/bin/bash

iterations=10

echo load*.sh
rm -rf /root/bp-test /root/bp-test-gen /root/bp-test-blkreplay bpsda.replay.* bpsda.blktrace.* *.json log

for iter in $(seq 1 $iterations); do
    echo ====iter $iter start====
    echo iter $iter >> log
    for load in load*.sh; do
        echo ====load $load start====
        biologycli proxy create name bpsda device /dev/sda
        biologycli proxy enable name bpsda dump /root/bp-test
        blktrace -d /dev/bpsda &
        blktrace_pid=$!

        bash $load
        sleep 1

        biologycli proxy disable name bpsda
        kill $blktrace_pid

        btrecord bpsda
        biologycli proxy enable name bpsda dump /root/bp-test-blkreplay

        btreplay -W bpsda
        sleep 1

        biologycli proxy disable name bpsda

        biologycli proxy enable name bpsda dump /root/bp-test-gen

        biologycli gen start name gensda device /dev/bpsda dump /root/bp-test cpus 8
        sleep 1

        biologycli proxy disable name bpsda
        biologycli proxy destroy name bpsda

        /root/biology/uspace/util/to-json/to-json /root/bp-test > orig.json
        /root/biology/uspace/util/to-json/to-json /root/bp-test-gen > gen.json
        /root/biology/uspace/util/to-json/to-json /root/bp-test-blkreplay > blkreplay.json

        source /root/biology-venv/bin/activate
        python3 /root/biology/uspace/util/distance/distance.py orig.json gen.json >> log
        python3 /root/biology/uspace/util/distance/distance.py orig.json blkreplay.json >> log
        echo >> log

        rm -rf /root/bp-test /root/bp-test-gen /root/bp-test-blkreplay bpsda.replay.* bpsda.blktrace.* *.json
    done
done
