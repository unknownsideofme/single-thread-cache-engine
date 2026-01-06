#pragma once
#include <iostream>
#include <unordered_map>
#include <string> 
#include <mutex>
#include "json.hpp"
#include <chrono>
#include <optional>
using json = nlohmann::json;
class Cache {
    private: 
        std::unordered_map<std::string, json> cache_map;
        std::mutex mtx;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiration_map;
        bool isExpired(const std::string &key);
    public:
        Cache() = default ; 
        ~Cache() = default ;        
        void set(const std::string &key , const json &value );
        std::optional<json> get( const std::string key) ; 
        void del(const std::string &key) ;
        void ttlExpire() ; 


};