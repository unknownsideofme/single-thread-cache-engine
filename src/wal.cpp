#include "wal.h"
#include <fstream>
#include <iostream>

WAL::WAL(const std::string& filename)
    : filename(filename), stop(false)
{
    worker_thread = std::thread(&WAL::worker, this);
}

WAL::~WAL() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();

    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

void WAL::logSet(const std::string& key, const std::string& value) {
    std::string entry = "SET " + key + " " + value;

    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(entry);
    }

    cv.notify_one();
}

void WAL::logDel(const std::string& key) {
    std::string entry = "DEL " + key;

    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(entry);
    }

    cv.notify_one();
}

void WAL::worker() {
    std::ofstream out(filename, std::ios::app);

    if (!out.is_open()) {
        std::cerr << "Failed to open WAL file\n";
        return;
    }

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [&]() {
            return !queue.empty() || stop;
        });

        // drain queue
        while (!queue.empty()) {
            out << queue.front() << "\n";
            queue.pop();
        }

        out.flush();

        // exit condition AFTER draining
        if (stop) break;
    }

    out.close();
}

