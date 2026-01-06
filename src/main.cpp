#include<iostream>
#include<chrono>
#include "cache.h"
#include "server.h"
#include "ttl.h"
#include <thread>

int main(){
    Cache cache; 
    std::cout << "Starting Cache Server on port 8080..." << std::endl;
    std::thread ttl_thread(start_ttl_cleaner, std::ref(cache));
    start_server(cache);
    ttl_thread.join();
    return 0;

}