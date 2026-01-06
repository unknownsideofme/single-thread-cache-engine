# 🚀 Cache Engine

<div align="center">

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![REST API](https://img.shields.io/badge/REST-API-green?style=for-the-badge)
![Thread Safe](https://img.shields.io/badge/Thread-Safe-orange?style=for-the-badge)

**A lightweight, high-performance in-memory cache server with TTL support and RESTful API**

[Features](#-features) • [Installation](#-installation) • [API Reference](#-api-reference) • [Architecture](#-architecture) • [Contributing](#-contributing)

</div>

---

## 📋 Overview

Cache Engine is a modern, thread-safe in-memory caching solution built with C++. It provides a simple HTTP REST API for storing and retrieving JSON data with automatic expiration through Time-To-Live (TTL) mechanisms.

Perfect for microservices, distributed systems, or any application requiring fast, temporary data storage.

## ✨ Features

- **🔒 Thread-Safe Operations** - Mutex-based synchronization for concurrent access
- **⏰ Automatic TTL Expiration** - Default 5-minute expiration with background cleanup
- **🌐 RESTful HTTP API** - Easy integration via HTTP endpoints
- **📦 JSON Support** - Native JSON data storage and retrieval
- **⚡ High Performance** - In-memory storage with O(1) lookups
- **🧹 Background Cleanup** - Automatic removal of expired entries every second
- **🔧 Simple & Lightweight** - Minimal dependencies, easy to deploy

## 🛠️ Technology Stack

- **Language:** C++17
- **HTTP Server:** [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- **JSON Processing:** [nlohmann/json](https://github.com/nlohmann/json)
- **Threading:** C++ Standard Library (`std::thread`, `std::mutex`)
- **Data Structures:** `std::unordered_map` for O(1) access

## 📦 Installation

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake (optional, for building)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/cache-engine.git
cd cache-engine

# Compile (example using g++)
g++ -std=c++17 -pthread \
    src/main.cpp \
    src/cache.cpp \
    src/server.cpp \
    src/ttl.cpp \
    -I include \
    -I third-party \
    -o cache_server

# Run the server
./cache_server
```

### Windows Build

```powershell
# Using MSVC
cl /std:c++17 /EHsc /I include /I third-party src/*.cpp /Fe:cache_server.exe

# Run
.\cache_server.exe
```

## 🚦 Quick Start

1. **Start the server:**
   ```bash
   ./cache_server
   ```
   The server will start on `http://0.0.0.0:8080`

2. **Store data:**
   ```bash
   curl -X POST http://localhost:8080/set \
     -H "Content-Type: application/json" \
     -d '{"key": "user:123", "value": {"name": "John Doe", "email": "john@example.com"}}'
   ```

3. **Retrieve data:**
   ```bash
   curl -X GET http://localhost:8080/get \
     -H "Content-Type: application/json" \
     -d '{"key": "user:123"}'
   ```

## 📚 API Reference

### Base URL
```
http://localhost:8080
```

### Endpoints

#### `POST /set`
Store a key-value pair in the cache.

**Request Body:**
```json
{
  "key": "string",
  "value": "any valid JSON"
}
```

**Response:**
```json
{
  "status": "ok"
}
```

**Example:**
```bash
curl -X POST http://localhost:8080/set \
  -H "Content-Type: application/json" \
  -d '{
    "key": "product:42",
    "value": {
      "name": "Laptop",
      "price": 999.99,
      "stock": 15
    }
  }'
```

---

#### `GET /get`
Retrieve a value from the cache by key.

**Request Body:**
```json
{
  "key": "string"
}
```

**Response (Success):**
```json
{
  "status": "ok",
  "value": "cached JSON value"
}
```

**Response (Not Found/Expired):**
```json
{
  "status": "not_found"
}
```

**Example:**
```bash
curl -X GET http://localhost:8080/get \
  -H "Content-Type: application/json" \
  -d '{"key": "product:42"}'
```

## 🏗️ Architecture

### System Design

```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │ HTTP
       ▼
┌─────────────────────────────────────┐
│        HTTP Server (Port 8080)      │
│         (cpp-httplib)               │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│         Cache Manager               │
│  ┌───────────────────────────────┐  │
│  │   Thread-Safe Operations      │  │
│  │   (std::mutex)                │  │
│  └───────────────────────────────┘  │
│                                     │
│  ┌────────────┐  ┌──────────────┐  │
│  │ Cache Map  │  │ Expiration   │  │
│  │ (K-V Data) │  │   Map (TTL)  │  │
│  └────────────┘  └──────────────┘  │
└─────────────────────────────────────┘
              ▲
              │
       ┌──────┴──────┐
       │  TTL Thread │
       │  (1s cycle) │
       └─────────────┘
```

### Core Components

#### 1. **Cache** (`cache.h`, `cache.cpp`)
- Thread-safe key-value storage
- Automatic expiration checking
- Mutex-protected operations
- Methods: `set()`, `get()`, `del()`, `ttlExpire()`

#### 2. **HTTP Server** (`server.h`, `server.cpp`)
- RESTful API endpoints
- JSON request/response handling
- Listens on `0.0.0.0:8080`

#### 3. **TTL Manager** (`ttl.h`, `ttl.cpp`)
- Background thread for cleanup
- Runs every 1 second
- Removes expired entries automatically

#### 4. **Main Application** (`main.cpp`)
- Initializes cache instance
- Spawns TTL cleaner thread
- Starts HTTP server

### Data Flow

1. **SET Operation:**
   ```
   Client → HTTP POST → Server → Cache::set() → Store in cache_map + expiration_map
   ```

2. **GET Operation:**
   ```
   Client → HTTP GET → Server → Cache::get() → Check expiration → Return value or not_found
   ```

3. **TTL Cleanup:**
   ```
   TTL Thread (1s) → Cache::ttlExpire() → Check all entries → Remove expired
   ```

## ⚙️ Configuration

### TTL Duration
Currently set to **5 minutes** (default). To modify:

Edit `src/cache.cpp`, line 23:
```cpp
expiration_map[key] = std::chrono::steady_clock::now() + std::chrono::minutes(5);
```

### Cleanup Interval
Currently runs every **1 second**. To modify:

Edit `src/ttl.cpp`, line 9:
```cpp
this_thread::sleep_for(chrono::seconds(1));
```

### Server Port
Currently listens on **port 8080**. To modify:

Edit `src/server.cpp`, line 30:
```cpp
server.listen("0.0.0.0", 8080);
```

## 📊 Performance Characteristics

| Operation | Time Complexity | Thread Safety |
|-----------|----------------|---------------|
| SET       | O(1) average   | ✅ Yes        |
| GET       | O(1) average   | ✅ Yes        |
| DELETE    | O(1) average   | ✅ Yes        |
| TTL Check | O(n)           | ✅ Yes        |

## 🔒 Thread Safety

All cache operations are protected by mutex locks (`std::lock_guard<std::mutex>`), ensuring:
- ✅ Safe concurrent reads and writes
- ✅ No race conditions
- ✅ Consistent state across threads
- ✅ Atomic operations

## 🧪 Testing

### Manual Testing

```bash
# Terminal 1: Start server
./cache_server

# Terminal 2: Test SET
curl -X POST http://localhost:8080/set \
  -H "Content-Type: application/json" \
  -d '{"key": "test", "value": "Hello, World!"}'

# Test GET
curl -X GET http://localhost:8080/get \
  -H "Content-Type: application/json" \
  -d '{"key": "test"}'

# Wait 5+ minutes and test again (should return not_found)
curl -X GET http://localhost:8080/get \
  -H "Content-Type: application/json" \
  -d '{"key": "test"}'
```

## 📁 Project Structure

```
cache-engine/
├── include/
│   ├── cache.h          # Cache class definition
│   ├── server.h         # HTTP server interface
│   └── ttl.h            # TTL cleaner interface
├── src/
│   ├── cache.cpp        # Cache implementation
│   ├── main.cpp         # Application entry point
│   ├── server.cpp       # HTTP server implementation
│   └── ttl.cpp          # TTL cleaner implementation
├── third-party/
│   ├── httplib.h        # HTTP library (single-header)
│   └── json.hpp         # JSON library (single-header)
├── cache_server.exe     # Compiled executable (Windows)
└── README.md            # This file
```

## 🚀 Future Enhancements

- [ ] Configurable TTL per key
- [ ] DELETE endpoint for manual removal
- [ ] FLUSH endpoint to clear all cache
- [ ] Statistics endpoint (hit/miss ratio, size, etc.)
- [ ] Persistence to disk
- [ ] LRU/LFU eviction policies
- [ ] Authentication/Authorization
- [ ] Clustering support
- [ ] Binary protocol option (faster than JSON)
- [ ] Metrics and monitoring (Prometheus integration)

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is open source and available under the [MIT License](LICENSE).

## 👤 Author

**Your Name**
- GitHub: [@unknownsideofme](https://github.com/unknownsideofme)

## 🙏 Acknowledgments

- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - HTTP server library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++

---

<div align="center">

**If you found this project helpful, please consider giving it a ⭐!**

Made with ❤️ using C++

</div>
