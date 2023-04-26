#!/bin/bash

### Install Nvidia runtime on local computer (not inside docker)
# Example usage: ./install_nvidia-runtime.sh

export NAME="[install_nvidia-runtime.sh] "

set -eE -o functrace
failure() {
  local lineno=$1
  local msg=$2
  echo "${NAME} Failed at $lineno: $msg"
}
trap '${NAME} failure ${LINENO} "$BASH_COMMAND"' ERR

## Script start
echo "${NAME} STARTING "

wget https://nvidia.github.io/nvidia-docker/gpgkey --no-check-certificate
sudo apt-key add gpgkey && rm gpgkey;

distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
curl -s -L "https://nvidia.github.io/nvidia-docker/${distribution}/nvidia-docker.list" | sudo tee /etc/apt/sources.list.d/nvidia-docker.list
sudo apt-get update -y;

echo "${NAME} Install base apt package "
sudo apt-get install -y -qq \
  nvidia-container-runtime

echo "${NAME} Install docker engine deps "
sudo mkdir -p /etc/systemd/system/docker.service.d
sudo tee /etc/systemd/system/docker.service.d/override.conf <<EOF
[Service]
ExecStart=
ExecStart=/usr/bin/dockerd --host=fd:// --add-runtime=nvidia=/usr/bin/nvidia-container-runtime
EOF
sudo systemctl daemon-reload
sudo systemctl restart docker

echo "${NAME} Install docker daemon deps "
sudo tee /etc/docker/daemon.json <<EOF
{
    "runtimes": {
        "nvidia": {
            "path": "/usr/bin/nvidia-container-runtime",
            "runtimeArgs": []
        }
    }
}
EOF
sudo pkill -SIGHUP dockerd
sudo systemctl restart docker

echo "--(test)-- running nvidia's nvidia/cuda container and checking that 'nvidia-smi' works on local system"
echo "--(info)-- enter this in your terminal: lsb_release -r"
echo "--(info)-- replace ubuntu20.04 below with your version"
echo "docker run --rm --gpus 'all,\"capabilities=compute,utility\"' --runtime=nvidia nvidia/cuda:11.6.2-base-ubuntu20.04 nvidia-smi"

echo "${NAME} FINISHED "