# Cache Engine

A high-performance, thread-safe in-memory cache server with HTTP API interface, built in C++. This cache engine implements LRU (Least Recently Used) eviction policy with automatic TTL (Time-To-Live) expiration support.

## Table of Contents
- [Features](#features)
- [Architecture](#architecture)
- [Methodology](#methodology)
- [Project Structure](#project-structure)
- [Implementation Details](#implementation-details)
- [API Endpoints](#api-endpoints)
- [Building the Project](#building-the-project)
- [Usage](#usage)
- [Technical Specifications](#technical-specifications)

## Features

- **Thread-Safe Operations**: All cache operations are protected with mutex locks for concurrent access
- **LRU Eviction Policy**: Automatically evicts least recently used items when capacity is reached
- **TTL (Time-To-Live)**: Automatic expiration of cache entries after 5 minutes
- **HTTP REST API**: Easy-to-use REST API for cache operations
- **JSON Support**: Store and retrieve any JSON-serializable data
- **Background TTL Cleaner**: Dedicated thread for periodic cleanup of expired entries
- **Configurable Capacity**: Set maximum cache size (default: 100 entries)

## Architecture

The cache engine is built on a layered architecture:

```
┌─────────────────────────────────┐
│      HTTP Server (Port 8080)    │  ← REST API Layer
├─────────────────────────────────┤
│      Cache Manager              │  ← Business Logic
├─────────────────────────────────┤
│  LRU + TTL + Thread Safety      │  ← Core Features
├─────────────────────────────────┤
│   Data Structures (STL)         │  ← Storage Layer
└─────────────────────────────────┘
```

## Methodology

### 1. **Design Phase**
- Identified core requirements: fast access, automatic eviction, expiration, and thread safety
- Chose LRU as the eviction strategy for optimal cache hit rates
- Decided on HTTP REST API for platform-independent access
- Selected JSON for flexible data storage

### 2. **Data Structure Selection**

#### Cache Storage
- **`std::unordered_map<std::string, json>`**: O(1) average-case lookup for cache entries
- **`std::unordered_map<std::string, time_point>`**: Tracks expiration times for each key
- **`std::list<std::string>`**: Maintains LRU order (doubly-linked list for O(1) insertion/deletion)
- **`std::unordered_map<std::string, iterator>`**: Maps keys to their position in LRU list for O(1) access

#### Synchronization
- **`std::mutex`**: Ensures thread-safe operations across all cache methods

### 3. **Algorithm Implementation**

#### LRU Cache Algorithm
```
1. On GET/SET: Move accessed key to front of LRU list
2. On capacity overflow: Remove key from back of list (least recently used)
3. Maintain O(1) access time using iterator mapping
```

#### TTL Management
```
1. Set expiration time on every SET operation (current_time + 5 minutes)
2. Check expiration on GET operations
3. Background thread runs every 1 second to clean expired entries
```

### 4. **Thread Safety Strategy**
- Single mutex (`mtx`) protects all shared data structures
- Lock guard pattern (`std::lock_guard`) ensures automatic lock release
- All public methods acquire lock before any operation
- Private helper methods assume caller holds the lock

### 5. **HTTP Server Integration**
- Used **cpp-httplib** for lightweight HTTP server implementation
- RESTful design with JSON request/response format
- Runs on port 8080 with binding to all interfaces (0.0.0.0)

## Project Structure

```
cache-engine/
├── include/
│   ├── cache.h          # Cache class declaration
│   ├── server.h         # HTTP server interface
│   └── ttl.h            # TTL cleaner thread interface
├── src/
│   ├── cache.cpp        # Cache implementation
│   ├── server.cpp       # HTTP server implementation
│   ├── ttl.cpp          # TTL background cleaner
│   └── main.cpp         # Entry point
├── third-party/
│   ├── httplib.h        # cpp-httplib (HTTP server)
│   └── json.hpp         # nlohmann/json (JSON parsing)
├── cache_server.exe     # Compiled executable
└── README.md
```

## Implementation Details

### Cache Class

The `Cache` class is the core component with the following key methods:

#### Public Methods

**`void set(const std::string &key, const json &value)`**
- Stores a key-value pair in the cache
- Sets expiration time to 5 minutes from now
- Updates LRU order (moves to front)
- Triggers eviction if capacity exceeded

**`std::optional<json> get(const std::string &key)`**
- Retrieves value for given key
- Returns `std::nullopt` if key doesn't exist or has expired
- Updates LRU order on successful retrieval

**`void del(const std::string &key)`**
- Removes a key from all data structures (cache, expiration, LRU)

**`void ttlExpire()`**
- Scans all entries and removes expired ones
- Called periodically by background thread

#### Private Methods

**`bool isExpired(const std::string &key)`**
- Checks if a key has passed its expiration time

**`void touchKey(const std::string &key)`**
- Moves key to front of LRU list
- Updates iterator mapping

**`void evictIfNeeded()`**
- Removes least recently used entries when cache exceeds max_size

### TTL Cleaner Thread

A dedicated background thread runs continuously:
- Sleeps for 1 second intervals
- Calls `cache.ttlExpire()` to remove expired entries
- Ensures automatic cleanup without blocking main operations

### HTTP Server

The server provides a simple REST API:
- **POST /set**: Add or update cache entry
- **GET /get**: Retrieve cache entry

## API Endpoints

### POST /set
Store a value in the cache.

**Request:**
```json
{
  "key": "user:123",
  "value": {"name": "John Doe", "age": 30}
}
```

**Response:**
```json
{
  "status": "ok"
}
```

### GET /get
Retrieve a value from the cache.

**Request:**
```json
{
  "key": "user:123"
}
```

**Response (found):**
```json
{
  "status": "ok",
  "value": {"name": "John Doe", "age": 30}
}
```

**Response (not found/expired):**
```json
{
  "status": "not_found"
}
```

## Building the Project

### Prerequisites
- C++17 compatible compiler (GCC, Clang, MSVC)
- Make or CMake (optional)

### Compilation

**Using g++:**
```bash
g++ -std=c++17 -pthread -I./include -I./third-party \
    src/main.cpp src/cache.cpp src/server.cpp src/ttl.cpp \
    -o cache_server
```

**Using MSVC (Windows):**
```powershell
cl /EHsc /std:c++17 /I.\include /I.\third-party ^
   src\main.cpp src\cache.cpp src\server.cpp src\ttl.cpp ^
   /Fe:cache_server.exe
```

## Usage

### Starting the Server

```bash
./cache_server  # Linux/Mac
cache_server.exe  # Windows
```

Output:
```
Starting Cache Server on port 8080...
```

### Example Usage with curl

**Set a value:**
```bash
curl -X POST http://localhost:8080/set \
  -H "Content-Type: application/json" \
  -d '{"key": "user:1", "value": {"name": "Alice", "email": "alice@example.com"}}'
```

**Get a value:**
```bash
curl -X GET http://localhost:8080/get \
  -H "Content-Type: application/json" \
  -d '{"key": "user:1"}'
```

## Technical Specifications

### Performance Characteristics
- **Set Operation**: O(1) average case
- **Get Operation**: O(1) average case
- **Delete Operation**: O(1) average case
- **Eviction**: O(1) per eviction
- **TTL Cleanup**: O(n) where n = number of entries (runs every 1 second)

### Capacity & Limits
- **Default Max Size**: 100 entries (configurable via constructor)
- **TTL Duration**: 5 minutes (hardcoded in `cache.cpp`)
- **Cleanup Interval**: 1 second

### Thread Safety
- All public operations are thread-safe
- Single mutex protects all shared state
- Lock-free reads are not implemented (future optimization)

### Dependencies
- **cpp-httplib**: Header-only HTTP server library
- **nlohmann/json**: Header-only JSON library
- **C++ STL**: Standard library containers and threading

## Future Enhancements

Potential improvements for the cache engine:

1. **Configurable TTL**: Allow per-key TTL settings
2. **Additional Eviction Policies**: LFU (Least Frequently Used), FIFO
3. **Persistence**: Save/load cache state to disk
4. **Metrics & Monitoring**: Hit/miss rates, memory usage
5. **DELETE Endpoint**: HTTP endpoint for cache deletion
6. **Batch Operations**: Set/get multiple keys in one request
7. **Read-Write Locks**: Improve concurrent read performance
8. **Memory Pooling**: Reduce allocation overhead
9. **Compression**: Compress large values
10. **Distributed Cache**: Multi-node support with consistent hashing


## Author

Developed as a demonstration of C++ system programming, data structures, and concurrent programming concepts.

---

**Last Updated**: January 2026
