#include "include/tasker_manager.h"
// };
static server::Logger serv_log_empty;
server::Logger* server::server_log = &serv_log_empty;
void server::init_logg_server(Logger* log) {
  server_log = log;
}
void server::mutex_n::lock() {
  if (n == 0) {
    mt.lock();
  }
  n++;
}
void server::mutex_n::unlock() {
  if (n == 1) {
    mt.unlock();
  }
  if (n != 0)
    n--;
}
bool server::isPortOccupied(int port) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    std::cerr << "Error creating socket" << std::endl;
    return false;
  }

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  int result = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));

  close(sockfd);

  if (result == 0) {
    // Порт вільний
    return false;
  } else {
    // Порт зайнятий
    return true;
  }
}
std::string server::sha256(const std::string& input) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input.c_str(), input.length());
  SHA256_Final(hash, &sha256);

  std::stringstream ss;
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
  }

  return ss.str();
}
void server::init_event(event* ev) {
  ev->hash_worker = "";
  ev->hash_event = "";
  ev->busy = false;
  ev->json.clear();
  ev->count_restart = 0;
  ev->process = false;
}
void server::init_client(client* cl) {
  cl->hash_worker = "";
  cl->group = "";
  cl->busy = false;
  cl->last_update = false;
}

// group_clients

std::string server::group_clients::generate_hash_event(
    std::string strindex,
    std::string count_restart) {
  server_log->log("group_clients|generate_hash_event", "START FUNCTION\n");
  std::string currentTime = std::to_string(time(nullptr));
  std::string mil = std::to_string(clock());

  scope_lock_mutex s_var(&mt_var);

  std::string hash = sha256(server_hash + group + currentTime + strindex + mil +
                            count_restart);

  return hash;
}
bool server::group_clients::check_client(std::string hash_worker) {
  scope_lock_mutex s_var(&mt_client);
  for (int i = 0; i < clients.size(); i++) {
    if (clients[i].busy == true && clients[i].hash_worker == hash_worker) {
      return true;
    }
  }
  return false;
}
void server::group_clients::set_group(std::string group) {
  server_log->log("group_clients|set_group", "START FUNCTION\n");
  scope_lock_mutex s_var(&mt_var);

  this->group = group;
}
void server::group_clients::init(std::string server_hash) {
  server_log->log("group_clients|init", "START FUNCTION\n");
  scope_lock_mutex s_var(&mt_var);

  this->server_hash = server_hash;
}
std::string server::group_clients::get_group() {
  server_log->log("group_clients|get_group", "START FUNCTION\n");
  scope_lock_mutex s_var(&mt_var);

  std::string gr = group;

  return gr;
}

server::client server::group_clients::get_new_client(std::string hash_worker) {
  server_log->log("group_clients|get_new_client", "START FUNCTION\n");
  scope_lock_mutex s_cl(&mt_client);

  client cl;
  cl.hash_worker = hash_worker;
  cl.group = this->group;
  cl.busy = true;
  cl.last_update = std::time(nullptr);
  for (int i = 0; i < clients.size(); i++) {
    if (clients[i].busy == false) {
      clients[i] = cl;

      return cl;
    }
  }
  clients.push_back(cl);

  return cl;
}
// void server::group_clients::exit_client(std::string hash_worker) {
//   server_log->log("group_clients|exit_client", "START FUNCTION\n");
//   scope_lock_mutex s_cl(&mt_client);

//   for (int i = 0; i < clients.size(); i++) {
//     if (clients[i].hash_worker == hash_worker) {
//       scope_lock_mutex s_ev(&mt_event);

//       for (int j = 0; j < events.size(); j++) {
//         if (events[j].process == true && events[j].hash_worker == hash_worker) {
//           events[j].hash_worker = "";
//           events[j].count_restart++;
//           events[j].hash_event = generate_hash_event(
//               std::to_string(j), std::to_string(events[j].count_restart));
//           events[j].process = false;
//         }
//       }
//       init_client(&clients[i]);

//       break;
//     }
//   }
// }
int server::group_clients::start_event(std::string hash_worker,
                                       std::string event_id) {
  server_log->log("group_clients|start_event", "START FUNCTION\n");
  scope_lock_mutex s_ev(&mt_event);
  if(!check_client(hash_worker)){
    get_new_client(hash_worker);
  }
  for (int i = 0; i < events.size(); i++) {
    if (events[i].hash_event == event_id) {
      if (events[i].process == true) {
        return -2;
      }
      events[i].process = true;

      return 0;
    }
  }

  return -3;
}
int server::group_clients::clear_event(std::string hash_worker,
                                       std::string event_id) {
  server_log->log("group_clients|clear_event", "START FUNCTION\n");
  scope_lock_mutex s_ev(&mt_event);
  if(!check_client(hash_worker)){
    get_new_client(hash_worker);
  }
  std::string strindex;
  for (int i = 0; i < events.size(); i++) {
    if (events[i].hash_event == event_id) {
      if (events[i].process == true) {
        events[i].count_restart++;
        strindex = std::to_string(i);
        events[i].hash_event =
          generate_hash_event(strindex, std::to_string(events[i].count_restart));
        events[i].process = false;
      }

      return 0;
    }
  }

  return -2;
}
int server::group_clients::end_event(std::string hash_worker,
                                     std::string event_id) {
  server_log->log("group_clients|end_event", "START FUNCTION\n");
  scope_lock_mutex s_ev(&mt_event);
if(!check_client(hash_worker)){
    get_new_client(hash_worker);
  }
  for (int i = 0; i < events.size(); i++) {
    if (events[i].hash_event == event_id) {
      scope_lock_mutex s_var(&mt_var);
      init_event(&events[i]);

      return 0;
    }
  }

  return -2;
}
int server::group_clients::add_new_event(std::string hash_worker, event ev) {
  server_log->log("group_clients|add_new_event", "START FUNCTION\n");
  scope_lock_mutex s_ev(&mt_event);
if(!check_client(hash_worker)){
    get_new_client(hash_worker);
  }
  time_t currentTime = time(nullptr);
  std::string time = std::to_string(currentTime);
  std::string strindex;
  ev.busy = true;
  for (int i = 0; i < events.size(); i++) {
    if (events[i].busy == false) {
       strindex = std::to_string(i);
      ev.hash_event =
          generate_hash_event(strindex, std::to_string(ev.count_restart));
      events[i] = ev;
      return 0;
    }
  }
  strindex = events.size();
  ev.hash_event =
      generate_hash_event(strindex, std::to_string(ev.count_restart));
  events.push_back(ev);

  
  return 0;
}

std::string server::group_clients::get_events_json(std::string hash_worker) {
  server_log->log("group_clients|get_events_json", "START FUNCTION\n");
  scope_lock_mutex s_cl(&mt_client);
  scope_lock_mutex s_ev(&mt_event);
  if(!check_client(hash_worker)){
    get_new_client(hash_worker);
  }
  static std::string respon;

  respon.resize(0);
  int cap = respon.capacity();
  for (int i = 0; i < clients.size(); i++) {
    if (clients[i].hash_worker == hash_worker) {
      clients[i].last_update = true;
      break;
    }
  }

  t_json json_id;
  json_id["id"] = "0";
  int n = 0;
  std::string str_id;
  std::string str_meta;
  std::string str_data;
  for (int i = 0; i < events.size(); i++) {
    if (events[i].busy == true && events[i].process == false) {
      json_id["id"] = events[i].hash_event;
      str_id = json_id.dump();
      str_meta = events[i].json["meta"].dump();

      respon += std::to_string(str_id.size()) + str_id +
                std::to_string(str_meta.size()) + str_meta + "\0";
      if (events[i].json.contains("data")) {
        str_data = events[i].json["data"].dump();
        respon += std::to_string(str_data.size()) + str_data;
      }
      respon += "\n";
      // json["events"][n] = {{"id", events[i].hash_event}, {"meta",
      // events[i].json["meta"]}, {"data", events[i].json["data"]}}; n++;
    }
  }
  char* ad = &respon[0];

  if (respon == "")
    return "{}";
  return respon;
}

std::string server::tasker_manager::gethash() {
  server_log->log("tasker_manager|gethash", "START FUNCTION\n");
  time_t currentTime = time(nullptr);
  // std::cout<<"TIME: "<<currentTime<<"\n";
  std::string hash = sha256(std::to_string(currentTime));
  return hash;
}

int server::tasker_manager::find_group(std::string group) {
  server_log->log("tasker_manager|find_group", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);

  for (int i = 0; i < clients_group.size(); i++) {
    if (clients_group[i].get_group() == group) {
      return i;
    }
  }

  return -1;
}
std::string server::tasker_manager::get_server_hash() {
  server_log->log("tasker_manager|get_server_hash", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);

  std::string sr = server_hash;

  return sr;
}
server::client server::tasker_manager::get_new_client(std::string hash_worker,
                                                      std::string group) {
  server_log->log("tasker_manager|get_new_client", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);
  int index = find_group(group);
  if (index == -1) {
    group_clients gr;
    gr.set_group(group);
    gr.init(server_hash);
    clients_group.push_back(std::move(gr));
    index = clients_group.size() - 1;
  }
  client cl = clients_group[index].get_new_client(hash_worker);

  return cl;
}
std::string server::tasker_manager::get_events_json(std::string group,
                                                    std::string hash_worker) {
  server_log->log("tasker_manager|get_events_json", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);
  std::string ev;
  int index = find_group(group);
  if (index == -1) {
    group_clients gr;
    gr.set_group(group);
    gr.init(server_hash);
    clients_group.push_back(std::move(gr));
    index = clients_group.size() - 1;
  }
  ev = clients_group[index].get_events_json(hash_worker);

  if (index != -1)
    return ev;
  return "{}";
}
server::t_json server::tasker_manager::start_event(std::string group,
                                                   std::string hash_worker,
                                                   std::string event_id) {
  server_log->log("tasker_manager|start_event", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);

  t_json res;
  res["$status"] = -1;
  int index = find_group(group);
  if (index != -1) {
    int s = clients_group[index].start_event(hash_worker, event_id);
    res["$status"] = s;
  }

  return res;
}
server::t_json server::tasker_manager::clear_event(std::string group,
                                                   std::string hash_worker,
                                                   std::string event_id) {
  server_log->log("tasker_manager|clear_event", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);
  t_json res;
  res["$status"] = -1;
  int index = find_group(group);
  if (index != -1) {
    int s = clients_group[index].clear_event(hash_worker, event_id);
    res["$status"] = s;
  }

  return res;
}
server::t_json server::tasker_manager::end_event(std::string group,
                                                 std::string hash_worker,
                                                 std::string event_id) {
  server_log->log("tasker_manager|end_event", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);

  t_json res;
  res["$status"] = -1;
  int index = find_group(group);
  if (index != -1) {
    int s = clients_group[index].end_event(hash_worker, event_id);
    res["$status"] = s;
  }

  return res;
}
int server::tasker_manager::add_new_event(std::string hash_worker, event ev) {
  server_log->log("tasker_manager|add_new_event", "START FUNCTION\n");
  scope_lock_mutex s_mt(&mt);

  std::string group;
  int size_list = ev.json["meta"]["$list_servers"].size();
  std::cout << "LIST: " << ev.json["meta"]["$list_servers"].dump() << "\n";
  for (int i = 0; i < size_list; i++) {
    if (ev.json["meta"]["$list_servers"][i]["name"] == name_server) {
      int n = i;
      if (i + 1 != size_list)
        n++;
      group = ev.json["meta"]["$list_servers"][n]["name"];
    }
  }
  int index = find_group(group);
  if (index == -1) {
    group_clients gr;
    gr.set_group(group);
    gr.init(server_hash);
    clients_group.push_back(std::move(gr));
    index = clients_group.size() - 1;
  }
  if(!ev.json["meta"].contains("$respon_id")){
    return -1;
  }
  ev.json["meta"]["$server_hash"] = server_hash;
  int res = clients_group[index].add_new_event(hash_worker, ev);
  return res;
}