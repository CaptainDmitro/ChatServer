#include <iostream>
#include <uwebsockets/App.h>
#include <regex>

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

    uWS::App().ws<UserConnection>("/*",
        {
            .open = [&latest_user_id](auto* ws) {
                UserConnection* data = (UserConnection*) ws->getUserData();
                data->user_id = latest_user_id++;
                data->name = "UNNAMED";
                cout << "New user connected. ID = " << data->user_id << endl;
                ws->subscribe("broadcast");
                ws->subscribe("user#" + to_string(data->user_id));
                //on connection open
            },          
            .message = [](auto* ws, string_view message, uWS::OpCode opCode) {
                string SET_NAME("SET_NAME=");
                string MESSAGE_TO("MESSAGE_TO=");

                UserConnection* data = (UserConnection*)ws->getUserData();
                cout << "New messgae recieved: " << message << endl;
                if (message.find(SET_NAME) == 0)
                {
                    cout << "User sets their name" << endl;
                    data->name = (string)message.substr(SET_NAME.length());
                }
                if (message.find(MESSAGE_TO) == 0)
                {
                    auto rest = message.substr(MESSAGE_TO.length()); // ID, text
                    int comma_position = rest.find(',');
                    auto ID = rest.substr(0, comma_position);
                    auto text = rest.substr(comma_position + 1);
                    ws->publish("user#" + (string)ID, text);
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