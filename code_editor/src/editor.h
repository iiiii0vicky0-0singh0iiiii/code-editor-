#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>

class Editor {
private:
    std::vector<std::string> buffer; // Stores the lines of the file
    int cursorLine; // Line where the cursor is located
    int cursorColumn; // Column where the cursor is located

     // Private helper methods 

    void moveCursorUp();
    void moveCursorDown();  
    void moveCursorLeft(); 
    void moveCursorRight(); 

void Editor::moveCursorUp() {
    if (cursorLine > 0) {
        cursorLine--;
        cursorColumn = std::min(cursorColumn, static_cast<int>(buffer[cursorLine].size()));
    }
}

void Editor::moveCursorDown() {
    if (cursorLine < static_cast<int>(buffer.size()) - 1) {
        cursorLine++;
        cursorColumn = std::min(cursorColumn, static_cast<int>(buffer[cursorLine].size()));
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

void Editor::insertCharacter(char ch) {
    if (cursorLine < static_cast<int>(buffer.size())) {
        buffer[cursorLine].insert(cursorColumn, 1, ch);
        cursorColumn++;
    }
}

void Editor::deleteCharacter() {
    if (cursorLine < static_cast<int>(buffer.size()) && cursorColumn > 0) {
        buffer[cursorLine].erase(cursorColumn - 1, 1);
        cursorColumn--;
    } else if (cursorLine > 0 && cursorColumn == 0) {
        cursorColumn = static_cast<int>(buffer[cursorLine - 1].size());
        buffer[cursorLine - 1] += buffer[cursorLine];
        buffer.erase(buffer.begin() + cursorLine);
        cursorLine--;
    }
}

public:
    Editor();
    void start();
    void render();
    void handleInput();
    void openFile(const std::string& filePath);
    void saveFile(const std::string& filePath);
    void insertCharacter(char ch);
    void deleteCharacter();
};
#endif 
