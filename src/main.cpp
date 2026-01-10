#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

#include "tui.h"

int main() {

    screen scr;
    screen::TermRawMode termMode;
    termMode.enableRawMode();

    std::vector<std::string> lines = {"Item 1", "Item 2", "Item 3", "Item 4", "Item 5",
                                      "Item 6", "Item 7", "Item 8", "Item 9", "Item 10",
                                      "Item 11", "Item 12", "Item 13", "Item 14", "Item 15"};

    // 116x40 Windowed
    // 236x64 Fullscreen
    scr.drawBorder(116, 40, 1, 1, 34, "TUI Ledger", screen::Align::Left);
    
    scr.drawBorder(55, 18, 4, 16, 32, "Main Menu", screen::Align::Center);

    scr.writeText("                              Press \"↑/↓\" to scroll up or down and press \"Q\" to quit                              ", 44, 2, 40);

    while (true) {
        int selection = scr.scrollCache(lines, 10, 5, 20, 20, 33, 44);

        if (selection == -1) {
            scr.clearScreen();
            break;
        }

        scr.writeText("You selected line: " + lines[selection], 0, 15, 30);
    }

    return 0;
}