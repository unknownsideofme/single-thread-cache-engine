FROM ubuntu:latest 

WORKDIR /app   

COPY . .

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    && rm -rf /var/lib/apt/lists/*

RUN g++ src/*.cpp -std=c++17 -pthread -Iinclude -Ithird-party -o cache_server.exe
CMD ["./cache_server.exe"]

EXPOSE 8080


