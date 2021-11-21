#!/usr/bin/env bash

set -eux

ci/mingw-install.sh

ci/out-of-source-configure.sh
