#include "include/server_logic.h"
void server::handle_transfer(connector::connector_manager *conn_m,
                             t_json json) {
  std::string group;
  int size_list = json["meta"]["$list_servers"].size();
  for (int i = 0; i < size_list; i++) {
    if (json["meta"]["$list_servers"][i]["name"] == NAME_SERVER) {
      int n = i;
      if (i + 1 != size_list)
        n++;
      group = json["meta"]["$list_servers"][n]["name"];
    }
  }
  int index = tasker->find_group(group);
  if (index == -1)
    return;
  if (conn_m->start_event(json) == 0) {

    conn_m->end_event(json);
  }
  std::cout << "TRANSFER JSON: " << json.dump() << "\n";
}