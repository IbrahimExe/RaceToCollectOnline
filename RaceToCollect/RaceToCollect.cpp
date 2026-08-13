// RaceToCollect.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
        2 Player Race to Collect

Goal is to create a console application that can play a simple Sever Controlled 2
player Race To Collect Game.

Create a display that shows the board in the console.
- Display the boundaries of the board ("|" on the left/ right side is enough)
- Use "1" to represent Player 1.
- Use "2" to represent Player 2.
- Use "0" for the collect item.
- Can make more detailed if desired.
- Use system("cls") to rebuild the board each frame.

Create a server that will manage the game.
- Listen for 2 clients to connect.
- As clients press inputs, the server updates the board
- Sends updated board data to the clients

Create a client:
- Connects to the server
- Sends inputs to the server
- Updates the board when there is a response from the server

Main Flow:
- Create a server that waits for 2 clients to connect
- Create a client that connects to the server
- Players can move independently to a pickup item
- If a player picks up the item, they get a point
- First player to 15 points wins the game
- Show who won and allow the players to restart
- If both players choose Restart, play again, otherwise exit the game

Bonus:
- Enable up to 4 players

*/

#include <iostream>
#include <string>
#include <conio.h>   // _kbhit() / _getch()
#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Game Configurations
const int BOARD_WIDTH = 30;
const int BOARD_HEIGHT = 15;
const int WIN_SCORE = 15;

// Network Configurations 
const int PORT = 54000;
const string SERVER_IP = "127.0.0.1";

// Data Structures 
struct GameState 
{
    int p1X = 2, p1Y = 7;
    int p2X = 27, p2Y = 7;
    int itemX = 15, itemY = 7;
    int p1Score = 0;
    int p2Score = 0;
    bool gameOver = false;
    int winner = 0;
};

// Rendering 
void DrawBoard(const GameState& state) 
{
    system("cls");

    cout << "P1 Score: " << state.p1Score << "  |  P2 Score: " << state.p2Score << "\n";
    cout << string(BOARD_WIDTH + 2, '-') << "\n";

    for (int y = 0; y < BOARD_HEIGHT; y++) 
    {
        cout << "|";
        for (int x = 0; x < BOARD_WIDTH; x++) 
        {
            if (x == state.p1X && y == state.p1Y) cout << "1";
            else if (x == state.p2X && y == state.p2Y) cout << "2";
            else if (x == state.itemX && y == state.itemY) cout << "0";
            else cout << " ";
        }
        cout << "|\n";
    }

    cout << string(BOARD_WIDTH + 2, '-') << "\n";
}

// Networking Helpers 
bool InitializeWinsock() 
{
    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    int wsResult = WSAStartup(version, &data);
    if (wsResult != 0) 
    {
        cout << "Can't start Winsock, Err #" << wsResult << "\n";
        return false;
    }
    return true;
}

void RunServer() 
{
    if (!InitializeWinsock())
    {
        return;
    }

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) 
    {
        cout << "Can't create a socket! Quitting\n";
        return;
    }

    // bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // tell Winsock the socket is for listening 
    listen(listening, SOMAXCONN);

    cout << "Server listening on port " << PORT << "...\nWaiting for Player 1...\n";

    // Wait for a connection
    sockaddr_in client1Hint;
    int client1Size = sizeof(client1Hint);
    SOCKET client1 = accept(listening, (sockaddr*)&client1Hint, &client1Size);
    cout << "Player 1 connected!\nWaiting for Player 2...\n";

    sockaddr_in client2Hint;
    int client2Size = sizeof(client2Hint);
    SOCKET client2 = accept(listening, (sockaddr*)&client2Hint, &client2Size);
    cout << "Player 2 connected!\n\nAll players ready! (Game loop coming in next commit)\n";

    closesocket(client1);
    closesocket(client2);
    closesocket(listening);
    WSACleanup();
}

void RunClient() 
{
    if (!InitializeWinsock())
    {
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) 
    {
        cout << "Can't create socket, Err #" << WSAGetLastError() << "\n";
        return;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &hint.sin_addr);

    // Connect to server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) 
    {
        cout << "Can't connect to server, Err #" << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return;
    }

    cout << "Connected to server!\nWaiting for game to start...\n";


    closesocket(sock);
    WSACleanup();
}

int main() 
{
    cout << "Race To Collect!\n";
    cout << "Select Mode:\n1. Server\n2. Client\nChoice: ";
    int choice;
    cin >> choice;

    if (choice == 1) 
    {
        cout << "\nStarting Server...\n";
        RunServer();
    }
    else if (choice == 2) 
    {
        cout << "\nStarting Client...\n";
        RunClient();
    }
    else 
    {
        cout << "Invalid choice. Exiting.\n";
    }

    system("pause"); // Keep terminal open to see results
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
