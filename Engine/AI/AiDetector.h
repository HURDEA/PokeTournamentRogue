#pragma once
#include <string>
#include <vector>
#include "../../Domain/Pokemon.h"

class Controller;

namespace AI {

    class AiDetector {
    public:
        explicit AiDetector(Controller& ctrl);
        ~AiDetector() = default;

        std::string chooseBestAction(
            const std::vector<Pokemon>& aiParty,
            size_t activeAiIdx,
            const std::vector<Pokemon>& playerParty,
            size_t activePlayerIdx,
            bool aiCanMega,
            bool playerCanMega
        );

    private:
        Controller* controller;
    };

}