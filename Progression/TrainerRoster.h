#pragma once
#include "Trainer.h"
#include <vector>

class TrainerRoster {
public:
    static std::vector<TrainerTeam> getGauntletTrainers(int stage);
    static std::vector<TrainerTeam> getTournamentGymLeaders();
    static std::vector<TrainerTeam> getTournamentEliteFour();
    static TrainerTeam getTournamentChampion();

    // --- NEW: Secret Boss ---
    static TrainerTeam getSecretBoss();
};