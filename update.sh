#!/bin/bash

REPO_DIR=$(dirname "$(realpath "$0")")

cd "$REPO_DIR" || exit

echo "Updating repository..."
git pull

