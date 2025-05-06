#!/bin/bash

docker build -t my-app .

docker run -d --rm --name my-app-container my-app 

# docker stop my-app-container #This command is for early stopping the container
echo "Container is running in background. Press ENTER to stop it..."
read
docker stop my-app-container

# wsl chmod +x ./all.sh                                                                                                                                   ─╯
# wsl ./all.sh