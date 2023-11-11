#include "url.h"
void url::init_api_url(crow::SimpleApp &app) {
  CROW_ROUTE(app, "/")
  ([]() { return "tasker node"; });
  // TELEGRAM
  CROW_ROUTE(app, "/api/get/<string>/<string>/command/<string>/event")
  ([]( crow::request &req, crow::response &res,std::string server_hash,std::string group,std::string hash_id)  { controller::get_events(req,res,server_hash,group,hash_id); });
  CROW_ROUTE(app, "/api/send/<string>/<string>/command/<string>/event").methods("POST"_method)
  ([]( crow::request &req, crow::response &res,std::string server_hash,std::string group,std::string hash_id)  { controller::send_event(req,res,server_hash,group,hash_id); });
    CROW_ROUTE(app, "/api/send/<string>/<string>/command/<string>/event/start/<string>")
  ([]( crow::request &req, crow::response &res,std::string server_hash,std::string group,std::string hash_id,std::string event_id)  { controller::start_event(req,res,server_hash,group,hash_id,event_id); });
      CROW_ROUTE(app, "/api/send/<string>/<string>/command/<string>/event/finish/<string>")
  ([]( crow::request &req, crow::response &res,std::string server_hash,std::string group,std::string hash_id,std::string event_id)  { controller::end_event(req,res,server_hash,group,hash_id,event_id); });
  CROW_ROUTE(app, "/api/client/<string>/getid").methods("POST"_method)
  ([]( crow::request &req, crow::response &res,std::string str)  { controller::get_id(req,res,str); });
  CROW_ROUTE(app, "/api/client/<string>/<string>/command/<string>/exit")
  ([]( crow::request &req, crow::response &res,std::string server_hash,std::string group,std::string hash_id)  { controller::exit_auth(req,res,server_hash,group,hash_id); });
  //MANAGER

  //CHATGPT


}