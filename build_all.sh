#!/bin/bash

REPO_DIR=$(dirname "$(realpath "$0")")

cd "$REPO_DIR" || exit

for dir in Lab*/; do
    if [ -d "$dir" ]; then
        echo "Building project in folder $dir..."
        cd "$dir" || exit
        mkdir -p build
        cd build || exit
        cmake -G "Unix Makefiles" ..
        cmake --build .
        cd ../.. || exit
    fi
done

echo "All projects have been built successfully!"
