FROM ubuntu:24.04
ARG HTTP_PROXY=http://172.17.0.1:7288
ARG HTTPS_PROXY=http://172.17.0.1:7288

ENV http_proxy=$HTTP_PROXY
ENV https_proxy=$HTTPS_PROXY

RUN apt update && apt install -y \
    git cmake g++ gcc make autoconf automake libtool \
    libssl-dev libboost-all-dev libevent-dev \
    libgoogle-glog-dev libgflags-dev libdouble-conversion-dev \
    liblz4-dev libsnappy-dev libsodium-dev libzstd-dev \
    libaio-dev libjemalloc-dev libunwind-dev libelf-dev libdwarf-dev \
    python3 ninja-build curl wget && \
    rm -rf /var/lib/apt/lists/*


WORKDIR /opt
RUN git clone https://github.com/axboe/liburing.git && \
    cd liburing && \
    ./configure && \
    make -j$(nproc) && \
    make install && \
    ldconfig 
RUN git clone https://github.com/facebook/CacheLib  && cd CacheLib &&  git checkout v20240320_stable

RUN apt update && apt install -y lld
ENV CMAKE_LINKER=lld
ENV LDFLAGS="-fuse-ld=lld"

#RUN find CacheLib/build/fbcode_builder/manifests -type f -exec sed -i 's|https://github\.com/facebook|https://gh-proxy.org/https://github.com/facebook|g' {} +
#RUN find CacheLib/build/fbcode_builder/manifests -type f -exec sed -i 's|https://zlib.net/zlib-1.3.1.tar.gz|https://zlib.net/fossils/zlib-1.3.1.tar.gz|g' {} +
#RUN cd CacheLib &&  python3 ./build/fbcode_builder/getdeps.py build cachelib
#RUN find CacheLib/build/ -type f -exec sed -i 's|https://zlib.net/zlib-1.3.1.tar.gz|https://zlib.net/fossils/zlib-1.3.1.tar.gz|g' {} +
WORKDIR /opt/CacheLib
RUN sed -i 's/ubuntu18.04|ubuntu20.04|ubuntu21.04|ubuntu22.04)/ubuntu18.04|ubuntu20.04|ubuntu21.04|ubuntu22.04|ubuntu24.04)/g' contrib/build.sh
RUN sed -i 's/sudo //g' contrib/prerequisites-ubuntu18.sh
RUN ./contrib/build.sh -j $(nproc) --skip-system-packages

ENV CACHELIB_HOME=/opt/CacheLib/opt/cachelib
ENV LD_LIBRARY_PATH=${CACHELIB_HOME}/lib:$LD_LIBRARY_PATH

RUN ${CACHELIB_HOME}/bin/cachebench --help || true
