#include "include/controller.h"
//logintg* controller::tg;
server::server_logic* controller::sl;
// void controller::telegram::get_auth_code(crow::request &req,
//                                          crow::response &res,int id) {
//     servers::data_telegram d=mt.get_data_id(id);
//     std::cout<<"stat: "<<d.status<<"\n";
//     if(mt.get_data_id(id).status!=servers::status_telegram::RESTART&&tg->startauth() != 0){
//         res.body = "{\"code\":\"0\"}";
//         res.end();
//         return;
//     }       
//     mt.all_restart();
    
//     res.body = "{\"code\":\"-1\"}";
//     res.end();
// }
void controller::get_events(crow::request& req, crow::response& res,std::string hash_worker,std::string group){
    
    
    res.body=sl->tasker.get_events_json(group,hash_worker);
    res.end();
}
void controller::send_event(crow::request& req, crow::response& res,std::string hash_worker,std::string group){  
    server::t_json respon;
    server::event ev;
    ev.json=server::t_json::parse(req.body);
    respon["$respon_id"]=sl->tasker.add_new_event(hash_worker,ev);
    res.body=respon.dump();
    res.end();
}

void controller::start_event(crow::request& req, crow::response& res,std::string hash_worker,std::string group,std::string event_id){
    res.body=sl->tasker.start_event(group,hash_worker,event_id).dump();
    res.end();
}  
void controller::clear_event(crow::request& req, crow::response& res,std::string hash_worker,std::string group,std::string event_id){
    res.body=sl->tasker.clear_event(group,hash_worker,event_id).dump();
    res.end();
}
    void controller::end_event(crow::request& req, crow::response& res,std::string hash_worker,std::string group,std::string event_id){
        res.body=sl->tasker.end_event(group,hash_worker,event_id).dump();
         res.end();
    }  
