#include"iostream"
#include"web_server.h"
int main(){
    std::cout<<"START SERVER LOGIC";
    server::web_server web;
    web.start_server(3000);
    return 0;
}