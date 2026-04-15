#pragma once

#include <string>
#include <vector>

#include "model_interface.h"
#include "memory_manager.h"

class Orchestrator {
public:
    explicit Orchestrator(const std::string &dbpath);
    // Handle a single user input and return assistant response
    std::string handle_user(const std::string &user_input);

private:
    ModelInterface model_;
    MemoryManager mem_;
    std::vector<std::string> get_recent_history();
};
