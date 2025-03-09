#include <iostream>
#include "editor.h"




class Editor {
public:
    void start() {
        std::cout << "Starting the editor..." << std::endl;
        handleInput();
    }

private:
    void handleInput() {
        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input);

            if (input == "quit") {
                std::cout << "Exiting the editor. Goodbye!" << std::endl;
                break;
            } else if (input == "compile") {
                std::cout << "Compilation feature coming soon!" << std::endl;
            }
              else if (input.rfind("open ", 0) == 0) {
                filename = input.substr(5);
                openFile();
            } else if (input == "edit") {
                editFile();
              }
            else {
                std::cout << "Unrecognized command: " << input << std::endl;
            }
        }
    }
};




int main() {
    std::cout << "Welcome to the Native Code Editor!" << std::endl;

    Editor editor;
    editor.start();

    return 0;
}
