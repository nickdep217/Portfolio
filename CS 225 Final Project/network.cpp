#include <iostream>
#include "User.h"
#include "network.h"
#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <queue>

Network::Network(){
    id_map_ = std::unordered_map<int,User*>();
    user_map_ = std::unordered_map<User*,std::vector<bool>>();
    central_node_ = new User();
    level_ = 0;
    total_nodes_ = 0; 
}

Network::~Network(){
    if(id_map_.empty()==false){
        for (auto pair : id_map_)
        {
            User * temp = pair.second;
            pair.second = NULL;
            delete temp;
            
        }
    }


    //delete user_map_;
    //delete id_map_;
    delete central_node_;
    central_node_ = NULL;
    //update destructor with each user
}

int Network::getTotalNodes(){
    return total_nodes_;
}

void Network::populate_tree(std::string filename_target_name, std::string filename_edges, std::string filename_target_id){
    //open csv file with user info
    //Edit these files to do the right thing
    std::vector<std::string> name = read_csv_string(filename_target_name);
    std::vector<int> id_1 = read_csv_int(filename_edges, 0, 2);
    std::vector<int> id_2 = read_csv_int(filename_edges, 1, 2);
    std::vector<int> id = read_csv_int(filename_target_id, 0, 2);
    //std::cout<<"sheesh"<<std::endl;
    //while lines in csv, pass line into create node and repeat for all lines of csv
    for(unsigned long i = 0; i<id.size(); i++){
        create_user_(int(id.at(i)),name.at(i));
    }
    //when finished iterating close csv file

    //open csv file with user edges
    for(unsigned long i = 0; i<id_1.size(); i++){
        add_edge(int(id_1.at(i)), int(id_2.at(i)));
    }

    //while there are still edges get a new line
    //add_edge(13939,383839);
    return;
}

User * Network::search_by_id(int id){
    std::unordered_map<int,User*>::const_iterator found = id_map_.find(id);
    if(found==id_map_.end()){ //checks if the user id is not found
        return NULL;
    }else{
        return found->second; //pointer to user
    }
}

void Network::create_user_(int id, std::string username){
    User * newUser = new User(id,username,this->central_node_);
    std::pair<int,User*> newPair(id,newUser); //creates pair newPair
    std::pair<User*,std::vector<bool>> userPair(newUser,std::vector<bool>()); // creates user_map_ 
    user_map_.insert(userPair);
    id_map_.insert(newPair);
    total_nodes_+=1;
}

int Network::add_edge(int id_1, int id_2){

    User* User1= search_by_id(id_1); 
    User* User2= search_by_id(id_2); 

    if(User1 == NULL || User2 == NULL){
        return -1;
    }else{

        User1->add_connection(User2);
        User2->add_connection(User1);

        return 1;
    }

    //if either node cannot be found, return -1
   
}

bool Network::was_visited(User* user, int level){

    std::vector<bool> & Vector = user_map_[user];
    if(Vector.empty()){
        return false;
    }else if((int(Vector.size())-1)<level){
        return false;
    }else if(Vector[level]==true){
        return true;
    }else{
        return false;
    }
}

void Network::new_visit(User * user, int level){

    /*cases:
    1. Vector dont exist
    2. Vector exists for levels smaller than current level
    3. Vector already exists for level
    */
    std::vector<bool> & Vector = (user_map_[user]);
    /*
    if(Vector.empty()==true){
        for(int i=0; i<level; i++){
            Vector.push_back(false);
        }
        Vector.push_back(true);
    }
    else if(int(Vector.size())<level){
        for(int i=int(Vector.size()); i<level; i++){
            Vector.push_back(false);
        }
        Vector.push_back(true);
    }
    else{
        Vector[level] = true;
    }
    */
   if(int(Vector.size()-1)<level){
       for(int i = int(int(Vector.size())-1); i <= level ;i++){
           Vector.push_back(false);
       }
       Vector[level] = true; 
   }else{
       Vector[level] = true;
   }

}

std::vector<User*> Network::BFS_username(std::string query){

  User* start = get_random_node();
  if(start==NULL){
      return std::vector<User*>();
  }
  User* current_node;
  std::vector<User*> result;
  //a queue to store references to nodes we should visit later
  std::queue<User*> q; 
  q.push(start);
  new_visit(start, level_);

  while(q.empty()==false){
    current_node = q.front();
    q.pop();

    //process currentnode Ex: # for example, print(current_node.value)
        //check if string matches given node
        if(name_check(query, current_node)==true){
            result.push_back(current_node);
        }
        
    for(int i=0; i < current_node->num_connections(); i++)
    {
      if(was_visited((current_node->get_connections())[i] , level_) == false)
      {
        q.push((current_node->get_connections())[i]);

        new_visit( (current_node->get_connections())[i] , level_);
      }
    }
  }
    level_++; //increase level_ (dont touch)
    return result;
}

std::string Network::BFS_username_string(std::string query){
    return vector_to_string(BFS_username(query));
}

bool Network::name_check(std::string query, User* current_node){

    bool result;
    for(unsigned long i=0; i<std::min(query.size(),current_node->get_username().size()); i++){
        if((current_node->get_username())[i] == query[i]){
            result = true;
        }else{
            return false;
        }
    
    }
    return result;
}

std::string Network::vector_to_string(std::vector<User*> v){

    std::string result = "";
    for(unsigned long i=0; i<v.size(); i++){
        if(v[i]!=NULL){
            result += v[i]->user_string();
        }else{
            result += "NuLL \n";
        }
    }
    return result;
}

User* Network::get_random_node(){

   if(id_map_.empty()==true){
        return NULL;
    }
    
    auto iter = id_map_.begin();
    return iter->second;
}
    
std::vector<User*> Network::shortest_path(User * user1, User * user2){
    std::unordered_map<User*,int> distance_map;
    std::unordered_map<User*,User*> previos; //Syntax <*current_user,*previos_user>
    auto compare = [](std::pair<User*,int> a,std::pair<User*,int> b) { return a.second < b.second; }; //create comparator
    std::priority_queue<std::pair<User*,int>,std::vector<std::pair<User*,int>>,decltype(compare)> p_q(compare); //creates priority queue
    //visited already initialized

    //check for improper parameters
    if(user1==NULL || user2==NULL) return std::vector<User*>();
    if(user1==user2) return std::vector<User*>();
    if(user1->get_connections().empty() || user2->get_connections().empty()) return std::vector<User*>();
    

    //create user and add to priority queue
    std::pair<User*,int> newPair(user1,0);
    previos[user1] = NULL; //push null to the previos for starting node
    p_q.push(newPair); 

    //variables for while loop
    std::pair<User*,int> pair; 
    User * curr_user;
    int distance;
    bool all_visited = true;
    
    while((p_q.empty()!= true) && (p_q.top().first!=user2)){//figure out how to check if the cluseter has been visited (all neighbors visited)
        pair = p_q.top(); 
        p_q.pop();
        curr_user = pair.first;
        distance = pair.second;
        all_visited = true;
        for(int i = 0; i < curr_user->num_connections(); i++){
            User * curr_neighbor = curr_user->get_connection(i);
            if(was_visited(curr_neighbor,level_)==false){
                //update neighbor distances;
                //check if there has already been a distance found for node, if not create pair and distance
                bool updated = false;
                if(distance_map.find(curr_neighbor)==distance_map.end()){
                    std::pair<User*,int> newPair(user1,distance+1);
                    distance_map.insert(newPair);
                    updated = true;
                }else{
                    if(distance_map[curr_neighbor]>distance+1){
                        distance_map[curr_neighbor] = distance + 1;
                        updated = true;
                    }
                }
                //if new distance is less, update current path
                if(updated){
                    previos[curr_neighbor] = curr_user;
                    p_q.push(std::pair<User*,int>(curr_neighbor, distance_map[curr_neighbor]));
                }

                all_visited = false;
            }  
            //check to see if all of the nodes where visited and the queue is empty
        }
        if(all_visited && p_q.empty()){ //node was not found
            level_++;
            return std::vector<User*>();
        }
        new_visit(curr_user,level_);
    }

    //if user is found extract path from previos
    if(p_q.top().first==user2){
        std::vector<User*> result;
        curr_user = user2;
        while(curr_user!=NULL){
            result.push_back(curr_user);
            curr_user = previos[curr_user];
        }
        level_++;
        return result;
    }
    //return empty vector
    level_++; //increment level comeback:make sure to increment any other return statements
    return std::vector<User*>();
}

std::string Network::shortest_path_string(User * user1, User * user2){
    std::string result = "";
    std::vector<User*> path = shortest_path(user1,user2);
    //Added case where there is no path;
    if(path.size()==0){
        result += "There are no path between the two!";
    }else{
        result += "Distance: " + std::to_string(path.size()) + "\n";
        result += "Path: ";
    }
    for(unsigned long i = 0; i < path.size(); i++){
        result+= path[i]->get_username();
        if(i!=(path.size()-1)){
            result+= " <-> ";
        }
    }
    return result;
}

void Network::network_betweeness_centrality(int depth){
    for(auto user: id_map_){
        betweeness_centrality(user.second,depth);
    }
}


int Network::betweeness_centrality(User * user,int depth){
    std::vector<User*> users = get_connection_level(user,depth);
    int thru_paths = 0;
    int total_paths = 0; 
    for(int x = 0; x < int(users.size());x++){
        for(int y = x+1; y < int(users.size());y++){//can optomize which users are iterated through so iteration does not happen twice
            std::pair<int, int> result = paths_through_node(users[x],users[y],user);
            thru_paths += result.first;
            total_paths += result.second;
        }
    }
    //make case to check if 0 for total_paths or thru paths
    //multiply by normalization connstance
    double normalization_const = ((1.0/double(total_nodes_-2))*(1.0/double(total_nodes_-1))*10000*(2.0));
    user->set_centrality(int((double(thru_paths)/double(total_paths))*normalization_const)); //sets centrality of user
    return int((double(thru_paths)/double(total_paths))*normalization_const);
    //int(normalization_const*(thru_paths/total_paths)*1000);
}

std::pair<int, int> Network::paths_through_node(User * start,User * end, User * central){
    //start
    std::unordered_map<User*,int> distance_map;
    std::unordered_map<User*,std::vector<User*>> previos; //Syntax <*current_user,*previos_user>
    std::queue<User*> q;
    User * curr_user;
    User * neighbor;

    //check for improper parameters
    if(start==NULL || end==NULL) return std::pair<int, int>(0,0);
    if(start==end) return std::pair<int, int>(0,0);
    if(start->get_connections().empty() || end->get_connections().empty()) return std::pair<int, int>();

    //initialize first element
    q.push(start);
    previos[start].push_back(NULL);
    std::pair<User*,int> newPair(start,0);
    distance_map.insert(newPair);

    while (!q.empty()){
        User * curr_user = q.front();
        q.pop();
        for(int i = 0; i < curr_user->num_connections();i++){
            neighbor = curr_user->get_connection(i);
            if(distance_map.find(neighbor)==distance_map.end()){//check if the neighbor does not have a distance
                distance_map[neighbor] = distance_map[curr_user] + 1;
                q.push(neighbor);
                previos[neighbor].clear();
                previos[neighbor].push_back(curr_user);
            }
            else if(distance_map[neighbor] > distance_map[curr_user] +1){
                distance_map[neighbor] = distance_map[curr_user] + 1;
                q.push(neighbor);
                previos[neighbor].clear();
                previos[neighbor].push_back(curr_user);
            }else if(distance_map[neighbor] == distance_map[curr_user] + 1){
                previos[neighbor].push_back(curr_user);
            }
        }
    }

    //backtracing
    //std::pair<User*,int> newPair(user1,0);
    std::vector<int> result = backtrace_(previos,end,central);
    return std::make_pair(result[0],result[1]);
}


std::vector<int> Network::backtrace_(std::unordered_map<User*,std::vector<User*>> & previos, User * curr_user, User * central){
    std::vector<int> result = {0,0,0};

    if (curr_user == NULL){
        result[0] = 0; //paths that go through the node
        result[1] = 1; //total paths
        return result;
    }

    for(int i = 0; i < int(previos[curr_user].size());i++){
        std::vector<int> temp = backtrace_(previos,previos[curr_user].at(i),central);
        if(curr_user == central){
            result[0] +=temp[1];
            result[2] = 1;
        }
        else if((temp[2] == 1)||(curr_user==central)){
            result[0] += temp[0];
            result[2]  = 1; // set that the node has been visited
        }
        result[1] += temp[1];
    }
    return result;
}



std::vector<User*> Network::get_connection_level(User * user, int depth){
    std::vector<User*> users;
    std::queue<User*> q;
    int level_num = 1;
    int level_count = 1;
    User * curr_user = user;
    q.push(curr_user);
    new_visit(curr_user,level_);

    for(int x = 0; x<depth ;x++){
        for(int y = 0; y < level_count; y++){
            //add a users connections
            //comback and check if q is empty
            curr_user = q.front();
            q.pop();
            level_num = 0; 
            for(int i = 0; i < curr_user->num_connections();i++){
                if(was_visited(curr_user->get_connection(i),level_)==false){
                    users.push_back(curr_user->get_connection(i));
                    q.push(curr_user->get_connection(i));
                    new_visit(curr_user->get_connection(i),level_);
                    level_num +=1;
                }
            }
        }
        level_count = level_num;
        if(q.empty()){
            level_++;
            return users;
        } 
    }
    level_++;
    return users;
}

std::string Network::get_connection_level_string(User * user, int depth){
    return vector_to_string(get_connection_level(user,depth));
}


//code inspired by https://www.gormanalysis.com/blog/reading-and-writing-csv-files-with-cpp/
std::vector<int> Network::read_csv_int(std::string filename, int columnIndex, int totalColumns){
    std::vector<int> result;
    // Create an input filestream
    std::ifstream myFile(filename);

    // Make sure the file is open
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");

    // Helper vars
    std::string line;
    int val;

    // Read data, line by line
    while(std::getline(myFile, line))
    {
        // Create a stringstream of the current line
        std::stringstream ss(line);
        
        int colIdx = 0;
        // Extract each integer
        while(ss >> val){

            
            if(colIdx == columnIndex){
            // Add the current integer to the 'colIdx' column's values vector
            result.push_back(val);
            
            }
            
            // If the next token is a comma, ignore it and move on
            if(ss.peek() == ',') ss.ignore();

            if(colIdx == totalColumns - 1){
                colIdx = 0;
                break;
            }

            // Increment the column index
            colIdx++;
        }
    }

    // Close file
    myFile.close();

    //std::cout<< filename << " is done!" << std::endl;

    return result;


}

//code inspired by https://www.gormanalysis.com/blog/reading-and-writing-csv-files-with-cpp/
std::vector<std::string> Network::read_csv_string(std::string filename){
    std::vector<std::string> result;
    // Create an input filestream
    std::ifstream myFile(filename);

    // Make sure the file is open
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");

    // Helper vars
    std::string line;
    std::string getstring;

    // Read data, line by line
    while(std::getline(myFile, line))
    {
        result.push_back(line);
    }

    // Close file
    myFile.close();

    //std::cout<< filename << " is done!" << std::endl;

    return result;
}

std::string Network::network_string(){
    std::string str = "NETWORK \n";
    for(auto entry: id_map_){
        //str+= user_to_string(entry.second);
        str += entry.second->user_string();
    }
    return str;
}

