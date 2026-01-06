#include "ttl.h"
#include <thread>
#include <chrono>

using namespace std;

void start_ttl_cleaner(Cache& cache) {
    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
        cache.ttlExpire();
    }
}
