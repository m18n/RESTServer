#pragma once
#include"tasker_manager.h"
#include"connector_manager.h"
namespace server{
static tasker_manager tasker;
void handle_transfer(connector::connector_manager* conn_m,t_json json);
class server_logic{
public:
    server_logic(){
        conn.set_transfer(handle_transfer);
        
    }
    ~server_logic(){
        tasker.~tasker_manager();
    }
private:
    connector::connector_manager conn;
    
};
};