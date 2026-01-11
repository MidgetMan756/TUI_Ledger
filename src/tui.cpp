#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <algorithm>

#include "tui.h"

using namespace std;

// diable raw mode on destruction
screen::TermRawMode::~TermRawMode() {
    disableRawMode();
}

// Enable raw mode for terminal input
void screen::TermRawMode::enableRawMode() {

    tcgetattr(STDIN_FILENO, &orig_termios); // Get current terminal attributes

    termios raw = orig_termios; // Make a copy

    raw.c_lflag &= ~(ECHO | ICANON); // No line buffering and no echo
    raw.c_cc[VMIN] = 1;              // Minimum number of bytes of input before read returns
    raw.c_cc[VTIME] = 0;             // No timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // Apply raw mode
}

// Disable raw mode and restore original terminal settings
void screen::TermRawMode::disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); // Restore original attributes
}

// Get a key press from the user
screen::Key screen::getKeyPress() {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return Key::Unknown;

    if (c == 'q') return Key::Quit;

    if (c == 'b') return Key::Back;

    if (c == '\r' || c == '\n') return Key::Enter;

    if (c == '\x1B') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return Key::Unknown;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return Key::Unknown;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return Key::Up;
                case 'B': return Key::Down;
                case 'C': return Key::Right;
                case 'D': return Key::Left;
            }
        }
    }

    return Key::Unknown;
}

// Draw a box at (x, y) with specified width, height, color, and optional text
void screen::drawBox(int width, int height, int x, int y, int color, string text, Align align) {

    // Set color
    cout << "\x1B[" << color << "m";

    // Draw box
    for (int i = 0; i < height; ++i) {
        cout << "\x1B[" << (y + i) << ";" << x << "H";

        for (int j = 0; j < width; ++j) {
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
                cout << "*";
            else
                cout << " ";
        }
    }

    cout << "\x1B[" << y << ";" << (x + 1) << "H";
    cout << text;

    cout << "\x1B[1;1H"; // Move cursor to top-left
    cout << "\x1B[0m"; // Reset color
    cout.flush();
}

void screen::drawBorder(int width, int height, int x, int y, int color, string text, Align align) {
    // Set color
    cout << "\x1B[" << color << "m";

    // Draw box using Unicode box-drawing characters
    for (int i = 0; i < height; ++i) {
        cout << "\x1B[" << (y + i) << ";" << x << "H";

        for (int j = 0; j < width; ++j) {
            if (i == 0 && j == 0)
                cout << "╔";
            else if (i == 0 && j == width - 1)
                cout << "╗";
            else if (i == height - 1 && j == 0)
                cout << "╚";
            else if (i == height - 1 && j == width - 1)
                cout << "╝";
            else if (i == 0 || i == height - 1)
                cout << "═";
            else if (j == 0 || j == width - 1)
                cout << "║";
            else
                cout << " ";
        }
    }

    if (align == Align::Center) {
        int text_start = x + (width - static_cast<int>(text.size())) / 2;
        cout << "\x1B[" << y << ";" << text_start << "H";
    } else if (align == Align::Right) {
        int text_start = x + width - static_cast<int>(text.size()) - 1;
        cout << "\x1B[" << y << ";" << text_start << "H";
    } else {
        cout << "\x1B[" << y << ";" << (x + 1) << "H";
    }

    cout << text;

    cout << "\x1B[1;1H"; // Move cursor to top-left
    cout << "\x1B[0m"; // Reset color
    cout.flush();
}
// Write text at (x, y) with specified color
void screen::writeText(string text, int color, int x, int y) {
    // Set color
    cout << "\x1B[" << color << "m";

    // Move cursor and write text
    cout << "\x1B[" << y << ";" << x << "H" << text;

    cout << "\x1B[1;1H"; // Move cursor to top-left
    cout << "\x1B[0m"; // Reset color
    cout.flush();
}

// Scroll through a cache of strings within a defined area
int screen::scrollCache(const vector<string>& cache,
                        int width, int height,
                        int x, int y,
                        int FG_color, int HL_color) {
    if (cache.empty() || width <= 0 || height <= 0) return -1;

    // Hide cursor while menu is active
    cout << "\x1B[?25l";
    cout.flush();

    size_t selected = 0; // absolute index in cache
    size_t start = 0;    // first visible line index

    auto redraw = [&]() {
        // Clear area (also removes leftovers from previous longer lines)
        for (int row = 0; row < height; ++row) {
            cout << "\x1B[" << (y + row) << ";" << x << "H";
            cout << "\x1B[0m"; // reset
            for (int col = 0; col < width; ++col) cout << ' ';
        }

        // Draw visible lines
        size_t end = min(start + static_cast<size_t>(height), cache.size());

        for (size_t idx = start; idx < end; ++idx) {
            int row = static_cast<int>(idx - start);

            // Move cursor to row
            cout << "\x1B[" << (y + row) << ";" << x << "H";

            // Set highlight or normal colors
            if (idx == selected) {
                cout << "\x1B[" << HL_color << "m";
            } else {
                cout << "\x1B[" << FG_color << "m";
            }

            // Print line padded/truncated to width
            string line = cache[idx];
            if ((int)line.size() > width) line.resize(width);
            cout << line;

            // Pad remaining space so highlight covers full width
            for (int pad = static_cast<int>(line.size()); pad < width; ++pad) {
                cout << ' ';
            }

            // Reset after each line
            cout << "\x1B[0m";
        }

        // Put cursor somewhere non-annoying
        cout << "\x1B[1;1H";
        cout.flush();
    };

    redraw();

    while (true) {
        Key k = getKeyPress();

        if (k == Key::Quit) {
            cout << "\x1B[?25h"; // show cursor
            cout << "\x1B[0m";
            cout.flush();
            return -1;
        }

        if (k == Key::Enter) {
            cout << "\x1B[?25h"; // show cursor
            cout << "\x1B[0m";
            cout.flush();
            return static_cast<int>(selected);
        }

        if (k == Key::Back) {
            cout << "\x1B[?25h"; // show cursor
            cout << "\x1B[0m";
            cout.flush();
            return -2;
        }

        if (k == Key::Up) {
            if (selected > 0) {
                --selected;
                if (selected < start) start = selected;
                redraw();
            }
        } else if (k == Key::Down) {
            if (selected + 1 < cache.size()) {
                ++selected;
                if (selected >= start + static_cast<size_t>(height)) {
                    start = selected - static_cast<size_t>(height) + 1;
                }
                redraw();
            }
        }
    }
}

void screen::clearScreen() {
    // CSI[2J clears the screen, CSI[H moves the cursor to the top-left corner
    cout << "\x1B[2J\x1B[H"; 
    // Flush the output buffer to ensure the command is sent immediately
    cout.flush();
}