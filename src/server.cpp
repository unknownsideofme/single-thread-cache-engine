#include <iostream>
#include <chrono>
#include <cstdlib>
#include "server.h"
#include "httplib.h"
#include "json.hpp"
#include <string>
using namespace httplib;
using json = nlohmann::json;

void start_server(Cache &cache){
    Server server  ;
    server.Post("/set" , [&](const Request &req , Response &res){
        json body = json::parse(req.body) ;
        std::string key  = body["key"] ;
        json value = body["value"] ;
        cache.set(key , value , 0 ) ;
        res.set_content( json({{"status" , "ok"}}).dump() , "application/json") ;
    }); 

    server.Get("/get" , [&]( const Request & req , Response & res){
        auto key = json::parse(req.body)["key"] ;
        auto value = cache.get(key) ; 
        if(value.has_value()){
            res.set_content( json({ {"status" , "ok"} , {"value" , value.value()} }).dump() , "application/json") ;
        }else{
            res.set_content( json({ {"status" , "not_found"} }).dump() , "application/json") ;
        }
    }) ;

    auto start_time = std::chrono::steady_clock::now();
    server.Get("/health", [start_time](const Request &req, Response &res) {
        auto now = std::chrono::steady_clock::now();
        auto uptime_sec = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        res.set_content(json({{"status", "ok"}, {"uptime_seconds", uptime_sec}}).dump(), "application/json");
    });

    int port = 8080;
    if (const char* env_p = std::getenv("PORT")) {
        port = std::stoi(env_p);
    }
    std::cout << "Starting Cache Server on port " << port << "..." << std::endl;

    if (!server.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
    }
}