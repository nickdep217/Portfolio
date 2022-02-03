#include "User.h"
#include <iostream>
#include <vector>


    //Default constructor
    User::User(){
        //nothing
        id_ = -1;
        username_ = "";
        central_node_ = NULL;
        connections_ = new std::vector<User*>();
        betweeness_centrality_ = -1;
    }


    //other constructor
    User::User(int id){
        id_ = id;
        //assign null value for username and empty value for username
        username_ = "";
        central_node_ = NULL;
        connections_ = new std::vector<User*>();
        betweeness_centrality_ = -1;
    }


    //even another constructor
    User::User(int id, std::string username, User * central_node){

        id_ = id;
        username_ = username;
        central_node_ = central_node;
        connections_ = new std::vector<User*>();
        betweeness_centrality_ = -1;
    }


    User::~User(){
        connections_->clear();
        delete connections_;
    }


    void User::add_connection(User * user){

        //makes sure it is not adding a connection to itself
        if(this!=user){
            //makes sure connection has not already been added
            for(int i = 0; i < int(connections_->size());i++){
                if((*connections_)[i]==user){
                    return;
                }
            }
            connections_->push_back(user);
        }
        return;

    }

    const std::string User::get_id(){
        return std::to_string(id_);
    }


    const std::string User::get_username(){
        return username_;
    }


    std::vector<User*> User::get_connections(){
        return *connections_;
    }

    User * User::get_connection(int index){
        if((index<0)||(int(connections_->size())<index)){
            return NULL;
        }else{
            return connections_->at(index);
        }
    }


    int User::num_connections(){
        return connections_->size();
    }

    void User::set_centrality(int centrality){
        if((centrality<10000)&&(centrality>=0)){
            betweeness_centrality_ = centrality;
        }
        return;
    }

    const std::string User::get_centrality(){
        return std::to_string(betweeness_centrality_);
    }




    std::string User::user_string(){
        std::string str = "Username: " + get_username() + "\n";
        str += "ID: " + get_id() + "\n" + "Connections: " + "\n";
        if(betweeness_centrality_!=-1){
            str += "Betweeness Centrality: " + get_centrality()+ "\n";
        }
        for(int i = 0; i < int(connections_->size());i++){
            str+= "   "+(*connections_)[i]->get_id() + "\n";
        }
        return str;
    }
