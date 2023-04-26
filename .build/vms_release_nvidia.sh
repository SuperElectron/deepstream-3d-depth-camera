#!/bin/bash

#  usage: ./vms_release_nvidia.sh <branch> <stack: (x86, arm)> <inference: (yes, no)>

#### EXAMPLES
## Runs cloud stack `with` inference in cloud or x86 device architectures
# $ ./vms_release_nvidia.sh 'master' 'x86' 'yes'

## Runs cloud stack `without` inference in cloud
# $ ./vms_release_nvidia.sh 'master' 'x86'

## Runs inference on edge
# $ ./vms_release_nvidia.sh 'master' 'x86'

export WORKDIR="/home/$USER/raddev/linux-vms"
export NAME="[vms_release_nvidia.sh]: "
echo "${NAME} STARTING "

# Bash failure reporting for the script
set -eE -o functrace
failure() {
  local lineno=$1
  local msg=$2
  echo "${NAME} Failed at $lineno: $msg"
}
trap '${NAME} failure ${LINENO} "$BASH_COMMAND"' ERR

# defaults to 'git checkout main'
export BRANCH=${1:-'main'}
# defaults to '.docker/x86.Makefile'
export STACK=${2:-'x86'}
export INFERENCE=${3:-'no'}

## Get recent code
echo "${NAME} [git] Pull recent code "
cd "${WORKDIR}"
git fetch
git checkout "${BRANCH}"
git pull
export CURRENT_BRANCH="$(git branch --show-current)"

echo "${NAME} Current git branch for this build: ${CURRENT_BRANCH}"

## login to azure container registry
az acr login --name mlopsradcontainerregistry
## Stop docker, clean up ANYTHING on server, and re-run all
echo "${NAME} [docker] Stop all and clean EVERYTHING off the server"
make kill_stack
make clean_all


## download the config zip file and put it in a proper directory
# az storage blob download -c device --account-name perception --file "ant-media-server-enterprise-2.4.3-20220418_2241.zip" --name ant-media-server-enterprise-2.4.3-20220418_2241.zip
# sudo mkdir -p /device_media/media_server/
# sudo mv ant-media-server-enterprise-2.4.3-20220418_2241.zip /device_media/media_server/

## Spin up the media server on the x86 subnet
if [[ "$STACK" == "x86" ]]; then
  echo "${NAME} [docker] Rebuild & run latest images  (antmedia)  "
  echo "${NAME} [docker] Rebuild & run latest images kafka images  "
  # make build_stack
  make run_kafka run_kafkacat
  make build_kafka network
else
  echo "${NAME} -- [docker] skipped antmedia (running edge) "
fi
# wait until kafka broker is available
sleep 10
docker exec kafkacat kafkacat -b 172.23.0.7:9092 -L

## Spin up inference modules if required
if [[ "$INFERENCE" == "yes" ]]; then
  echo "${NAME} [docker] Rebuild & run latest images  (nvidia)  "
  make build_nvidia
  make run_nvidia
else
  echo "${NAME} -- [docker] skipped inference "
fi

## Run tests
if [[ "$STACK" == "x86" ]]; then
  echo "${NAME} -- Running tests for ${STACK} commit"
#  make kafka_cloud_supervisor_mock
  make test_build_nvidia
  make test_nvidia_end_to_end
else
  echo "${NAME} -- skipped tests for ${STACK} commit"
fi
