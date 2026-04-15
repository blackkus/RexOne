#pragma once

#include <string>
#include <vector>
#include <memory>

#include "model_interface.h"
#include "memory_manager.h"
#include "planner.h"

class Orchestrator {
public:
    explicit Orchestrator(const std::string &dbpath);
    ~Orchestrator();
    
    // Handle a single user input and return assistant response
    std::string handle_user(const std::string &user_input);
    
    // Autonomous decision-making: choose best action from candidates
    Planner::Decision make_decision(const std::vector<Planner::Action>& candidates);
    
    // Get audit log from planner (for explainability)
    std::vector<std::string> get_decisions_log() const;
    
    // Enable/disable autonomous mode
    void set_autonomous_mode(bool enabled) { autonomous_mode_ = enabled; }
    bool is_autonomous() const { return autonomous_mode_; }

private:
    ModelInterface model_;
    MemoryManager mem_;
    std::unique_ptr<Planner> planner_;
    bool autonomous_mode_;
    
    std::vector<std::string> get_recent_history();
    
    // Autonomous action candidates for memory management
    std::vector<Planner::Action> get_memory_management_candidates();
    
    // Execute a decision (placeholder for future action execution)
    void execute_decision(const Planner::Decision& decision);
};
