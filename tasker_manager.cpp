#include "include/tasker_manager.h"
// };
static server::Logger serv_log_empty;
server::Logger* server::server_log=&serv_log_empty;
void server::init_logg_server(Logger* log){
  server_log=log;
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

  int result = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

  close(sockfd);

  if (result == 0) {
    // Порт вільний
    return false;
  } else {
    // Порт зайнятий
    return true;
  }
}
std::string server::sha256(const std::string &input) {
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
void server::init_event(event *ev) {
  ev->hash_worker = "";
  ev->hash_event = "";
  ev->busy = false;
  ev->json.clear();
  ev->count_restart = 0;
  ev->process = false;
}
void server::init_client(client *cl) {
  cl->hash_worker = "";
  cl->group = "";
  cl->busy = false;
  cl->last_update = false;
}