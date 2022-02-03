Final Project Proposal (Due April 9th)


Leading Question
For our project we will be using the github-musue database. The leading question for our project is how could we provide additional data that could help a user examine their position in the social network?  To do this we will implement the backend of a few features that could help the users if this was a social media network. The features that we will be implementing include:
1. User Search: user will be able to search for a username and will be given the users information if the user exists. To implement this we will use a Breadth First Search algorithm. This will help users to find other users' information.
2. Mutual Connection Finder: We will implement an algorithm that finds if and how two users are connected in the graph(shortest path). When two users are connected, the program will return a list of the people that connect them in the shortest path. To implement this feature we will use  Dijkstra’s Algorithm. This could be useful to users as it could help them figure out who they need to talk to to connect with a targeted user.
3.  Centrality Ranker: The last feature we will implement is a centrality ranker. This will rank users based off of their centrality and how many people they connect. This could be helpful to users as it could help users identify who it might be good to connect with to increase their reach in the network.



Dataset Acquisition and Processing

Our data is downloaded from the Stanford Large Network Dataset. The files are in the CSV format with all of the edges in one file, and user attributes in the other file. We will use some CSV library to read in the data and to turn each user ID into a node. Each node will have information including the username of each user as well as which other nodes they are connected to. We will use this undirected graph to implement a user search feature, mutual connection finder and centrality ranker. As there are going to be some nodes that are not connected to other nodes, we will also have a central node that all nodes will be connected to, which will be used for the breadth first search but not the mutual connection finder and centrality ranker. Before we upload our data we will find discrepancies in the data and try to fix them by deleting these data points.




Graph Algorithms

User Search: We will implement a depth first search, with a goal time complexity of O(n). the input for this method will be a user ID and, the output of this function will be a pointer to the user node with all of their information. If that user ID does not exist the function will return NULL. We may put this function inside another function to print it in a cleaner way with user data.
Mutual Connection Finder: We will implement a mutual connection finder which will use Dijkstra’s algorithm which has time complexity of, O(|E|+|V|log|V|) worst case. The inputs for this function will be the two nodes that we are trying to find the shortest path between. The output will be a list of pointers to the nodes connecting them, which is the shortest path between the two. If there is no way to get from one node to the other it will return NULL. This function may be placed within another function to print the connections in a cleaner way.
Centrality Ranker: We will implement a betweenness centrality algorithm to rank a person's connectedness to the network the time complexity of this algorithm will be O(|V|^3) worst case. For this algorithm the inputs will be a node that you want to find the betweenness centrality of a node. This will be a void function but the function will reassign the given nodes attribute for betweenness centrality.


Timeline
Before the Mid-Project Check In we will:

1. Outline the classes and main functions that we will need to create our project
2. Figure out how to transform the given CSV files into nodes with attributes
3. Write helper functions for user_search function
4. Implement user_search function
5. Write tests for user_search function
6. Write helper functions for mutual_connection finder function
7. Implement mutual finder function


After Mid-Project Check in:
1. Write tests for the mutual finder function
2. Write helper functions and pseudocode for the centrality ranker
3. Implement Centrality ranker
4. Test functions
5. Write functions to print out function outputs
6. Write main function/user interface
