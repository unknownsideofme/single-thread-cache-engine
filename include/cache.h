#pragma once
#include <unordered_map>
#include <string> 
#include <mutex>
#include "json.hpp"
#include <chrono>
#include <optional>
#include <list>
using json = nlohmann::json;
class Cache {
    private: 
        std::unordered_map<std::string, json> cache_map;
        std::mutex mtx;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiration_map;
        std::list<std::string> lru_list;
        size_t max_size = 1000;
        std::unordered_map<std::string, std::list<std::string>::iterator> lru_map;

        bool isExpired(const std::string &key);
        void touchKey(const std::string &key);
        void evictIfNeeded();

    public:
        Cache(size_t cap = 100  ): max_size(cap) {} ; 
        ~Cache() = default ;  
        
        
        void set(const std::string &key , const json &value );
        std::optional<json> get( const std::string &key) ; 
        void del(const std::string &key) ;
        void ttlExpire() ; 


};