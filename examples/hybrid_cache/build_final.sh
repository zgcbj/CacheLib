#!/bin/sh
# Final build script for hybrid cache example - compiles and runs successfully

set -e

echo "=== Hybrid Cache Example Build ==="
echo "Building simplified but functional hybrid cache example..."

# Compile the simple version
echo "Compiling simple_main.cpp..."
g++ -std=c++17 -O2 simple_main.cpp -o hybrid-cache-example-simple -pthread

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo "=== Running the example ==="
    ./hybrid-cache-example-simple
    echo ""
    echo "✅ Example completed successfully!"
    echo ""
    echo "Files created:"
    echo "  - hybrid-cache-example-simple (executable)"
    echo "  - /tmp/hybrid_demo.dat (temporary cache file - will be cleaned up)"
    echo ""
    echo "To run again: ./hybrid-cache-example-simple"
else
    echo "❌ Build failed!"
    exit 1
fi