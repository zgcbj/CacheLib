#!/bin/sh
# Build complete CacheLib using Docker

set -e

echo "Building complete CacheLib using Docker..."

# Check if Docker is running
if ! docker info >/dev/null 2>&1; then
    echo "Docker is not running. Please start Docker first."
    exit 1
fi

# Build the Docker image
echo "Building Docker image with complete CacheLib..."
docker build -f Dockerfile.full -t cachelib-full-build .

# Run the container to verify build
echo "Running container to verify CacheLib build..."
docker run --rm cachelib-full-build

echo "CacheLib built successfully in Docker!"
echo "You can now use this image to build and run CacheLib examples."