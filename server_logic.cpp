#include "include/server_logic.h"
server::tasker_manager *server::tm_local = NULL;
void server::handle_transfer(connector::connector_manager *conn_m,
                             t_json json) {
  std::string group;
  int size_list = json["meta"]["$list_servers"].size();
  bool checkhash=false;
  for (int i = 0; i < size_list; i++) {
    if (json["meta"]["$list_servers"][i]["name"] == tm_local->name_server) {
      int n = i;
      if (i + 1 != size_list){
        n++;
      }else{
        
        if(json["meta"]["$type_event"]=="res"&&json["meta"]["$server_hash"]!=tm_local->get_server_hash()){
          return;//last
        }
      }
      group = json["meta"]["$list_servers"][n]["name"];
    }
  }
  int index = tm_local->find_group(group);
  if (index == -1)
    return;
  event ev;
  ev.json = json;
  if (conn_m->start_event(json) == 0) {
    int res = tm_local->add_new_event(ev);
    if (res == -1) {
      int res = -1;
      while (res != 0) {
        res = conn_m->clear_event(json);
      }
    } else {
      conn_m->end_event(json);
    }
  }

  std::cout << "TRANSFER JSON: " << json.dump() << "\n";
}