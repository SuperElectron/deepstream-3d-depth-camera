# syntax = docker/dockerfile:1.2
FROM nvcr.io/nvidia/deepstream:6.2-devel
LABEL maintainer="RAD <matt.m@radskunkworks.com>"

# Environment variables
ENV NVIDIA_DRIVER_CAPABILITIES all
ENV NVIDIA_VISIBLE_DEVICES all
ENV DEBIAN_FRONTEND noninteractive

RUN rm -f /etc/apt/apt.conf.d/docker-clean

COPY apt /start/apt
RUN --mount=type=cache,target=/var/cache/apt apt-get update -yqq --fix-missing && \
    apt-get install -yqq --no-install-recommends \
    $(cat /start/apt/general.pkglist) \
    $(cat /start/apt/gstreamer.pkglist) \
    $(cat /start/apt/project.pkglist) && \
    rm -rf /var/lib/apt/lists/*

RUN --mount=type=cache,target=/var/cache/apt \
    add-apt-repository "deb https://librealsense.intel.com/Debian/apt-repo bionic main" -u
RUN --mount=type=cache,target=/var/cache/apt \
    apt-get install -yqq librealsense2-utils librealsense2-dev

# Base installations for the container
COPY scripts /start/scripts
RUN chmod 777 /start -R
RUN cd /start/scripts && ./install_cmake.sh
RUN cd /start/scripts && ./install.sh

# clean up downloaded packages
RUN rm -rf /var/lib/apt/lists/*
RUN rm -rf ~/.cache/gstreamer-1.0/registry.x86_64.bin
# Command executed for container spin up ( docker-compose, docker run)
HEALTHCHECK CMD ping -c3 -i 10 127.0.0.1
WORKDIR /src
CMD ["bash"]