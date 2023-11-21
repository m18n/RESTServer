#include "include/url.h"
void url::init_api_url(crow::SimpleApp &app) {
  CROW_ROUTE(app, "/")
  ([]() { return "tasker node"; });
  // TELEGRAM
  CROW_ROUTE(app, "/api/get/<string>/<string>/command/event")
  ([]( crow::request &req, crow::response &res,std::string hash_worker,std::string group)  { controller::get_events(req,res,hash_worker,group); });
  CROW_ROUTE(app, "/api/send/<string>/<string>/command/event").methods("POST"_method)
  ([]( crow::request &req, crow::response &res,std::string hash_worker,std::string group)  { controller::send_event(req,res,hash_worker,group); });
    CROW_ROUTE(app, "/api/send/<string>/<string>/command/event/start/<string>")
  ([]( crow::request &req, crow::response &res,std::string hash_worker,std::string group,std::string event_id)  { controller::start_event(req,res,hash_worker,group,event_id); });
      CROW_ROUTE(app, "/api/send/<string>/<string>/command/event/clear/<string>")
  ([]( crow::request &req, crow::response &res,std::string hash_worker,std::string group,std::string event_id)  { controller::clear_event(req,res,hash_worker,group,event_id); });
      CROW_ROUTE(app, "/api/send/<string>/<string>/command/event/finish/<string>")
  ([]( crow::request &req, crow::response &res,std::string hash_worker,std::string group,std::string event_id)  { controller::end_event(req,res,hash_worker,group,event_id); });
  //MANAGER

  //CHATGPT


}