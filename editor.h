#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>

class Editor {
private:
    std::vector<std::string> buffer; // Stores the lines of the file
    int cursorLine; // Line where the cursor is located
    int cursorColumn; // Column where the cursor is located

public:
    Editor();
    void start();
    void render();
    void handleInput();
    void openFile(const std::string& filePath);
    void saveFile(const std::string& filePath);
};
