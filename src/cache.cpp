#include "cache.h"
#include <iostream>
#include <chrono>
#include <optional>
#include "json.hpp"
#include <mutex>
using json = nlohmann::json;

bool Cache::isExpired(const std:: string &key){
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
    expiration_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(5) ;

}

std::optional<json> Cache::get ( std::string const &key){
    std::lock_guard<std::mutex> lock(mtx) ; 
    auto it = cache_map.find(key) ; 
    if(it != cache_map.end()){
        if(isExpired(key)){
            cache_map.erase(it) ; 
            expiration_map.erase(key) ; 
            return std::nullopt ; 
        }
        return it->second ; 
    }
    return std::nullopt ; 
}

void Cache:: del ( std::string const & key){
    std::lock_guard<std::mutex> lock(mtx) ; 
    cache_map.erase(key) ; 
    expiration_map.erase(key) ; 
}