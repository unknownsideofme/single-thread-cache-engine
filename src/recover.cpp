#include "recover.h"

#include <mutex> 
#include <string>
#include <fstream>
#include <sstream>

void Recover(Cache& cache){
    const std::string filename = "wal.log" ; 

    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }


    std::string line;
    while (std::getline(file, line)) {
        // Process each line from the file

        
        std::istringstream iss(line);
        std::string op, key, value;
        iss >> op;

        if (op == "SET") {
            iss >> key;
            std::getline(iss, value);
            nlohmann::json json_value = nlohmann::json::parse(value);

            cache.set(key, json_value , 1 );
            // Handle SET operation
        } else if (op == "DEL") {
            iss >> key;
            // Handle DEL operation
            cache.del(key, 1 ) ; 
        }
    }

    file.close();
}