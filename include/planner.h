#ifndef PLANNER_H
#define PLANNER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

/**
 * @class Planner
 * @brief Autonomous decision-making module with cost/risk evaluation and safety constraints
 * 
 * Enables RexOne to autonomously plan actions based on:
 * - Symbolic constraints (safety rules, e.g., "never harm")
 * - Utility function (cost vs benefit)
 * - Risk assessment
 * - Human-in-the-loop thresholds
 */
class Planner {
public:
    /**
     * @struct Action
     * @brief Represents a candidate action with associated metrics
     */
    struct Action {
        std::string name;        ///< Action identifier (e.g., "delete_old_memory")
        std::string description; ///< Human-readable description
        double cost;             ///< Execution cost (time, resources, energy)
        double risk;             ///< Risk level (0.0 = safe, 1.0 = dangerous)
        double benefit;          ///< Expected utility/benefit (0.0 = none, 1.0 = maximum)
        bool requires_approval;  ///< If true, requires human approval before execution
        
        /// Compute net utility: benefit / cost, penalized by risk
        double utility() const;
    };

    /**
     * @struct Decision
     * @brief Represents a planner's decision
     */
    struct Decision {
        Action chosen_action;          ///< Selected action
        double score;                  ///< Utility score of chosen action
        bool approved;                 ///< Whether human approved this decision
        std::string reasoning;         ///< Explanation for the decision
        std::vector<Action> candidates; ///< All considered alternatives
    };

    /**
     * @typedef SafetyConstraint
     * @brief Predicate function that returns true if action is safe
     * 
     * Used to enforce symbolic constraints:
     * - "never_delete_recent_memories"
     * - "max_risk_threshold"
     * - "require_approval_for_risky_actions"
     */
    using SafetyConstraint = std::function<bool(const Action&)>;

    Planner();
    ~Planner();

    /**
     * Register a safety constraint (symbolic rule)
     * @param name Constraint identifier
     * @param constraint Predicate that must be satisfied
     */
    void add_constraint(const std::string& name, SafetyConstraint constraint);

    /**
     * Remove a safety constraint by name
     */
    void remove_constraint(const std::string& name);

    /**
     * Evaluate a set of candidate actions and return the best decision
     * @param candidates Vector of candidate actions
     * @param context Additional context (e.g., "user_approved_risky_actions")
     * @return Decision with chosen action and reasoning
     */
    Decision plan(const std::vector<Action>& candidates, const std::string& context = "");

    /**
     * Evaluate whether a single action satisfies all constraints
     * @param action Action to check
     * @return true if action violates no constraints
     */
    bool is_safe(const Action& action) const;

    /**
     * Get human approval for a high-risk action
     * @param action Action requiring approval
     * @param human_response User's decision (true = approve, false = reject)
     * @return Updated Decision with approval status
     */
    Decision request_approval(const Action& action, bool human_response);

    /**
     * Log a decision and its outcome for auditing
     * @param decision The decision made
     * @param outcome Result description (e.g., "successfully executed", "user rejected")
     */
    void log_decision(const Decision& decision, const std::string& outcome);

    /**
     * Get audit log entries (for explainability)
     * @return Vector of audit log messages
     */
    std::vector<std::string> get_audit_log() const;

    /**
     * Clear audit log
     */
    void clear_audit_log();

    /**
     * Pre-defined safety constraint: max risk threshold
     */
    static SafetyConstraint safety_max_risk(double threshold);

    /**
     * Pre-defined safety constraint: prevent deletion of recent memories
     */
    static SafetyConstraint safety_preserve_recent();

    /**
     * Pre-defined safety constraint: require approval for risky actions
     */
    static SafetyConstraint safety_require_approval();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

#endif // PLANNER_H
