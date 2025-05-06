#!/bin/bash

# Primer comando
docker build -t mi_app .

# Segundo comando
docker run -d mi_app