#include "cache.h"
#include <iostream>
#include <chrono>
#include <optional>
#include "json.hpp"
#include <mutex>
#include "wal.h"
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

void Cache:: set(std::string const &key , json const &value , std::int8_t mode ){
    std::lock_guard<std::mutex> lock(mtx) ; 
    if(mode == 0 )  wal.logSet(key, value.dump());
    if( cache_map.find(key) == cache_map.end() && cache_map.size() >= max_size ) LFUevict() ;
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

void Cache::del(const std::string &key , std::int8_t mode) {
    std::lock_guard<std::mutex> lock(mtx);
    if(mode == 0 )  wal.logDel(key);    
    cache_map.erase(key);
    expiration_map.erase(key);

    if (freq.find(key) != freq.end()) {
        int f = freq[key];

        flist[f].erase(itlist[key]);

        if (flist[f].empty()) {
            flist.erase(f);
            freqSet.erase(f);
        }

        freq.erase(key);
        itlist.erase(key);
    }
}

void Cache::ttlExpire() {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto it = expiration_map.begin(); it != expiration_map.end(); ) {
        if (isExpired(it->first)) {
            const std::string key = it->first;
            if (freq.find(key) != freq.end()) {
                int frequency = freq[key] ;
                auto fit = flist.find(frequency);
                if (fit != flist.end()) {
                    fit->second.erase(itlist[key]);

                    if (fit->second.empty()) {
                        flist.erase(frequency);
                        freqSet.erase(frequency);
                    }
                }
                freq.erase(key);
                itlist.erase(key);
            }
            cache_map.erase(key);
            it = expiration_map.erase(it);

        } else {
            ++it;
        }
    }
}

void Cache::touchKey(const std::string &key) {
    // if(lru_map.find(key) != lru_map.end()){
    //     lru_list.erase(lru_map[key]) ; 
    // }

    // lru_list.push_front(key) ;
    // lru_map[key] = lru_list.begin() ;
    std::int32_t frequency = 0;

    if (freq.find(key) != freq.end()) {
        frequency = freq[key];
    }

    // remove from old freq bucket (if exists)
    if (frequency != 0) {
        auto it = flist.find(frequency);
        if (it != flist.end()) {
            it->second.erase(itlist[key]);

            if (it->second.empty()) {
                flist.erase(frequency);
                freqSet.erase(frequency);
            }
        }
    }
    frequency++;
    freq[key] = frequency;
    flist[frequency].push_front(key);
    freqSet.insert(frequency);
    itlist[key] = flist[frequency].begin();
}

void Cache::evictIfNeeded(){
    while(cache_map.size() > max_size){
        LFUevict() ;
    }
}

void Cache::LFUevict(){
    std::int32_t min_freq = 0 ; 
    if(freqSet.size() == 0 )return  ;
    while(freqSet.size() >0 ){
        min_freq = *freqSet.begin() ; 
        if( flist[min_freq].size() > 0 )break ; 
        else { 
            flist.erase(min_freq) ; 
            freqSet.erase(min_freq); 

        }
    }
    std::string key = flist[min_freq].back() ; 
    cache_map.erase(key); 
    itlist.erase(key) ; 
    flist[min_freq].pop_back() ; 
    freq.erase(key) ; 
    expiration_map.erase(key) ; 
    if( flist[min_freq].size() == 0 ) {
        flist.erase(min_freq) ; 
        freqSet.erase(min_freq ) ; 

    }

}