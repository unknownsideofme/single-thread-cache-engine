FROM ubuntu

RUN apt-get update && apt-get install -y g++

WORKDIR /app
COPY . .

RUN g++ -std=c++17 src/*.cpp -I include -I third-party -o cache_server -pthread


EXPOSE 8080

CMD ["sh", "-c", "./cache_server"]