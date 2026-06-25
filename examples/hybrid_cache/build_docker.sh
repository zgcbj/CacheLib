#!/bin/sh
# Build and run hybrid cache example using Docker

set -e

echo "Building hybrid-cache-example with Docker..."

# Build Docker image
docker build -t cachelib-hybrid-example .

# Run the example
echo "Running hybrid cache example..."
docker run --rm -it cachelib-hybrid-example

echo "Docker build and run completed!"