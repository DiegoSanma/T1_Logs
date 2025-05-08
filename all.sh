#!/bin/bash
echo "Starting the script..."
# Build the Docker image
docker build -t my-app .

# Run the container in detached mode with a known name
# docker run -d --name my-app-container my-app
mkdir -p "$(pwd)/data"
mkdir -p "$(pwd)/buckets"
docker run -d \
  --name my-app-container \
  -m 50m \
  -v "$(pwd)/data:/app/data:rw" \
  -v "$(pwd)/buckets:/app/buckets:rw" \
  -e BUCKET_DIR=/app/buckets \
  -e DATA_DIR=/app/data \
  my-app > log_file.txt 2>&1

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

echo "Container has exited."

docker stop my-app-container 2>/dev/null
# docker rm my-app-container 2>/dev/null

# wsl chmod +x ./all.sh
# wsl ./all.sh