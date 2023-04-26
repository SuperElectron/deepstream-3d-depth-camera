#################################################
# Makefile to build x86 docker images
#################################################
# Top level vars
PROJECT_DIR?=$(shell pwd)
DOCKER_DIR:=$(PROJECT_DIR)/.docker
include .env

# sets env variable used for C++ code to generate_timestamp()
TZ=$(shell cat /etc/timezone)

#################################################
# HELPERS
help:
	@echo "++   VARIABLES"
	@echo "++	PROJECT_NAME: 	$(PROJECT_NAME)"
	@echo "++	IMAGE_PREFIX: 	$(IMAGE_PREFIX)"
	@echo "++	TZ:           	$(TZ)"
	@echo "++   PROJECT COMMANDS"
	@echo "++ 	make build"
	@echo "++ 	make start"
	@echo "++ 	make run"
	@echo "++ 	make build_app"
	@echo "++ 	make stop"
	@echo "++ 	make clean"

#################################################
# GENERAL BUILD COMMANDS
build: network build_nvidia
start: start_nvidia build_app
stop: stop_nvidia
run: run_app

network:
	docker network create \
      --driver=bridge \
      --subnet="$(SUBNET_BASE).0/24" \
      --gateway="$(SUBNET_BASE).1" \
      $(PROJECT_NAME)_server || echo "Network alive"

#################################################
# DOCKER PROJECT COMMANDS
# BUILD COMMANDS
build_nvidia:
	docker buildx build -t $(REGISTRY_URL)/$(IMAGE_PREFIX)-nvidia $(DOCKER_DIR)/nvidia/ -f $(DOCKER_DIR)/nvidia/Dockerfile

# RUN COMMANDS
start_nvidia: network
	docker run -d --name nvidia \
	    --privileged \
	    --net=$(PROJECT_NAME)_server \
	    --ip=$(SUBNET_BASE).2 \
	    --user=0 \
	    --security-opt seccomp=unconfined  \
	    --runtime nvidia \
	    --gpus all \
	    --env="XAUTHORITY=$(XAUTH)" \
	    -e DISPLAY=$(DISPLAY) \
	    -e TZ=$(TZ) \
	    --device /dev/dri \
	    --device /dev/video0 \
	    -v "/etc/timezone:/etc/timezone:ro" \
	    -v "/etc/localtime:/etc/localtime:ro" \
	    -v /tmp/.X11-unix/:/tmp/.X11-unix \
        -v ~/.Xauthority:/root/.Xauthority \
	    -v "/dev:/dev" \
	    -v $(PROJECT_DIR)/src:/src \
	    -w /src \
		$(REGISTRY_URL)/$(IMAGE_PREFIX)-nvidia:latest bash -c "sleep infinity"
build_app:
	xhost +
	docker exec nvidia bash -c "cmake -B build -S . "
	docker exec nvidia bash -c "cmake --build build -j"

# RUN COMMANDS
run_app:
	docker exec nvidia bash -c "./build/depthCam -c ./configs/$(CONFIG_FILE) "

# STOP COMMANDS
stop_nvidia:
	docker kill nvidia && docker container rm nvidia

# CLEAN COMMANDS
clean:
	docker kill $(shell docker ps -q) || echo "no containers are running ... ready to go!"
	docker container prune -f;
	docker network prune -f;
	docker volume prune -f;
