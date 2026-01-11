#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

#include "tui.h"

using namespace std;

int main() {

    int mainMenuX = 14;
    int mainMenuY = 16;

    int selection = -3;

    screen scr;
    screen::TermRawMode termMode;
    termMode.enableRawMode();

    vector<string> lines = {"Accounts", "Transactions", "Categories", "Budget", "Reports",
                                      "Bills", "Import/Export", "Settings"};

    vector<string> accountLines = {
                    "Checking Account",
                    "Savings Account",
                    "Credit Card",
                    "Cash Wallet",
                    "Investment Account",
                    "Retirement Fund",
                    "Emergency Fund",
                    "Travel Fund",
                    "Education Fund",
                    "Health Savings Account"
                };

    // 116x40 Windowed
    // 236x64 Fullscreen
    // App border
    scr.drawBorder(116, 40, 1, 1, 34, "TUI Ledger", screen::Align::Left);
    
    // Main Menu border
    scr.drawBorder(15, 9, mainMenuX, mainMenuY, 34, "Main Menu", screen::Align::Center);
    
    // Instructions
    scr.writeText("                              Press \"↑/↓\" to scroll up or down and press \"Q\" to quit                              ", 44, 2, 40);

    // Main loop
    while (true) {
        // Main Menu
        selection = scr.scrollCache(lines, 13, 5, mainMenuX + 1, mainMenuY + 2, 34, 44);

        // Exit application
        if (selection == -1) {
            scr.clearScreen();
            return 0;
        }

        // Check selection of main menu item
        switch(selection) {
            case 0:
                // Accounts
                scr.drawBorder(50, 20, 40, 10, 34, "Accounts", screen::Align::Center);

                // Accounts submenu
                while (true) {
                    int accts = scr.scrollCache(accountLines, 48, 15, 41, 12, 34, 44);

                    // Exit application on 'q' press
                    if (accts == -1) {
                        scr.clearScreen();
                        return 0;
                    }

                    // Go back to main menu
                    if (accts == -2) {
                        // Clear Accounts border area
                        for (int i = 0; i < 20; ++i) {
                            scr.writeText(string(50, ' '), 0, 40, 10 + i);
                        }
                        scr.writeText("                                            ", 0, 45, 30); // Clear previous selection
                        break;
                    }
                    scr.writeText("                                            ", 0, 45, 30); // Clear previous selection
                    scr.writeText("You selected account: " + accountLines[accts], 0, 45, 30);
                }
                break;
            case 1:
                // Transactions
                break;
            case 2:
                // Categories
                break;
            case 3:
                // Budget
                break;
            case 4:
                // Reports
                break;
            case 5:
                // Bills
                break;
            case 6:
                // Import/Export
                break;
            case 7:
                // Settings
                break;
            default:
                break;
        }
    }

    return 0;
}