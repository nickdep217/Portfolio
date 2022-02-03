#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include "User.h"
#include "network.h"

std::string WELCOME_MESSAGE = "Hello, welcome to our CS225 final project!";
std::string EXIT_MESSAGE = "Thank you for checking out our project! Don't forget to give us an A on the project!";

void print(Network & network){
    std::cout<< network.network_string();
};

void print(User & user){
    std::cout << user.user_string();
};

void print(User * user){
    std::cout << user->user_string();
};

void print(std::string string){
    std::cout << string;
}

//code inspired by https://www.geeksforgeeks.org/converting-strings-numbers-cc/
int stringToInt(std::string input){
    std::stringstream geek(input);
 
    int x = 0;
    geek >> x;

    return x;
}

void userInterface(){

    //Welcome Message Code
    std::cout << std::endl << std::endl<< std::endl<< std::endl;
    std::cout << WELCOME_MESSAGE << std::endl;
    while(true){
        Network network;

        //Database Selection Code
        std::string dataBaseSelection = "-1";
        while(dataBaseSelection != "1" && dataBaseSelection != "2"){

            //Displays options for database selection to users
            std::cout << "Which database would you like to access?" << std::endl << std::endl;
            std::cout << "0: Exit Program" << std::endl;
            std::cout << "1: Test Database" << std::endl;
            std::cout << "2: Large Database" << std::endl << std::endl;


            //Output for database selection
            std::cin >> dataBaseSelection;
            std::cout << std::endl;
            if(dataBaseSelection == "0"){
                std::cout << EXIT_MESSAGE <<std::endl;
                return;
            }else if(dataBaseSelection == "1"){
                std::cout << "You have selected the Test Database!" << std::endl << std::endl;
                network.populate_tree("namesTestData.csv", "edgesTestData.csv", "targetTestData.csv" );
            }else if(dataBaseSelection == "2"){
                std::cout << "You have selected the Large Database!" << std::endl << std::endl;
                network.populate_tree("names.csv", "musae_git_edges.csv", "targetdata.csv" );
            }else{
                std::cout << "Could not understand input, please try again:" << std::endl << std::endl;
        }
        }

        //The meat and potatoes of the User Interface
        std::string commandSelection = "-1";
        while(commandSelection != "7"){
            std::cout << "Which command would you like to execute?" << std::endl << std::endl;


            //List of commands provided to User
            //TODO: implement centrality
            std::cout << "0: Exit Program" << std::endl;
            std::cout << "1: Print out entire database" << std::endl;
            std::cout << "2: Search up a User Id" << std::endl;
            std::cout << "3: Insert new User" << std::endl;
            std::cout << "4: Add edge to User" << std::endl;
            std::cout << "5: Find shortest path between two Users" << std::endl;
            std::cout << "6: Calculate the centrality of a User" << std::endl;
            std::cout << "7: Return to database selection" << std::endl << std::endl;


            //Selects commands
            std::cin >> commandSelection;
            std::cout << std::endl;

            //Exits Program
            if(commandSelection == "0"){
                std::cout << EXIT_MESSAGE <<std::endl;
                return;
            }

            //Prints out entire network
            else if(commandSelection == "1"){
                print(network);
                std::cout << std::endl << "Command Successful!" << std::endl;
            }
            
            //Prints out specific User
            else if(commandSelection == "2"){
                std::string selectedUserIdString = "-1";
                while(true){
                    std::cout << "Input the User Id" << std::endl << std::endl;
                    std::cin >> selectedUserIdString;
                    int selectedUserIdInt = stringToInt(selectedUserIdString);
                    std::cout << std::endl;
                    if(selectedUserIdInt >= 0 && selectedUserIdInt < network.getTotalNodes()){
                        print(network.search_by_id(selectedUserIdInt));
                        std::cout << std::endl << "Command Successful!" << std::endl <<std::endl;
                        break;
                    }else{
                        std::cout << "Invalid User Id, please try again!" <<std::endl <<std::endl;
                    }
                }
            }
            
            //Inserts new User into the Network
            else if(commandSelection == "3"){
                std::string newUserName;
                std::cout << "Input the User Name" << std::endl << std::endl;
                std::cin >> newUserName;
                std::cout << std::endl;
                network.create_user_(network.getTotalNodes(),newUserName);
                std::cout << "Here is your new user!" << std::endl;
                print(network.search_by_id(network.getTotalNodes()-1));
                std::cout << std::endl << "Command Successful!" << std::endl;
            }
            
            //Adds an edge to an existing User
            else if(commandSelection == "4"){
                std::string selectedUserIdString = "-1";
                int firstUserId = -1;
                while(true){
                    std::cout << "Input the User Id to add edge to" << std::endl << std::endl;
                    std::cin >> selectedUserIdString;
                    int selectedUserIdInt = stringToInt(selectedUserIdString);
                    std::cout << std::endl;
                    if(selectedUserIdInt >= 0 && selectedUserIdInt < network.getTotalNodes()){
                        std::cout << "User Id selected:" <<std::endl;
                        print(network.search_by_id(selectedUserIdInt));
                        firstUserId = selectedUserIdInt;
                        break;
                    }else{
                        std::cout << "Invalid User Id, please try again!" <<std::endl <<std::endl;
                    }
                }
                while(true){
                    std::cout << "Input the second User Id to create edge" << std::endl << std::endl;
                    std::cin >> selectedUserIdString;
                    int selectedUserIdInt = stringToInt(selectedUserIdString);
                    std::cout << std::endl;
                    if(selectedUserIdInt == firstUserId){
                        std::cout << "Can not make edge with self, please pick again!" <<std::endl <<std::endl;
                    }else if(selectedUserIdInt >= 0 && selectedUserIdInt < network.getTotalNodes()){
                        network.add_edge(firstUserId,selectedUserIdInt);
                        std::cout << "First User Id selected: "<< std::endl;
                        print(network.search_by_id(firstUserId));
                        std::cout << std::endl <<"Second User Id selected: "<< std::endl;
                        print(network.search_by_id(selectedUserIdInt));
                        std::cout << std::endl << "Command Successful!" << std::endl <<std::endl;
                        break;
                    }else{
                        std::cout << "Invalid User Id, please try again!" <<std::endl <<std::endl;
                    }
                }
            }
            
            //Displays the shortest path between two Users
            else if(commandSelection == "5"){
                std::string selectedUserIdString = "-1";
                int firstUserId = -1;
                while(true){
                    std::cout << "Input the first User Id" << std::endl << std::endl;
                    std::cin >> selectedUserIdString;
                    int selectedUserIdInt = stringToInt(selectedUserIdString);
                    std::cout << std::endl;
                    if(selectedUserIdInt >= 0 && selectedUserIdInt < network.getTotalNodes()){
                        std::cout << "User Id selected:" <<std::endl;
                        print(network.search_by_id(selectedUserIdInt));
                        firstUserId = selectedUserIdInt;
                        break;
                    }else{
                        std::cout << "Invalid User Id, please try again!" <<std::endl <<std::endl;
                    }
                }
                while(true){
                    std::cout << "Input the second User Id" << std::endl << std::endl;
                    std::cin >> selectedUserIdString;
                    int selectedUserIdInt = stringToInt(selectedUserIdString);
                    std::cout << std::endl;
                    if(selectedUserIdInt == firstUserId){
                        std::cout << "Can not pick same node twice, try again!" <<std::endl <<std::endl;
                    }else if(selectedUserIdInt >= 0 && selectedUserIdInt < network.getTotalNodes()){
                        std::cout << "First User Id selected: "<< std::endl;
                        print(network.search_by_id(firstUserId));
                        std::cout << std::endl <<"Second User Id selected: "<< std::endl;
                        print(network.search_by_id(selectedUserIdInt));
                        std::cout << network.shortest_path_string(network.search_by_id(firstUserId),network.search_by_id(selectedUserIdInt));
                        std::cout << std::endl << "Command Successful!" << std::endl <<std::endl;
                        break;
                    }else{
                        std::cout << "Invalid User Id, please try again!" <<std::endl <<std::endl;
                    }
                }
            }

            //Calculates centrality
            else if(commandSelection == "6"){
                std::string selectedUserIdString = "-1";
                std::string selectedDepthString = "-1";
                int firstUserId = -1;
                while(true){
                    std::cout << "Input the first User Id" << std::endl << std::endl;
                    std::cin >> selectedUserIdString;
                    int selectedUserIdInt = stringToInt(selectedUserIdString);
                    std::cout << std::endl;
                    if(selectedUserIdInt >= 0 && selectedUserIdInt < network.getTotalNodes()){
                        std::cout << "User Id selected:" <<std::endl;
                        print(network.search_by_id(selectedUserIdInt));
                        firstUserId = selectedUserIdInt;
                        break;
                    }else{
                        std::cout << "Invalid User Id, please try again!" <<std::endl <<std::endl;
                    }
                }
                while(true){
                    std::cout << "Input a depth" << std::endl << std::endl;
                    std::cin >> selectedDepthString;
                    int selectedUserIdInt = stringToInt(selectedUserIdString);
                    int selectedInt = stringToInt(selectedDepthString);
                    std::cout << std::endl;
                    if(selectedInt >= 1){
                        std::cout << "The centrality of the user is: " << network.betweeness_centrality(network.search_by_id(selectedUserIdInt),selectedInt)<<std::endl;
                        std::cout << std::endl << "Command Successful!" << std::endl <<std::endl;
                        break;
                    }else{
                        std::cout << "Invalid integer, please try again!" <<std::endl <<std::endl;
                    }
                }
            }
            
            //Returns to Database Selection
            else if(commandSelection == "7"){
                break;
            }
            
            //Used when user makes a bad command
            else{
                std::cout << "Could not understand input, please try again:" << std::endl << std::endl;
            }
        }
    }
}




int main(){
    
    userInterface();

    return 1;
};
