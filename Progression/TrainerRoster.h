#pragma once
#include "Trainer.h"
#include <vector>

class TrainerRoster {
public:
    // --- PRACTICE GAUNTLET POOLS ---
    // Stage 1: Novice (3 Pokémon, Basic)
    // Stage 2: Intermediate (4 Pokémon, Synergistic)
    // Stage 3: Advanced (5 Pokémon, Weather/Trick Room/Hazards)
    // Stage 4: Expert (6 Pokémon, Fully Competitive OU/UU, No Legendaries)
    static std::vector<TrainerTeam> getGauntletTrainers(int stage);

    // --- TOURNAMENT POOLS ---
    // 8 Monotype Gym Leaders (No Legendaries)
    static std::vector<TrainerTeam> getTournamentGymLeaders();

    // 4 Elite Four Members (Monotype core + 2 Top-Tier Coverage threats)
    static std::vector<TrainerTeam> getTournamentEliteFour();

    // 1 Champion (NatDex Ubers, Legendaries allowed)
    static TrainerTeam getTournamentChampion();
};