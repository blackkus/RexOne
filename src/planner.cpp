#include "planner.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <ctime>
#include <iomanip>

/**
 * @struct Planner::Impl
 * @brief Private implementation (Pimpl pattern)
 */
struct Planner::Impl {
    std::map<std::string, SafetyConstraint> constraints;
    std::vector<std::string> audit_log;

    Impl() = default;
};

// ============================================================================
// Action Implementation
// ============================================================================

double Planner::Action::utility() const {
    // Utility = (benefit / cost) * (1 - risk)
    // If cost is 0 or very small, clamp to avoid division issues
    double cost_normalized = std::max(cost, 0.01);
    double benefit_cost_ratio = benefit / cost_normalized;
    double risk_penalty = 1.0 - (risk * 0.5); // Risk reduces utility by up to 50%
    return benefit_cost_ratio * risk_penalty;
}

// ============================================================================
// Planner Implementation
// ============================================================================

Planner::Planner()
    : pimpl_(std::make_unique<Impl>()) {
    // Set up default safety constraints
    add_constraint("max_risk_0.7", safety_max_risk(0.7));
    add_constraint("preserve_recent", safety_preserve_recent());
}

Planner::~Planner() = default;

void Planner::add_constraint(const std::string& name, SafetyConstraint constraint) {
    if (!constraint) {
        return;
    }
    pimpl_->constraints[name] = constraint;
}

void Planner::remove_constraint(const std::string& name) {
    pimpl_->constraints.erase(name);
}

bool Planner::is_safe(const Action& action) const {
    // Check all constraints
    for (const auto& [name, constraint] : pimpl_->constraints) {
        if (!constraint(action)) {
            return false; // Action violates this constraint
        }
    }
    return true;
}

Planner::Decision Planner::plan(const std::vector<Action>& candidates,
                                 const std::string& context) {
    Decision decision;
    decision.candidates = candidates;

    // Filter out unsafe actions
    std::vector<Action> safe_actions;
    for (const auto& action : candidates) {
        if (is_safe(action)) {
            safe_actions.push_back(action);
        }
    }

    if (safe_actions.empty()) {
        // No safe action available; return a default "wait" action
        Action wait_action;
        wait_action.name = "wait";
        wait_action.description = "No safe action available; wait for user input";
        wait_action.cost = 0.1;
        wait_action.risk = 0.0;
        wait_action.benefit = 0.0;
        wait_action.requires_approval = false;
        
        decision.chosen_action = wait_action;
        decision.score = 0.0;
        decision.approved = true;
        decision.reasoning = "All candidate actions violate safety constraints. Waiting for further instruction.";
        
        log_decision(decision, "no_safe_action");
        return decision;
    }

    // Select action with highest utility
    const Action& best_action = *std::max_element(
        safe_actions.begin(),
        safe_actions.end(),
        [](const Action& a, const Action& b) {
            return a.utility() < b.utility();
        }
    );

    decision.chosen_action = best_action;
    decision.score = best_action.utility();
    decision.approved = !best_action.requires_approval;
    
    // Generate reasoning
    std::stringstream ss;
    ss << "Selected action '" << best_action.name << "' with utility score "
       << std::fixed << std::setprecision(3) << decision.score << ". "
       << "Benefit: " << best_action.benefit
       << ", Cost: " << best_action.cost
       << ", Risk: " << best_action.risk << ".";
    decision.reasoning = ss.str();

    log_decision(decision, "planned");
    return decision;
}

Planner::Decision Planner::request_approval(const Action& action, bool human_response) {
    Decision decision;
    decision.chosen_action = action;
    decision.score = action.utility();
    decision.approved = human_response;
    decision.reasoning = human_response
        ? "Action approved by user."
        : "Action rejected by user.";

    std::string outcome = human_response ? "user_approved" : "user_rejected";
    log_decision(decision, outcome);

    return decision;
}

void Planner::log_decision(const Decision& decision, const std::string& outcome) {
    // Format: [YYYY-MM-DD HH:MM:SS] Action: {name}, Outcome: {outcome}, Score: {score}
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    std::stringstream ss;
    ss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] "
       << "Action: " << decision.chosen_action.name << ", "
       << "Outcome: " << outcome << ", "
       << "Score: " << std::fixed << std::setprecision(3) << decision.score;

    pimpl_->audit_log.push_back(ss.str());
}

std::vector<std::string> Planner::get_audit_log() const {
    return pimpl_->audit_log;
}

void Planner::clear_audit_log() {
    pimpl_->audit_log.clear();
}

// ============================================================================
// Pre-defined Safety Constraints
// ============================================================================

Planner::SafetyConstraint Planner::safety_max_risk(double threshold) {
    return [threshold](const Action& action) {
        return action.risk <= threshold;
    };
}

Planner::SafetyConstraint Planner::safety_preserve_recent() {
    return [](const Action& action) {
        // Reject any action that tries to delete recent memories
        if (action.name.find("delete") != std::string::npos &&
            action.name.find("recent") != std::string::npos) {
            return false;
        }
        return true;
    };
}

Planner::SafetyConstraint Planner::safety_require_approval() {
    return [](const Action& action) {
        // Actions with high risk should require approval
        // (This is more of a marker; actual approval is handled elsewhere)
        if (action.risk > 0.5 && !action.requires_approval) {
            return false; // Should require approval but doesn't
        }
        return true;
    };
}
