#pragma once

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

class WAL {
public:
    // constructor
    explicit WAL(const std::string& filename);

    // destructor (important for flushing)
    ~WAL();

    // log operations
    void logSet(const std::string& key, const std::string& value);
    void logDel(const std::string& key);

private:
    void worker();  // background thread

    std::string filename;

    std::queue<std::string> queue;
    std::mutex mtx;
    std::condition_variable cv;

    std::thread worker_thread;
    std::atomic<bool> stop;

    
};