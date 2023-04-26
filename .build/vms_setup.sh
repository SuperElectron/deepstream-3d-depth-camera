#!/bin/bash

#  usage: ./vms_setup.sh

#### EXAMPLES
## Install deps for first time vms setup on a device (before we have the code there!)
# $ ./vms_setup.sh

export NAME="[vms_setup.sh]: "
echo "${NAME} STARTING "
export CWD=$(pwd)

# Bash failure reporting for the script
set -eE -o functrace
failure() {
  local lineno=$1
  local msg=$2
  echo "${NAME} Failed at $lineno: $msg"
}
trap '${NAME} failure ${LINENO} "$BASH_COMMAND"' ERR

echo "${NAME} Installing apt packages "
sudo apt-get update -y -qq &&
  sudo apt-get install $(cat project.pkglist)

echo "${NAME} starting docker installation steps, visit link for debugging and more info: https://get.docker.com/"
cd ~; curl -fsSL https://get.docker.com -o get-docker.sh
cd ~; sh get-docker.sh
cd ~; rm -rf get-docker.sh

echo "${NAME} docker post-installation: adding user to the docker group"
sudo usermod -aG docker "${USER}"
newgrp docker
sudo mkdir /home/"$USER"/.docker
sudo chown "$USER":"$USER" /home/"$USER"/.docker -R
sudo chmod g+rwx "$HOME/.docker" -R
docker run hello-world

echo "${NAME} Installing nvidia container runtime"
cd $CWD && ./install_nvidia-runtime.sh

#echo "${NAME} Install az tools"
#python3 -m pip install azure-cli

echo "${NAME} FINISHED"