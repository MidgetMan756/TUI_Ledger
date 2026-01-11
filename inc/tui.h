#ifndef TUI_H
#define TUI_H

#include <string>
#include <vector>
#include <termios.h>

using namespace std;

class screen{
public:

    struct TermRawMode{
        termios orig_termios;

        void enableRawMode();
        void disableRawMode();

        ~TermRawMode();
    };

    enum class Key {
        Unknown,
        Up, Down, Left, Right,
        Enter,
        Quit,
        Back
    };

    enum class Align {
        Left,
        Center,
        Right
    };
    
    Key getKeyPress();

    void drawBox(int width, int height, int x = 1, int y = 1, int color = 0, string text = "", Align align = Align::Left);
    void drawBorder(int width, int height, int x = 1, int y = 1, int color = 0, string text = "", Align align = Align::Left);

    void writeText(string text, int color = 0, int x = 1, int y = 1);

    int scrollCache(const vector<string>& cache, int width, int height, int x = 1, int y = 1, int FG_color = 0, int HL_color = 44);

    void clearScreen();
};
#endif // TUI_H