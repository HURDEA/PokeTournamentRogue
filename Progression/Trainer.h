#pragma once
#include <string>
#include <vector>

struct EnemyPokemon {
    std::string species;
    std::string heldItem;
    std::string ability;
    std::vector<std::string> moves;
    int evHp = 0, evAtk = 0, evDef = 0, evSpA = 0, evSpD = 0, evSpe = 0;
};

struct TrainerTeam {
    std::string trainerName;
    std::string trainerClass; // e.g., "Gym Leader", "Elite Four"
    std::string spritePath;
    std::vector<EnemyPokemon> team;
};