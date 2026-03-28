#include "editor.h"

#include <algorithm>  // std::min
#include <fstream>
#include <iostream>
#include <limits>

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Editor::Editor()
    : cursorLine(0), cursorColumn(0)
{
    buffer.push_back(""); // start with one empty line
}

// ─────────────────────────────────────────────
// Cursor movement (private helpers)
// ─────────────────────────────────────────────
void Editor::moveCursorUp() {
    if (cursorLine > 0) {
        cursorLine--;
        cursorColumn = std::min(cursorColumn,
                                static_cast<int>(buffer[cursorLine].size()));
    }
}

void Editor::moveCursorDown() {
    if (cursorLine < static_cast<int>(buffer.size()) - 1) {
        cursorLine++;
        cursorColumn = std::min(cursorColumn,
                                static_cast<int>(buffer[cursorLine].size()));
    }
}

void Editor::moveCursorLeft() {
    if (cursorColumn > 0) {
        cursorColumn--;
    } else if (cursorLine > 0) {
        cursorLine--;
        cursorColumn = static_cast<int>(buffer[cursorLine].size());
    }
}

void Editor::moveCursorRight() {
    if (cursorColumn < static_cast<int>(buffer[cursorLine].size())) {
        cursorColumn++;
    } else if (cursorLine < static_cast<int>(buffer.size()) - 1) {
        cursorLine++;
        cursorColumn = 0;
    }
}

// ─────────────────────────────────────────────
// Text editing (public)
// ─────────────────────────────────────────────
void Editor::insertCharacter(char ch) {
    if (ch == '\n') {
        // Split the current line at the cursor position
        std::string newLine = buffer[cursorLine].substr(cursorColumn);
        buffer[cursorLine].erase(cursorColumn);
        buffer.insert(buffer.begin() + cursorLine + 1, newLine);
        cursorLine++;
        cursorColumn = 0;
    } else {
        buffer[cursorLine].insert(cursorColumn, 1, ch);
        cursorColumn++;
    }
}

void Editor::deleteCharacter() {
    if (cursorColumn > 0) {
        // Delete the character immediately to the left of the cursor
        buffer[cursorLine].erase(cursorColumn - 1, 1);
        cursorColumn--;
    } else if (cursorLine > 0) {
        // At the start of a line: merge with the previous line
        cursorColumn = static_cast<int>(buffer[cursorLine - 1].size());
        buffer[cursorLine - 1] += buffer[cursorLine];
        buffer.erase(buffer.begin() + cursorLine);
        cursorLine--;
    }
}

// ─────────────────────────────────────────────
// File I/O
// ─────────────────────────────────────────────
void Editor::openFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file \"" << filePath << "\"\n";
        return;
    }

    buffer.clear();
    std::string line;
    while (std::getline(file, line)) {
        buffer.push_back(line);
    }

    if (buffer.empty()) {
        buffer.push_back(""); // always keep at least one line
    }

    cursorLine   = 0;
    cursorColumn = 0;
}

void Editor::saveFile(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: cannot save file \"" << filePath << "\"\n";
        return;
    }

    for (std::size_t i = 0; i < buffer.size(); ++i) {
        file << buffer[i];
        if (i + 1 < buffer.size()) {
            file << '\n';
        }
    }
}

// ─────────────────────────────────────────────
// Rendering
// ─────────────────────────────────────────────
void Editor::render() {
    // Clear the screen (ANSI escape)
    std::cout << "\033[2J\033[H";

    for (int i = 0; i < static_cast<int>(buffer.size()); ++i) {
        // Highlight the cursor line with a '>' prefix
        if (i == cursorLine) {
            std::cout << "> ";
            const std::string& line = buffer[i];
            // Print characters, inserting a visual cursor marker '|'
            for (int j = 0; j <= static_cast<int>(line.size()); ++j) {
                if (j == cursorColumn) {
                    std::cout << '|';
                }
                if (j < static_cast<int>(line.size())) {
                    std::cout << line[j];
                }
            }
        } else {
            std::cout << "  " << buffer[i];
        }
        std::cout << '\n';
    }

    std::cout << "\n-- Line " << cursorLine + 1
              << ", Col "  << cursorColumn + 1
              << " --\n";
}

// ─────────────────────────────────────────────
// Input handling
// ─────────────────────────────────────────────
void Editor::handleInput() {
    // Read one character without waiting for Enter.
    // On POSIX you would set the terminal to raw mode first;
    // here we fall back to standard cin for portability.
    int ch = std::cin.get();
    if (ch == std::char_traits<char>::eof()) {
        return;
    }

    switch (ch) {
        // Arrow keys send ESC sequences: ESC [ A/B/C/D
        case '\033': {
            int seq1 = std::cin.get();
            if (seq1 == '[') {
                int seq2 = std::cin.get();
                switch (seq2) {
                    case 'A': moveCursorUp();    break;
                    case 'B': moveCursorDown();  break;
                    case 'C': moveCursorRight(); break;
                    case 'D': moveCursorLeft();  break;
                }
            }
            break;
        }
        case 127: // Backspace (DEL)
        case '\b':
            deleteCharacter();
            break;
        case '\n':
        case '\r':
            insertCharacter('\n');
            break;
        default:
            if (ch >= 32 && ch < 127) { // printable ASCII
                insertCharacter(static_cast<char>(ch));
            }
            break;
    }
}

// ─────────────────────────────────────────────
// Main loop
// ─────────────────────────────────────────────
void Editor::start() {
    // In a real terminal editor you would enable raw mode here,
    // e.g. with tcsetattr() on POSIX or SetConsoleMode() on Windows.
    std::string command;
    while (true) {
        render();
        std::cout << "[:w <file>  :o <file>  :q  or type to edit]: ";
        std::getline(std::cin, command);

        if (command.empty()) {
            continue;
        }

        if (command == ":q") {
            break;
        } else if (command.substr(0, 3) == ":w ") {
            saveFile(command.substr(3));
        } else if (command.substr(0, 3) == ":o ") {
            openFile(command.substr(3));
        } else {
            // Treat every character in the command as typed input
            for (char c : command) {
                insertCharacter(c);
            }
        }
    }
}
