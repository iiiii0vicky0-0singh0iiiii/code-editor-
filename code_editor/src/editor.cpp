#include "editor.h"
#include <iostream>
#include <fstream>

Editor::Editor() : cursorLine(0), cursorColumn(0) {
    // Initialize buffer with an empty line
    buffer.push_back("");
}

void Editor::start() {
    std::cout << "Editor started." << std::endl;
    // Basic rendering loop (placeholder)
    while (true) {
        render();
        handleInput();
    }
}

void Editor::render() {
    // Render the buffer line by line
    std::cout << "\x1B[2J\x1B[H"; // Clear screen and reset cursor
    for (size_t i = 0; i < buffer.size(); ++i) {
        std::cout << (i + 1) << ": " << buffer[i] << std::endl;
    }
    std::cout << "Cursor at line: " << cursorLine + 1 << ", column: " << cursorColumn + 1 << std::endl;
}

void Editor::handleInput() {
    // Handle basic input (placeholder)
    std::string command;
    std::cout << "> ";
    std::getline(std::cin, command);

    if (command == "quit") {
        exit(0);
    } else if (command.rfind("open ", 0) == 0) {
        openFile(command.substr(5));
    } else if (command.rfind("save ", 0) == 0) {
        saveFile(command.substr(5));
    } else {
        buffer[cursorLine] += command;
    }
}

void Editor::openFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    buffer.clear();
    std::string line;
    while (std::getline(file, line)) {
        buffer.push_back(line);
    }
    file.close();
    cursorLine = 0;
    cursorColumn = 0;
    std::cout << "File opened successfully: " << filePath << std::endl;
}

void Editor::saveFile(const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to save file: " << filePath << std::endl;
        return;
    }

    for (const auto& line : buffer) {
        file << line << "\n";
    }
    file.close();
    std::cout << "File saved successfully: " << filePath << std::endl;
}
