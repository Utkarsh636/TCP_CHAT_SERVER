FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y \
    g++ \
    libwebsockets-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY src ./src
COPY include ./include
COPY web ./web

RUN g++ -std=c++17 \
    -Iinclude \
    src/websocket_server.cpp \
    src/chat_server.cpp \
    -lwebsockets \
    -o server

EXPOSE 8081

CMD ["./server"]