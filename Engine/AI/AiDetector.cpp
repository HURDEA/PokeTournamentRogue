#include "AiDetector.h"
#include "Simulator.h"
#include "../../Controller/Controller.h"
#include <limits>

using namespace AI;

AiDetector::AiDetector(Controller& ctrl) : controller(&ctrl) {}

std::string AiDetector::chooseBestAction(
    const std::vector<Pokemon>& aiParty,
    size_t activeAiIdx,
    const std::vector<Pokemon>& playerParty,
    size_t activePlayerIdx,
    bool aiCanMega,
    bool playerCanMega
) {
    std::vector<std::string> aiActions = Simulator::generateValidActions(aiParty[activeAiIdx], aiParty, activeAiIdx, aiCanMega, *controller);

    if (aiActions.empty()) return "move:struggle";

    if (controller == nullptr) {
        return aiActions[0];
    }

    std::string bestAction = aiActions[0];
    double bestMaximinScore = -std::numeric_limits<double>::infinity();

    double alpha = -std::numeric_limits<double>::infinity();
    double beta = std::numeric_limits<double>::infinity();

    // Depth 2: Simulates the AI's action, the Player's best response, and evaluates the resulting board state.
    for (const std::string& aiAct : aiActions) {
        double worstCaseScore = Simulator::evaluateActionBranches(
            aiParty, activeAiIdx, playerParty, activePlayerIdx,
            aiAct, *controller, 2, aiCanMega, playerCanMega,
            alpha, beta
        );

        if (worstCaseScore > bestMaximinScore) {
            bestMaximinScore = worstCaseScore;
            bestAction = aiAct;
        }

        if (bestMaximinScore > alpha) {
            alpha = bestMaximinScore;
        }
    }

    return bestAction;
}