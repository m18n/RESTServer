#pragma once
#include "crow.h"
//#include"telegram.h"
#include <thread>
#include"server_logic.h"

namespace controller {
   
   // extern logintg* tg;
    extern server::server_logic* sl;
    //void get_auth_code(crow::request& req, crow::response& res,int id);
    void get_events(crow::request& req, crow::response& res,std::string server_hash,std::string group,std::string hash_worker);  
    void send_event(crow::request& req, crow::response& res,std::string server_hash,std::string group,std::string hash_worker);
    void start_event(crow::request& req, crow::response& res,std::string server_hash,std::string group,std::string hash_worker,std::string event_id);  
    void end_event(crow::request& req, crow::response& res,std::string server_hash,std::string group,std::string hash_worker,std::string event_id);  
    void get_id(crow::request& req, crow::response& res,std::string group);
    void exit_auth(crow::request& req, crow::response& res,std::string server_hash,std::string group,std::string hash_worker);
      
} // namespace controller
