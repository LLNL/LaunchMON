#!/bin/sh

set -ex

echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
cd test/src
export PATH=./:$PATH
cat test.launch_1
./test.launch_1
sleep 60
cat test.attach_1
./test.attach_1
sleep 60
echo 'done'
