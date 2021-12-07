#!/usr/bin/env bash

# this script does something close to the work flow end users may follow when
# building Graphviz

set -x
set -e
set -o pipefail

# output some info for debugging
uname -rms
cat /etc/os-release

GV_VERSION=$(cat GRAPHVIZ_VERSION)

# go to the configured directory
cd build

# build Graphviz
make

# install Graphviz
make install
