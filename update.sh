#!/bin/bash

# Путь к корневой директории проекта
REPO_DIR=$(dirname "$(realpath "$0")")

# Перейти в папку с репозиторием
cd "$REPO_DIR" || exit

# Обновить репозиторий с помощью Git
echo "Updating repository..."
git pull

# Для каждой папки с лабораторной работой, например, Lab1, Lab2, Lab3...
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
