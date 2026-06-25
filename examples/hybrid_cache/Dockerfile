FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    git \
    cmake \
    build-essential \
    libboost-all-dev \
    libgoogle-glog-dev \
    libgflags-dev \
    libdouble-conversion-dev \
    liblz4-dev \
    libsnappy-dev \
    libsodium-dev \
    libzstd-dev \
    libaio-dev \
    libjemalloc-dev \
    libunwind-dev \
    libelf-dev \
    libdwarf-dev \
    python3 \
    ninja-build \
    curl \
    wget

# Install liburing
RUN apt-get install -y liburing-dev

# Clone CacheLib
WORKDIR /opt
RUN git clone https://github.com/facebook/CacheLib.git

# Build CacheLib
WORKDIR /opt/CacheLib
RUN ./contrib/build.sh -j $(nproc) -d

# Copy our example
WORKDIR /opt/CacheLib/examples/hybrid_cache
COPY main.cpp .
COPY CMakeLists.txt .

# Build our example
RUN mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Set environment
ENV CACHELIB_HOME=/opt/CacheLib/opt/cachelib
ENV LD_LIBRARY_PATH=${CACHELIB_HOME}/lib:$LD_LIBRARY_PATH

CMD ["./build/hybrid-cache-example"]