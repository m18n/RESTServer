#pragma once
#include <netinet/in.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>
namespace server {
using t_json = nlohmann::json;

// namespace servers{
//     enum status_telegram{
//         RESTART,
//         STATE,
//     };
// struct data_telegram
// {
//     int id=-1;
//     status_telegram status=status_telegram::STATE;
//     void init(){
//         this->id=-1;
//         this->status=status_telegram::STATE;
//         this->busy=false;
//     }
//     bool busy=false;
// };
bool isPortOccupied(int port);

std::string sha256(const std::string& input);
struct client {
  std::string hash_worker;
  std::string group;
  bool busy = false;
  bool last_update = false;
};
void init_client(client* cl);
struct event {
  std::string hash_worker;
  std::string hash_event;
  t_json json;
  int count_restart = 0;
  bool busy = false;
  bool process = false;
};
void init_event(event* ev);


class mutex_n {
 public:
  mutex_n() = default;
  void lock();
  void unlock();

 private:
  int n = 0;
  std::mutex mt;
};
class scope_lock_mutex {
 public:
  scope_lock_mutex() { b = false; }
  scope_lock_mutex(mutex_n* m) : scope_lock_mutex() {
    this->m = m;
    lock();
  }
  void lock() {
    if (b == false)
      m->lock();
    b = true;
  }
  void unlock() {
    if (b == true)
      m->unlock();
    b = false;
  }
  ~scope_lock_mutex() { unlock(); }

 private:
  mutex_n* m;
  bool b;
};
struct file {
  std::ofstream file;
  std::string name_file;
};
class Logger {
 private:
  std::vector<file> logFiles;
  std::string currentfile;
  mutex_n mt;
  std::string getCurrentTime() {
    std::time_t currentTime = std::time(nullptr);
    char* timeString = std::ctime(&currentTime);
    timeString[std::strlen(timeString) - 1] =
        '\0';  // Видаляємо символ нового рядка \n
    return std::string(timeString);
  }

 public:
  Logger() {}
  void close_file(file* file) {
    scope_lock_mutex s_mt(&mt);
    if (file->file.is_open()) {
      file->file.close();
    }
  }
  void delete_file(std::string filename) {
    scope_lock_mutex s_mt(&mt);
    for (int i = 0; i < logFiles.size(); i++) {
      if (logFiles[i].name_file == filename) {
        close_file(&logFiles[i]);
        logFiles.erase(logFiles.begin() + i);
        break;
      }
    }
  }
  int find_file(std::string filename) {
    scope_lock_mutex s_mt(&mt);
    for (int i = 0; i < logFiles.size(); i++) {
      if (logFiles[i].name_file == filename) {
        return i;
      }
    }
    return -1;
  }
  void set_current_file(std::string filename) {
    scope_lock_mutex s_mt(&mt);
    this->currentfile = filename;
  }
  void add_file(std::string filename) {
    scope_lock_mutex s_mt(&mt);
    delete_file(filename);
    file logFile;
    logFile.name_file = filename;
    logFile.file.open(
        filename,
        std::ios::out | std::ios::app);  // Відкриваємо файл для логування
                                         // (додаємо до вже існуючого)
    if (!logFile.file.is_open()) {
      exit(1);
    }
    this->currentfile = filename;
    logFiles.push_back(std::move(logFile));
  }
  void log(std::string name_log, std::string message) {
    scope_lock_mutex s_mt(&mt);
    std::string time = getCurrentTime();
    if (logFiles.size() == 0) {
      std::cout << time << " - " << name_log << " - " << message << std::endl;
      return;
    }
    int index = find_file(currentfile);
    if (index == -1) {
      exit(1);
    }
    if (logFiles[index].file.is_open()) {
      logFiles[index].file
          << time << " - " << name_log << " - " << message
          << std::endl;  // Записуємо повідомлення з часом у файл
    }
  }
  void log(std::string filename, std::string name_log, std::string message) {
    scope_lock_mutex s_mt(&mt);
    std::string time = getCurrentTime();
    if (logFiles.size() == 0) {
      std::cout << time << " - " << name_log << " - " << message << std::endl;
      return;
    }
    int index = find_file(filename);
    if (index == -1) {
      exit(1);
    }
    if (logFiles[index].file.is_open()) {
      logFiles[index].file
          << time << " - " << name_log << " - " << message
          << std::endl;  // Записуємо повідомлення з часом у файл
    }
  }
  ~Logger() {
    scope_lock_mutex s_mt(&mt);
    for (int i = 0; i < logFiles.size(); i++) {
      close_file(&logFiles[i]);
    }
  }
};

extern Logger* server_log;
void init_logg_server(Logger* log);
class group_clients {
 public:
  group_clients() {
    server_log->log("group_clients|group_clients", "START FUNCTION\n");
    scope_lock_mutex s_cl(&mt_client);
    scope_lock_mutex s_ev(&mt_event);
    scope_lock_mutex s_ri(&mt_respon_id);
    scope_lock_mutex s_var(&mt_var);

    group = "";
    clients.resize(25);
    events.resize(100);
  }
  group_clients(group_clients& other) {
    server_log->log("group_clients|group_clients(copy)", "START FUNCTION\n");
    scope_lock_mutex s_cl(&other.mt_client);
    scope_lock_mutex s_ev(&other.mt_event);
    scope_lock_mutex s_ri(&other.mt_respon_id);
    scope_lock_mutex s_var(&other.mt_var);
    clients = other.clients;
    events = other.events;
    group = other.group;
    server_hash = server_hash;
  }

  // Move constructor
  group_clients(group_clients&& other) noexcept {
    server_log->log("group_clients|group_clients(move)", "START FUNCTION\n");
    scope_lock_mutex s_cl(&other.mt_client);
    scope_lock_mutex s_ev(&other.mt_event);
    scope_lock_mutex s_ri(&other.mt_respon_id);
    scope_lock_mutex s_var(&other.mt_var);
    clients = std::move(other.clients);
    events = std::move(other.events);
    group = std::move(other.group);
    server_hash = std::move(server_hash);

    // Implement the move constructor logic here
  }

 private:
  std::string generate_hash_event(std::string strindex,
                                  std::string count_restart);
    bool check_client(std::string hash_worker);
 public:
  void set_group(std::string group);
  void init(std::string server_hash);
  std::string get_group();
  client get_new_client(std::string hash_worker);
  //void exit_client(std::string hash_worker);
  int start_event(std::string hash_worker,std::string event_id);
  int clear_event(std::string hash_worker,std::string event_id);
  int end_event(std::string hash_worker,std::string event_id);
  int add_new_event(std::string hash_worker,event ev);
  std::string get_events_json(std::string hash_worker);

 private:
  std::vector<client> clients;
  std::vector<event> events;
  std::string group = "";
  std::string server_hash;
  mutex_n mt_client;
  mutex_n mt_respon_id;
  mutex_n mt_event;
  mutex_n mt_var;
};

class tasker_manager {
 private:
  std::string gethash();

 public:
  std::string name_server = "tasker";
  tasker_manager() {
    server_log->log("tasker_manager|tasker_manager", "START FUNCTION\n");
    scope_lock_mutex s_mt(&mt);

    server_hash = gethash();
    last_check_client = time(nullptr);
  }

  ~tasker_manager() {
    server_log->log("tasker_manager|~tasker_manager", "START FUNCTION\n");
  }
  int find_group(std::string group);
  std::string get_server_hash();
  client get_new_client(std::string hash_worker, std::string group);
  
  std::string get_events_json(std::string group, std::string hash_worker);
  t_json start_event(std::string group,
                     std::string hash_worker,
                     std::string event_id);
  t_json clear_event(std::string group,
                     std::string hash_worker,
                     std::string event_id);
  t_json end_event(std::string group,std::string hash_worker, std::string event_id);
  int add_new_event(std::string hash_worker,event ev);

 private:
  time_t last_check_client;
  std::string server_hash;
  mutex_n mt;
  std::vector<group_clients> clients_group;
};
// class manager_telegram{
//     public:
//     manager_telegram(){
//         telegrams.resize(25);
//         for(int i=0;i<telegrams.size();i++){
//             telegrams[i].id=i;
//         }
//     }
//     data_telegram get_new_telegram(){
//          data_telegram t;
//          t.busy=true;
//          for(int i=0;i<telegrams.size();i++){
//             if(telegrams[i].busy==false){
//                 telegrams[i].init();
//                 telegrams[i].id=i;
//                 telegrams[i].busy=true;
//                 return telegrams[i];
//             }
//          }
//         t.id=telegrams.size();
//         telegrams.push_back(t);
//         return t;
//     }
//     void exit_auth(int id){
//         telegrams[id].init();
//     }
//     void all_restart(){
//         for(int i=0;i<telegrams.size();i++){
//             if(telegrams[i].busy==true){
//                 telegrams[i].status=status_telegram::RESTART;
//             }
//         }
//     }
//     data_telegram get_data_id(int id){

//         return telegrams[id];
//     }
//     private:
//     std::vector<data_telegram> telegrams;
// };
// }
}  // namespace server