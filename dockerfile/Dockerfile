FROM ubuntu:22.04

RUN apt update && apt install -y \
    git cmake g++ gcc make autoconf automake libtool \
    libssl-dev libboost-all-dev libevent-dev \
    libgoogle-glog-dev libgflags-dev libdouble-conversion-dev \
    liblz4-dev libsnappy-dev libsodium-dev libzstd-dev \
    libaio-dev libjemalloc-dev libunwind-dev libelf-dev libdwarf-dev \
    liburing-dev python3 ninja-build curl wget && \
    rm -rf /var/lib/apt/lists/*


WORKDIR /opt
RUN git clone https://github.com/facebook/CacheLib && \
    cd CacheLib && \
    git submodule update --init --recursive

WORKDIR /opt/CacheLib
RUN ./contrib/build.sh -j$(nproc) --skip-system-packages

ENV CACHELIB_HOME=/opt/CacheLib/opt/cachelib
ENV LD_LIBRARY_PATH=${CACHELIB_HOME}/lib:$LD_LIBRARY_PATH

RUN ${CACHELIB_HOME}/bin/cachebench --help || true
