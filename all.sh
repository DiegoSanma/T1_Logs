#!/bin/bash

# Build the Docker image
docker build -t my-app .

# Run the container in detached mode with a known name
docker run -d --name my-app-container my-app

# Define a cleanup function to copy the log and stop the container
cleanup() {
  echo "Script interrupted. Attempting to copy merge.log..."
  timestamp=$(date +%Y%m%d-%H%M%S)
  docker cp my-app-container:/app/merge.log ./merge-onexit-$timestamp.log 2>/dev/null || echo "merge.log not found."
  docker stop my-app-container 2>/dev/null
  docker rm my-app-container 2>/dev/null
  exit 1
}

# Set up traps to catch termination signals and execute the cleanup function
trap cleanup SIGINT SIGTERM

# Wait for the container to exit naturally
docker wait my-app-container

# After the container exits, attempt to copy the log
timestamp=$(date +%Y%m%d-%H%M%S)
docker cp my-app-container:/app/merge.log ./merge-onexit-$timestamp.log 2>/dev/null || echo "merge.log not found."

docker stop my-app-container 2>/dev/null
docker rm my-app-container 2>/dev/null

# wsl chmod +x ./all.sh
# wsl ./all.sh