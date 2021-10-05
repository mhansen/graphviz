#!/usr/bin/env bash

set -eux

pacman -S --noconfirm --needed \
    bison \
    ${MINGW_PACKAGE_PREFIX}-cairo \
    ${MINGW_PACKAGE_PREFIX}-cmake \
    ${MINGW_PACKAGE_PREFIX}-diffutils \
    ${MINGW_PACKAGE_PREFIX}-expat \
    flex \
    ${MINGW_PACKAGE_PREFIX}-gcc \
    ${MINGW_PACKAGE_PREFIX}-libgd \
    ${MINGW_PACKAGE_PREFIX}-libtool \
    ${MINGW_PACKAGE_PREFIX}-pango \
    ${MINGW_PACKAGE_PREFIX}-pkgconf \
    ${MINGW_PACKAGE_PREFIX}-python \
    ${MINGW_PACKAGE_PREFIX}-ninja \
    ${MINGW_PACKAGE_PREFIX}-nsis \
    ${MINGW_PACKAGE_PREFIX}-tcl \
    ${MINGW_PACKAGE_PREFIX}-zlib \

export PATH=$PATH:/c/Git/cmd

# use the libs installed with pacman instead of those in
# https://gitlab.com/graphviz/graphviz-windows-dependencies
export CMAKE_OPTIONS=-Duse_win_pre_inst_libs=OFF

ci/build.sh
