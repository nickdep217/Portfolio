#pragma once
#include <iostream> 
#include <string>




class User{
    public:
    User();
    User(int id);
    User(int id, std::string username, User * central_node);
    ~User();

    //adds connection to instance of User
    void add_connection(User * user);

    const std::string get_id();
    const std::string get_username();
    const std::string get_centrality();
    User * get_connection(int index);
    std::vector<User*> get_connections();
    int num_connections();
    std::string user_string();
    void set_centrality(int centrality);
    

    private:
    int id_; 
    std::string username_;
    std::vector<User*> * connections_; 
    User * central_node_;
    int betweeness_centrality_; //0-10,000 (-1 if none found)
};