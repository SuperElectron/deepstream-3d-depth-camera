#!/bin/bash

# Example usage: ./install.sh

export WORKDIR="/start/scripts"
export NAME="[install.sh]: "
echo "${NAME} STARTING "

# Bash failure reporting for the script
set -eE -o functrace
failure() {
  local lineno=$1
  local msg=$2
  echo "${NAME} Failed at $lineno: $msg"
}
trap '${NAME} failure ${LINENO} "$BASH_COMMAND"' ERR

echo "${NAME} pre-flight check for exiting folder $WORKDIR from docker COPY "
export INSTALL_DIR_EXISTS="$(test -d $WORKDIR && echo 'yes' || echo 'no')"
if [[ "$INSTALL_DIR_EXISTS" == "no" ]]; then
  echo "--(fail)-- install directory exists: ${WORKDIR}"
  exit 1;
else
  echo "--(pass)-- install directory exists: ${WORKDIR}"
fi

echo "${NAME} install vcpkg "
VCPKG_VER=2022.10.19
cd /tmp && wget "https://github.com/microsoft/vcpkg/archive/refs/tags/${VCPKG_VER}.zip" && \
  cd /tmp && unzip "${VCPKG_VER}.zip" && \
  cd /tmp && rm -rf "${VCPKG_VER}.zip" && \
  mv /tmp/vcpkg-"${VCPKG_VER}" /start/vcpkg

export VCPKG_FORCE_SYSTEM_BINARIES=1
cd "/start/vcpkg"; ./bootstrap-vcpkg.sh;
cd "/start/vcpkg"; ./vcpkg integrate install;
cd "/start/vcpkg"; ./vcpkg install gtest;

echo "${NAME} FINISHED "
