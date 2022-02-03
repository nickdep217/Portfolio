#pragma once
#include <iostream> 
#include <string>
#include <unordered_map>
#include <vector>
#include "User.h"



class Network{
    public:

    Network();
    ~Network();

    //PART #1
    //THIS SECTION IS FOR READING IN FILES

    /***
     * takes a user ID and returns the location of the node.
     * Use unordered map to implement
     * @return the address of the node
     * Null if address not found
     */
    User * search_by_id(int id);
    
    /***
     * populates the network using the data from the csv files
     * 1. read through all of the users and create a node for each of them (User constructor with attributes)
     * 2. store each node in a std::unordered_map<int,*User> (int is the user ID user is a pointer to the user)
     * 3. go through the csv file with all of the edges and add each edge to each node
     * using the unordered_map
     * Note: figure out how to handle seperate clusers
     */
    void populate_tree(std::string filename_target_name, std::string filename_edges, std::string filename_target_id);

    /***
     * helper function for step #2 in populate_tree()
     * 1.create a node with User constructor
     * 2. add the User to id_map_
     */
    void create_user_(int id, std::string username);

    /***
     * helper function for step #3 in populate_tree()
     * use id_map_ to update both users connections with the edge
     * return 1 if successfull and node existed
     * return -1 if either node could not be found
    */
    int add_edge(int id_1, int id_2);




    //USER SEARCH SECTION

   /***
     * helper function for BFS_username
     * returns if node was visited
     * 
     */
    bool was_visited(User * user, int level);

    /***
     * helper function for BFS_username
     * marks a node as visited
     * 
     */
    void new_visit(User * user, int level);


    /***
     * given a starting node and a string the function will BFS
     * through the tree and finds all users that have those letters in order in
     * the tree and return a list of the usernames in a list
     * use the user_map_ to implement the visited feature
     * Algorithm Here: https://www.geeksforgeeks.org/breadth-first-search-or-bfs-for-a-graph/
     */
    std::vector<User*> BFS_username(std::string query);

    //returns string of users found in BTS username
    std::string BFS_username_string(std::string query);

    //helper function for BFS_username to check if string matches username in usermap
    bool name_check(std::string query, User* current_node);

    //turns vector of users into a string
    std::string vector_to_string(std::vector<User*> v);

    //Returns random node
    User* get_random_node();

 
    /***
     * takes the address of two connections
     * return int with the shortest path between the two connections return -1 if not connected at all
     * Note: figure out if it would be possible to return the shortest path
     * Dijkstra(Graph, source, destination):
     * PSUEDOCODE
      initialize distances  // initialize tentative distance value
      initialize previous   // initialize a map that maps current node -> its previous node
      initialize priority_queue   // initialize the priority queue
      initialize visited

      while the top of priority_queue is not destination:
          get the current_node from priority_queue
          for neighbor in current_node's neighbors and not in visited:
              if update its neighbor's distances:
                  previous[neighbor] = current_node
          save current_node into visited

      extract path from previous
      return path and distance
      return path comeback and change
     */ 

    std::vector<User*> shortest_path(User * user1, User * user2);

    std::string shortest_path_string(User * user1, User * user2);

    std::vector<int> read_csv_int(std::string filename, int columnIndex, int totalColumns);

    std::vector<std::string> read_csv_string(std::string filename);

    int getTotalNodes();

    //Part #2
    
    //turns network into string
    std::string network_string();

    //calculates the betweeness centrality of a user
    int betweeness_centrality(User * user, int depth);

    //BFS traversal of graph starting at user, with specified depth
    std::vector<User*> get_connection_level(User * user, int depth);

    //String version of get_connection_level
    std::string get_connection_level_string(User * user, int depth);

    /***
     * helpter function for betweeness centrality
     * returns #shortest paths that pass through central 
     * and #shortest paths total, respectively in a pair
     * psuedocode found at: https://www.geeksforgeeks.org/print-all-shortest-paths-between-given-source-and-destination-in-an-undirected-graph/
     */
    std::pair<int, int> paths_through_node(User * start,User * end, User * central);


    /***
     *helper function for paths through node
     *psuedocode found at: https://www.geeksforgeeks.org/print-all-shortest-paths-between-given-source-and-destination-in-an-undirected-graph/
     */
    //[thru paths, paths, seen(1 or 0)]
    std::vector<int> backtrace_(std::unordered_map<User*,std::vector<User*>> & previos, User * curr_user, User * central);

    /***
     * updates betweeness centrality of all of the nodes in the network
     * not optomized for time complexity, 
     * not one of the promised functions
     */
    void network_betweeness_centrality(int depth);
    

    private:
    std::unordered_map<int,User*> id_map_;
    std::unordered_map<User*,std::vector<bool>> user_map_;
    int level_;
    int total_nodes_;
    User * central_node_;

    




};