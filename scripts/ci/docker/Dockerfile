FROM ubuntu:21.10

ENV TZ=Europe/London
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && \
    echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
    # Needed for vcpkg
    pkg-config curl zip unzip git \
    # Compiler independent build tools
    ccache cmake ninja-build clang-format \
    # Compilers
    clang-12 clang-9 g++ g++-9  && \
    rm -rf /var/lib/apt/lists/*
