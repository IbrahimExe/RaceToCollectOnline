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
const int MAX_PLAYERS = 4;

// Network Configurations 
const int PORT = 54000;
const string SERVER_IP = "127.0.0.1";

// Data Structures 
struct GameState
{
    int pX[MAX_PLAYERS] = { 2, 27, 2, 27 };
    int pY[MAX_PLAYERS] = { 2, 2, 12, 12 };
    int pScore[MAX_PLAYERS] = { 0, 0, 0, 0 };
    int itemX = 15, itemY = 7;
    bool gameOver = false;
    int winner = 0;
};

// Rendering 
void DrawBoard(const GameState& state)
{
    system("cls");

    cout << "P1: " << state.pScore[0] << " | P2: " << state.pScore[1]
        << " | P3: " << state.pScore[2] << " | P4: " << state.pScore[3] << "\n";
    cout << string(BOARD_WIDTH + 2, '-') << "\n";

    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        cout << "|";
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            bool playerDrawn = false;
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (x == state.pX[i] && y == state.pY[i])
                {
                    cout << (i + 1);
                    playerDrawn = true;
                    break;
                }
            }

            if (!playerDrawn)
            {
                if (x == state.itemX && y == state.itemY) cout << "0";
                else cout << " ";
            }
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

    SOCKET clients[MAX_PLAYERS];
    cout << "Waiting for " << MAX_PLAYERS << " players...\n";

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        clients[i] = accept(listening, nullptr, nullptr);
        cout << "Player " << (i + 1) << " connected!\n";
        SetNonBlocking(clients[i]);
    }

    GameState state;
    srand(GetTickCount());

    bool quit = false;
    bool pReady[MAX_PLAYERS] = { false };

    while (!quit)
    {
        char inputs[MAX_PLAYERS] = { 0 };
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            recv(clients[i], &inputs[i], sizeof(char), 0);
        }

        if (!state.gameOver)
        {
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (inputs[i] != 0)
                {
                    ProcessInput(inputs[i], &state.pX[i], &state.pY[i]);
                }

                if (state.pX[i] == state.itemX && state.pY[i] == state.itemY)
                {
                    state.pScore[i]++;
                    SpawnItem(&state);
                }

                if (state.pScore[i] >= WIN_SCORE)
                {
                    state.gameOver = true;
                    state.winner = i + 1;
                }
            }
        }
        else
        {
            int readyCount = 0;
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (inputs[i] == 'r') pReady[i] = true;
                if (inputs[i] == 'q') quit = true;
                if (pReady[i]) readyCount++;
            }

            if (readyCount == MAX_PLAYERS)
            {
                for (int i = 0; i < MAX_PLAYERS; i++)
                {
                    state.pScore[i] = 0;
                    pReady[i] = false;
                }

                state.pX[0] = 2; state.pY[0] = 2;
                state.pX[1] = 27; state.pY[1] = 2;
                state.pX[2] = 2; state.pY[2] = 12;
                state.pX[3] = 27; state.pY[3] = 12;

                state.gameOver = false;
                state.winner = 0;
                SpawnItem(&state);
            }
        }

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            send(clients[i], (char*)&state, sizeof(GameState), 0);
        }

        Sleep(30);
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        closesocket(clients[i]);
    }
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
                cout << "Waiting for all players to press 'r' to restart or 'q' to quit.\n";
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
