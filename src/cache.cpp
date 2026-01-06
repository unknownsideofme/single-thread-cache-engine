#include "cache.h"
#include <iostream>
#include <chrono>
#include <optional>
#include "json.hpp"
#include <mutex>
using json = nlohmann::json;

bool Cache::isExpired(const std:: string &key){
    // This method assumes the mutex is already locked by the caller
    auto it = expiration_map.find(key);
    if(it != expiration_map.end()){
        auto now = std::chrono::steady_clock::now();
        if(now >= it->second){
            return true;
        }
    }
    return false;
}

void Cache:: set(std::string const &key , json const &value){
    std::lock_guard<std::mutex> lock(mtx) ; 
    cache_map[key] = value ;
    expiration_map[key] = std::chrono::steady_clock::now() + std::chrono::minutes(5) ;
    touchKey(key) ;
    evictIfNeeded() ;

}

std::optional<json> Cache::get ( const std::string &key){
    std::lock_guard<std::mutex> lock(mtx) ;     
    auto it = cache_map.find(key) ; 
    if(it == cache_map.end()) return std::nullopt ;
    if ( isExpired(key)) return std::nullopt ;
    touchKey(key) ;
    return it->second ;
    

}

void Cache:: del ( std::string const & key){
    std::lock_guard<std::mutex> lock(mtx) ; 
    cache_map.erase(key) ; 
    expiration_map.erase(key) ; 
    if(lru_map.find(key) != lru_map.end()){
        lru_list.erase(lru_map[key]) ; 
        lru_map.erase(key) ; 
    }
}

void Cache::ttlExpire() {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto it = expiration_map.begin(); it != expiration_map.end(); ) {
        if (isExpired(it->first)) {
            const std::string key = it->first;
            cache_map.erase(key);
            it = expiration_map.erase(it);
            auto lru_it = lru_map.find(key);
            if (lru_it != lru_map.end()) {
                lru_list.erase(lru_it->second);
                lru_map.erase(lru_it);
            }

        } else {
            ++it;
        }
    }
}


void Cache::touchKey( const std::string &key){
    if(lru_map.find(key) != lru_map.end()){
        lru_list.erase(lru_map[key]) ; 
    }

    lru_list.push_front(key) ;
    lru_map[key] = lru_list.begin() ;
}

void Cache::evictIfNeeded(){
    while(cache_map.size() > max_size){
        std::string lru_key = lru_list.back() ;
        lru_list.pop_back() ; 
        lru_map.erase(lru_key) ; 
        cache_map.erase(lru_key) ; 
        expiration_map.erase(lru_key) ; 
    }
}