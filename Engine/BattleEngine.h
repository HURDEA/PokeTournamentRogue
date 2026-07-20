#pragma once
#include "../Domain/Pokemon.h"
#include "../Controller/Controller.h"
#include <string>
#include <vector>
#include <map>
#include <functional> 

struct SideState {
    bool stealthRock = false;
    int spikes = 0;
    int toxicSpikes = 0;
    bool stickyWeb = false;
    int substituteHp = 0;

    int tailwind = 0;
    int reflect = 0;
    int lightScreen = 0;
    int auroraVeil = 0;
    int safeguard = 0;

    int faintedCount = 0; // NEW: Tracks faints symmetrically for abilities
};

struct FieldState {
    std::string weather = "None";
    int weatherTurns = 0;

    std::string terrain = "None";
    int terrainTurns = 0;

    int trickRoom = 0;
    int magicRoom = 0;
    int wonderRoom = 0;
    int gravity = 0;
};

class BattleEngine {
private:
    Controller& controller;
    FieldState field;
    SideState playerSide;
    SideState enemySide;

    double getTypeEffectiveness(const std::string& atkType, const std::string& defType);
    std::string normalizeString(const std::string& str);

public:
    BattleEngine(Controller& ctrl);
    ~BattleEngine() = default;

    const FieldState& getFieldState() const { return field; }
    const SideState& getPlayerSide() const { return playerSide; }
    const SideState& getEnemySide() const { return enemySide; }

    void clearSubstitute(bool isPlayerSide) {
        if (isPlayerSide) playerSide.substituteHp = 0;
        else enemySide.substituteHp = 0;
    }

    void setFaintedCount(bool isPlayerSide, int count) {
        if (isPlayerSide) playerSide.faintedCount = count;
        else enemySide.faintedCount = count;
    }

    std::string getActiveWeather(const Pokemon& p1, const Pokemon& p2);
    void checkExtremeWeather(const Pokemon& p1, const Pokemon& p2, std::vector<std::string>& log);

    int getEffectiveSpeed(const Pokemon& p, const Pokemon& opp, bool isPlayerSide);
    void onSwitchIn(Pokemon& p, bool isPlayer, std::vector<std::string>& log, Pokemon* opp = nullptr);
    int calculateDamageInternal(Pokemon& attacker, Pokemon& defender, const MoveData& move, bool isCrit, std::vector<std::string>& log, bool isFirstHit, bool isPlayerAttacker);
    void executeMove(Pokemon& attacker, Pokemon& defender, const std::string& moveId, bool isPlayerAttacking, std::vector<std::string>& log, const std::string& defenderMoveId = "", std::function<void(bool)> animateFn = nullptr);
    void runEndOfTurn(Pokemon& p1, Pokemon& p2, std::vector<std::string>& log);
};