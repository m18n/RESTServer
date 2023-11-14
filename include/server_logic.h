#pragma once
#include"tasker_manager.h"
#include"../connector_manager/include/connector_manager.h"
namespace server{
extern tasker_manager* tm_local;
void handle_transfer(connector::connector_manager* conn_m,t_json json);
class server_logic{
public:
    tasker_manager tasker;
    connector::connector_manager conn;
    connector::Logger log_conn;
public:
    server_logic(){
        server::tm_local=&this->tasker;
        connector::init_logg_connector(&log_conn);
        conn.set_transfer(handle_transfer);
    }
    
    ~server_logic(){
    }

    
};

};