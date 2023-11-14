#pragma once
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <openssl/sha.h>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include<mutex>
namespace server{
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

std::string sha256(const std::string &input);
struct client
{
    std::string hash_worker;
    std::string group;
    bool busy = false;
    bool last_update=false;
};
void init_client(client *cl);
struct event
{
    std::string hash_worker;
    std::string hash_event;
    t_json json;
    int count_restart=0;
    bool busy = false;
    bool process = false;
};
void init_event(event *ev);

struct respon_id
{
    int id = -1;
};
class mutex_n{
public:
mutex_n()=default;
    void lock();
    void unlock();
private:
    int n=0;
    std::mutex mt;
};

class group_clients
{
public:
    group_clients()
    {
        mt_client.lock();
        mt_event.lock();
        mt_respon_id.lock();
        mt_var.lock();
        group = "";
        clients.resize(25);
        events.resize(100);
        ids.resize(100);
        mt_client.unlock();
        mt_event.unlock();
        mt_respon_id.unlock();
        mt_var.unlock();
    }
      group_clients(const group_clients& other)
    {
        // Implement the copy constructor logic here
    }

    // Move constructor
    group_clients(group_clients&& other) noexcept
    {
        // Implement the move constructor logic here
    }
private:
std::string generate_hash_event(std::string strindex,std::string count_restart){
    std::string currentTime = std::to_string(time(nullptr));
    std::string mil=std::to_string(clock());
    mt_var.lock();
    std::string hash=sha256(server_hash + group + currentTime + strindex+mil+ count_restart);
    mt_var.unlock();
    return hash;
}
public:
    void set_group(std::string group)
    {
        mt_var.lock();
        this->group = group;
        mt_var.unlock();
    }
    void init(std::string server_hash)
    {
        mt_var.lock();
        this->server_hash = server_hash;
        mt_var.unlock();
    }
    std::string get_group()
    {
        
        mt_var.lock();
        std::string gr=group;
        mt_var.unlock();
        return gr;
    }

    respon_id get_respon_id()
    {
        mt_respon_id.lock();
        respon_id r;
        for (int i = 0; i < ids.size(); i++)
        {
            if (ids[i].id == -1)
            {
                ids[i].id = i;
                r=ids[i];
                mt_respon_id.unlock();
                return r;
            }
        }
        r.id = ids.size();
        ids.push_back(r);
        mt_respon_id.unlock();
        return r;
    }
    void finish_respon_id(respon_id r)
    {
        mt_respon_id.lock();
        if (r.id < ids.size())
        {
            ids[r.id].id = -1;
        }
        mt_respon_id.unlock();
    }
    client get_new_client(std::string time, std::string ip)
    {
        mt_client.lock();
        client cl;
        cl.hash_worker = sha256(time + ip);
        cl.group = this->group;
        cl.busy=true;
        cl.last_update = std::time(nullptr);
        for (int i = 0; i < clients.size(); i++)
        {
            if (clients[i].busy == false)
            {
                clients[i] = cl;
                mt_client.unlock();
                return cl;
            }
        }
        clients.push_back(cl);
        mt_client.unlock();
        return cl;
    }
    void exit_client(std::string hash_worker)
    {
        
        mt_client.lock();
        for (int i = 0; i < clients.size(); i++)
        {
            if (clients[i].hash_worker == hash_worker)
            {
                mt_event.lock();
                for(int j=0;j<events.size();j++){
                    if(events[j].process==true&&events[j].hash_worker==hash_worker){
                        events[j].hash_worker="";
                        events[j].count_restart++;
                        events[j].hash_event=generate_hash_event(std::to_string(j),std::to_string(events[j].count_restart));
                        events[j].process=false;
                    }
                }
                init_client(&clients[i]);
                mt_event.unlock();
                
                break;
            }
        }
        mt_client.unlock();
    }
    int start_event(std::string hash_worker,std::string event_id)
    {
        mt_event.lock();
        for (int i = 0; i < events.size(); i++)
        {
            if (events[i].hash_event == event_id)
            {
                if (events[i].process == true){
                    mt_event.unlock();
                    return -2;
                }
                events[i].process = true;
                mt_event.unlock();
                return 0;
            }
        }
        mt_event.unlock();
        return -3;

    }
    int clear_event(std::string hash_worker,std::string event_id)
    {
        mt_event.lock();
        for (int i = 0; i < events.size(); i++)
        {
            if (events[i].hash_event == event_id)
            {
                if (events[i].process == true){
                    events[i].process=false;
                }
                mt_event.unlock();
                return 0;
            }
        }
        mt_event.unlock();
        return -2;
    }
    int end_event(std::string event_id)
    {
        mt_event.lock();
        for (int i = 0; i < events.size(); i++)
        {
            if (events[i].hash_event == event_id)
            {
                mt_var.lock();
                if (events[i].json["meta"]["$server_hash"] == server_hash && events[i].json["meta"]["$type_event"] == "res")
                {
                    respon_id r;
                    r.id = events[i].json["meta"]["$respon_id"];
                    finish_respon_id(r);
                    
                }
                init_event(&events[i]);
                mt_event.unlock();
                mt_var.unlock();
                return 0;
            }
        }
        mt_event.unlock();
        return -2;
    }
    int add_new_event(event ev, std::string server_hash)
    {
        mt_event.lock();
        time_t currentTime = time(nullptr);
        std::string time = std::to_string(currentTime);
        std::string strindex;
        ev.busy = true;
        for (int i = 0; i < events.size(); i++)
        {
            if (events[i].busy == false)
            {
                ev.hash_event = generate_hash_event(strindex,std::to_string(ev.count_restart));
                events[i] = ev;
                strindex = std::to_string(i);
                mt_event.unlock();
                if (ev.json["meta"]["$type_event"] == "req")
                    return ev.json["meta"]["$respon_id"];
                return -2;
            }
        }
        strindex = events.size();
        ev.hash_event= generate_hash_event(strindex,std::to_string(ev.count_restart));
        events.push_back(ev);
        mt_event.unlock();
        if (ev.json["meta"]["$type_event"] == "req")
            return ev.json["meta"]["$respon_id"];
        return -2;
    }
    void ping_client(){
        mt_client.lock();
        for (int i = 0; i < clients.size(); i++)
        {
            if (clients[i].busy==true)
            {
                if(clients[i].last_update==false){
                    exit_client(clients[i].hash_worker);
                }else{
                    clients[i].last_update=false;
                }
            }
        }
        mt_client.unlock();
    }
    std::string get_events_json(std::string hash_worker)
    {
        mt_client.lock();
        mt_event.lock();
        static std::string respon;
        
        respon.resize(0);
        int cap=respon.capacity();
        for (int i = 0; i < clients.size(); i++)
        {
            if (clients[i].hash_worker == hash_worker)
            {
                clients[i].last_update=true;
                break;
            }
        }
        
        t_json json_id;
        json_id["id"]="0";
        int n = 0;
        std::string str_id;
        std::string str_meta;
        std::string str_data;
        for (int i = 0; i < events.size(); i++)
        {
            if (events[i].busy == true && events[i].process == false)
            {
                json_id["id"]=events[i].hash_event;
                str_id=json_id.dump();
                str_meta=events[i].json["meta"].dump();

                respon+=std::to_string(str_id.size())+str_id+std::to_string(str_meta.size())+str_meta+"\0"; 
                if(events[i].json.contains("data")){
                    str_data=events[i].json["data"].dump();
                    respon+=std::to_string(str_data.size())+str_data;
                }
                respon+="\n";
                // json["events"][n] = {{"id", events[i].hash_event}, {"meta", events[i].json["meta"]}, {"data", events[i].json["data"]}};
                // n++;
            }
        }
        char* ad=&respon[0];
        mt_client.unlock();
        mt_event.unlock();
        if(respon=="")
            return "{}";
        return respon;
    }

private:
    std::vector<client> clients;
    std::vector<event> events;
    std::vector<respon_id> ids;
    std::string group = "";
    std::string server_hash;
    mutex_n mt_client;
    mutex_n mt_respon_id;
    mutex_n mt_event;
    mutex_n mt_var;
    
};

class tasker_manager
{
private:
    
    std::string gethash()
    {
        time_t currentTime = time(nullptr);
        // std::cout<<"TIME: "<<currentTime<<"\n";
        std::string hash = sha256(std::to_string(currentTime));
        return hash;
    }

public:
    std::string name_server="tasker";
    tasker_manager()
    {
        mt.lock();
        server_hash = gethash();
        last_check_client=time(nullptr);
        mt.unlock();
    }

    ~tasker_manager()
    {
    }
    int find_group(std::string group)
    {
        mt.lock();
        for (int i = 0; i < clients_group.size(); i++)
        {
            if (clients_group[i].get_group() == group)
            {
                mt.unlock();
                return i;
            }
        }
        mt.unlock();
        return -1;
    }
    std::string get_server_hash()
    {
        mt.lock();
        std::string sr=server_hash;
        mt.unlock();
        return sr;
    }
    client get_new_client(std::string time, std::string ip, std::string group)
    {
        mt.lock();
        client cl;
        for (int i = 0; i < clients_group.size(); i++)
        {
            if (clients_group[i].get_group() == group)
            {
                cl=clients_group[i].get_new_client(time, ip);
                mt.unlock();
                return cl;
            }
        }
        group_clients gr;
        gr.set_group(group);
        gr.init(server_hash);
        clients_group.push_back(std::move(gr));
        cl=clients_group[clients_group.size() - 1].get_new_client(time, ip);
        mt.unlock();
        return cl;
    }
    void exit_client(std::string hash_worker, std::string group)
    {
        mt.lock();
        int index = find_group(group);
        if (index != -1)
            clients_group[index].exit_client(hash_worker);
        mt.unlock();
    }
    std::string get_events_json(std::string group, std::string hash_worker)
    {
        mt.lock();
        std::string ev;
        //last_check_client=time(nullptr);
        time_t currentTime = time(nullptr);
        time_t del=currentTime-last_check_client;
        if(del==10){
            for(int i=0;i<clients_group.size();i++){
                clients_group[i].ping_client();
            }
            last_check_client=time(nullptr);
        }
        
        int index = find_group(group);
        std::string res=clients_group[index].get_events_json(hash_worker);
        char* ad=&res[0];
        ev=clients_group[index].get_events_json(hash_worker);
        mt.unlock();
        if (index != -1)
            return ev;
        return "{}";
    }
    t_json start_event(std::string group,std::string hash_worker, std::string event_id)
    {
        mt.lock();
        t_json res;
        res["$status"] = -1;
        int index = find_group(group);
        if (index != -1)
        {
            int s = clients_group[index].start_event(hash_worker,event_id);
            res["$status"] = s;
        }
        mt.unlock();
        return res;
    }
     t_json clear_event(std::string group,std::string hash_worker, std::string event_id)
    {
        mt.lock();
        t_json res;
        res["$status"] = -1;
        int index = find_group(group);
        if (index != -1)
        {
            int s = clients_group[index].clear_event(hash_worker,event_id);
            res["$status"] = s;
        }
        mt.unlock();
        return res;
    }
    t_json end_event(std::string group, std::string event_id)
    {
        mt.lock();
        t_json res;
        res["$status"] = -1;
        int index = find_group(group);
        if (index != -1)
        {
            int s = clients_group[index].end_event(event_id);
            res["$status"] = s;
        }
        mt.unlock();
        return res;
    }
    int add_new_event(event ev)
    {
        mt.lock();

        std::string group;
        int size_list = ev.json["meta"]["$list_servers"].size();
        std::cout << "LIST: " << ev.json["meta"]["$list_servers"].dump() << "\n";
        for (int i = 0; i < size_list; i++)
        {
            if (ev.json["meta"]["$list_servers"][i]["name"] == name_server)
            {
                int n = i;
                if (i + 1 != size_list)
                    n++;
                group = ev.json["meta"]["$list_servers"][n]["name"];
            }
        }
        int index = find_group(group);
        if (index != -1)
        {
            if (ev.json["meta"]["$type_event"] == "req")
            {
                if (!ev.json["meta"].contains("$respon_id"))
                {
                    std::string group = ev.json["meta"]["$list_servers"][0]["name"];
                    int index = find_group(group);
                    if (index == -1)
                    {
                        mt.unlock();
                        return -1;
                    }
                    respon_id r = clients_group[index].get_respon_id();
                    ev.json["meta"]["$respon_id"] = r.id;
                    ev.json["meta"]["$server_hash"] = server_hash;
                }
            }
            mt.unlock();
            return clients_group[index].add_new_event(ev, server_hash);
        }
        mt.unlock();
        return -1;
    }

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
}