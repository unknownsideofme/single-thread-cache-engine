#include <iostream>
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
        cache.set(key , value) ;
        res.set_content( json({"status" , "ok"}).dump() , "application/json") ;


    }); 

    server.Get("/get" , [&]( const Request & req , Response & res){
        auto key = req.get_param_value("key") ;
        auto value = cache.get(key) ; 
        if(value.has_value()){
            res.set_content( json({ {"status" , "ok"} , {"value" , value.value()} }).dump() , "application/json") ;
        }else{
            res.set_content( json({ {"status" , "not_found"} }).dump() , "application/json") ;
        }
    }) ;

    server.listen("0.0.0.0" , 8080) ;


}