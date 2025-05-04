FROM gcc:latest
WORKDIR /app
COPY . .
RUN g++ -o main main.cpp crear_array.cpp mergesort.cpp find_alpha.cpp
CMD ["./main"]
