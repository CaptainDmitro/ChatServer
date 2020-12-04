#include <iostream>
#include <uwebsockets/App.h>
#include <regex>
#include <map>

using namespace std;

struct UserConnection
{
    string name;
    unsigned long user_id;
};

// SET_NAME=Mike
// MESSAGE_TO=11,Hello To you, mr Ivan
// MESSAGE_TO=10,Hello To you, dear Mike

int main()
{
    int port = 8888;
    unsigned long latest_user_id = 10;
    unsigned long total_users = 0;
    map<unsigned long, string> online_users;

    uWS::App().ws<UserConnection>("/*",
        {
            .open = [&total_users, &latest_user_id, &online_users](auto* ws) {
                UserConnection* data = (UserConnection*) ws->getUserData();
                data->user_id = latest_user_id++;
                data->name = "UNNAMED";
                cout << "New user connected. ID = " << data->user_id << endl;
                cout << "Total users: " << ++total_users << endl;
                ws->subscribe("broadcast");
                ws->subscribe("user#" + to_string(data->user_id));
                //on connection open
                online_users[data->user_id] = data->name;
            },          
            .message = [&online_users](auto* ws, string_view message, uWS::OpCode opCode) {
                string SET_NAME("SET_NAME=");
                string MESSAGE_TO("MESSAGE_TO=");

                UserConnection* data = (UserConnection*)ws->getUserData();
                cout << "New message recieved: " << message << endl;
                if (message.find(SET_NAME) == 0)
                {
                    cout << "User sets their name" << endl;
                    string temp_name = (string)message.substr(SET_NAME.length());
                    if (temp_name.find(',') != string::npos)
                    {
                        cout << "',' cannot be used in name" << endl;
                        return;
                    }
                    if (temp_name.length() > 255)
                    {
                        cout << "It cannot be that big, can it?" << endl;
                        return;
                    }
                    data->name = temp_name;
                    ws->publish("broadcast", "NEW_USER=" + to_string(data->user_id) + "," + data->name);
                    online_users[data->user_id] = data->name;
                }
                if (message.find(MESSAGE_TO) == 0)
                {
                    auto rest = message.substr(MESSAGE_TO.length()); // ID, text
                    int comma_position = rest.find(',');
                    auto ID = rest.substr(0, comma_position);
                    auto text = rest.substr(comma_position + 1);
                    if (online_users.contains(std::stoi((string)ID)))
                    {
                        ws->publish("user#" + (string)ID, text);
                    }
                    else
                    {
                        ws->publish("user#" + to_string(data->user_id), "User not found");
                    }
                }
                //on new message received
            }
        }
    ).listen(port, [port](auto* token) {
                if (token) {
                    cout << "Server started successfully on port " << port << endl;
                }
                else {
                    cout << "Server failted to start" << endl;
                }
        }).run();
}