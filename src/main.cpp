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

    scr.drawBox(111, 35, 5, 5, 34, "Big Box");
    
    scr.drawBox(55, 18, 10, 10, 32, "Small Box");

    scr.writeText("Hello, TUI!", 0, 15, 15);

    while (true) {
        int selection = scr.scrollCache(lines, 10, 5, 20, 20, 33, 44);
        scr.writeText("You selected line: " + lines[selection], 0, 15, 30);
        if (selection == -1) break;
    }

    return 0;
}