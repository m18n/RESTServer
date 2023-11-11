#pragma once
#include"url.h"
namespace server{
    class web_server{
public:
web_server(){
    url::init_api_url(web_api);
    controller::sl=&serv;
}
void start_server(int port){
    serv.conn.on();
    web_api.port(port).run();
}
void set_name_server(std::string name){
    serv.tasker.name_server=name;
    serv.conn.name_client=name;
}
void stop_server(){
    serv.conn.off();
    web_api.stop();
}
void add_conn_server(std::string ip){
    serv.conn.add_connection(ip);
}

private:
    int port;
    crow::SimpleApp web_api;
    server_logic serv;
};
}