#!/usr/bin/env bash

set -e
set -o pipefail
set -u
set -x

brew update
brew install pango || brew upgrade pango
brew install qt5 || brew upgrade qt5
export PATH="/usr/local/opt/qt@5/bin:$PATH"
export LDFLAGS="-L/usr/local/opt/qt@5/lib"
export CPPFLAGS="-I/usr/local/opt/qt@5/include"
export PKG_CONFIG_PATH="/usr/local/opt/qt@5/lib/pkgconfig"
brew install libxaw || brew upgrade libxaw

# install MacPorts for libANN
curl --retry 3 --location --no-progress-meter -O \
  https://github.com/macports/macports-base/releases/download/v2.7.2/MacPorts-2.7.2-12-Monterey.pkg
sudo installer -package MacPorts-2.7.2-12-Monterey.pkg -target /
export PATH=/opt/local/bin:${PATH}

# lib/mingle dependency
sudo port install libANN
