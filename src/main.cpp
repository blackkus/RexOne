#include <iostream>
#include <string>

#include "orchestrator.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    Orchestrator orchestrator("./db");
    std::cout << "RexOne demo REPL. Type 'quit' to exit.\n";
    std::string line;
    while (true) {
        std::cout << "You> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "quit" || line == "exit") break;
        std::string resp = orchestrator.handle_user(line);
        std::cout << "RexOne> " << resp << "\n\n";
    }
    return 0;
}
