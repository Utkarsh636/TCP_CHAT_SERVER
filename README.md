# ⚡ TCP Chat Server

<div align="center">

### Real-Time Multi-Client Communication using C++ & WebSockets

A real-time chat server built from the networking layer up — starting with TCP socket programming and extended into a browser-accessible WebSocket application, containerized with Docker and deployed publicly.

<br>

[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![TCP/IP](https://img.shields.io/badge/Network-TCP%2FIP-0078D4?style=for-the-badge)](#)
[![WebSocket](https://img.shields.io/badge/WebSocket-Real--Time-7C3AED?style=for-the-badge)](#)
[![libwebsockets](https://img.shields.io/badge/libwebsockets-5.0-00A98F?style=for-the-badge)](https://libwebsockets.org/)
[![Docker](https://img.shields.io/badge/Docker-Containerized-2496ED?style=for-the-badge&logo=docker&logoColor=white)](https://www.docker.com/)
[![GitHub](https://img.shields.io/badge/GitHub-Repository-181717?style=for-the-badge&logo=github)](https://github.com/Utkarsh636/TCP_CHAT_SERVER)

<br>

🌐 **Live Demo**

**https://tcp-chat-server-t4ez.onrender.com/**

</div>

---

## 📌 Overview

**TCP Chat Server** is a real-time multi-client communication system implemented primarily in **C++**.

The project began as a low-level TCP socket programming exercise and evolved into a browser-accessible chat application using **WebSockets**.

The server supports multiple connected clients, usernames, message broadcasting, connection events, and synchronized client management.

The application is packaged using **Docker** and deployed publicly through **Render**.

---

## ✨ Features

### 🔌 Networking

- TCP socket programming
- `socket()`
- `bind()`
- `listen()`
- `accept()`
- `send()` / `recv()`
- Client-server communication
- Multi-client connections

### 💬 Chat System

- Real-time messaging
- Message broadcasting
- Username support
- `/users` command
- `/quit` command
- Join notifications
- Leave notifications
- Message framing

### 🧵 Concurrency

- Multiple client handling
- Thread-based client handling in the TCP implementation
- Shared client management
- Mutex-based synchronization
- Protected access to shared client state

### 🌐 Web Interface

- Browser-based chat
- WebSocket communication
- Username selection
- Online users
- Connection status
- Message bubbles
- Timestamps
- Responsive UI
- Enter-to-send
- Shift + Enter for new lines

### 🐳 Deployment

- Dockerized application
- Ubuntu-based container
- C++17 compilation inside container
- libwebsockets dependency
- Public deployment using Render

---

# 🏗️ Architecture

```text
                         INTERNET
                            │
                            ▼
                  ┌──────────────────┐
                  │     BROWSER      │
                  │                  │
                  │ HTML / CSS / JS  │
                  └────────┬─────────┘
                           │
                    HTTP / WebSocket
                           │
                           ▼
                  ┌──────────────────┐
                  │  C++ WEBSOCKET   │
                  │      SERVER      │
                  └────────┬─────────┘
                           │
                           ▼
                  ┌──────────────────┐
                  │    ChatServer    │
                  │ Client Manager   │
                  └────────┬─────────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
              ▼            ▼            ▼
          CLIENT A      CLIENT B      CLIENT C
