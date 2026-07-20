#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <map>

struct MoveData {
    std::string id;
    std::string name;
    std::string type;
    std::string category;
    int basePower = 0;
    int accuracy = 100;
    int priority = 0;
    bool isContact = false;

    int minHits = 1;
    int maxHits = 1;
    bool isCharge = false;

    int pp = 10; // NEW: Base PP from the JSON

    int effectChance = 0;
    std::string status = "";
    std::map<std::string, int> boosts;
    std::map<std::string, int> selfBoosts;

    int healNum = 0;
    int healDen = 0;

    int recoilNum = 0;
    int recoilDen = 0;
    bool ohko = false;

    bool flagsProtect = true;
    bool isProtectMove = false;
    std::string volatileStatus = "";

    std::string overrideOffensiveStat = "";
    std::string overrideDefensiveStat = "";
    std::string overrideOffensivePokemon = "";
};

class Pokemon {
private:
    int id;
    int speciesId;
    std::string name;
    std::string nickname;
    std::string type1;
    std::string type2;
    std::map<std::string, int> movePPs;
    std::map<std::string, int> maxMovePPs;

    int baseHp, baseAttack, baseDefense, baseSpAttack, baseSpDefense, baseSpeed;
    int hp, attack, defense, spAttack, spDefense, speed;
    int evHp, evAtk, evDef, evSpA, evSpD, evSpe;
    std::string nature;
    std::string ability;

    int level;
    std::string heldItem;
    int boxNumber;
    std::vector<std::string> moves;

    double weight;
    int happiness;

    int currentHp;
    int stageAtk, stageDef, stageSpA, stageSpD, stageSpe;
    int stageAcc, stageEva, stageCrit;

    std::string statusCondition;
    int statusTurns;

    std::string chargingMove;
    std::map<std::string, int> volatileStatuses;

    int protectCounter;
    std::string lockedMove;
    int lockedTurns;

    std::string origType1, origType2, origAbility;
    std::vector<std::string> origMoves;
    int origBaseAtk, origBaseDef, origBaseSpA, origBaseSpD, origBaseSpe;

    void calculateStats() {
        int oldMaxHp = hp;

        // NO IVs. EV points are added directly to the base stat pool 1:1
        // matching the 66 Total / 26 Max limit established in the Trainer Hub.
        hp = ((2 * baseHp + evHp) * level / 100) + level + 10;

        auto calcOther = [&](int base, int ev, const std::string& statName) {
            double stat = ((2 * base + ev) * level / 100) + 5;
            double modifier = 1.0;
            if (nature == "Adamant") { if (statName == "Atk") modifier = 1.1; if (statName == "SpA") modifier = 0.9; }
            else if (nature == "Jolly") { if (statName == "Spe") modifier = 1.1; if (statName == "SpA") modifier = 0.9; }
            else if (nature == "Timid") { if (statName == "Spe") modifier = 1.1; if (statName == "Atk") modifier = 0.9; }
            else if (nature == "Modest") { if (statName == "SpA") modifier = 1.1; if (statName == "Atk") modifier = 0.9; }
            else if (nature == "Bold") { if (statName == "Def") modifier = 1.1; if (statName == "Atk") modifier = 0.9; }
            else if (nature == "Calm") { if (statName == "SpD") modifier = 1.1; if (statName == "Atk") modifier = 0.9; }
            else if (nature == "Impish") { if (statName == "Def") modifier = 1.1; if (statName == "SpA") modifier = 0.9; }
            else if (nature == "Careful") { if (statName == "SpD") modifier = 1.1; if (statName == "SpA") modifier = 0.9; }
            else if (nature == "Brave") { if (statName == "Atk") modifier = 1.1; if (statName == "Spe") modifier = 0.9; }
            else if (nature == "Quiet") { if (statName == "SpA") modifier = 1.1; if (statName == "Spe") modifier = 0.9; }
            return static_cast<int>(std::floor(stat * modifier));
            };

        attack = calcOther(baseAttack, evAtk, "Atk");
        defense = calcOther(baseDefense, evDef, "Def");
        spAttack = calcOther(baseSpAttack, evSpA, "SpA");
        spDefense = calcOther(baseSpDefense, evSpD, "SpD");
        speed = calcOther(baseSpeed, evSpe, "Spe");

        if (oldMaxHp == 0 || currentHp <= 0) currentHp = hp;
    }

public:
    // --- PP SYSTEM METHODS ---
    int getMovePP(const std::string& moveId) const {
        auto it = movePPs.find(moveId);
        return it != movePPs.end() ? it->second : 0;
    }
    int getMaxMovePP(const std::string& moveId) const {
        auto it = maxMovePPs.find(moveId);
        return it != maxMovePPs.end() ? it->second : 0;
    }
    void setMovePP(const std::string& moveId, int pp) {
        movePPs[moveId] = std::max(0, pp);
    }
    void setMaxMovePP(const std::string& moveId, int maxPp) {
        maxMovePPs[moveId] = maxPp;
    }
    void restoreAllPP() {
        for (const auto& pair : maxMovePPs) {
            movePPs[pair.first] = pair.second;
        }
    }
    bool isSwitchingOut = false;
    bool isForcedRandomSwitch = false;
    bool statLoweredThisTurn = false;
    bool isBatonPassing = false;
    std::string lastMoveUsed = "";
    std::string disabledMove = "";

    int lastDamageTaken = 0;
    std::string lastDamageCategory = "";

    Pokemon(int id = 0, int speciesId = 0, std::string speciesName = "", std::string t1 = "None", std::string t2 = "None",
        int hp = 10, int atk = 10, int def = 10, int spAtk = 10, int spDef = 10, int spd = 10,
        int lvl = 50, std::string item = "None", int box = 1, std::string nick = "", std::vector<std::string> startingMoves = {},
        std::string nat = "Hardy", std::string ab = "None", int eVHp = 0, int eVAtk = 0, int eVDef = 0, int eVSpA = 0, int eVSpD = 0, int eVSpe = 0,
        double weightKg = 10.0, int baseHappiness = 70)
        : id(id), speciesId(speciesId), name(speciesName), type1(t1), type2(t2),
        baseHp(hp), baseAttack(atk), baseDefense(def), baseSpAttack(spAtk), baseSpDefense(spDef), baseSpeed(spd),
        level(lvl), heldItem(item), boxNumber(box), nickname(nick.empty() ? speciesName : nick), moves(startingMoves),
        nature(nat), ability(ab), evHp(eVHp), evAtk(eVAtk), evDef(eVDef), evSpA(eVSpA), evSpD(eVSpD), evSpe(eVSpe),
        weight(weightKg), happiness(baseHappiness), hp(0), currentHp(0), statusCondition(""), statusTurns(0), chargingMove(""),
        protectCounter(0), lockedMove(""), lockedTurns(0) {

        origType1 = type1; origType2 = type2; origAbility = ability; origMoves = moves;
        origBaseAtk = baseAttack; origBaseDef = baseDefense; origBaseSpA = baseSpAttack; origBaseSpD = baseSpDefense; origBaseSpe = baseSpeed;

        resetStatStages();
        calculateStats();
    }

    int getId() const { return id; }
    int getSpeciesId() const { return speciesId; }
    std::string getName() const { return name; }
    std::string getNickname() const { return nickname; }
    std::string getType1() const { return type1; }
    std::string getType2() const { return type2; }

    int getBaseHp() const { return baseHp; }
    int getBaseAttack() const { return baseAttack; }
    int getBaseDefense() const { return baseDefense; }
    int getBaseSpAttack() const { return baseSpAttack; }
    int getBaseSpDefense() const { return baseSpDefense; }
    int getBaseSpeed() const { return baseSpeed; }

    int getHp() const { return hp; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }
    int getSpAttack() const { return spAttack; }
    int getSpDefense() const { return spDefense; }
    int getSpeed() const { return speed; }

    int getBattleStat(int stat, int stage, bool isCrit = false, bool isAttacker = true) const {
        if (isCrit) {
            if (isAttacker && stage < 0) stage = 0;
            if (!isAttacker && stage > 0) stage = 0;
        }
        int num = 2, den = 2;
        if (stage > 0) num += stage;
        else if (stage < 0) den -= stage;
        return (stat * num) / den;
    }

    int getBattleAttack(bool isCrit = false) const { return getBattleStat(attack, stageAtk, isCrit, true); }
    int getBattleDefense(bool isCrit = false) const { return getBattleStat(defense, stageDef, isCrit, false); }
    int getBattleSpAttack(bool isCrit = false) const { return getBattleStat(spAttack, stageSpA, isCrit, true); }
    int getBattleSpDefense(bool isCrit = false) const { return getBattleStat(spDefense, stageSpD, isCrit, false); }
    int getBattleSpeed() const {
        int spd = getBattleStat(speed, stageSpe);
        if (statusCondition == "par") spd /= 2;
        if (hasVolatile("quash")) spd = -999;
        return spd;
    }

    // --- ADDED MISSING STAT GETTERS ---
    int getStageAcc() const { return stageAcc; }
    int getStageEva() const { return stageEva; }
    int getStageAtk() const { return stageAtk; }
    int getStageDef() const { return stageDef; }
    int getStageSpA() const { return stageSpA; }
    int getStageSpD() const { return stageSpD; }
    int getStageSpe() const { return stageSpe; }

    double getWeight() const { return weight; }
    int getHappiness() const { return happiness; }

    int getCurrentHp() const { return currentHp; }
    void setCurrentHp(int newHp) { currentHp = newHp; }

    void fullyHeal() {
        currentHp = hp; statusCondition = ""; statusTurns = 0; chargingMove = "";
        volatileStatuses.clear(); protectCounter = 0; lockedMove = ""; lockedTurns = 0;
        isSwitchingOut = false; isBatonPassing = false; lastMoveUsed = ""; disabledMove = "";
        lastDamageTaken = 0; lastDamageCategory = "";
        statLoweredThisTurn = false; isForcedRandomSwitch = false;

        type1 = origType1; type2 = origType2; ability = origAbility; moves = origMoves;
        baseAttack = origBaseAtk; baseDefense = origBaseDef; baseSpAttack = origBaseSpA; baseSpDefense = origBaseSpD; baseSpeed = origBaseSpe;

        resetStatStages();
        calculateStats();
        restoreAllPP(); // NEW: Refresh PP on heal
    }

    void resetStatStages() {
        stageAtk = 0; stageDef = 0; stageSpA = 0; stageSpD = 0; stageSpe = 0;
        stageAcc = 0; stageEva = 0; stageCrit = 0;
    }

    std::string getStatus() const { return statusCondition; }
    int getStatusTurns() const { return statusTurns; }
    void setStatus(const std::string& s, int turns = 0) { statusCondition = s; statusTurns = turns; }
    void decrementStatusTurns() { if (statusTurns > 0) statusTurns--; }

    std::string getChargingMove() const { return chargingMove; }
    void setChargingMove(const std::string& m) { chargingMove = m; }

    int getProtectCounter() const { return protectCounter; }
    void incrementProtectCounter() { protectCounter++; }
    void resetProtectCounter() { protectCounter = 0; }

    std::string getLockedMove() const { return lockedMove; }
    int getLockedTurns() const { return lockedTurns; }
    void setLockedMove(const std::string& moveName, int turns) { lockedMove = moveName; lockedTurns = turns; }
    void decrementLockedTurns() { if (lockedTurns > 0) lockedTurns--; }

    void addVolatile(const std::string& v, int turns = -1) { volatileStatuses[v] = turns; }
    bool hasVolatile(const std::string& v) const { return volatileStatuses.count(v) > 0; }
    int getVolatileTurns(const std::string& v) { return volatileStatuses[v]; }
    void removeVolatile(const std::string& v) { volatileStatuses.erase(v); }
    void decrementVolatiles() {
        for (auto it = volatileStatuses.begin(); it != volatileStatuses.end(); ) {
            if (it->second > 0) {
                it->second--;
                if (it->second == 0) it = volatileStatuses.erase(it);
                else ++it;
            }
            else ++it;
        }
    }

    void modifyStat(const std::string& stat, int amount, std::vector<std::string>& log) {
        if (amount == 0) return;

        if (ability == "Contrary") amount = -amount;
        else if (ability == "Simple") amount *= 2;

        auto change = [&](int& stage, const std::string& name) {
            int oldStage = stage;
            stage += amount;
            if (stage > 6) stage = 6;
            else if (stage < -6) stage = -6;

            if (stage < oldStage) statLoweredThisTurn = true; // Flag for Eject Pack

            if (stage == oldStage) {
                if (stage == 6) log.push_back(nickname + "'s " + name + " won't go any higher!");
                if (stage == -6) log.push_back(nickname + "'s " + name + " won't go any lower!");
            }
            else {
                if (amount == 1) log.push_back(nickname + "'s " + name + " rose!");
                else if (amount >= 2) log.push_back(nickname + "'s " + name + " rose sharply!");
                else if (amount == -1) log.push_back(nickname + "'s " + name + " fell!");
                else if (amount <= -2) log.push_back(nickname + "'s " + name + " fell harshly!");
            }
            };

        if (stat == "atk") change(stageAtk, "Attack");
        if (stat == "def") change(stageDef, "Defense");
        if (stat == "spa") change(stageSpA, "Sp. Attack");
        if (stat == "spd") change(stageSpD, "Sp. Defense");
        if (stat == "spe") change(stageSpe, "Speed");
        if (stat == "accuracy") change(stageAcc, "Accuracy");
        if (stat == "evasion") change(stageEva, "Evasiveness");
    }

    void inheritBatonPass(const Pokemon& previous) {
        stageAtk = previous.stageAtk; stageDef = previous.stageDef;
        stageSpA = previous.stageSpA; stageSpD = previous.stageSpD;
        stageSpe = previous.stageSpe; stageAcc = previous.stageAcc; stageEva = previous.stageEva;
        stageCrit = previous.stageCrit;
        if (previous.hasVolatile("substitute")) addVolatile("substitute", const_cast<Pokemon*>(&previous)->getVolatileTurns("substitute"));
        if (previous.hasVolatile("confusion")) addVolatile("confusion", 3);
        if (previous.hasVolatile("focusenergy")) addVolatile("focusenergy");
    }

    void transformInto(const Pokemon& target) {
        addVolatile("transform");
        type1 = target.getType1(); type2 = target.getType2();
        baseAttack = target.getBaseAttack(); baseDefense = target.getBaseDefense();
        baseSpAttack = target.getBaseSpAttack(); baseSpDefense = target.getBaseSpDefense();
        baseSpeed = target.getBaseSpeed();
        ability = target.getAbility();
        moves = target.getMoves();

        // NEW: Transform canonically limits copied moves to 5 PP
        for (const auto& m : moves) {
            setMaxMovePP(m, target.getMaxMovePP(m));
            setMovePP(m, 5);
        }

        calculateStats();
    }

    void megaEvolve(const Pokemon& megaData) {
        name = megaData.getName();
        type1 = megaData.getType1();
        type2 = megaData.getType2();
        baseAttack = megaData.getBaseAttack();
        baseDefense = megaData.getBaseDefense();
        baseSpAttack = megaData.getBaseSpAttack();
        baseSpDefense = megaData.getBaseSpDefense();
        baseSpeed = megaData.getBaseSpeed();
        ability = megaData.getAbility();
        weight = megaData.getWeight();
        calculateStats();
    }

    std::string getNature() const { return nature; }
    std::string getAbility() const { return ability; }
    void setNature(const std::string& nat) { nature = nat; calculateStats(); }
    void setAbility(const std::string& ab) { ability = ab; }

    int getEvHp() const { return evHp; }
    int getEvAtk() const { return evAtk; }
    int getEvDef() const { return evDef; }
    int getEvSpA() const { return evSpA; }
    int getEvSpD() const { return evSpD; }
    int getEvSpe() const { return evSpe; }
    void setEvs(int h, int a, int d, int sa, int sd, int s) { evHp = h; evAtk = a; evDef = d; evSpA = sa; evSpD = sd; evSpe = s; calculateStats(); }

    int getLevel() const { return level; }
    std::string getHeldItem() const { return heldItem; }
    int getBoxNumber() const { return boxNumber; }

    void setId(int newId) { id = newId; }
    void setLevel(int lvl) { level = lvl; calculateStats(); }
    void setHeldItem(const std::string& item) { heldItem = item; }
    void setBoxNumber(int box) { boxNumber = box; }
    void setNickname(const std::string& nick) { nickname = nick; }

    const std::vector<std::string>& getMoves() const { return moves; }
    void setMoves(const std::vector<std::string>& newMoves) { moves = newMoves; origMoves = newMoves; }
};