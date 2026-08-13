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

bool InitializeWinsock()
{
    WSADATA data;
    WORD version = MAKEWORD(2, 2);
    return WSAStartup(version, &data) == 0;
}

// set sockets to non-blocking so the game doesn't freeze waiting for inputs
void SetNonBlocking(SOCKET sock)
{
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
}

void SpawnItem(GameState* state)
{
    state->itemX = rand() % BOARD_WIDTH;
    state->itemY = rand() % BOARD_HEIGHT;
}

void ProcessInput(char input, int* x, int* y)
{
    if (input == 'w' && *y > 0) (*y)--;
    if (input == 's' && *y < BOARD_HEIGHT - 1) (*y)++;
    if (input == 'a' && *x > 0) (*x)--;
    if (input == 'd' && *x < BOARD_WIDTH - 1) (*x)++;
}

// bind the ip address and port to a socket
void RunServer()
{
    if (!InitializeWinsock()) return;

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));
    listen(listening, SOMAXCONN);

    cout << "Waiting for players...\n";
    SOCKET client1 = accept(listening, nullptr, nullptr);
    cout << "Player 1 connected!\n";
    SOCKET client2 = accept(listening, nullptr, nullptr);
    cout << "Player 2 connected!\n";

    SetNonBlocking(client1);
    SetNonBlocking(client2);

    GameState state;
    srand(GetTickCount());

    bool quit = false;
    bool p1Ready = false;
    bool p2Ready = false;

    while (!quit)
    {
        char p1Input = 0;
        char p2Input = 0;

        recv(client1, &p1Input, sizeof(p1Input), 0);
        recv(client2, &p2Input, sizeof(p2Input), 0);

        if (!state.gameOver)
        {
            if (p1Input != 0) ProcessInput(p1Input, &state.p1X, &state.p1Y);
            if (p2Input != 0) ProcessInput(p2Input, &state.p2X, &state.p2Y);

            if (state.p1X == state.itemX && state.p1Y == state.itemY)
            {
                state.p1Score++;
                SpawnItem(&state);
            }
            else if (state.p2X == state.itemX && state.p2Y == state.itemY)
            {
                state.p2Score++;
                SpawnItem(&state);
            }

            // win check
            if (state.p1Score >= WIN_SCORE)
            {
                state.gameOver = true;
                state.winner = 1;
            }
            else if (state.p2Score >= WIN_SCORE)
            {
                state.gameOver = true;
                state.winner = 2;
            }
        }
        else
        {
            if (p1Input == 'r') p1Ready = true;
            if (p2Input == 'r') p2Ready = true;
            if (p1Input == 'q' || p2Input == 'q') quit = true;

            // restart game loop
            if (p1Ready && p2Ready)
            {
                state.p1Score = 0;
                state.p2Score = 0;
                state.p1X = 2; state.p1Y = 7;
                state.p2X = 27; state.p2Y = 7;
                state.gameOver = false;
                state.winner = 0;
                SpawnItem(&state);

                p1Ready = false;
                p2Ready = false;
            }
        }

        send(client1, (char*)&state, sizeof(GameState), 0);
        send(client2, (char*)&state, sizeof(GameState), 0);

        Sleep(30);
    }

    closesocket(client1);
    closesocket(client2);
    closesocket(listening);
    WSACleanup();
}

void RunClient()
{
    if (!InitializeWinsock()) return;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &hint.sin_addr);

    connect(sock, (sockaddr*)&hint, sizeof(hint));
    SetNonBlocking(sock);

    GameState state;
    bool localGameOver = false;

    while (true)
    {
        if (_kbhit())
        {
            char input = _getch();
            send(sock, &input, sizeof(input), 0);
        }

        int bytesIn = recv(sock, (char*)&state, sizeof(GameState), 0);

        // break out if the server drops connection or quits
        if (bytesIn == 0 || (bytesIn == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK))
        {
            cout << "\nDisconnected from server.\n";
            break;
        }

        if (bytesIn > 0)
        {
            if (!state.gameOver)
            {
                localGameOver = false;
                DrawBoard(state);
            }
            else if (!localGameOver)
            {
                localGameOver = true;
                DrawBoard(state);
                cout << "\nPlayer " << state.winner << " wins!\n";
                cout << "Press 'r' to restart or 'q' to quit.\n";
            }
        }

        Sleep(10);
    }

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
