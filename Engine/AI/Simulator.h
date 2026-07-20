#pragma once
#include <string>
#include <vector>
#include "../../Domain/Pokemon.h"
#include "../BattleEngine.h"

class Controller;

namespace AI {

    class Simulator {
    public:
        // Updated to require Controller to check move flags (like isProtectMove)
        static std::vector<std::string> generateValidActions(const Pokemon& activePkmn, const std::vector<Pokemon>& party, size_t activeIdx, bool allowMega, Controller& ctrl);

        static double evaluateActionBranches(
            const std::vector<Pokemon>& aiParty, size_t activeAiIdx,
            const std::vector<Pokemon>& playerParty, size_t activePlayerIdx,
            const std::string& forcedAiAction, Controller& ctrl, int depth,
            bool aiCanMega, bool playerCanMega,
            double alpha, double beta
        );

    private:
        static double runSimTurn(
            const std::vector<Pokemon>& aiParty, size_t activeAiIdx,
            const std::vector<Pokemon>& playerParty, size_t activePlayerIdx,
            Controller& ctrl, int depth, const std::string& forcedAiAct,
            bool aiCanMega, bool playerCanMega,
            double alpha, double beta
        );

        static double evaluateBoardState(const std::vector<Pokemon>& aiParty, size_t aiIdx, const std::vector<Pokemon>& playerParty, size_t playerIdx, Controller& ctrl);

        static void applyActionPhase(Pokemon& activeAttacker, Pokemon& activeDefender, const std::string& action, const std::string& defenderAction, bool isPlayerSide, BattleEngine& sandbox, std::vector<std::string>& log);
    };

}