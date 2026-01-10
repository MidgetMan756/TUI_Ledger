#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <algorithm>

#include "tui.h"

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
void screen::drawBox(int width, int height, int x, int y, int color, std::string text) {

    // Set color
    std::cout << "\x1B[" << color << "m";

    // Draw box
    for (int i = 0; i < height; ++i) {
        std::cout << "\x1B[" << (y + i) << ";" << x << "H";

        for (int j = 0; j < width; ++j) {
            if (i == 0 || i == height - 1 || j == 0 || j == width - 1)
                std::cout << "*";
            else
                std::cout << " ";
        }
    }

    std::cout << "\x1B[" << y << ";" << (x + 1) << "H";
    std::cout << text;

    std::cout << "\x1B[1;1H"; // Move cursor to top-left
    std::cout << "\x1B[0m"; // Reset color
    std::cout.flush();
}

// Write text at (x, y) with specified color
void screen::writeText(std::string text, int color, int x, int y) {
    // Set color
    std::cout << "\x1B[" << color << "m";

    // Move cursor and write text
    std::cout << "\x1B[" << y << ";" << x << "H" << text;

    std::cout << "\x1B[1;1H"; // Move cursor to top-left
    std::cout << "\x1B[0m"; // Reset color
    std::cout.flush();
}

// Scroll through a cache of strings within a defined area
int screen::scrollCache(const std::vector<std::string>& cache,
                        int width, int height,
                        int x, int y,
                        int FG_color, int HL_color) {
    if (cache.empty() || width <= 0 || height <= 0) return -1;

    // Hide cursor while menu is active
    std::cout << "\x1B[?25l";
    std::cout.flush();

    size_t selected = 0; // absolute index in cache
    size_t start = 0;    // first visible line index

    auto redraw = [&]() {
        // Clear area (also removes leftovers from previous longer lines)
        for (int row = 0; row < height; ++row) {
            std::cout << "\x1B[" << (y + row) << ";" << x << "H";
            std::cout << "\x1B[0m"; // reset
            for (int col = 0; col < width; ++col) std::cout << ' ';
        }

        // Draw visible lines
        size_t end = std::min(start + static_cast<size_t>(height), cache.size());

        for (size_t idx = start; idx < end; ++idx) {
            int row = static_cast<int>(idx - start);

            // Move cursor to row
            std::cout << "\x1B[" << (y + row) << ";" << x << "H";

            // Set highlight or normal colors
            if (idx == selected) {
                std::cout << "\x1B[" << HL_color << "m";
            } else {
                std::cout << "\x1B[" << FG_color << "m";
            }

            // Print line padded/truncated to width
            std::string line = cache[idx];
            if ((int)line.size() > width) line.resize(width);
            std::cout << line;

            // Pad remaining space so highlight covers full width
            for (int pad = static_cast<int>(line.size()); pad < width; ++pad) {
                std::cout << ' ';
            }

            // Reset after each line
            std::cout << "\x1B[0m";
        }

        // Put cursor somewhere non-annoying
        std::cout << "\x1B[1;1H";
        std::cout.flush();
    };

    redraw();

    while (true) {
        Key k = getKeyPress();

        if (k == Key::Quit) {
            std::cout << "\x1B[?25h"; // show cursor
            std::cout << "\x1B[0m";
            std::cout.flush();
            return -1;
        }

        if (k == Key::Enter) {
            std::cout << "\x1B[?25h"; // show cursor
            std::cout << "\x1B[0m";
            std::cout.flush();
            return static_cast<int>(selected);
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
