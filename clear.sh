#!/bin/bash

find . -type d -name "build" -exec rm -rf {} +
find . -type d -name "logs" -exec rm -rf {} +
find . -type d -name "database" -exec rm -rf {} +

echo "All build folders was deleted."
