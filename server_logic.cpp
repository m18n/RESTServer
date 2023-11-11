#include"server_logic.h"
void server::handle_transfer(connector::connector_manager* conn_m,t_json json){
    std::cout<<"TRANSFER JSON: "<<json.dump()<<"\n";
}