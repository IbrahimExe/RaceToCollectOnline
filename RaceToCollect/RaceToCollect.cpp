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
#include <windows.h> // system("cls") / Sleep()

#pragma comment(lib, "ws2_32.lib")

using namespace std;

//Game Configurations
const int BOARD_WIDTH = 30;
const int BOARD_HEIGHT = 15;
const int WIN_SCORE = 15;

// Data Structures
// This struct holds everything needed to render a frame.
struct GameState {
    int p1X = 2, p1Y = 7;
    int p2X = 27, p2Y = 7;
    int itemX = 15, itemY = 7;
    int p1Score = 0;
    int p2Score = 0;
    bool gameOver = false;
    int winner = 0; // 1 for P1, 2 for P2
};

// Rendering 
void DrawBoard(const GameState& state) {
    system("cls"); // Clear screen for the new frame

    // Header
    cout << "P1 Score: " << state.p1Score << "  |  P2 Score: " << state.p2Score << "\n";
    cout << string(BOARD_WIDTH + 2, '-') << "\n"; // top boundary

    // Board
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        cout << "|"; // left boundary
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (x == state.p1X && y == state.p1Y) cout << "1";
            else if (x == state.p2X && y == state.p2Y) cout << "2";
            else if (x == state.itemX && y == state.itemY) cout << "0";
            else cout << " ";
        }
        cout << "|\n"; // right boundary
    }

    // Footer
    cout << string(BOARD_WIDTH + 2, '-') << "\n";
}

int main() {
    cout << "Race To Collect!\n";
    cout << "Select Mode:\n1. Server\n2. Client\nChoice: ";
    int choice;
    cin >> choice;

    GameState initialState;

    if (choice == 1) {
        cout << "\nStarting Server...\n";
        //initialize server, wait for 2 clients, start game loop
    }
    else if (choice == 2) {
        cout << "\nStarting Client...\n";

        //test
        DrawBoard(initialState);
    }
    else {
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
