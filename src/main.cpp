#include <iostream>
#include <string>
#include <algorithm>

#include "orchestrator.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    Orchestrator orchestrator("./db");
    std::cout << "RexOne demo REPL. Type 'quit' to exit.\n";
    std::cout << "Commands: autonomous [on|off] | decisions | plan | help\n\n";
    std::string line;
    while (true) {
        std::cout << "You> ";
        if (!std::getline(std::cin, line)) break;
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line == "quit" || line == "exit") break;
        
        // Handle special commands
        if (line.substr(0, 10) == "autonomous") {
            std::string arg = line.substr(10);
            arg.erase(0, arg.find_first_not_of(" \t"));
            if (arg == "on") {
                orchestrator.set_autonomous_mode(true);
                std::cout << "RexOne> Autonomous mode enabled.\n\n";
            } else if (arg == "off") {
                orchestrator.set_autonomous_mode(false);
                std::cout << "RexOne> Autonomous mode disabled.\n\n";
            } else {
                std::cout << "RexOne> Usage: autonomous [on|off]\n\n";
            }
            continue;
        }
        
        if (line == "decisions" || line == "audit") {
            auto log = orchestrator.get_decisions_log();
            if (log.empty()) {
                std::cout << "RexOne> No decisions logged yet.\n\n";
            } else {
                std::cout << "RexOne> Decision Audit Log:\n";
                for (const auto& entry : log) {
                    std::cout << "  " << entry << "\n";
                }
                std::cout << "\n";
            }
            continue;
        }
        
        if (line == "plan") {
            // Show what the planner would decide for memory management
            Orchestrator temp_orch("./db");
            temp_orch.set_autonomous_mode(false);
            // This is just for demo; in a real system you might trigger this periodically
            std::cout << "RexOne> Planner demo: invoke 'autonomous on' to enable autonomous decisions.\n\n";
            continue;
        }
        
        if (line == "help" || line == "?") {
            std::cout << "RexOne> Commands:\n"
                      << "  quit/exit        - Exit the REPL\n"
                      << "  autonomous on/off - Enable/disable autonomous mode\n"
                      << "  decisions        - Show decision audit log\n"
                      << "  plan             - Show planner info\n"
                      << "  help/?           - Show this help\n\n";
            continue;
        }
        
        // Regular user input - process through orchestrator
        std::string resp = orchestrator.handle_user(line);
        std::cout << "RexOne> " << resp << "\n\n";
    }
    return 0;
}
