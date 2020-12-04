#include <iostream>
#include <uwebsockets/App.h>

using namespace std;

struct UserConnection
{
    string name;
    unsigned long user_id;
};

int main()
{
    int port = 8888;
    unsigned long latest_user_id = 10;
    uWS::App().ws<UserConnection>("/*",
        {
            .open = [&latest_user_id](auto* ws) {
                UserConnection* data = (UserConnection*) ws->getUserData();
                data->user_id = latest_user_id++;
                cout << "New user connected. ID = " << data->user_id << endl;
                //on connection open
            },          
            .message = [](auto* ws, string_view message, uWS::OpCode opCode) {
                cout << "New messgae recieved: " << message << endl;
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