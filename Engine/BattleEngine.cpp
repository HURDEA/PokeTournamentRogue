#include "BattleEngine.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cctype>

BattleEngine::BattleEngine(Controller& ctrl) : controller(ctrl) {}

std::string BattleEngine::normalizeString(const std::string& str) {
    std::string res;
    for (char c : str) {
        if (c != ' ' && c != '-' && c != '\'') {
            res += std::toupper(c);
        }
    }
    return res;
}

double BattleEngine::getTypeEffectiveness(const std::string& atk, const std::string& def) {
    if (atk == "None" || def == "None") return 1.0;
    static std::map<std::string, std::map<std::string, double>> chart = {
        {"Normal", {{"Rock", 0.5}, {"Ghost", 0.0}, {"Steel", 0.5}}},
        {"Fire", {{"Fire", 0.5}, {"Water", 0.5}, {"Grass", 2.0}, {"Ice", 2.0}, {"Bug", 2.0}, {"Rock", 0.5}, {"Dragon", 0.5}, {"Steel", 2.0}}},
        {"Water", {{"Fire", 2.0}, {"Water", 0.5}, {"Grass", 0.5}, {"Ground", 2.0}, {"Rock", 2.0}, {"Dragon", 0.5}}},
        {"Electric", {{"Water", 2.0}, {"Electric", 0.5}, {"Grass", 0.5}, {"Ground", 0.0}, {"Flying", 2.0}, {"Dragon", 0.5}}},
        {"Grass", {{"Fire", 0.5}, {"Water", 2.0}, {"Grass", 0.5}, {"Poison", 0.5}, {"Ground", 2.0}, {"Flying", 0.5}, {"Bug", 0.5}, {"Rock", 2.0}, {"Dragon", 0.5}, {"Steel", 0.5}}},
        {"Ice", {{"Fire", 0.5}, {"Water", 0.5}, {"Grass", 2.0}, {"Ice", 0.5}, {"Ground", 2.0}, {"Flying", 2.0}, {"Dragon", 2.0}, {"Steel", 0.5}}},
        {"Fighting", {{"Normal", 2.0}, {"Ice", 2.0}, {"Poison", 0.5}, {"Flying", 0.5}, {"Psychic", 0.5}, {"Bug", 0.5}, {"Rock", 2.0}, {"Ghost", 0.0}, {"Dark", 2.0}, {"Steel", 2.0}, {"Fairy", 0.5}}},
        {"Poison", {{"Grass", 2.0}, {"Poison", 0.5}, {"Ground", 0.5}, {"Rock", 0.5}, {"Ghost", 0.5}, {"Steel", 0.0}, {"Fairy", 2.0}}},
        {"Ground", {{"Fire", 2.0}, {"Electric", 2.0}, {"Grass", 0.5}, {"Poison", 2.0}, {"Flying", 0.0}, {"Bug", 0.5}, {"Rock", 2.0}, {"Steel", 2.0}}},
        {"Flying", {{"Electric", 0.5}, {"Grass", 2.0}, {"Fighting", 2.0}, {"Bug", 2.0}, {"Rock", 0.5}, {"Steel", 0.5}}},
        {"Psychic", {{"Fighting", 2.0}, {"Poison", 2.0}, {"Psychic", 0.5}, {"Dark", 0.0}, {"Steel", 0.5}}},
        {"Bug", {{"Fire", 0.5}, {"Grass", 2.0}, {"Fighting", 0.5}, {"Poison", 0.5}, {"Flying", 0.5}, {"Psychic", 2.0}, {"Ghost", 0.5}, {"Dark", 2.0}, {"Steel", 0.5}, {"Fairy", 0.5}}},
        {"Rock", {{"Fire", 2.0}, {"Ice", 2.0}, {"Fighting", 0.5}, {"Ground", 0.5}, {"Flying", 2.0}, {"Bug", 2.0}, {"Steel", 0.5}}},
        {"Ghost", {{"Normal", 0.0}, {"Psychic", 2.0}, {"Ghost", 2.0}, {"Dark", 0.5}}},
        {"Dragon", {{"Dragon", 2.0}, {"Steel", 0.5}, {"Fairy", 0.0}}},
        {"Dark", {{"Fighting", 0.5}, {"Psychic", 2.0}, {"Ghost", 2.0}, {"Dark", 0.5}, {"Fairy", 0.5}}},
        {"Steel", {{"Fire", 0.5}, {"Water", 0.5}, {"Electric", 0.5}, {"Ice", 2.0}, {"Rock", 2.0}, {"Steel", 0.5}, {"Fairy", 2.0}}},
        {"Fairy", {{"Fire", 0.5}, {"Fighting", 2.0}, {"Poison", 0.5}, {"Dragon", 2.0}, {"Dark", 2.0}, {"Steel", 0.5}}}
    };
    if (chart.count(atk) && chart[atk].count(def)) return chart[atk][def];
    return 1.0;
}

std::string BattleEngine::getActiveWeather(const Pokemon& p1, const Pokemon& p2) {
    std::string a1 = p1.getAbility();
    std::string a2 = p2.getAbility();
    if (a1 == "Cloud Nine" || a1 == "Air Lock" || a2 == "Cloud Nine" || a2 == "Air Lock") {
        return "None";
    }
    return field.weather;
}

void BattleEngine::checkExtremeWeather(const Pokemon& p1, const Pokemon& p2, std::vector<std::string>& log) {
    if (field.weather == "DesolateLand") {
        if (p1.getAbility() != "Desolate Land" && p2.getAbility() != "Desolate Land") {
            field.weather = "None";
            field.weatherTurns = 0;
            log.push_back("The harsh sunlight faded.");
        }
    }
    else if (field.weather == "PrimordialSea") {
        if (p1.getAbility() != "Primordial Sea" && p2.getAbility() != "Primordial Sea") {
            field.weather = "None";
            field.weatherTurns = 0;
            log.push_back("The heavy rain has lifted!");
        }
    }
    else if (field.weather == "DeltaStream") {
        if (p1.getAbility() != "Delta Stream" && p2.getAbility() != "Delta Stream") {
            field.weather = "None";
            field.weatherTurns = 0;
            log.push_back("The mysterious air current has dissipated!");
        }
    }
}

int BattleEngine::getEffectiveSpeed(const Pokemon& p, const Pokemon& opp) {
    int spd = p.getBattleSpeed();
    std::string abil = p.getAbility();
    std::string item = (field.magicRoom > 0) ? "NONE" : normalizeString(p.getHeldItem());
    std::string weather = getActiveWeather(p, opp);

    bool isPlayerSide = (p.getBoxNumber() == 0);
    const SideState& side = isPlayerSide ? playerSide : enemySide;

    if (abil == "Swift Swim" && (weather == "Rain" || weather == "PrimordialSea")) spd *= 2;
    if (abil == "Chlorophyll" && (weather == "Sun" || weather == "DesolateLand")) spd *= 2;
    if (abil == "Sand Rush" && weather == "Sandstorm") spd *= 2;
    if (abil == "Slush Rush" && (weather == "Snow" || weather == "Hail")) spd *= 2;
    if (abil == "Surge Surfer" && field.terrain == "Electric Terrain") spd *= 2;
    if (abil == "Quick Feet" && !p.getStatus().empty()) spd = std::floor(spd * 1.5);

    if (abil == "Unburden" && p.hasVolatile("unburden")) spd *= 2;
    if (abil == "Slow Start" && p.hasVolatile("slowstart")) spd = std::floor(spd * 0.5);

    if (item == "CHOICESCARF") spd = std::floor(spd * 1.5);
    if (item == "IRONBALL" || item == "MACHOBRACE" || item == "POWERWEIGHT" || item == "POWERBRACER" || item == "POWERBELT" || item == "POWERLENS" || item == "POWERBAND" || item == "POWERANKLET") spd = std::floor(spd * 0.5);
    if (item == "QUICKPOWDER" && p.getName() == "Ditto" && !p.hasVolatile("transform")) spd *= 2;
    if (item == "ROOMSERVICE" && field.trickRoom > 0) spd = std::floor(spd * 0.67);

    // Apply Tailwind Multiplier
    if (side.tailwind > 0) spd *= 2;

    bool proto = (abil == "Protosynthesis" && (weather == "Sun" || weather == "DesolateLand" || item == "BOOSTERENERGY"));
    bool quark = (abil == "Quark Drive" && (field.terrain == "Electric Terrain" || item == "BOOSTERENERGY"));
    if (proto || quark) {
        int stats[5] = { p.getBattleAttack(), p.getBattleDefense(), p.getBattleSpAttack(), p.getBattleSpDefense(), p.getBattleSpeed() };
        int maxIdx = 0;
        for (int j = 1; j < 5; ++j) { if (stats[j] > stats[maxIdx]) maxIdx = j; }
        if (maxIdx == 4) spd = std::floor(spd * 1.5);
    }

    return spd;
}

void BattleEngine::onSwitchIn(Pokemon& p, bool isPlayer, std::vector<std::string>& log, Pokemon* opp) {
    SideState& side = isPlayer ? playerSide : enemySide;

    p.removeVolatile("confusion");
    p.removeVolatile("taunt");
    p.removeVolatile("moved_this_turn");
    p.resetProtectCounter();

    p.isSwitchingOut = false;
    p.removeVolatile("forced_switch");
    p.lastDamageTaken = 0;
    p.lastDamageCategory = "";

    p.addVolatile("first_turn", 1);

    std::string abil = p.getAbility();
    if (abil == "As One (Glastrier)") abil = "Chilling Neigh";
    if (abil == "As One (Spectrier)") abil = "Grim Neigh";

    std::string item = (field.magicRoom > 0) ? "NONE" : normalizeString(p.getHeldItem());
    bool hdb = (item == "HEAVYDUTYBOOTS");

    bool gasActive = false;
    if (opp && opp->getCurrentHp() > 0 && opp->getAbility() == "Neutralizing Gas") gasActive = true;
    if (abil == "Neutralizing Gas") gasActive = true;

    if (gasActive && abil != "Neutralizing Gas" && abil != "Stance Change" && abil != "Multitype" && abil != "Zero to Hero" && abil != "Ice Face" && abil != "Disguise" && abil != "Gulp Missile") {
        abil = "None";
    }

    if (abil == "Neutralizing Gas") {
        log.push_back("Neutralizing gas filled the area!");
    }

    if (abil == "Illusion") {
        p.addVolatile("illusion", 1);
        log.push_back(p.getNickname() + " cast an Illusion!");
    }

    if (abil == "Zero to Hero" && p.hasVolatile("hero_form")) {
        log.push_back(p.getNickname() + " underwent a heroic transformation!");
    }

    auto removeItem = [&](Pokemon& pkm) {
        pkm.setHeldItem("None");
        if (pkm.getAbility() == "Unburden" && !pkm.hasVolatile("unburden")) {
            pkm.addVolatile("unburden", 1);
            log.push_back(pkm.getNickname() + "'s Unburden doubled its Speed!");
        }
        };

    if (abil == "Imposter" && opp && opp->getCurrentHp() > 0) {
        p.transformInto(*opp);
        log.push_back(p.getNickname() + " transformed into " + opp->getName() + " using Imposter!");
    }

    if (abil == "Slow Start") {
        p.addVolatile("slowstart", 5);
        log.push_back(p.getNickname() + " can't get it going!");
    }

    if (abil == "Ice Face" && p.hasVolatile("iceface_busted") && (field.weather == "Snow" || field.weather == "Hail")) {
        p.removeVolatile("iceface_busted");
        log.push_back(p.getNickname() + "'s Ice Face was restored by the snow!");
    }

    if (abil == "Frisk" && opp && opp->getCurrentHp() > 0 && opp->getHeldItem() != "None") {
        log.push_back(p.getNickname() + "'s Frisk revealed the opposing " + opp->getNickname() + "'s " + opp->getHeldItem() + "!");
    }

    if (abil == "Forewarn" && opp && opp->getCurrentHp() > 0) {
        std::string highestMove = "";
        int highestPower = -1;
        for (const auto& mId : opp->getMoves()) {
            MoveData md = controller.getMoveData(mId);
            int pow = md.ohko ? 150 : (md.basePower == 0 && md.category != "Status" ? 80 : md.basePower);
            if (pow > highestPower) { highestPower = pow; highestMove = md.name; }
        }
        if (highestPower > 0) log.push_back(p.getNickname() + "'s Forewarn alerted it to " + highestMove + "!");
    }

    if (abil == "Anticipation" && opp && opp->getCurrentHp() > 0) {
        bool shudder = false;
        for (const auto& mId : opp->getMoves()) {
            MoveData md = controller.getMoveData(mId);
            if (md.ohko || (md.category != "Status" && getTypeEffectiveness(md.type, p.getType1()) * getTypeEffectiveness(md.type, p.getType2()) > 1.0)) {
                shudder = true; break;
            }
        }
        if (shudder) log.push_back(p.getNickname() + " shuddered in anticipation!");
    }

    if (abil != "Magic Guard" && !hdb) {
        if (side.stealthRock) {
            double effect = getTypeEffectiveness("Rock", p.getType1()) * getTypeEffectiveness("Rock", p.getType2());
            int damage = p.getHp() * (0.125 * effect);
            p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, damage)));
            log.push_back("Pointed stones dug into " + p.getNickname() + "!");
        }
        if (side.spikes > 0 && p.getType1() != "Flying" && p.getType2() != "Flying" && abil != "Levitate" && item != "AIRBALLOON") {
            double fraction = (side.spikes == 1) ? (1.0 / 8.0) : (side.spikes == 2) ? (1.0 / 6.0) : (1.0 / 4.0);
            int damage = p.getHp() * fraction;
            p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, damage)));
            log.push_back(p.getNickname() + " was hurt by the spikes!");
        }
        if (side.toxicSpikes > 0 && p.getType1() != "Flying" && p.getType2() != "Flying" && abil != "Levitate" && item != "AIRBALLOON") {
            if (p.getType1() == "Poison" || p.getType2() == "Poison") {
                side.toxicSpikes = 0;
                log.push_back(p.getNickname() + " absorbed the Toxic Spikes!");
            }
            else if (p.getType1() != "Steel" && p.getType2() != "Steel" && p.getStatus().empty()) {
                p.setStatus(side.toxicSpikes == 1 ? "psn" : "tox");
                log.push_back(p.getNickname() + " was poisoned by the Toxic Spikes!");
            }
        }
    }
    if (side.stickyWeb && !hdb && p.getType1() != "Flying" && p.getType2() != "Flying" && abil != "Levitate" && item != "AIRBALLOON" && abil != "Clear Body" && abil != "White Smoke" && abil != "Full Metal Body") {
        if (item == "CLEARAMULET") {
            log.push_back(p.getNickname() + "'s Clear Amulet protected it from the sticky web!");
        }
        else {
            p.modifyStat("spe", -1, log);
            log.push_back(p.getNickname() + " was caught in a sticky web!");
        }
    }

    if (abil == "Intimidate" && opp && opp->getCurrentHp() > 0 &&
        opp->getAbility() != "Clear Body" && opp->getAbility() != "White Smoke" &&
        opp->getAbility() != "Full Metal Body" && opp->getAbility() != "Inner Focus" &&
        opp->getAbility() != "Oblivious" && opp->getAbility() != "Own Tempo" &&
        opp->getAbility() != "Scrappy" && opp->getAbility() != "Guard Dog" &&
        normalizeString(opp->getHeldItem()) != "CLEARAMULET") {

        log.push_back(p.getNickname() + "'s Intimidate cuts " + opp->getNickname() + "'s Attack!");
        opp->modifyStat("atk", -1, log);
        if (opp->getAbility() == "Defiant") {
            opp->modifyStat("atk", 2, log);
            log.push_back(opp->getNickname() + "'s Defiant sharply raised its Attack!");
        }
        else if (opp->getAbility() == "Competitive") {
            opp->modifyStat("spa", 2, log);
            log.push_back(opp->getNickname() + "'s Competitive sharply raised its Special Attack!");
        }
    }
    else if (abil == "Intimidate" && opp && (opp->getAbility() == "Inner Focus" || opp->getAbility() == "Oblivious" || opp->getAbility() == "Own Tempo" || opp->getAbility() == "Scrappy")) {
        log.push_back(opp->getNickname() + "'s " + opp->getAbility() + " prevents Intimidate!");
    }
    else if (abil == "Intimidate" && opp && opp->getAbility() == "Guard Dog") {
        log.push_back(opp->getNickname() + "'s Guard Dog prevents Intimidate and raises Attack!");
        opp->modifyStat("atk", 1, log);
    }

    if (abil == "Download" && opp && opp->getCurrentHp() > 0) {
        log.push_back(p.getNickname() + "'s Download activated!");
        if (opp->getBattleDefense() < opp->getBattleSpDefense()) p.modifyStat("atk", 1, log);
        else p.modifyStat("spa", 1, log);
    }
    if (abil == "Trace" && opp && opp->getAbility() != "None" && opp->getAbility() != "Trace") {
        p.setAbility(opp->getAbility());
        log.push_back(p.getNickname() + " traced " + opp->getNickname() + "'s " + opp->getAbility() + "!");
    }

    if (abil == "Intrepid Sword") { p.modifyStat("atk", 1, log); log.push_back(p.getNickname() + "'s Intrepid Sword raised its Attack!"); }
    if (abil == "Dauntless Shield") { p.modifyStat("def", 1, log); log.push_back(p.getNickname() + "'s Dauntless Shield raised its Defense!"); }
    if (abil == "Screen Cleaner") {
        playerSide.reflect = 0; playerSide.lightScreen = 0; playerSide.auroraVeil = 0;
        enemySide.reflect = 0; enemySide.lightScreen = 0; enemySide.auroraVeil = 0;
        log.push_back(p.getNickname() + "'s Screen Cleaner cleaned up all screens!");
    }
    if (abil == "Supersweet Syrup" && opp && opp->getCurrentHp() > 0 && opp->getAbility() != "Clear Body" && opp->getAbility() != "White Smoke" && opp->getAbility() != "Full Metal Body" && normalizeString(opp->getHeldItem()) != "CLEARAMULET") {
        log.push_back(p.getNickname() + "'s Supersweet Syrup lowers " + opp->getNickname() + "'s Evasiveness!");
        opp->modifyStat("evasion", -1, log);
    }
    if (abil == "Curious Medicine" && opp) {
        p.resetStatStages();
        log.push_back(p.getNickname() + "'s Curious Medicine reset its stat changes!");
    }
    if (abil == "Pastel Veil" && (p.getStatus() == "psn" || p.getStatus() == "tox")) {
        p.setStatus(""); log.push_back(p.getNickname() + " was cured of its poisoning by Pastel Veil!");
    }

    bool extremeWeather = (field.weather == "DesolateLand" || field.weather == "PrimordialSea" || field.weather == "DeltaStream");
    if (abil == "Desolate Land") { field.weather = "DesolateLand"; field.weatherTurns = -1; log.push_back("The sunlight turned extremely harsh!"); }
    else if (abil == "Primordial Sea") { field.weather = "PrimordialSea"; field.weatherTurns = -1; log.push_back("A heavy rain began to fall!"); }
    else if (abil == "Delta Stream") { field.weather = "DeltaStream"; field.weatherTurns = -1; log.push_back("A mysterious air current is protecting Flying-type Pokémon!"); }
    else if (!extremeWeather) {
        if (abil == "Drizzle" && field.weather != "Rain") { field.weather = "Rain"; field.weatherTurns = (item == "DAMPROCK") ? 8 : 5; log.push_back("It started to rain!"); }
        else if (abil == "Drought" && field.weather != "Sun") { field.weather = "Sun"; field.weatherTurns = (item == "HEATROCK") ? 8 : 5; log.push_back("The sunlight turned harsh!"); }
        else if (abil == "Sand Stream" && field.weather != "Sandstorm") { field.weather = "Sandstorm"; field.weatherTurns = (item == "SMOOTHROCK") ? 8 : 5; log.push_back("A sandstorm kicked up!"); }
        else if ((abil == "Snow Warning" || abil == "Warning") && field.weather != "Snow" && field.weather != "Hail") { field.weather = "Snow"; field.weatherTurns = (item == "ICYROCK") ? 8 : 5; log.push_back("It started to snow!"); }
        else if (abil == "Orichalcum Pulse") { field.weather = "Sun"; field.weatherTurns = (item == "HEATROCK") ? 8 : 5; log.push_back(p.getNickname() + " turned the sunlight harsh!"); }
    }

    if (abil == "Electric Surge" && field.terrain != "Electric Terrain") { field.terrain = "Electric Terrain"; field.terrainTurns = (item == "TERRAINEXTENDER") ? 8 : 5; log.push_back("An electric current runs across the battlefield!"); }
    else if (abil == "Grassy Surge" && field.terrain != "Grassy Terrain") { field.terrain = "Grassy Terrain"; field.terrainTurns = (item == "TERRAINEXTENDER") ? 8 : 5; log.push_back("Grass grew to cover the battlefield!"); }
    else if (abil == "Misty Surge" && field.terrain != "Misty Terrain") { field.terrain = "Misty Terrain"; field.terrainTurns = (item == "TERRAINEXTENDER") ? 8 : 5; log.push_back("Mist swirled around the battlefield!"); }
    else if (abil == "Psychic Surge" && field.terrain != "Psychic Terrain") { field.terrain = "Psychic Terrain"; field.terrainTurns = (item == "TERRAINEXTENDER") ? 8 : 5; log.push_back("The battlefield got weird!"); }
    else if (abil == "Hadron Engine" && field.terrain != "Electric Terrain") { field.terrain = "Electric Terrain"; field.terrainTurns = (item == "TERRAINEXTENDER") ? 8 : 5; log.push_back(p.getNickname() + " turned the ground into Electric Terrain!"); }

    auto checkSeeds = [&](Pokemon& pkmn) {
        std::string it = (field.magicRoom > 0) ? "NONE" : normalizeString(pkmn.getHeldItem());
        if (field.terrain == "Electric Terrain" && it == "ELECTRICSEED") { pkmn.modifyStat("def", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Electric Seed!"); }
        else if (field.terrain == "Grassy Terrain" && it == "GRASSYSEED") { pkmn.modifyStat("def", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Grassy Seed!"); }
        else if (field.terrain == "Psychic Terrain" && it == "PSYCHICSEED") { pkmn.modifyStat("spd", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Psychic Seed!"); }
        else if (field.terrain == "Misty Terrain" && it == "MISTYSEED") { pkmn.modifyStat("spd", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Misty Seed!"); }
        };
    checkSeeds(p);
}

int BattleEngine::calculateDamageInternal(Pokemon& attacker, Pokemon& defender, const MoveData& move, bool isCrit, std::vector<std::string>& log, bool isFirstHit) {
    if (move.name == "Seismic Toss" || move.name == "Night Shade") return std::max(1, attacker.getLevel());
    if (move.name == "Dragon Rage") return 40;
    if (move.name == "Sonic Boom") return 20;
    if (move.name == "Super Fang" || move.name == "Nature's Madness" || move.name == "Ruination") return std::max(1, defender.getCurrentHp() / 2);
    if (move.name == "Guardian of Alola") return std::max(1, (defender.getCurrentHp() * 3) / 4);
    if (move.name == "Endeavor") {
        if (defender.getCurrentHp() <= attacker.getCurrentHp()) return 0;
        return defender.getCurrentHp() - attacker.getCurrentHp();
    }
    if (move.name == "Final Gambit") return attacker.getCurrentHp();

    std::string currentType = move.type;
    int currentPower = move.basePower;

    std::string aAbil = attacker.getAbility();
    std::string dAbil = defender.getAbility();
    std::string aItem = (field.magicRoom > 0) ? "NONE" : normalizeString(attacker.getHeldItem());
    std::string dItem = (field.magicRoom > 0) ? "NONE" : normalizeString(defender.getHeldItem());

    bool gasActive = false;
    if (aAbil == "Neutralizing Gas" || dAbil == "Neutralizing Gas") gasActive = true;

    auto applyGas = [&](std::string& ability) {
        if (gasActive && ability != "Neutralizing Gas" && ability != "Stance Change" && ability != "Multitype" && ability != "Zero to Hero" && ability != "Ice Face" && ability != "Disguise" && ability != "Gulp Missile") {
            ability = "None";
        }
        };
    applyGas(aAbil);
    applyGas(dAbil);

    if (move.name == "Heat Crash" || move.name == "Heavy Slam" || move.name == "Grass Knot" || move.name == "Low Kick") {
        double aWeight = attacker.getWeight();
        if (aAbil == "Heavy Metal") aWeight *= 2.0;
        if (aAbil == "Light Metal") aWeight *= 0.5;
        if (aItem == "FLOATSTONE") aWeight *= 0.5;

        double dWeight = defender.getWeight();
        if (dAbil == "Heavy Metal") dWeight *= 2.0;
        if (dAbil == "Light Metal") dWeight *= 0.5;
        if (dItem == "FLOATSTONE") dWeight *= 0.5;

        if (move.name == "Grass Knot" || move.name == "Low Kick") {
            if (dWeight >= 200.0) currentPower = 120;
            else if (dWeight >= 100.0) currentPower = 100;
            else if (dWeight >= 50.0) currentPower = 80;
            else if (dWeight >= 25.0) currentPower = 60;
            else if (dWeight >= 10.0) currentPower = 40;
            else currentPower = 20;
        }
        else {
            double ratio = aWeight / dWeight;
            if (ratio >= 5.0) currentPower = 120;
            else if (ratio >= 4.0) currentPower = 100;
            else if (ratio >= 3.0) currentPower = 80;
            else if (ratio >= 2.0) currentPower = 60;
            else currentPower = 40;
        }
    }

    if (move.name == "Return" || move.name == "Frustration") currentPower = 102;
    else if (move.name == "Crush Grip" || move.name == "Wring Out" || move.name == "Hard Press" || move.name == "Trump Card" || move.name == "Fling" || move.name == "Natural Gift" || move.name == "Spit Up") {
        currentPower = 120;
    }
    else if (move.name == "Beat Up") {
        currentPower = 25;
        currentType = "Dark";
    }

    if (move.name.find("Hidden Power") != std::string::npos) {
        std::vector<std::string> allTypes = { "Normal", "Fire", "Water", "Electric", "Grass", "Ice", "Fighting", "Poison", "Ground", "Flying", "Psychic", "Bug", "Rock", "Ghost", "Dragon", "Dark", "Steel", "Fairy" };
        std::vector<std::string> attackerWeaknesses;

        std::string aT1 = attacker.getType1();
        std::string aT2 = attacker.getType2();
        if (attacker.hasVolatile("roost")) {
            if (aT1 == "Flying") aT1 = "Normal";
            if (aT2 == "Flying") aT2 = "None";
        }

        for (const auto& t : allTypes) {
            if (getTypeEffectiveness(t, aT1) * getTypeEffectiveness(t, aT2) > 1.0) {
                attackerWeaknesses.push_back(t);
            }
        }
        currentType = "Normal";
        if (!attackerWeaknesses.empty()) {
            for (const auto& candidate : allTypes) {
                if (getTypeEffectiveness(candidate, attackerWeaknesses[0]) > 1.0) {
                    currentType = candidate; break;
                }
            }
        }
        currentPower = 60;
    }

    bool isSound = (move.name == "Boomburst" || move.name == "Hyper Voice" || move.name == "Uproar" || move.name == "Snarl" || move.name == "Overdrive" || move.name == "Bug Buzz" || move.name == "Echoed Voice" || move.name == "Relic Song" || move.name == "Clanging Scales" || move.name == "Alluring Voice" || move.name == "Psychic Noise" || move.name == "Disarming Voice" || move.name == "Sparkling Aria");
    if (dAbil == "Soundproof" && isSound) { if (isFirstHit) log.push_back(defender.getNickname() + "'s Soundproof blocks the attack!"); return 0; }
    if (dAbil == "Bulletproof" && (move.name == "Shadow Ball" || move.name == "Sludge Bomb" || move.name == "Focus Blast" || move.name == "Energy Ball" || move.name == "Bullet Seed" || move.name == "Aura Sphere" || move.name == "Seed Bomb" || move.name == "Gyro Ball" || move.name == "Electro Ball" || move.name == "Weather Ball" || move.name == "Pyro Ball" || move.name == "Pollen Puff" || move.name == "Acid Spray" || move.name == "Beak Blast" || move.name == "Mist Ball" || move.name == "Octazooka" || move.name == "Rock Wrecker" || move.name == "Searing Shot")) {
        if (isFirstHit) log.push_back(defender.getNickname() + "'s Bulletproof blocks the attack!"); return 0;
    }

    if (move.name == "Aura Wheel") {
        if (attacker.hasVolatile("hangry")) currentType = "Dark"; else currentType = "Electric";
    }

    if (aAbil == "Gulp Missile" && (move.name == "Surf" || move.name == "Dive")) {
        if (attacker.getCurrentHp() > attacker.getHp() / 2) {
            attacker.addVolatile("gulping", 1);
            if (isFirstHit) log.push_back(attacker.getNickname() + " caught an Arrokuda!");
        }
        else {
            attacker.addVolatile("gorging", 1);
            if (isFirstHit) log.push_back(attacker.getNickname() + " caught a Pikachu!");
        }
    }

    if (aAbil == "Normalize") { currentType = "Normal"; currentPower = std::floor(currentPower * 1.2); }
    else if (currentType == "Normal") {
        if (aAbil == "Galvanize") { currentType = "Electric"; currentPower = std::floor(currentPower * 1.2); }
        else if (aAbil == "Aerilate") { currentType = "Flying"; currentPower = std::floor(currentPower * 1.2); }
        else if (aAbil == "Pixilate") { currentType = "Fairy"; currentPower = std::floor(currentPower * 1.2); }
        else if (aAbil == "Refrigerate") { currentType = "Ice"; currentPower = std::floor(currentPower * 1.2); }
        else if (aAbil == "Fire Mane") { currentType = "Fire"; currentPower = std::floor(currentPower * 1.2); }
        else if (aAbil == "Dragonize") { currentType = "Dragon"; currentPower = std::floor(currentPower * 1.2); }
    }

    if (aAbil == "Liquid Voice" && isSound) currentType = "Water";

    if (move.name == "Gyro Ball") {
        int aSpd = getEffectiveSpeed(attacker, defender);
        int dSpd = getEffectiveSpeed(defender, attacker);
        if (aSpd == 0) aSpd = 1;
        currentPower = std::min(150, static_cast<int>(25.0 * dSpd / aSpd) + 1);
    }
    else if (move.name == "Electro Ball") {
        int aSpd = getEffectiveSpeed(attacker, defender);
        int dSpd = getEffectiveSpeed(defender, attacker);
        if (dSpd == 0) dSpd = 1;
        double ratio = static_cast<double>(aSpd) / dSpd;
        if (ratio >= 4.0) currentPower = 150;
        else if (ratio >= 3.0) currentPower = 120;
        else if (ratio >= 2.0) currentPower = 80;
        else if (ratio >= 1.0) currentPower = 60;
        else currentPower = 40;
    }
    else if (move.name == "Rollout" || move.name == "Ice Ball") {
        int count = attacker.getVolatileTurns("rollout_count");
        currentPower = 30 * static_cast<int>(std::pow(2, std::min(4, count)));
        if (attacker.hasVolatile("defensecurl")) currentPower *= 2;
    }
    else if (move.name == "Eruption" || move.name == "Water Spout" || move.name == "Dragon Energy") {
        currentPower = std::max(1, (150 * attacker.getCurrentHp()) / attacker.getHp());
    }
    else if (move.name == "Flail" || move.name == "Reversal") {
        double p = 48.0 * attacker.getCurrentHp() / attacker.getHp();
        if (p <= 1) currentPower = 200; else if (p <= 4) currentPower = 150; else if (p <= 9) currentPower = 100; else if (p <= 16) currentPower = 80; else if (p <= 32) currentPower = 40; else currentPower = 20;
    }

    if (currentPower == 0) return 0;

    double aAtk = attacker.getBattleAttack(isCrit);
    double aSpA = attacker.getBattleSpAttack(isCrit);
    double aDef = (field.wonderRoom > 0) ? attacker.getBattleSpDefense(isCrit) : attacker.getBattleDefense(isCrit);

    double dDef = (field.wonderRoom > 0) ? defender.getBattleSpDefense(isCrit) : defender.getBattleDefense(isCrit);
    double dSpDef = (field.wonderRoom > 0) ? defender.getBattleDefense(isCrit) : defender.getBattleSpDefense(isCrit);

    if (dAbil == "Fur Coat" && move.category == "Physical") dDef *= 2.0;

    double a = (move.category == "Special") ? aSpA : aAtk;
    double d = (move.category == "Special") ? dSpDef : dDef;

    if (aAbil == "Slow Start" && attacker.hasVolatile("slowstart") && move.category == "Physical") a *= 0.5;

    if (dAbil == "Unaware") {
        a = (move.category == "Special") ? attacker.getSpAttack() : attacker.getAttack();
    }
    if (aAbil == "Unaware") {
        d = (move.category == "Special") ? defender.getSpDefense() : defender.getDefense();
        if (dAbil == "Fur Coat" && move.category == "Physical") d *= 2.0;
    }

    if (move.overrideOffensivePokemon == "target") a = (move.category == "Special") ? defender.getBattleSpAttack(isCrit) : defender.getBattleAttack(isCrit);
    if (move.overrideOffensiveStat == "def") a = aDef;
    if (move.overrideOffensiveStat == "spd") a = (field.wonderRoom > 0) ? attacker.getBattleDefense(isCrit) : attacker.getBattleSpDefense(isCrit);
    if (move.overrideDefensiveStat == "def") d = dDef;
    if (move.overrideDefensiveStat == "spd") d = dSpDef;

    if (move.name == "Sacred Sword" || move.name == "Darkest Lariat" || move.name == "Chip Away") {
        d = (move.category == "Special") ? defender.getSpDefense() : defender.getDefense();
        if (field.wonderRoom > 0) d = (move.category == "Special") ? defender.getDefense() : defender.getSpDefense();
        if (dAbil == "Fur Coat" && move.category == "Physical") d *= 2.0;
    }

    if (dItem == "ASSAULTVEST" && move.category == "Special") d *= 1.5;

    auto isNFE = [](const std::string& name) {
        static const std::vector<std::string> nfe = { "Chansey", "Porygon2", "Dusclops", "Clefairy", "Togetic", "Rhydon", "Electabuzz", "Magmar", "Sneasel", "Gligar", "Scyther", "Golbat", "Magneton", "Corsola-Galar", "Galarian Corsola", "Bisharp", "Duraludon", "Pikachu", "Doublade" };
        return std::find(nfe.begin(), nfe.end(), name) != nfe.end();
        };
    if (dItem == "EVIOLITE" && isNFE(defender.getName())) {
        d *= 1.5; // Eviolite boosts both Def and SpDef
    }

    // --- SCREEN / SHIELD MITIGATION FIX ---
    bool isPlayerAttacker = (attacker.getBoxNumber() == 0);
    const SideState& defSide = isPlayerAttacker ? enemySide : playerSide;

    if (!isCrit && aAbil != "Infiltrator") {
        // Brick Break and Psychic Fangs ignore screens completely
        if (move.name != "Brick Break" && move.name != "Psychic Fangs" && move.name != "Raging Bull") {
            if (move.category == "Physical" && (defSide.reflect > 0 || defSide.auroraVeil > 0)) {
                d *= 2.0; // In Singles, screens double effective defense
            }
            if (move.category == "Special" && (defSide.lightScreen > 0 || defSide.auroraVeil > 0)) {
                d *= 2.0;
            }
        }
    }

    if (aAbil == "Sword of Ruin" && dAbil != "Sword of Ruin") d *= 0.75;
    if (aAbil == "Beads of Ruin" && dAbil != "Beads of Ruin") d *= 0.75;
    if (dAbil == "Tablets of Ruin" && aAbil != "Tablets of Ruin") a *= 0.75;
    if (dAbil == "Vessel of Ruin" && aAbil != "Vessel of Ruin") a *= 0.75;

    if (d <= 0) d = 1;

    std::string weather = getActiveWeather(attacker, defender);

    bool protoAttacker = (aAbil == "Protosynthesis" && (weather == "Sun" || weather == "DesolateLand" || aItem == "BOOSTERENERGY"));
    bool quarkAttacker = (aAbil == "Quark Drive" && (field.terrain == "Electric Terrain" || aItem == "BOOSTERENERGY"));
    if (protoAttacker || quarkAttacker) {
        int stats[5] = { attacker.getBattleAttack(), attacker.getBattleDefense(), attacker.getBattleSpAttack(), attacker.getBattleSpDefense(), attacker.getBattleSpeed() };
        int maxIdx = 0;
        for (int j = 1; j < 5; ++j) { if (stats[j] > stats[maxIdx]) maxIdx = j; }
        if ((move.category == "Physical" && maxIdx == 0) || (move.category == "Special" && maxIdx == 2)) a *= 1.3;
    }

    bool protoDefender = (dAbil == "Protosynthesis" && (weather == "Sun" || weather == "DesolateLand" || dItem == "BOOSTERENERGY"));
    bool quarkDefender = (dAbil == "Quark Drive" && (field.terrain == "Electric Terrain" || dItem == "BOOSTERENERGY"));
    if (protoDefender || quarkDefender) {
        int stats[5] = { defender.getBattleAttack(), defender.getBattleDefense(), defender.getBattleSpAttack(), defender.getBattleSpDefense(), defender.getBattleSpeed() };
        int maxIdx = 0;
        for (int j = 1; j < 5; ++j) { if (stats[j] > stats[maxIdx]) maxIdx = j; }
        if ((move.category == "Physical" && maxIdx == 1) || (move.category == "Special" && maxIdx == 3)) d *= 1.3;
    }

    if (move.name == "Avalanche" || move.name == "Revenge") { if (attacker.lastDamageTaken > 0) currentPower *= 2; }
    if (move.name == "Facade" && (attacker.getStatus() == "brn" || attacker.getStatus() == "psn" || attacker.getStatus() == "tox" || attacker.getStatus() == "par")) currentPower *= 2;
    if (move.name == "Brine" && defender.getCurrentHp() <= defender.getHp() / 2) currentPower *= 2;
    if (move.name == "Venoshock" && (defender.getStatus() == "psn" || defender.getStatus() == "tox")) currentPower *= 2;
    if (move.name == "Hex" && !defender.getStatus().empty()) currentPower *= 2;
    if (move.name == "Acrobatics" && aItem == "NONE") currentPower *= 2;
    if (move.name == "Knock Off" && dItem != "NONE" && dAbil != "Sticky Hold") currentPower = std::floor(currentPower * 1.5);
    if (move.name == "Solar Beam" || move.name == "Solar Blade") {
        if (weather == "Rain" || weather == "PrimordialSea" || weather == "Sandstorm" || weather == "Snow" || weather == "Hail") currentPower /= 2;
    }
    if (move.name == "Weather Ball" && weather != "None") {
        currentPower *= 2;
        if (weather == "Sun" || weather == "DesolateLand") currentType = "Fire";
        else if (weather == "Rain" || weather == "PrimordialSea") currentType = "Water";
        else if (weather == "Sandstorm") currentType = "Rock";
        else if (weather == "Snow" || weather == "Hail") currentType = "Ice";
    }
    if (move.name == "Judgment") {
        static std::map<std::string, std::string> plates = {
            {"FLAMEPLATE", "Fire"}, {"SPLASHPLATE", "Water"}, {"ZAPPLATE", "Electric"},
            {"MEADOWPLATE", "Grass"}, {"ICICLEPLATE", "Ice"}, {"FISTPLATE", "Fighting"},
            {"TOXICPLATE", "Poison"}, {"EARTHPLATE", "Ground"}, {"SKYPLATE", "Flying"},
            {"MINDPLATE", "Psychic"}, {"INSECTPLATE", "Bug"}, {"STONEPLATE", "Rock"},
            {"SPOOKYPLATE", "Ghost"}, {"DRACOPLATE", "Dragon"}, {"DREADPLATE", "Dark"},
            {"IRONPLATE", "Steel"}, {"PIXIEPLATE", "Fairy"}
        };
        if (plates.count(aItem)) currentType = plates[aItem];
    }

    bool aGrounded = (field.gravity > 0 || attacker.hasVolatile("smackdown") || (attacker.getType1() != "Flying" && attacker.getType2() != "Flying" && aAbil != "Levitate" && aItem != "AIRBALLOON"));
    bool dGrounded = (field.gravity > 0 || defender.hasVolatile("smackdown") || (defender.getType1() != "Flying" && defender.getType2() != "Flying" && dAbil != "Levitate" && dItem != "AIRBALLOON"));

    if (move.name == "Rising Voltage" && field.terrain == "Electric Terrain" && dGrounded) currentPower *= 2;
    if (move.name == "Expanding Force" && field.terrain == "Psychic Terrain" && aGrounded) currentPower = std::floor(currentPower * 1.5);

    if (aAbil == "Defeatist" && attacker.getCurrentHp() <= attacker.getHp() / 2) a *= 0.5;
    if (aAbil == "Solar Power" && (weather == "Sun" || weather == "DesolateLand") && move.category == "Special") a *= 1.5;
    if (dAbil == "Marvel Scale" && !defender.getStatus().empty() && move.category == "Physical") dDef *= 1.5;
    if (attacker.getStatus() == "brn" && move.category == "Physical" && aAbil != "Guts" && move.name != "Facade") a /= 2.0;

    if (aAbil == "Technician" && currentPower <= 60) currentPower = std::floor(currentPower * 1.5);
    if (aAbil == "Sheer Force" && move.effectChance > 0) currentPower = std::floor(currentPower * 1.3);

    double damage = std::floor(std::floor(std::floor(2.0 * attacker.getLevel() / 5.0 + 2.0) * currentPower * a / d) / 50.0) + 2.0;

    if (aAbil == "Supreme Overlord") {
        int fainted = 0;
        auto all = controller.getAllPokemon();
        for (const auto& p : all) {
            if (p.getBoxNumber() == attacker.getBoxNumber() && p.getCurrentHp() <= 0) fainted++;
        }
        damage *= (1.0 + (std::min(5, fainted) * 0.1));
    }

    double stab = 1.0;
    if (currentType != "None" && (currentType == attacker.getType1() || currentType == attacker.getType2())) {
        stab = (aAbil == "Adaptability") ? 2.0 : 1.5;
    }
    if ((aAbil == "Protean" || aAbil == "Libero") && !attacker.hasVolatile("protean_used")) {
        stab = (aAbil == "Adaptability") ? 2.0 : 1.5;
    }
    damage = std::floor(damage * stab);

    if (aAbil == "Analytic" && defender.hasVolatile("moved_this_turn")) damage *= 1.3;
    if (aAbil == "Stakeout" && defender.hasVolatile("first_turn")) damage *= 2.0;

    if (aItem == "CHOICEBAND" && move.category == "Physical") damage *= 1.5;
    if (aItem == "CHOICESPECS" && move.category == "Special") damage *= 1.5;
    if (aItem == "LIFEORB") damage *= 1.3;
    if (aItem == "MUSCLEBAND" && move.category == "Physical") damage *= 1.1;
    if (aItem == "WISEGLASSES" && move.category == "Special") damage *= 1.1;
    if (aItem == "PUNCHINGGLOVE" && (move.name.find("Punch") != std::string::npos || move.name == "Meteor Mash" || move.name == "Hammer Arm" || move.name == "Dynamic Punch")) damage *= 1.1;

    static std::map<std::string, std::string> typeBoostItems = {
        {"SILKSCARF", "Normal"}, {"CHARCOAL", "Fire"}, {"MYSTICWATER", "Water"}, {"MAGNET", "Electric"},
        {"MIRACLESEED", "Grass"}, {"NEVERMELTICE", "Ice"}, {"BLACKBELT", "Fighting"}, {"POISONBARB", "Poison"},
        {"SOFTSAND", "Ground"}, {"SHARPBEAK", "Flying"}, {"TWISTEDSPOON", "Psychic"}, {"SILVERPOWDER", "Bug"},
        {"HARDSTONE", "Rock"}, {"SPELLTAG", "Ghost"}, {"DRAGONFANG", "Dragon"}, {"BLACKGLASSES", "Dark"},
        {"METALCOAT", "Steel"}, {"PIXIEPLATE", "Fairy"}, {"FAIRYFEATHER", "Fairy"}, {"PINKBOW", "Normal"}, {"POLKADOTBOW", "Normal"}
    };
    if (typeBoostItems.count(aItem) && currentType == typeBoostItems[aItem]) damage *= 1.2;

    if (aItem.find("GEM") != std::string::npos) {
        std::string gemType = aItem.substr(0, aItem.find("GEM"));
        if (currentType != "None" && normalizeString(currentType) == gemType) {
            damage *= 1.3;
            attacker.setHeldItem("None");
            if (aAbil == "Unburden") attacker.addVolatile("unburden", 1);
            log.push_back(attacker.getNickname() + " used its " + currentType + " Gem to strengthen its attack!");
        }
    }

    if (aItem == "THICKCLUB" && move.category == "Physical" && (attacker.getName() == "Cubone" || attacker.getName() == "Marowak" || attacker.getName() == "Marowak-Alola")) damage *= 2.0;
    if (aItem == "LIGHTBALL" && (attacker.getName().find("Pikachu") != std::string::npos)) damage *= 2.0;
    if (aItem == "SOULDEW" && (attacker.getName() == "Latios" || attacker.getName() == "Latias") && (currentType == "Dragon" || currentType == "Psychic")) damage *= 1.2;

    if ((aAbil == "Huge Power" || aAbil == "Pure Power") && move.category == "Physical") damage *= 2.0;
    if (aAbil == "Gorilla Tactics" && move.category == "Physical") damage *= 1.5;
    if (aAbil == "Parental Bond" && !isFirstHit) damage *= 0.25;

    bool moldBreaker = (aAbil == "Mold Breaker" || aAbil == "Teravolt" || aAbil == "Turboblaze");

    if (move.name == "Struggle" || move.name == "struggle") {
    }
    else {
        if (!moldBreaker) {
            if (currentType == "Ground" && move.name != "Thousand Arrows") {
                if (!dGrounded && dAbil == "Levitate") { if (isFirstHit) log.push_back(defender.getNickname() + " avoided the attack with " + dAbil + "!"); return 0; }
                if (!dGrounded && dItem == "AIRBALLOON") { if (isFirstHit) log.push_back(defender.getNickname() + " is floating with its Air Balloon!"); return 0; }
            }

            if (dAbil == "Earth Eater" && currentType == "Ground") {
                if (isFirstHit) log.push_back(defender.getNickname() + " restored HP using Earth Eater!");
                defender.setCurrentHp(std::min(defender.getHp(), defender.getCurrentHp() + (defender.getHp() / 4)));
                return 0;
            }
            if ((dAbil == "Water Absorb" || dAbil == "Dry Skin") && currentType == "Water") {
                if (isFirstHit) log.push_back(defender.getNickname() + " absorbed the water!");
                defender.setCurrentHp(std::min(defender.getHp(), defender.getCurrentHp() + (defender.getHp() / 4)));
                return 0;
            }
            if (dAbil == "Storm Drain" && currentType == "Water") {
                if (isFirstHit) log.push_back(defender.getNickname() + " drew the water in!");
                defender.modifyStat("spa", 1, log);
                return 0;
            }
            if ((dAbil == "Volt Absorb" || dAbil == "Motor Drive" || dAbil == "Lightning Rod") && currentType == "Electric") {
                if (isFirstHit) log.push_back(defender.getNickname() + " absorbed the electricity!");
                if (dAbil == "Volt Absorb") defender.setCurrentHp(std::min(defender.getHp(), defender.getCurrentHp() + (defender.getHp() / 4)));
                else if (dAbil == "Motor Drive") defender.modifyStat("spe", 1, log);
                else if (dAbil == "Lightning Rod") defender.modifyStat("spa", 1, log);
                return 0;
            }
            if (dAbil == "Flash Fire" && currentType == "Fire") { if (isFirstHit) log.push_back(defender.getNickname() + "'s Flash Fire absorbed the attack!"); return 0; }
            if (dAbil == "Well-Baked Body" && currentType == "Fire") {
                if (isFirstHit) log.push_back(defender.getNickname() + " took no damage and raised its Defense using Well-Baked Body!");
                defender.modifyStat("def", 2, log);
                return 0;
            }
            bool isWindMove = (move.name == "Hurricane" || move.name == "Bleakwind Storm" || move.name == "Sandsear Storm" || move.name == "Springtide Storm" || move.name == "Wildbolt Storm" || move.name == "Heat Wave" || move.name == "Icy Wind" || move.name == "Twister" || move.name == "Whirlwind" || move.name == "Gust" || move.name == "Blizzard");
            if (dAbil == "Wind Rider" && isWindMove) {
                if (isFirstHit) log.push_back(defender.getNickname() + " rode the wind and raised its Attack!");
                defender.modifyStat("atk", 1, log);
                return 0;
            }
            if (dAbil == "Sap Sipper" && currentType == "Grass") {
                if (isFirstHit) log.push_back(defender.getNickname() + "'s Sap Sipper absorbed the attack!");
                defender.modifyStat("atk", 1, log);
                return 0;
            }
        }
    }

    if (weather == "DesolateLand" && currentType == "Water") { if (isFirstHit) log.push_back("The Water-type attack evaporated in the harsh sunlight!"); return 0; }
    if (weather == "PrimordialSea" && currentType == "Fire") { if (isFirstHit) log.push_back("The Fire-type attack fizzled out in the heavy rain!"); return 0; }
    if ((weather == "Sun" || weather == "DesolateLand") && currentType == "Fire") damage *= 1.5;
    if ((weather == "Sun" || weather == "DesolateLand") && currentType == "Water") damage *= 0.5;

    std::string defType1 = defender.getType1();
    std::string defType2 = defender.getType2();

    if (defender.hasVolatile("roost")) {
        if (defType1 == "Flying") defType1 = "Normal";
        if (defType2 == "Flying") defType2 = "None";
    }

    double eff1 = getTypeEffectiveness(currentType, defType1);
    double eff2 = getTypeEffectiveness(currentType, defType2);

    if (move.name == "Freeze-Dry") {
        if (defType1 == "Water") eff1 = 2.0;
        if (defType2 == "Water") eff2 = 2.0;
    }
    if (move.name == "Thousand Arrows") {
        if (defType1 == "Flying") eff1 = 1.0;
        if (defType2 == "Flying") eff2 = 1.0;
    }

    double effect = eff1 * eff2;

    if (move.name == "Flying Press") effect *= getTypeEffectiveness("Flying", defType1 == "Ghost" ? "None" : defType1) * getTypeEffectiveness("Flying", defType2 == "Ghost" ? "None" : defType2);

    if (effect == 0.0 && move.name == "Thousand Arrows") effect = 1.0;

    if ((aAbil == "Scrappy" || aAbil == "Mind's Eye") && (currentType == "Normal" || currentType == "Fighting")) {
        effect = getTypeEffectiveness(currentType, defType1 == "Ghost" ? "None" : defType1) * getTypeEffectiveness(currentType, defType2 == "Ghost" ? "None" : defType2);
    }

    if ((move.name == "Struggle" || move.name == "struggle") && effect == 0.0) effect = 1.0;

    if (weather == "DeltaStream" && (defType1 == "Flying" || defType2 == "Flying")) {
        if (currentType == "Electric" || currentType == "Ice" || currentType == "Rock") effect /= 2.0;
    }
    if (field.gravity > 0 && currentType == "Ground" && effect == 0.0) effect = 1.0;

    if (!moldBreaker && dAbil == "Wonder Guard" && effect <= 1.0 && move.name != "Struggle" && move.name != "struggle") {
        if (isFirstHit) log.push_back(defender.getNickname() + "'s Wonder Guard evades the attack!");
        return 0;
    }

    if (!moldBreaker && dAbil == "Thick Fat" && (currentType == "Fire" || currentType == "Ice")) damage *= 0.5;
    if (dAbil == "Purifying Salt" && currentType == "Ghost") damage *= 0.5;

    if (aAbil == "Tinted Lens" && effect < 1.0) effect *= 2.0;
    if (aItem == "EXPERTBELT" && effect > 1.0) damage *= 1.2;

    if (effect > 1.0) {
        if ((dItem == "OCCABERRY" && currentType == "Fire") ||
            (dItem == "PASSHOBERRY" && currentType == "Water") ||
            (dItem == "WACANBERRY" && currentType == "Electric") ||
            (dItem == "RINDOBERRY" && currentType == "Grass") ||
            (dItem == "YACHEBERRY" && currentType == "Ice") ||
            (dItem == "CHOPLEBERRY" && currentType == "Fighting") ||
            (dItem == "KEBIABERRY" && currentType == "Poison") ||
            (dItem == "SHUCABERRY" && currentType == "Ground") ||
            (dItem == "COBABERRY" && currentType == "Flying") ||
            (dItem == "PAYAPABERRY" && currentType == "Psychic") ||
            (dItem == "TANGABERRY" && currentType == "Bug") ||
            (dItem == "CHARTIBERRY" && currentType == "Rock") ||
            (dItem == "KASIBBERRY" && currentType == "Ghost") ||
            (dItem == "HABANBERRY" && currentType == "Dragon") ||
            (dItem == "COLBURBERRY" && currentType == "Dark") ||
            (dItem == "BABIRIBERRY" && currentType == "Steel") ||
            (dItem == "ROSELIBERRY" && currentType == "Fairy")) {
            damage *= 0.5;
            log.push_back(defender.getNickname() + " used its " + defender.getHeldItem() + " to weaken the damage!");
            defender.setHeldItem("None");
            if (dAbil == "Unburden") defender.addVolatile("unburden", 1);
        }
    }
    if (dItem == "CHILANBERRY" && currentType == "Normal") {
        damage *= 0.5;
        log.push_back(defender.getNickname() + " used its " + defender.getHeldItem() + " to weaken the damage!");
        defender.setHeldItem("None");
        if (dAbil == "Unburden") defender.addVolatile("unburden", 1);
    }

    if (effect == 0.0) { if (isFirstHit) log.push_back("It doesn't affect " + defender.getNickname() + "..."); return 0; }
    damage = std::floor(damage * effect);

    if (aAbil == "Neuroforce" && effect > 1.0) damage *= 1.25;

    if (effect > 1.0 && dItem == "WEAKNESSPOLICY") {
        defender.modifyStat("atk", 2, log);
        defender.modifyStat("spa", 2, log);
        defender.setHeldItem("None");
        if (defender.getAbility() == "Unburden") defender.addVolatile("unburden", 1);
        log.push_back(defender.getNickname() + "'s Weakness Policy sharply raised its Attack and Sp. Atk!");
    }

    if (aAbil == "Rocky Payload" && currentType == "Rock") damage *= 1.5;
    if (aAbil == "Steelworker" && currentType == "Steel") damage *= 1.5;
    if (aAbil == "Transistor" && currentType == "Electric") damage *= 1.3;
    if (aAbil == "Dragon's Maw" && currentType == "Dragon") damage *= 1.5;

    if (aAbil == "Guts" && !attacker.getStatus().empty() && move.category == "Physical") damage *= 1.5;
    if (aAbil == "Flash Fire" && attacker.hasVolatile("flashfire") && currentType == "Fire") damage *= 1.5;
    if (aAbil == "Sand Force" && weather == "Sandstorm" && (currentType == "Rock" || currentType == "Ground" || currentType == "Steel")) damage *= 1.3;
    if (aAbil == "Steely Spirit" && currentType == "Steel") damage *= 1.5;

    if (aAbil == "Toxic Boost" && move.category == "Physical" && (attacker.getStatus() == "psn" || attacker.getStatus() == "tox")) damage *= 1.5;
    if (aAbil == "Flare Boost" && move.category == "Special" && attacker.getStatus() == "brn") damage *= 1.5;
    if (aAbil == "Hustle" && move.category == "Physical") damage *= 1.5;

    if (attacker.getCurrentHp() <= attacker.getHp() / 3) {
        if (aAbil == "Overgrow" && currentType == "Grass") damage *= 1.5;
        if (aAbil == "Blaze" && currentType == "Fire") damage *= 1.5;
        if (aAbil == "Torrent" && currentType == "Water") damage *= 1.5;
        if (aAbil == "Swarm" && currentType == "Bug") damage *= 1.5;
    }

    if (aAbil == "Punk Rock" && isSound) damage *= 1.3;
    if (dAbil == "Punk Rock" && isSound) damage *= 0.5;

    bool isPunch = (move.name.find("Punch") != std::string::npos || move.name == "Meteor Mash" || move.name == "Hammer Arm" || move.name == "Dynamic Punch");
    if (aAbil == "Iron Fist" && isPunch) damage *= 1.2;

    bool isBite = (move.name.find("Fang") != std::string::npos || move.name.find("Bite") != std::string::npos || move.name == "Crunch" || move.name == "Jaw Lock" || move.name == "Psychic Fangs");
    if (aAbil == "Strong Jaw" && isBite) damage *= 1.5;

    bool isPulse = (move.name == "Aura Sphere" || move.name == "Dark Pulse" || move.name == "Dragon Pulse" || move.name == "Water Pulse" || move.name == "Origin Pulse" || move.name == "Terrain Pulse");
    if (aAbil == "Mega Launcher" && isPulse) damage *= 1.5;

    if (aAbil == "Reckless" && move.recoilNum > 0) damage *= 1.2;

    bool fairyAura = (aAbil == "Fairy Aura" || dAbil == "Fairy Aura");
    bool darkAura = (aAbil == "Dark Aura" || dAbil == "Dark Aura");
    bool auraBreak = (aAbil == "Aura Break" || dAbil == "Aura Break");

    if (currentType == "Fairy" && fairyAura) { damage *= auraBreak ? 0.75 : 1.33; }
    if (currentType == "Dark" && darkAura) { damage *= auraBreak ? 0.75 : 1.33; }

    if (aAbil == "Tough Claws" && move.isContact) damage *= 1.3;
    if (aAbil == "Water Bubble" && currentType == "Water") damage *= 2.0;

    if (effect > 1.0 && (dAbil == "Filter" || dAbil == "Solid Rock" || dAbil == "Prism Armor")) {
        damage *= 0.75;
    }
    if ((dAbil == "Multiscale" || dAbil == "Shadow Shield") && defender.getCurrentHp() == defender.getHp()) {
        damage *= 0.5;
    }
    if (dAbil == "Fluffy") {
        if (move.isContact) damage *= 0.5;
        if (currentType == "Fire") damage *= 2.0;
    }
    if (dAbil == "Water Bubble" && currentType == "Fire") damage *= 0.5;
    if (dAbil == "Ice Scales" && move.category == "Special") damage *= 0.5;
    if (dAbil == "Dry Skin" && currentType == "Fire") damage *= 1.25;

    if (isCrit) {
        if (aAbil == "Sniper") damage *= 2.25;
        else damage *= 1.5;
        if (isFirstHit) log.push_back("A critical hit!");
    }

    int minDmg = std::floor(damage * 0.85);
    int maxDmg = std::floor(damage * 1.00);
    int actualDmg = minDmg + (rand() % (maxDmg - minDmg + 1));

    if (isFirstHit) {
        if (effect > 1.0) log.push_back("It's super effective!");
        if (effect < 1.0) log.push_back("It's not very effective...");
    }

    return std::max(1, actualDmg);
}

void BattleEngine::executeMove(Pokemon& attacker, Pokemon& defender, const std::string& moveId, bool isPlayerAttacking, std::vector<std::string>& log, const std::string& defenderMoveId, std::function<void(bool)> animateFn) {
    if (attacker.getCurrentHp() <= 0) return;

    if (attacker.hasVolatile("mustrecharge")) {
        log.push_back(attacker.getNickname() + " must recharge!");
        attacker.removeVolatile("mustrecharge");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    int oldDefHp = defender.getCurrentHp();
    size_t logIndexBeforeTurn = log.size();

    MoveData move = controller.getMoveData(moveId);

    std::string aAbil = attacker.getAbility();
    std::string dAbil = defender.getAbility();
    std::string aItem = (field.magicRoom > 0) ? "NONE" : normalizeString(attacker.getHeldItem());
    std::string dItem = (field.magicRoom > 0) ? "NONE" : normalizeString(defender.getHeldItem());

    bool gasActive = false;
    if (aAbil == "Neutralizing Gas" || dAbil == "Neutralizing Gas") gasActive = true;

    auto applyGas = [&](std::string& ability) {
        if (gasActive && ability != "Neutralizing Gas" && ability != "Stance Change" && ability != "Multitype" && ability != "Zero to Hero" && ability != "Ice Face" && ability != "Disguise" && ability != "Gulp Missile") {
            ability = "None";
        }
        };
    applyGas(aAbil);
    applyGas(dAbil);

    bool moldBreaker = (aAbil == "Mold Breaker" || aAbil == "Teravolt" || aAbil == "Turboblaze");
    std::string weather = getActiveWeather(attacker, defender);

    SideState& aSide = isPlayerAttacking ? playerSide : enemySide;
    SideState& dSide = isPlayerAttacking ? enemySide : playerSide;

    auto removeItem = [&](Pokemon& pkm) {
        pkm.setHeldItem("None");
        if (pkm.getAbility() == "Unburden" && !pkm.hasVolatile("unburden")) {
            pkm.addVolatile("unburden", 1);
            log.push_back(pkm.getNickname() + "'s Unburden doubled its Speed!");
        }
        };

    if (aAbil == "Stance Change") {
        if ((move.category == "Physical" || move.category == "Special") && !attacker.hasVolatile("blade_forme")) {
            attacker.addVolatile("blade_forme", 1);
            log.push_back(attacker.getNickname() + " changed to Blade Forme!");
        }
        else if (move.name == "King's Shield" && attacker.hasVolatile("blade_forme")) {
            attacker.removeVolatile("blade_forme");
            log.push_back(attacker.getNickname() + " changed to Shield Forme!");
        }
    }

    if ((aAbil == "Protean" || aAbil == "Libero") && !attacker.hasVolatile("protean_used")) {
        log.push_back(attacker.getNickname() + "'s " + aAbil + " changed its type!");
        attacker.addVolatile("protean_used", 1);
    }

    if (move.category == "Status" && dAbil == "Magic Bounce" && move.flagsProtect && move.overrideOffensivePokemon != "target") {
        log.push_back(defender.getNickname() + " bounced the " + move.name + " back with Magic Bounce!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (aAbil == "Prankster" && move.category == "Status" && (defender.getType1() == "Dark" || defender.getType2() == "Dark")) {
        log.push_back("It doesn't affect " + defender.getNickname() + "...");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if ((move.name == "Fake Out" || move.name == "First Impression" || move.name == "Mat Block") && !attacker.hasVolatile("first_turn")) {
        log.push_back(attacker.getNickname() + " used " + move.name + "!");
        log.push_back("But it failed!");
        attacker.addVolatile("moved_this_turn", 1);
        attacker.lastMoveUsed = move.name;
        return;
    }

    if (move.name == "Blood Moon" || move.name == "Gigaton Hammer") {
        if (attacker.lastMoveUsed == move.name) {
            log.push_back(attacker.getNickname() + " used " + move.name + "!");
            log.push_back("But it failed! It can't be used twice in a row!");
            attacker.addVolatile("moved_this_turn", 1);
            attacker.lastMoveUsed = move.name;
            return;
        }
    }

    if (move.name == "Poltergeist" && dItem == "NONE") {
        log.push_back(attacker.getNickname() + " used " + move.name + "!");
        log.push_back("But it failed! The target doesn't have an item!");
        attacker.addVolatile("moved_this_turn", 1);
        attacker.lastMoveUsed = move.name;
        return;
    }

    if ((move.name == "Sleep Talk" || move.name == "Snore") && attacker.getStatus() != "slp" && aAbil != "Comatose") {
        log.push_back(attacker.getNickname() + " used " + move.name + "!");
        log.push_back("But it failed! " + attacker.getNickname() + " is not asleep!");
        attacker.addVolatile("moved_this_turn", 1);
        attacker.lastMoveUsed = move.name;
        return;
    }

    if (move.name == "Dream Eater" && defender.getStatus() != "slp" && dAbil != "Comatose") {
        log.push_back(attacker.getNickname() + " used " + move.name + "!");
        log.push_back("But it failed! The target is not asleep!");
        attacker.addVolatile("moved_this_turn", 1);
        attacker.lastMoveUsed = move.name;
        return;
    }

    attacker.lastMoveUsed = move.name;

    if (move.name == "Sucker Punch" || move.name == "Thunderclap" || move.name == "Upper Hand") {
        bool failed = false;
        if (defender.hasVolatile("moved_this_turn")) {
            failed = true;
        }
        else if (defenderMoveId.empty() || defenderMoveId == "SKIP_TURN" || defenderMoveId.rfind("switch", 0) == 0 || defenderMoveId.rfind("mega", 0) == 0) {
            failed = true;
        }
        else {
            MoveData defMove = controller.getMoveData(defenderMoveId);
            if (defMove.category == "Status") failed = true;
        }

        if (failed) {
            log.push_back(attacker.getNickname() + " used " + move.name + "!");
            log.push_back("But it failed!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }

    if (move.priority > 0 && !moldBreaker) {
        if (dAbil == "Queenly Majesty" || dAbil == "Dazzling" || dAbil == "Armor Tail") {
            log.push_back(defender.getNickname() + " cannot be hit by priority moves due to " + dAbil + "!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }

    if (attacker.hasVolatile("flinch")) {
        log.push_back(attacker.getNickname() + " flinched and couldn't move!");
        attacker.removeVolatile("flinch");
        attacker.resetProtectCounter();
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (field.gravity > 0) {
        if (move.name == "Fly" || move.name == "Bounce" || move.name == "Splash" || move.name == "Jump Kick" || move.name == "High Jump Kick") {
            log.push_back(attacker.getNickname() + " can't use " + move.name + " because of gravity!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }

    if (attacker.getStatus() == "slp" && aAbil != "Comatose") {
        attacker.decrementStatusTurns();
        if (attacker.getStatusTurns() <= 0) {
            attacker.setStatus("");
            log.push_back(attacker.getNickname() + " woke up!");
        }
        else {
            if (move.name != "Sleep Talk" && move.name != "Snore") {
                log.push_back(attacker.getNickname() + " is fast asleep.");
                attacker.resetProtectCounter();
                attacker.addVolatile("moved_this_turn", 1);
                return;
            }
        }
    }
    if (attacker.getStatus() == "frz") {
        if (rand() % 100 < 20 || move.type == "Fire" || move.name == "Scald" || move.name == "Steam Eruption" || move.name == "Scorching Sands" || move.name == "Matcha Gotcha" || move.name == "Flare Blitz" || move.name == "Sacred Fire" || move.name == "Flame Wheel" || move.name == "Burn Up") {
            attacker.setStatus("");
            log.push_back(attacker.getNickname() + " thawed out!");
        }
        else {
            log.push_back(attacker.getNickname() + " is frozen solid!");
            attacker.resetProtectCounter();
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }
    if (attacker.getStatus() == "par" && rand() % 100 < 25) {
        log.push_back(attacker.getNickname() + " is paralyzed! It can't move!");
        attacker.resetProtectCounter();
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (attacker.hasVolatile("confusion")) {
        log.push_back(attacker.getNickname() + " is confused!");
        attacker.addVolatile("confusion", attacker.getVolatileTurns("confusion") - 1);

        if (attacker.getVolatileTurns("confusion") <= 0) {
            log.push_back(attacker.getNickname() + " snapped out of its confusion!");
            attacker.removeVolatile("confusion");
        }
        else if (rand() % 100 < 33) {
            log.push_back("It hurt itself in its confusion!");
            double a = attacker.getBattleAttack();
            double d = attacker.getBattleDefense();
            if (d <= 0) d = 1;
            int selfDmg = std::floor(std::floor(std::floor(2.0 * attacker.getLevel() / 5.0 + 2.0) * 40 * a / d) / 50.0) + 2.0;
            attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - std::max(1, selfDmg)));
            attacker.resetProtectCounter();
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }

    if (attacker.hasVolatile("taunt") && move.category == "Status") {
        log.push_back(attacker.getNickname() + " can't use " + move.name + " after the taunt!");
        attacker.resetProtectCounter();
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.isCharge && attacker.getChargingMove() != move.name) {
        if ((move.name == "Solar Beam" || move.name == "Solar Blade") && (weather == "Sun" || weather == "DesolateLand")) {
        }
        else if (aItem == "POWERHERB") {
            log.push_back(attacker.getNickname() + " became fully charged due to its Power Herb!");
            removeItem(attacker);
        }
        else {
            log.push_back(attacker.getNickname() + " began charging " + move.name + "!");
            attacker.setChargingMove(move.name);
            attacker.resetProtectCounter();

            if (move.name == "Meteor Beam" || move.name == "Electro Shot") attacker.modifyStat("spa", 1, log);
            else if (move.name == "Skull Bash") attacker.modifyStat("def", 1, log);

            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }
    if (attacker.getChargingMove() == move.name) attacker.setChargingMove("");

    log.push_back(attacker.getNickname() + " used " + move.name + "!");

    if (move.name == "Minimize") {
        attacker.modifyStat("evasion", 2, log);
        attacker.addVolatile("minimize", 999);
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Scald" || move.name == "Steam Eruption" || move.name == "Scorching Sands" || move.name == "Matcha Gotcha" || move.name == "Hydro Steam") {
        if (defender.getStatus() == "frz") {
            defender.setStatus("");
            log.push_back(defender.getNickname() + " was thawed out by the attack!");
        }
    }

    if (move.volatileStatus == "lockedmove" || move.name == "Outrage" || move.name == "Petal Dance" || move.name == "Thrash" || move.name == "Rollout" || move.name == "Ice Ball") {
        if (attacker.getLockedTurns() <= 0) {
            attacker.setLockedMove(move.name, (move.name == "Rollout" || move.name == "Ice Ball") ? 5 : ((rand() % 2) + 2));
            if (move.name == "Rollout" || move.name == "Ice Ball") attacker.addVolatile("rollout_count", 0);
        }
        else if (move.name == "Rollout" || move.name == "Ice Ball") {
            attacker.addVolatile("rollout_count", attacker.getVolatileTurns("rollout_count") + 1);
        }
    }

    if (move.isProtectMove) {
        double chance = 1.0 / std::pow(3.0, attacker.getProtectCounter());
        if ((rand() % 100) < (chance * 100)) {
            attacker.addVolatile("protect", 1);
            attacker.incrementProtectCounter();
            if (animateFn) animateFn(isPlayerAttacking);
            log.push_back(attacker.getNickname() + " protected itself!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
        else {
            log.push_back("But it failed!");
            attacker.resetProtectCounter();
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }
    else {
        attacker.resetProtectCounter();
    }

    bool breaksProtect = (move.name == "Feint" || move.name == "Phantom Force" || move.name == "Shadow Force" || move.name == "Hyperspace Fury" || move.name == "Hyperspace Hole" || (aAbil == "Unseen Fist" && move.isContact));
    if (defender.hasVolatile("protect") && move.flagsProtect) {
        if (breaksProtect) {
            defender.removeVolatile("protect");
            log.push_back(attacker.getNickname() + " broke through " + defender.getNickname() + "'s protection!");
        }
        else {
            log.push_back(defender.getNickname() + " protected itself!");
            if (move.name == "High Jump Kick" || move.name == "Jump Kick" || move.name == "Axe Kick") {
                if (aAbil != "Magic Guard") {
                    int crashDamage = std::max(1, attacker.getHp() / 2);
                    attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - crashDamage));
                    log.push_back(attacker.getNickname() + " kept going and crashed!");
                }
            }
            attacker.addVolatile("moved_this_turn", 1);
            if (move.name == "Rollout" || move.name == "Ice Ball") {
                attacker.setLockedMove("", 0);
                attacker.removeVolatile("rollout_count");
            }
            return;
        }
    }

    if (move.ohko) {
        bool hasSturdy = (!moldBreaker && dAbil == "Sturdy");
        bool hasSash = (dItem == "FOCUSSASH");

        if (hasSturdy || hasSash) {
            log.push_back(defender.getNickname() + " avoided the OHKO due to its resilience!");
            attacker.addVolatile("moved_this_turn", 1);
            if (hasSash) removeItem(defender);
            return;
        }
        if (attacker.getLevel() < defender.getLevel()) {
            log.push_back(defender.getNickname() + " is immune to the attack!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
        int ohkoAcc = (attacker.getLevel() - defender.getLevel()) + 30;
        if (move.name == "Sheer Cold" && attacker.getType1() != "Ice" && attacker.getType2() != "Ice") ohkoAcc -= 10;

        if (rand() % 100 >= ohkoAcc) { log.push_back(attacker.getNickname() + "'s attack missed!"); attacker.addVolatile("moved_this_turn", 1); return; }

        double effect = getTypeEffectiveness(move.type, defender.getType1()) * getTypeEffectiveness(move.type, defender.getType2());
        if (effect == 0.0) { log.push_back("It doesn't affect " + defender.getNickname() + "..."); attacker.addVolatile("moved_this_turn", 1); return; }

        if (animateFn) animateFn(isPlayerAttacking);
        defender.setCurrentHp(0);
        log.push_back("It's a one-hit KO!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    bool bypassAccuracy = false;
    if (defender.hasVolatile("minimize")) {
        if (move.name == "Body Slam" || move.name == "Stomp" || move.name == "Dragon Rush" || move.name == "Flying Press" || move.name == "Heat Crash" || move.name == "Heavy Slam" || move.name == "Malicious Moonsault" || move.name == "Steamroller") bypassAccuracy = true;
    }
    if (defender.hasVolatile("glaiverush")) bypassAccuracy = true;

    if (!bypassAccuracy && move.accuracy <= 100 && aAbil != "No Guard" && dAbil != "No Guard") {
        int aAccStage = attacker.getStageAcc();
        int dEvaStage = defender.getStageEva();

        if (!moldBreaker && dAbil == "Unaware") aAccStage = std::max(0, aAccStage);
        if (aAbil == "Keen Eye" || aAbil == "Illuminate" || aAbil == "Mind's Eye") dEvaStage = std::min(0, dEvaStage);

        int accStage = aAccStage - dEvaStage;
        if (accStage > 6) accStage = 6;
        if (accStage < -6) accStage = -6;

        double accMultiplier = 1.0;
        if (accStage >= 0) accMultiplier = (3.0 + accStage) / 3.0;
        else accMultiplier = 3.0 / (3.0 - accStage);

        double effectiveAccuracy = move.accuracy * accMultiplier;

        if (field.gravity > 0) effectiveAccuracy = std::min(100.0, effectiveAccuracy * 1.67);
        if (aAbil == "Compound Eyes") effectiveAccuracy = std::min(100.0, effectiveAccuracy * 1.3);
        if (aAbil == "Hustle" && move.category == "Physical") effectiveAccuracy *= 0.8;
        if (aAbil == "Victory Star") effectiveAccuracy *= 1.1;
        if (aItem == "WIDELENS") effectiveAccuracy = std::min(100.0, effectiveAccuracy * 1.1);

        if (!moldBreaker) {
            if (dAbil == "Sand Veil" && weather == "Sandstorm") effectiveAccuracy *= 0.8;
            if (dAbil == "Snow Cloak" && (weather == "Snow" || weather == "Hail")) effectiveAccuracy *= 0.8;
            if (dAbil == "Tangled Feet" && defender.hasVolatile("confusion")) effectiveAccuracy *= 0.5;
            if (dItem == "BRIGHTPOWDER" || dItem == "LAXINCENSE") effectiveAccuracy *= 0.9;
        }

        if (move.name != "Population Bomb" && move.name != "Triple Axel" && move.name != "Triple Kick") {
            if (rand() % 100 >= std::floor(effectiveAccuracy)) {
                log.push_back(attacker.getNickname() + "'s attack missed!");

                if (aItem == "BLUNDERPOLICY") {
                    attacker.modifyStat("spe", 2, log);
                    removeItem(attacker);
                    log.push_back(attacker.getNickname() + " used its Blunder Policy to sharply raise its Speed!");
                }

                if (move.name == "High Jump Kick" || move.name == "Jump Kick" || move.name == "Axe Kick") {
                    if (aAbil != "Magic Guard") {
                        int crashDamage = std::max(1, attacker.getHp() / 2);
                        attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - crashDamage));
                        log.push_back(attacker.getNickname() + " kept going and crashed!");
                    }
                }

                attacker.addVolatile("moved_this_turn", 1);
                if (move.name == "Rollout" || move.name == "Ice Ball") {
                    attacker.setLockedMove("", 0);
                    attacker.removeVolatile("rollout_count");
                }
                return;
            }
        }
    }

    if (move.name == "Mind Blown" || move.name == "Steel Beam" || move.name == "Chloroblast") {
        if (aAbil != "Magic Guard") {
            int recoilAmt = std::max(1, attacker.getHp() / 2);
            attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - recoilAmt));
            log.push_back(attacker.getNickname() + " lost half of its HP to power the attack!");
            if (attacker.getCurrentHp() <= 0) {
                attacker.addVolatile("moved_this_turn", 1);
                return;
            }
        }
    }

    if (animateFn) animateFn(isPlayerAttacking);

    if (move.name == "Brick Break" || move.name == "Psychic Fangs" || move.name == "Raging Bull") {
        if (dSide.reflect > 0 || dSide.lightScreen > 0 || dSide.auroraVeil > 0) {
            dSide.reflect = 0;
            dSide.lightScreen = 0;
            dSide.auroraVeil = 0;
            log.push_back(attacker.getNickname() + " shattered the opposing team's screens!");
        }
    }

    auto triggerSeeds = [&](Pokemon& pkmn) {
        std::string it = (field.magicRoom > 0) ? "NONE" : normalizeString(pkmn.getHeldItem());
        if (field.terrain == "Electric Terrain" && it == "ELECTRICSEED") { pkmn.modifyStat("def", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Electric Seed!"); }
        else if (field.terrain == "Grassy Terrain" && it == "GRASSYSEED") { pkmn.modifyStat("def", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Grassy Seed!"); }
        else if (field.terrain == "Psychic Terrain" && it == "PSYCHICSEED") { pkmn.modifyStat("spd", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Psychic Seed!"); }
        else if (field.terrain == "Misty Terrain" && it == "MISTYSEED") { pkmn.modifyStat("spd", 1, log); removeItem(pkmn); log.push_back(pkmn.getNickname() + " used its Misty Seed!"); }
        };

    if (move.name == "Gravity") { field.gravity = (aAbil == "Persistent") ? 7 : 5; log.push_back("Gravity intensified!"); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Trick Room") { field.trickRoom = (aAbil == "Persistent") ? 7 : 5; log.push_back(attacker.getNickname() + " twisted the dimensions!"); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Magic Room") { field.magicRoom = (aAbil == "Persistent") ? 7 : 5; log.push_back("It created a bizarre area in which items lose their effects!"); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Wonder Room") { field.wonderRoom = (aAbil == "Persistent") ? 7 : 5; log.push_back("It created a bizarre area in which Defense and Sp. Def stats are swapped!"); attacker.addVolatile("moved_this_turn", 1); return; }

    if (move.name == "Electric Terrain") { field.terrain = "Electric Terrain"; field.terrainTurns = (aItem == "TERRAINEXTENDER") ? 8 : 5; log.push_back("An electric current runs across the battlefield!"); triggerSeeds(attacker); triggerSeeds(defender); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Grassy Terrain") { field.terrain = "Grassy Terrain"; field.terrainTurns = (aItem == "TERRAINEXTENDER") ? 8 : 5; log.push_back("Grass grew to cover the battlefield!"); triggerSeeds(attacker); triggerSeeds(defender); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Misty Terrain") { field.terrain = "Misty Terrain"; field.terrainTurns = (aItem == "TERRAINEXTENDER") ? 8 : 5; log.push_back("Mist swirled around the battlefield!"); triggerSeeds(attacker); triggerSeeds(defender); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Psychic Terrain") { field.terrain = "Psychic Terrain"; field.terrainTurns = (aItem == "TERRAINEXTENDER") ? 8 : 5; log.push_back("The battlefield got weird!"); triggerSeeds(attacker); triggerSeeds(defender); attacker.addVolatile("moved_this_turn", 1); return; }

    bool extremeWeather = (weather == "DesolateLand" || weather == "PrimordialSea" || weather == "DeltaStream");
    if (!extremeWeather) {
        if (move.name == "Sunny Day") { field.weather = "Sun"; field.weatherTurns = (aItem == "HEATROCK") ? 8 : 5; log.push_back("The sunlight turned harsh!"); attacker.addVolatile("moved_this_turn", 1); return; }
        if (move.name == "Rain Dance") { field.weather = "Rain"; field.weatherTurns = (aItem == "DAMPROCK") ? 8 : 5; log.push_back("It started to rain!"); attacker.addVolatile("moved_this_turn", 1); return; }
        if (move.name == "Sandstorm") { field.weather = "Sandstorm"; field.weatherTurns = (aItem == "SMOOTHROCK") ? 8 : 5; log.push_back("A sandstorm kicked up!"); attacker.addVolatile("moved_this_turn", 1); return; }
        if (move.name == "Hail" || move.name == "Snowscape") { field.weather = "Snow"; field.weatherTurns = (aItem == "ICYROCK") ? 8 : 5; log.push_back("It started to snow!"); attacker.addVolatile("moved_this_turn", 1); return; }
    }
    else if (move.name == "Sunny Day" || move.name == "Rain Dance" || move.name == "Sandstorm" || move.name == "Hail" || move.name == "Snowscape") {
        log.push_back("There is no relief from this terrible weather!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Reflect") { aSide.reflect = (aItem == "LIGHTCLAY") ? 8 : 5; log.push_back("Reflect raised the team's Defense!"); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Light Screen") { aSide.lightScreen = (aItem == "LIGHTCLAY") ? 8 : 5; log.push_back("Light Screen raised the team's Sp. Def!"); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Aurora Veil") {
        if (weather == "Snow" || weather == "Hail") {
            aSide.auroraVeil = (aItem == "LIGHTCLAY") ? 8 : 5; log.push_back("Aurora Veil made the team stronger against physical and special moves!");
        }
        else { log.push_back("But it failed!"); }
        attacker.addVolatile("moved_this_turn", 1); return;
    }
    if (move.name == "Tailwind") { aSide.tailwind = (aAbil == "Persistent") ? 6 : 4; log.push_back("The Tailwind blew from behind the team!"); attacker.addVolatile("moved_this_turn", 1); return; }
    if (move.name == "Safeguard") { aSide.safeguard = (aAbil == "Persistent") ? 7 : 5; log.push_back("Your team became cloaked in a mystical veil!"); attacker.addVolatile("moved_this_turn", 1); return; }

    if (move.name == "Quash") {
        if (defender.hasVolatile("moved_this_turn")) log.push_back("But it failed!");
        else { defender.addVolatile("quash", 1); log.push_back(defender.getNickname() + "'s move was postponed!"); }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }
    if (move.name == "Imprison") {
        attacker.addVolatile("imprison", 999);
        log.push_back(attacker.getNickname() + " sealed the opponent's moves!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }
    if (move.name == "Transform") {
        attacker.transformInto(defender);
        log.push_back(attacker.getNickname() + " transformed into " + defender.getName() + "!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Pain Split") {
        int totalHp = attacker.getCurrentHp() + defender.getCurrentHp();
        int avgHp = totalHp / 2;
        attacker.setCurrentHp(std::min(attacker.getHp(), avgHp));
        defender.setCurrentHp(std::min(defender.getHp(), avgHp));
        log.push_back("The battlers shared their pain!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Court Change") {
        std::swap(playerSide.stealthRock, enemySide.stealthRock);
        std::swap(playerSide.spikes, enemySide.spikes);
        std::swap(playerSide.toxicSpikes, enemySide.toxicSpikes);
        std::swap(playerSide.stickyWeb, enemySide.stickyWeb);
        std::swap(playerSide.reflect, enemySide.reflect);
        std::swap(playerSide.lightScreen, enemySide.lightScreen);
        std::swap(playerSide.auroraVeil, enemySide.auroraVeil);
        std::swap(playerSide.safeguard, enemySide.safeguard);
        std::swap(playerSide.tailwind, enemySide.tailwind);
        log.push_back(attacker.getNickname() + " swapped the battlefield effects!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Tidy Up") {
        aSide.stealthRock = false; aSide.spikes = 0; aSide.toxicSpikes = 0; aSide.stickyWeb = false;
        dSide.stealthRock = false; dSide.spikes = 0; dSide.toxicSpikes = 0; dSide.stickyWeb = false;
        aSide.substituteHp = 0; dSide.substituteHp = 0;
        log.push_back("The battlefield was tidied up!");
        attacker.modifyStat("atk", 1, log);
        attacker.modifyStat("spe", 1, log);
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Heal Bell" || move.name == "Aromatherapy" || move.name == "Jungle Healing" || move.name == "Lunar Blessing") {
        attacker.setStatus("");
        log.push_back("A soothing aroma cured " + attacker.getNickname() + "'s status!");
        if (move.name == "Jungle Healing" || move.name == "Lunar Blessing") {
            attacker.setCurrentHp(std::min(attacker.getHp(), attacker.getCurrentHp() + std::max(1, attacker.getHp() / 4)));
            log.push_back(attacker.getNickname() + " regained health!");
        }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Substitute") {
        if (aSide.substituteHp > 0) { log.push_back(attacker.getNickname() + " already has a substitute!"); }
        else if (attacker.getCurrentHp() <= attacker.getHp() / 4) { log.push_back("It was too weak to make a substitute!"); }
        else {
            int cost = std::max(1, attacker.getHp() / 4);
            attacker.setCurrentHp(attacker.getCurrentHp() - cost);
            aSide.substituteHp = cost;
            log.push_back(attacker.getNickname() + " put in a substitute!");
        }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Rest") {
        if (attacker.getCurrentHp() == attacker.getHp() || attacker.getStatus() == "slp") {
            log.push_back("But it failed!");
        }
        else {
            attacker.setCurrentHp(attacker.getHp());
            attacker.setStatus("slp", 2);
            log.push_back(attacker.getNickname() + " went to sleep and became healthy!");
        }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Roost") {
        if (attacker.getCurrentHp() == attacker.getHp()) { log.push_back("But it failed! " + attacker.getNickname() + " is already at full health!"); }
        else {
            attacker.setCurrentHp(std::min(attacker.getHp(), attacker.getCurrentHp() + std::max(1, attacker.getHp() / 2)));
            attacker.addVolatile("roost", 1);
            log.push_back(attacker.getNickname() + " regained health and lost its Flying type this turn!");
        }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Curse") {
        if (attacker.getType1() == "Ghost" || attacker.getType2() == "Ghost") {
            int curseCost = std::max(1, attacker.getHp() / 2);
            attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - curseCost));
            defender.addVolatile("curse");
            log.push_back(attacker.getNickname() + " cut its own HP and laid a curse on " + defender.getNickname() + "!");
        }
        else {
            attacker.modifyStat("spe", -1, log); attacker.modifyStat("atk", 1, log); attacker.modifyStat("def", 1, log);
        }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }
    else if (move.healNum > 0 && move.healDen > 0 && move.category == "Status") {
        if (attacker.getCurrentHp() == attacker.getHp()) { log.push_back(attacker.getNickname() + " is already at full health!"); }
        else {
            int num = move.healNum;
            int den = move.healDen;

            if (move.name == "Shore Up") {
                if (weather == "Sandstorm") { num = 2; den = 3; }
                else { num = 1; den = 2; }
            }
            else if (move.name == "Moonlight" || move.name == "Morning Sun" || move.name == "Synthesis") {
                if (weather == "Sun" || weather == "DesolateLand") { num = 2; den = 3; }
                else if (weather != "None") { num = 1; den = 4; }
                else { num = 1; den = 2; }
            }

            int healAmt = (attacker.getHp() * num) / den;
            attacker.setCurrentHp(std::min(attacker.getHp(), attacker.getCurrentHp() + std::max(1, healAmt)));
            log.push_back(attacker.getNickname() + " regained health!");
        }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    if (move.name == "Stealth Rock") {
        if (!dSide.stealthRock) { dSide.stealthRock = true; log.push_back("Pointed stones float in the air around the opponent's team!"); }
        else log.push_back("But it failed!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }
    else if (move.name == "Spikes") {
        if (dSide.spikes < 3) { dSide.spikes++; log.push_back("Spikes were scattered all around the feet of the opponent's team!"); }
        else log.push_back("But it failed!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }
    else if (move.name == "Toxic Spikes") {
        if (dSide.toxicSpikes < 2) { dSide.toxicSpikes++; log.push_back("Poison spikes were scattered all around the feet of the opponent's team!"); }
        else log.push_back("But it failed!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }
    else if (move.name == "Sticky Web") {
        if (!dSide.stickyWeb) { dSide.stickyWeb = true; log.push_back("A sticky web has been laid out beneath the opponent's team!"); }
        else log.push_back("But it failed!");
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }
    else if (move.name == "Defog") {
        enemySide.stealthRock = false; enemySide.spikes = 0; enemySide.toxicSpikes = 0; enemySide.stickyWeb = false;
        playerSide.stealthRock = false; playerSide.spikes = 0; playerSide.toxicSpikes = 0; playerSide.stickyWeb = false;
        log.push_back("A strong wind blew away the hazards!");
        if (dAbil != "Clear Body" && dAbil != "White Smoke" && dAbil != "Full Metal Body" && dItem != "CLEARAMULET") {
            defender.modifyStat("evasion", -1, log);
        }
        attacker.addVolatile("moved_this_turn", 1);
        return;
    }

    bool isSound = (move.name == "Boomburst" || move.name == "Hyper Voice" || move.name == "Uproar" || move.name == "Snarl" || move.name == "Overdrive" || move.name == "Bug Buzz" || move.name == "Echoed Voice" || move.name == "Relic Song" || move.name == "Clanging Scales" || move.name == "Alluring Voice" || move.name == "Psychic Noise" || move.name == "Disarming Voice" || move.name == "Sparkling Aria");
    bool blocksEffects = (dSide.substituteHp > 0) && !isSound && aAbil != "Infiltrator";
    bool isPowder = (move.name == "Spore" || move.name == "Sleep Powder" || move.name == "Poison Powder" || move.name == "Stun Spore" || move.name == "Rage Powder" || move.name == "Magic Powder" || move.name == "Powder");

    if (move.volatileStatus == "partiallytrapped") {
        int trapTurns = (aItem == "GRIPCLAW") ? 7 : ((rand() % 2) + 4);
        defender.addVolatile("partiallytrapped", trapTurns);
        log.push_back(defender.getNickname() + " became trapped by " + move.name + "!");
    }

    if (move.category == "Status") {
        if (dAbil == "Good as Gold" && move.overrideOffensivePokemon != "target") {
            log.push_back(defender.getNickname() + "'s Good as Gold prevented the status move!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }

        if (isPowder && (dAbil == "Overcoat" || defender.getType1() == "Grass" || defender.getType2() == "Grass" || dItem == "SAFETYGOGGLES")) {
            log.push_back("It doesn't affect " + defender.getNickname() + "...");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }

        if (blocksEffects && move.selfBoosts.empty() && (!move.status.empty() || !move.boosts.empty() || !move.volatileStatus.empty())) {
            log.push_back("But it failed against the substitute!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }

        for (const auto& [stat, stage] : move.selfBoosts) attacker.modifyStat(stat, stage, log);

        for (const auto& [stat, stage] : move.boosts) {
            if (stage < 0 && dItem == "CLEARAMULET") {
                log.push_back(defender.getNickname() + "'s Clear Amulet prevents its stats from being lowered!");
                continue;
            }

            if (stage < 0 && dAbil == "Mirror Armor") {
                log.push_back(defender.getNickname() + "'s Mirror Armor reflected the stat drop!");
                attacker.modifyStat(stat, stage, log);
                continue;
            }

            bool immuneToDrop = false;
            if (stage < 0) {
                if (dAbil == "Clear Body" || dAbil == "White Smoke" || dAbil == "Full Metal Body") immuneToDrop = true;
                if (dAbil == "Hyper Cutter" && stat == "atk") immuneToDrop = true;
                if (dAbil == "Big Pecks" && stat == "def") immuneToDrop = true;
                if (dAbil == "Keen Eye" && stat == "accuracy") immuneToDrop = true;
            }

            if (!immuneToDrop) {
                defender.modifyStat(stat, stage, log);
                if (stage < 0) {
                    if (dItem == "WHITEHERB") {
                        defender.modifyStat(stat, -stage, log);
                        removeItem(defender);
                        log.push_back(defender.getNickname() + " restored its status using its White Herb!");
                    }
                    if (dAbil == "Defiant") {
                        defender.modifyStat("atk", 2, log);
                        log.push_back(defender.getNickname() + "'s Defiant sharply raised its Attack!");
                    }
                    else if (dAbil == "Competitive") {
                        defender.modifyStat("spa", 2, log);
                        log.push_back(defender.getNickname() + "'s Competitive sharply raised its Special Attack!");
                    }
                }
            }
            else if (stage < 0) {
                log.push_back(defender.getNickname() + "'s " + dAbil + " prevents stat drops!");
            }
        }

        if (move.volatileStatus == "torment") {
            if (dAbil == "Aroma Veil") {
                log.push_back(defender.getNickname() + "'s Aroma Veil protects it from Torment!");
            }
            else {
                defender.addVolatile("torment", 999);
                log.push_back(defender.getNickname() + " was subjected to torment!");
            }
        }
        else if (move.volatileStatus == "taunt") {
            if (dAbil == "Oblivious" || dAbil == "Aroma Veil") {
                log.push_back(defender.getNickname() + "'s " + dAbil + " protects it from Taunt!");
            }
            else {
                defender.addVolatile("taunt", 3);
                log.push_back(defender.getNickname() + " fell for the taunt!");
            }
        }
        else if (move.volatileStatus == "encore") {
            if (dAbil == "Aroma Veil") {
                log.push_back(defender.getNickname() + "'s Aroma Veil protects it from Encore!");
            }
            else {
                defender.addVolatile("encore", 3);
                log.push_back(defender.getNickname() + " received an encore!");
            }
        }
        else if (move.volatileStatus == "disable") {
            if (dAbil == "Aroma Veil") {
                log.push_back(defender.getNickname() + "'s Aroma Veil protects it from Disable!");
            }
            else {
                defender.addVolatile("disable", 4);
                log.push_back(defender.getNickname() + "'s move was disabled!");
            }
        }
        else if (move.volatileStatus == "confusion") {
            if (dAbil == "Own Tempo") {
                log.push_back(defender.getNickname() + "'s Own Tempo prevents confusion!");
            }
            else {
                defender.addVolatile("confusion", (rand() % 4) + 2);
                log.push_back(defender.getNickname() + " became confused!");
            }
        }

        if (dItem == "MENTALHERB") {
            bool usedHerb = false;
            if (defender.hasVolatile("taunt")) { defender.removeVolatile("taunt"); usedHerb = true; }
            if (defender.hasVolatile("torment")) { defender.removeVolatile("torment"); usedHerb = true; }
            if (defender.hasVolatile("confusion")) { defender.removeVolatile("confusion"); usedHerb = true; }
            if (defender.hasVolatile("encore")) { defender.removeVolatile("encore"); usedHerb = true; }
            if (defender.hasVolatile("disable")) { defender.removeVolatile("disable"); usedHerb = true; }
            if (usedHerb) {
                removeItem(defender);
                log.push_back(defender.getNickname() + " cured its mental state using its Mental Herb!");
            }
        }

        if (move.name == "Parting Shot") attacker.isSwitchingOut = true;
        if (move.name == "Chilly Reception") { field.weather = "Snow"; attacker.isSwitchingOut = true; log.push_back("It started to snow!"); }
        if (move.name == "Teleport") attacker.isSwitchingOut = true;
        if (move.name == "Baton Pass") { attacker.isSwitchingOut = true; attacker.isBatonPassing = true; }

        if (move.name == "Shed Tail") {
            if (attacker.getCurrentHp() > attacker.getHp() / 2) {
                int cost = std::max(1, attacker.getHp() / 2);
                attacker.setCurrentHp(attacker.getCurrentHp() - cost);
                aSide.substituteHp = cost / 2;
                attacker.isSwitchingOut = true;
                attacker.isBatonPassing = true;
                log.push_back(attacker.getNickname() + " shed its tail to create a decoy!");
            }
            else log.push_back("But it failed!");
        }

        if (!move.status.empty() && defender.getStatus().empty()) {
            bool immune = false;
            if (dSide.safeguard > 0 && aAbil != "Infiltrator") {
                log.push_back(defender.getNickname() + " is protected by Safeguard!");
                attacker.addVolatile("moved_this_turn", 1);
                return;
            }
            if (dAbil == "Shields Down" && !defender.hasVolatile("shields_down")) immune = true;
            if (dAbil == "Comatose") immune = true;
            if (move.status == "slp" && (dAbil == "Insomnia" || dAbil == "Vital Spirit" || dAbil == "Sweet Veil")) immune = true;
            if (move.status == "brn" && (defender.getType1() == "Fire" || defender.getType2() == "Fire" || dAbil == "Water Veil" || dAbil == "Water Bubble" || dAbil == "Thermal Exchange")) immune = true;

            if (move.status == "psn" || move.status == "tox") {
                if (dAbil == "Immunity" || dAbil == "Pastel Veil" || (dAbil == "Flower Veil" && (defender.getType1() == "Grass" || defender.getType2() == "Grass")) || (dAbil == "Leaf Guard" && (weather == "Sun" || weather == "DesolateLand"))) {
                    immune = true;
                }
                else if (defender.getType1() == "Poison" || defender.getType2() == "Poison" || defender.getType1() == "Steel" || defender.getType2() == "Steel") {
                    immune = (aAbil != "Corrosion");
                }
            }

            if (move.status == "par" && (defender.getType1() == "Ground" || defender.getType2() == "Ground" || defender.getType1() == "Electric" || defender.getType2() == "Electric" || dAbil == "Limber")) immune = true;
            if (move.status == "frz" && (defender.getType1() == "Ice" || defender.getType2() == "Ice" || dAbil == "Magma Armor")) immune = true;

            if (dAbil == "Leaf Guard" && (weather == "Sun" || weather == "DesolateLand")) immune = true;
            if (dAbil == "Flower Veil" && (defender.getType1() == "Grass" || defender.getType2() == "Grass")) immune = true;

            if (immune) log.push_back("It doesn't affect " + defender.getNickname() + "...");
            else {
                defender.setStatus(move.status, (move.status == "slp") ? (rand() % 3) + 1 : 0);
                if (move.status == "brn") log.push_back(defender.getNickname() + " was burned!");
                else if (move.status == "psn" || move.status == "tox") {
                    log.push_back(defender.getNickname() + " was poisoned!");
                    if (aAbil == "Poison Puppeteer") {
                        defender.addVolatile("confusion", (rand() % 4) + 2);
                        log.push_back(defender.getNickname() + " was confused by Poison Puppeteer!");
                    }
                }
                else if (move.status == "par") log.push_back(defender.getNickname() + " was paralyzed!");
                else if (move.status == "slp") log.push_back(defender.getNickname() + " fell asleep!");
                else if (move.status == "frz") log.push_back(defender.getNickname() + " was frozen!");

                if (dItem == "LUMBERRY") {
                    defender.setStatus("");
                    removeItem(defender);
                    log.push_back(defender.getNickname() + " cured its status using its Lum Berry!");
                }

                if (dAbil == "Synchronize" && (move.status == "brn" || move.status == "psn" || move.status == "tox" || move.status == "par")) {
                    if (attacker.getStatus().empty()) {
                        attacker.setStatus(move.status);
                        log.push_back(defender.getNickname() + "'s Synchronize passed the condition back to " + attacker.getNickname() + "!");
                    }
                }
            }
        }
        attacker.addVolatile("moved_this_turn", 1);

        bool statsFell = false;
        for (size_t i = logIndexBeforeTurn; i < log.size(); i++) {
            if (log[i].find("fell") != std::string::npos) statsFell = true;
        }

        if (statsFell && aItem == "EJECTPACK") {
            removeItem(attacker);
            attacker.isSwitchingOut = true;
            log.push_back(attacker.getNickname() + " is switched out by its Eject Pack!");
        }

        return;
    }

    if (move.name == "Counter") {
        if (attacker.lastDamageCategory == "Physical" && attacker.lastDamageTaken > 0) {
            defender.setCurrentHp(std::max(0, defender.getCurrentHp() - (attacker.lastDamageTaken * 2)));
            log.push_back("Counter reflected the damage!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
        else {
            log.push_back("But it failed!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }
    if (move.name == "Mirror Coat") {
        if (attacker.lastDamageCategory == "Special" && attacker.lastDamageTaken > 0) {
            defender.setCurrentHp(std::max(0, defender.getCurrentHp() - (attacker.lastDamageTaken * 2)));
            log.push_back("Mirror Coat reflected the damage!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
        else {
            log.push_back("But it failed!");
            attacker.addVolatile("moved_this_turn", 1);
            return;
        }
    }

    int hits = move.minHits;
    if (move.maxHits > move.minHits) {
        if (aAbil == "Skill Link") hits = move.maxHits;
        else if (aItem == "LOADEDDICE" && move.maxHits >= 4) {
            hits = (move.maxHits - 1) + (rand() % 2);
        }
        else {
            int r = rand() % 100;
            if (r < 35) hits = 2; else if (r < 70) hits = 3; else if (r < 85) hits = 4; else hits = 5;
        }
    }
    else if (aAbil == "Parental Bond" && move.category != "Status" && move.name != "Endeavor" && move.name != "Explosion" && move.name != "Self-Destruct" && move.name != "Final Gambit") {
        hits = 2;
    }

    if (move.name == "Population Bomb") {
        if (aAbil == "Skill Link") hits = 10;
        else if (aItem == "LOADEDDICE") hits = 4 + (rand() % 7);
        else hits = 10;
    }
    else if (move.name == "Beat Up") {
        if (aAbil == "Skill Link") hits = 5;
        else if (aItem == "LOADEDDICE") hits = 4 + (rand() % 2);
        else {
            int r = rand() % 100;
            if (r < 35) hits = 2; else if (r < 70) hits = 3; else if (r < 85) hits = 4; else hits = 5;
        }
    }

    int actualHits = 0;
    bool isCrit = false;
    int totalDamageDealt = 0;

    if (move.name == "Glaive Rush") {
        attacker.addVolatile("glaiverush", 2);
    }

    for (int i = 0; i < hits; ++i) {
        if (defender.getCurrentHp() <= 0) break;

        if (move.name == "Population Bomb" && aItem != "LOADEDDICE") {
            if (rand() % 100 >= move.accuracy) {
                if (i == 0) log.push_back(attacker.getNickname() + "'s attack missed!");
                break;
            }
        }

        bool canCrit = (dAbil != "Battle Armor" && dAbil != "Shell Armor");
        if (move.name == "Frost Breath" || move.name == "Storm Throw" || move.name == "Wicked Blow" || move.name == "Surging Strikes" || move.name == "Flower Trick") {
            isCrit = canCrit;
        }
        else if (aAbil == "Merciless" && (defender.getStatus() == "psn" || defender.getStatus() == "tox")) {
            isCrit = canCrit;
        }
        else if (canCrit) {
            int critStage = 0;
            if (aAbil == "Super Luck") critStage++;
            if (move.name == "Slash" || move.name == "Night Slash" || move.name == "Psycho Cut" || move.name == "Shadow Claw" || move.name == "Cross Poison" || move.name == "Leaf Blade" || move.name == "Stone Edge" || move.name == "Attack Order" || move.name == "Drill Run" || move.name == "Spacial Rend" || move.name == "Snipe Shot") critStage++;
            if (aItem == "SCOPELENS" || aItem == "RAZORCLAW" || aItem == "STICK" || aItem == "LEEK" || aItem == "LUCKYPUNCH") critStage++;
            if (defender.hasVolatile("focusenergy") || dItem == "LANSATBERRY") critStage += 2;

            if (critStage == 0) isCrit = (rand() % 24 == 0);
            else if (critStage == 1) isCrit = (rand() % 8 == 0);
            else if (critStage == 2) isCrit = (rand() % 2 == 0);
            else isCrit = true;
        }
        else {
            isCrit = false;
        }

        int dmg = calculateDamageInternal(attacker, defender, move, isCrit, log, (i == 0));
        if (dmg == 0) break;

        std::string dItemLoop = (field.magicRoom > 0) ? "NONE" : normalizeString(defender.getHeldItem());
        bool hasSturdy = (!moldBreaker && dAbil == "Sturdy" && defender.getCurrentHp() == defender.getHp());
        bool hasSash = (dItemLoop == "FOCUSSASH" && defender.getCurrentHp() == defender.getHp());
        bool hasBand = (dItemLoop == "FOCUSBAND" && (rand() % 100 < 10));

        if (!blocksEffects) {
            if (dAbil == "Disguise" && !defender.hasVolatile("disguise_busted") && dmg > 0) {
                defender.addVolatile("disguise_busted", 1);
                log.push_back(defender.getNickname() + "'s disguise was busted!");
                dmg = 0;
                int bustDmg = std::max(1, defender.getHp() / 8);
                defender.setCurrentHp(std::max(0, defender.getCurrentHp() - bustDmg));
            }
            else if (dAbil == "Ice Face" && move.category == "Physical" && !defender.hasVolatile("iceface_busted") && dmg > 0) {
                defender.addVolatile("iceface_busted", 1);
                log.push_back(defender.getNickname() + "'s Ice Face took the hit!");
                dmg = 0;
            }
            else if (defender.hasVolatile("illusion") && dmg > 0) {
                defender.removeVolatile("illusion");
                log.push_back(defender.getNickname() + "'s Illusion wore off!");
            }

            if ((hasSturdy || hasSash || hasBand) && dmg >= defender.getCurrentHp()) {
                dmg = defender.getCurrentHp() - 1;
                if (hasSturdy) log.push_back(defender.getNickname() + " hung on with Sturdy!");
                else if (hasSash) { log.push_back(defender.getNickname() + " hung on using its Focus Sash!"); removeItem(defender); }
                else if (hasBand) log.push_back(defender.getNickname() + " hung on using its Focus Band!");
            }

            int actualHpLost = std::min(defender.getCurrentHp(), dmg);
            defender.setCurrentHp(std::max(0, defender.getCurrentHp() - dmg));
            totalDamageDealt += actualHpLost;
            actualHits++;

            if (dItemLoop == "AIRBALLOON" && dmg > 0 && defender.getCurrentHp() > 0) {
                removeItem(defender);
                log.push_back(defender.getNickname() + "'s Air Balloon popped!");
            }
        }
        else {
            int dmgToSub = std::min(dSide.substituteHp, dmg);
            dSide.substituteHp -= dmgToSub;
            totalDamageDealt += dmgToSub;
            actualHits++;
            if (dSide.substituteHp <= 0) {
                log.push_back(defender.getNickname() + "'s substitute faded!");
                blocksEffects = false;
            }
            else {
                log.push_back("The substitute took damage for " + defender.getNickname() + "!");
            }
        }

        if (defender.getCurrentHp() <= 0) {
            if (dAbil == "Aftermath" && move.isContact && aAbil != "Magic Guard" && aAbil != "Damp") {
                int afterDmg = std::max(1, attacker.getHp() / 4);
                attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - afterDmg));
                log.push_back(attacker.getNickname() + " was hurt by " + defender.getNickname() + "'s Aftermath!");
            }
            else if (dAbil == "Innards Out" && aAbil != "Magic Guard") {
                attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - dmg));
                log.push_back(attacker.getNickname() + " was hurt by " + defender.getNickname() + "'s Innards Out!");
            }

            if (aAbil == "Moxie" || aAbil == "Chilling Neigh") {
                log.push_back(attacker.getNickname() + "'s " + aAbil + " triggered!");
                attacker.modifyStat("atk", 1, log);
            }
            else if (aAbil == "Grim Neigh") {
                log.push_back(attacker.getNickname() + "'s Grim Neigh triggered!");
                attacker.modifyStat("spa", 1, log);
            }
            else if (aAbil == "Beast Boost") {
                int stats[5] = { attacker.getBattleAttack(), attacker.getBattleDefense(), attacker.getBattleSpAttack(), attacker.getBattleSpDefense(), attacker.getBattleSpeed() };
                std::string statNames[5] = { "atk", "def", "spa", "spd", "spe" };
                int maxIdx = 0;
                for (int j = 1; j < 5; ++j) { if (stats[j] > stats[maxIdx]) maxIdx = j; }
                log.push_back(attacker.getNickname() + "'s " + aAbil + " triggered!");
                attacker.modifyStat(statNames[maxIdx], 1, log);
            }
            break;
        }
    }

    if (actualHits > 1) log.push_back("Hit " + std::to_string(actualHits) + " time(s)!");

    if (actualHits > 0) {
        if (move.name == "Thousand Arrows" || move.name == "Smack Down") {
            if (defender.getType1() == "Flying" || defender.getType2() == "Flying" || dAbil == "Levitate" || dItem == "AIRBALLOON") {
                defender.addVolatile("smackdown", 999);
                log.push_back(defender.getNickname() + " fell straight down!");
            }
        }
        if (move.name == "Roar of Time" || move.name == "Hyper Beam" || move.name == "Giga Impact" || move.name == "Frenzy Plant" || move.name == "Blast Burn" || move.name == "Hydro Cannon" || move.name == "Rock Wrecker" || move.name == "Meteor Assault" || move.name == "Prismatic Laser" || move.name == "Eternabeam") {
            attacker.addVolatile("mustrecharge", 2);
        }
    }

    if (actualHits == 0 && (move.name == "Rollout" || move.name == "Ice Ball")) {
        attacker.setLockedMove("", 0);
        attacker.removeVolatile("rollout_count");
    }

    if (totalDamageDealt > 0) {
        if (!blocksEffects) {
            defender.lastDamageTaken = totalDamageDealt;
            defender.lastDamageCategory = move.category;

            if (dAbil == "Opportunist") {
                bool boosted = false;
                for (const auto& [stat, stage] : move.selfBoosts) {
                    if (stage > 0) { defender.modifyStat(stat, stage, log); boosted = true; }
                }
                if (boosted) log.push_back(defender.getNickname() + "'s Opportunist copied the stat boosts!");
            }

            if (dAbil == "Stamina") defender.modifyStat("def", 1, log);
            if (dAbil == "Rattled" && (move.type == "Dark" || move.type == "Ghost" || move.type == "Bug")) defender.modifyStat("spe", 1, log);
            if (dAbil == "Justified" && move.type == "Dark") defender.modifyStat("atk", 1, log);
            if (dAbil == "Water Compaction" && move.type == "Water") defender.modifyStat("def", 2, log);
            if (dAbil == "Thermal Exchange" && move.type == "Fire") {
                log.push_back(defender.getNickname() + "'s Thermal Exchange raised its Attack!");
                defender.modifyStat("atk", 1, log);
            }
            if (dAbil == "Cursed Body" && rand() % 100 < 30) {
                attacker.addVolatile("disable", 4);
                log.push_back(attacker.getNickname() + "'s move was disabled by Cursed Body!");
            }
            if (dAbil == "Cotton Down" && aAbil != "Clear Body" && aAbil != "White Smoke" && aAbil != "Full Metal Body" && aItem != "CLEARAMULET") {
                log.push_back(defender.getNickname() + "'s Cotton Down lowered Speed!");
                attacker.modifyStat("spe", -1, log);
                defender.modifyStat("spe", -1, log);
            }
            if (dAbil == "Weak Armor" && move.category == "Physical") {
                log.push_back(defender.getNickname() + "'s Weak Armor activated!");
                defender.modifyStat("def", -1, log);
                defender.modifyStat("spe", 2, log);
            }
            if (dAbil == "Anger Shell" && defender.getCurrentHp() <= defender.getHp() / 2 && oldDefHp > defender.getHp() / 2) {
                log.push_back(defender.getNickname() + "'s Anger Shell activated!");
                defender.modifyStat("atk", 1, log);
                defender.modifyStat("spa", 1, log);
                defender.modifyStat("spe", 1, log);
                defender.modifyStat("def", -1, log);
                defender.modifyStat("spd", -1, log);
            }
            if (dAbil == "Berserk" && defender.getCurrentHp() <= defender.getHp() / 2 && oldDefHp > defender.getHp() / 2) {
                log.push_back(defender.getNickname() + "'s Berserk activated!");
                defender.modifyStat("spa", 1, log);
            }

            if (dAbil == "Sand Spit" && field.weather != "Sandstorm") {
                field.weather = "Sandstorm";
                field.weatherTurns = 5;
                log.push_back(defender.getNickname() + "'s Sand Spit whipped up a sandstorm!");
            }
            if (dAbil == "Seed Sower" && field.terrain != "Grassy Terrain") {
                field.terrain = "Grassy Terrain";
                field.terrainTurns = 5;
                log.push_back(defender.getNickname() + "'s Seed Sower turned the ground into Grassy Terrain!");
            }
            if (dAbil == "Toxic Debris" && move.category == "Physical") {
                if (dSide.toxicSpikes < 2) {
                    dSide.toxicSpikes++;
                    log.push_back(defender.getNickname() + "'s Toxic Debris scattered poison spikes around the enemy team!");
                }
            }

            if (move.isContact && defender.getCurrentHp() > 0) {
                if (dItem == "ROCKYHELMET" && aAbil != "Magic Guard") {
                    attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - std::max(1, attacker.getHp() / 6)));
                    log.push_back(attacker.getNickname() + " was hurt by the Rocky Helmet!");
                }

                if ((dAbil == "Rough Skin" || dAbil == "Iron Barbs") && aAbil != "Magic Guard") {
                    attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - std::max(1, attacker.getHp() / 8)));
                    log.push_back(attacker.getNickname() + " was hurt by " + dAbil + "!");
                }

                if (dAbil == "Poison Point" && rand() % 100 < 30 && attacker.getStatus().empty() && aAbil != "Shield Dust") {
                    attacker.setStatus("psn");
                    log.push_back(attacker.getNickname() + " was poisoned by Poison Point!");
                }
                if (dAbil == "Effect Spore" && rand() % 100 < 30 && attacker.getStatus().empty() && aAbil != "Shield Dust") {
                    int rnd = rand() % 3;
                    if (rnd == 0) { attacker.setStatus("psn"); log.push_back(attacker.getNickname() + " was poisoned by Effect Spore!"); }
                    else if (rnd == 1) { attacker.setStatus("par"); log.push_back(attacker.getNickname() + " was paralyzed by Effect Spore!"); }
                    else { attacker.setStatus("slp", 2); log.push_back(attacker.getNickname() + " fell asleep from Effect Spore!"); }
                }
                if (dAbil == "Flame Body" && rand() % 100 < 30 && attacker.getStatus().empty() && aAbil != "Shield Dust") {
                    attacker.setStatus("brn");
                    log.push_back(attacker.getNickname() + " was burned by Flame Body!");
                }
                if (dAbil == "Static" && rand() % 100 < 30 && attacker.getStatus().empty() && aAbil != "Shield Dust") {
                    attacker.setStatus("par");
                    log.push_back(attacker.getNickname() + " was paralyzed by Static!");
                }

                if ((dAbil == "Gooey" || dAbil == "Tangling Hair") && aAbil != "Clear Body" && aAbil != "White Smoke" && aAbil != "Full Metal Body" && aItem != "CLEARAMULET") {
                    log.push_back(attacker.getNickname() + "'s Speed fell due to " + dAbil + "!");
                    attacker.modifyStat("spe", -1, log);
                }

                if ((dAbil == "Mummy" || dAbil == "Lingering Aroma") && aAbil != dAbil && aAbil != "Multitype" && aAbil != "Stance Change") {
                    attacker.setAbility(dAbil);
                    log.push_back(attacker.getNickname() + "'s Ability became " + dAbil + "!");
                }

                if (dAbil == "Wandering Spirit" && aAbil != "Wandering Spirit" && aAbil != "Multitype" && aAbil != "Stance Change") {
                    std::string temp = attacker.getAbility();
                    attacker.setAbility("Wandering Spirit");
                    defender.setAbility(temp);
                    log.push_back(attacker.getNickname() + " swapped Abilities with its target!");
                }

                if (aAbil == "Poison Touch" && defender.getStatus().empty() && dAbil != "Shield Dust") {
                    if (rand() % 100 < 30) {
                        defender.setStatus("psn");
                        log.push_back(defender.getNickname() + " was poisoned by Poison Touch!");
                    }
                }
            }

            if (aAbil == "Toxic Chain" && defender.getStatus().empty() && dAbil != "Shield Dust") {
                if (rand() % 100 < 30) {
                    defender.setStatus("tox");
                    log.push_back(defender.getNickname() + " was badly poisoned by the Toxic Chain!");
                }
            }
        }
    }

    if (actualHits > 0) {
        if (move.name == "U-turn" || move.name == "Volt Switch" || move.name == "Flip Turn") {
            attacker.isSwitchingOut = true;
        }
    }

    if (move.name == "Explosion" || move.name == "Self-Destruct" || move.name == "Misty Explosion" || move.name == "Final Gambit") {
        attacker.setCurrentHp(0);
        log.push_back(attacker.getNickname() + " fainted!");
    }

    if (move.name == "Struggle" || move.name == "struggle") {
        int recoilDmg = std::max(1, attacker.getHp() / 4);
        attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - recoilDmg));
        log.push_back(attacker.getNickname() + " was damaged by the recoil!");
    }
    else if (actualHits > 0 && totalDamageDealt > 0) {
        bool isDrainMove = (move.name == "Drain Punch" || move.name == "Leech Life" || move.name == "Absorb" ||
            move.name == "Mega Drain" || move.name == "Giga Drain" || move.name == "Horn Leech" ||
            move.name == "Parabolic Charge" || move.name == "Draining Kiss" || move.name == "Bitter Blade" ||
            move.name == "Bouncy Bubble" || move.name == "Matcha Gotcha" || move.name == "Oblivion Wing");

        if (aItem == "SHELLBELL" && attacker.getCurrentHp() < attacker.getHp()) {
            int shellHeal = std::max(1, totalDamageDealt / 8);
            attacker.setCurrentHp(std::min(attacker.getHp(), attacker.getCurrentHp() + shellHeal));
            log.push_back(attacker.getNickname() + " restored a little HP using its Shell Bell!");
        }

        if ((move.healNum > 0 && move.healDen > 0 && move.category != "Status") || isDrainMove) {
            int num = move.healNum > 0 ? move.healNum : 1;
            int den = move.healDen > 0 ? move.healDen : 2;

            if (move.name == "Draining Kiss" || move.name == "Oblivion Wing") { num = 3; den = 4; }

            int healAmt = (totalDamageDealt * num) / den;

            if (aItem == "BIGROOT") {
                healAmt = std::floor(healAmt * 1.3);
            }

            if (dAbil == "Liquid Ooze") {
                log.push_back(defender.getNickname() + "'s Liquid Ooze damaged " + attacker.getNickname() + "!");
                attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - std::max(1, healAmt)));
            }
            else {
                attacker.setCurrentHp(std::min(attacker.getHp(), attacker.getCurrentHp() + std::max(1, healAmt)));
                log.push_back(defender.getNickname() + " had its energy drained!");
            }
        }

        if (move.recoilNum > 0 && move.recoilDen > 0) {
            int recoilDmg = (totalDamageDealt * move.recoilNum) / move.recoilDen;
            if (aAbil != "Rock Head" && aAbil != "Magic Guard") {
                attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - std::max(1, recoilDmg)));
                log.push_back(attacker.getNickname() + " was damaged by the recoil!");
            }
        }

        if (aAbil != "Magic Guard" && aItem == "LIFEORB") {
            if (!(aAbil == "Sheer Force" && move.effectChance > 0)) {
                int loRecoil = std::max(1, attacker.getHp() / 10);
                attacker.setCurrentHp(std::max(0, attacker.getCurrentHp() - loRecoil));
                log.push_back(attacker.getNickname() + " lost some of its HP!");
            }
        }
    }

    if (move.volatileStatus == "lockedmove") {
        if (attacker.getLockedTurns() <= 0) attacker.setLockedMove(move.name, (rand() % 2) + 2);
    }

    if (actualHits > 0 && defender.getCurrentHp() > 0 && !blocksEffects) {
        if ((move.name == "Thief" || move.name == "Covet") && aItem == "NONE" && dItem != "NONE" && dAbil != "Sticky Hold") {
            attacker.setHeldItem(defender.getHeldItem());
            removeItem(defender);
            log.push_back(attacker.getNickname() + " stole " + defender.getNickname() + "'s item!");
        }
        if (aAbil == "Magician" && aItem == "NONE" && dItem != "NONE" && dAbil != "Sticky Hold") {
            attacker.setHeldItem(defender.getHeldItem());
            removeItem(defender);
            log.push_back(attacker.getNickname() + " stole " + defender.getNickname() + "'s item using Magician!");
        }
        if (dAbil == "Pickpocket" && dItem == "NONE" && aItem != "NONE" && move.isContact && aAbil != "Sticky Hold") {
            defender.setHeldItem(attacker.getHeldItem());
            removeItem(attacker);
            log.push_back(defender.getNickname() + " stole " + attacker.getNickname() + "'s item using Pickpocket!");
        }

        if (move.name == "Knock Off" && dItem != "NONE" && dAbil != "Sticky Hold") {
            removeItem(defender);
            log.push_back(defender.getNickname() + " dropped its item!");
        }
        if (move.name == "Clear Smog") {
            defender.resetStatStages();
            log.push_back(defender.getNickname() + "'s stat changes were removed!");
        }
        if (move.name == "Haze") {
            attacker.resetStatStages();
            defender.resetStatStages();
            log.push_back("All stat changes were eliminated!");
        }
        if (move.name == "Rapid Spin" || move.name == "Mortal Spin") {
            SideState& mySide = isPlayerAttacking ? playerSide : enemySide;
            if (mySide.stealthRock || mySide.spikes > 0 || mySide.toxicSpikes > 0 || mySide.stickyWeb || attacker.hasVolatile("leechseed") || attacker.hasVolatile("partiallytrapped")) {
                mySide.stealthRock = false;
                mySide.spikes = 0;
                mySide.toxicSpikes = 0;
                mySide.stickyWeb = false;
                attacker.removeVolatile("leechseed");
                attacker.removeVolatile("partiallytrapped");
                log.push_back(attacker.getNickname() + " blew away hazards and binding effects!");
            }
            if (move.name == "Rapid Spin") attacker.modifyStat("spe", 1, log);
            if (move.name == "Mortal Spin" && defender.getStatus().empty()) {
                defender.setStatus("psn");
                log.push_back(defender.getNickname() + " was poisoned by Mortal Spin!");
            }
        }
        if (move.name == "Leech Seed") {
            if (defender.getType1() != "Grass" && defender.getType2() != "Grass" && !defender.hasVolatile("leechseed")) {
                defender.addVolatile("leechseed", 999);
                log.push_back(defender.getNickname() + " was seeded!");
            }
            else {
                log.push_back("But it failed!");
            }
        }
    }

    if (actualHits > 0 && defender.getCurrentHp() > 0 && !blocksEffects) {
        int effChance = move.effectChance;

        if (aAbil == "Serene Grace") effChance *= 2;

        if (effChance > 0 && (rand() % 100 < effChance)) {
            if (aAbil != "Sheer Force" && dAbil != "Shield Dust" && dItem != "COVERTCLOAK") {

                if (move.volatileStatus == "flinch") defender.addVolatile("flinch", 1);
                if (move.volatileStatus == "confusion" && !defender.hasVolatile("confusion")) {
                    defender.addVolatile("confusion", (rand() % 4) + 2);
                    log.push_back(defender.getNickname() + " became confused!");
                }

                if (!move.status.empty() && defender.getStatus().empty()) {
                    bool immune = false;
                    if (dSide.safeguard > 0 && aAbil != "Infiltrator") {
                        log.push_back(defender.getNickname() + " is protected by Safeguard!");
                        attacker.addVolatile("moved_this_turn", 1);
                        return;
                    }
                    if (dAbil == "Shields Down" && !defender.hasVolatile("shields_down")) immune = true;
                    if (dAbil == "Comatose") immune = true;
                    if (move.status == "slp" && (dAbil == "Insomnia" || dAbil == "Vital Spirit" || dAbil == "Sweet Veil")) immune = true;
                    if (move.status == "brn" && (defender.getType1() == "Fire" || defender.getType2() == "Fire" || dAbil == "Water Veil" || dAbil == "Water Bubble" || dAbil == "Thermal Exchange")) immune = true;

                    if (move.status == "psn" || move.status == "tox") {
                        if (dAbil == "Immunity" || dAbil == "Pastel Veil" || (dAbil == "Flower Veil" && (defender.getType1() == "Grass" || defender.getType2() == "Grass")) || (dAbil == "Leaf Guard" && (weather == "Sun" || weather == "DesolateLand"))) {
                            immune = true;
                        }
                        else if (defender.getType1() == "Poison" || defender.getType2() == "Poison" || defender.getType1() == "Steel" || defender.getType2() == "Steel") {
                            immune = (aAbil != "Corrosion");
                        }
                    }

                    if (move.status == "par" && (defender.getType1() == "Ground" || defender.getType2() == "Ground" || defender.getType1() == "Electric" || defender.getType2() == "Electric" || dAbil == "Limber")) immune = true;
                    if (move.status == "frz" && (defender.getType1() == "Ice" || defender.getType2() == "Ice" || dAbil == "Magma Armor")) immune = true;

                    if (dAbil == "Leaf Guard" && (weather == "Sun" || weather == "DesolateLand")) immune = true;
                    if (dAbil == "Flower Veil" && (defender.getType1() == "Grass" || defender.getType2() == "Grass")) immune = true;

                    if (immune) log.push_back("It doesn't affect " + defender.getNickname() + "...");
                    else {
                        defender.setStatus(move.status, (move.status == "slp") ? (rand() % 3) + 1 : 0);
                        if (move.status == "brn") log.push_back(defender.getNickname() + " was burned!");
                        else if (move.status == "psn" || move.status == "tox") {
                            log.push_back(defender.getNickname() + " was poisoned!");
                            if (aAbil == "Poison Puppeteer") {
                                defender.addVolatile("confusion", (rand() % 4) + 2);
                                log.push_back(defender.getNickname() + " was confused by Poison Puppeteer!");
                            }
                        }
                        else if (move.status == "par") log.push_back(defender.getNickname() + " was paralyzed!");
                        else if (move.status == "slp") log.push_back(defender.getNickname() + " fell asleep!");
                        else if (move.status == "frz") log.push_back(defender.getNickname() + " was frozen!");

                        if (dItem == "LUMBERRY") {
                            defender.setStatus("");
                            removeItem(defender);
                            log.push_back(defender.getNickname() + " cured its status using its Lum Berry!");
                        }

                        if (dAbil == "Synchronize" && (move.status == "brn" || move.status == "psn" || move.status == "tox" || move.status == "par")) {
                            if (attacker.getStatus().empty()) {
                                attacker.setStatus(move.status);
                                log.push_back(defender.getNickname() + "'s Synchronize passed the condition back to " + attacker.getNickname() + "!");
                            }
                        }
                    }
                }
                for (const auto& [stat, stage] : move.boosts) {
                    if (dAbil != "Clear Body" && dAbil != "White Smoke" && dAbil != "Full Metal Body") {
                        defender.modifyStat(stat, stage, log);
                        if (stage < 0) {
                            if (dItem == "WHITEHERB") {
                                defender.modifyStat(stat, -stage, log);
                                removeItem(defender);
                                log.push_back(defender.getNickname() + " restored its status using its White Herb!");
                            }
                            if (dAbil == "Defiant") {
                                defender.modifyStat("atk", 2, log);
                                log.push_back(defender.getNickname() + "'s Defiant sharply raised its Attack!");
                            }
                            else if (dAbil == "Competitive") {
                                defender.modifyStat("spa", 2, log);
                                log.push_back(defender.getNickname() + "'s Competitive sharply raised its Special Attack!");
                            }
                        }
                    }
                    else if (stage < 0) {
                        log.push_back(defender.getNickname() + "'s " + dAbil + " prevents stat drops!");
                    }
                }
            }
        }
        for (const auto& [stat, stage] : move.selfBoosts) attacker.modifyStat(stat, stage, log);
    }

    if (actualHits > 0 && defender.getCurrentHp() > 0 && defender.getCurrentHp() <= defender.getHp() / 2) {
        if (dItem == "SITRUSBERRY") {
            int sitrusHeal = defender.getHp() / 4;
            if (dAbil == "Ripen") sitrusHeal *= 2;
            defender.setCurrentHp(std::min(defender.getHp(), defender.getCurrentHp() + std::max(1, sitrusHeal)));
            removeItem(defender);
            log.push_back(defender.getNickname() + " restored health using its Sitrus Berry!");
            if (dAbil == "Cheek Pouch") {
                defender.setCurrentHp(std::min(defender.getHp(), defender.getCurrentHp() + std::max(1, defender.getHp() / 3)));
                log.push_back(defender.getNickname() + "'s Cheek Pouch restored more health!");
            }
        }
    }

    if (isSound && (actualHits > 0 || move.category == "Status")) {
        if (aItem == "THROATSPRAY") {
            attacker.modifyStat("spa", 1, log);
            removeItem(attacker);
            log.push_back(attacker.getNickname() + " used its Throat Spray to raise its Sp. Atk!");
        }
    }

    bool statsFell = false;
    for (size_t i = logIndexBeforeTurn; i < log.size(); i++) {
        if (log[i].find("fell") != std::string::npos) statsFell = true;
    }

    if (statsFell && aItem == "EJECTPACK") {
        removeItem(attacker);
        attacker.isSwitchingOut = true;
        log.push_back(attacker.getNickname() + " is switched out by its Eject Pack!");
    }
    else if (defender.statLoweredThisTurn && dItem == "EJECTPACK") {
        removeItem(defender);
        defender.isSwitchingOut = true;
        attacker.isSwitchingOut = false;
        log.push_back(defender.getNickname() + " is switched out by its Eject Pack!");
    }

    if (actualHits > 0 && totalDamageDealt > 0 && oldDefHp > defender.getCurrentHp()) {
        if (dItem == "REDCARD" && attacker.getCurrentHp() > 0) {
            removeItem(defender);
            attacker.isSwitchingOut = true;
            attacker.isForcedRandomSwitch = true;
            log.push_back(defender.getNickname() + " held up its Red Card against " + attacker.getNickname() + "!");
        }
        else if (dItem == "EJECTBUTTON" && defender.getCurrentHp() > 0) {
            removeItem(defender);
            defender.isSwitchingOut = true;
            attacker.isSwitchingOut = false;
            log.push_back(defender.getNickname() + " is switched out by its Eject Button!");
        }

        if ((dAbil == "Emergency Exit" || dAbil == "Wimp Out") && defender.getCurrentHp() <= defender.getHp() / 2 && oldDefHp > defender.getHp() / 2 && defender.getCurrentHp() > 0) {
            defender.isSwitchingOut = true;
            attacker.isSwitchingOut = false;
            log.push_back(defender.getNickname() + " fled using " + dAbil + "!");
        }
    }

    // --- PHAZING FIX ---
    // Moved completely out of the damage block so Roar and Whirlwind finally trigger!
    if (move.name == "Roar" || move.name == "Whirlwind" || (actualHits > 0 && (move.name == "Dragon Tail" || move.name == "Circle Throw"))) {
        if (defender.hasVolatile("ingrain") || dAbil == "Suction Cups") {
            log.push_back(defender.getNickname() + " anchors itself!");
        }
        else {
            defender.isSwitchingOut = true;
            defender.isForcedRandomSwitch = true;
            attacker.isSwitchingOut = false;
            log.push_back(defender.getNickname() + " was blown away!");
        }
    }

    if (attacker.getAbility() == "Zero to Hero" && attacker.getName() == "Palafin" && attacker.isSwitchingOut) {
        log.push_back(attacker.getNickname() + " felt a heroic transformation coming on!");
        attacker.addVolatile("hero_form", 1);
    }

    attacker.addVolatile("moved_this_turn", 1);
}
void BattleEngine::runEndOfTurn(Pokemon& p1, Pokemon& p2, std::vector<std::string>& log) {
    auto tickField = [&](int& counter, const std::string& expirationMsg) {
        if (counter > 0) {
            counter--;
            if (counter == 0) log.push_back(expirationMsg);
        }
        };

    tickField(field.trickRoom, "The twisted dimensions returned to normal!");
    tickField(field.magicRoom, "Magic Room wore off, and items' effects returned!");
    tickField(field.wonderRoom, "Wonder Room wore off, and Defense and Sp. Def stats returned to normal!");
    tickField(field.gravity, "Gravity returned to normal!");

    if (field.weatherTurns > 0) {
        field.weatherTurns--;
        if (field.weatherTurns == 0) {
            field.weather = "None";
            log.push_back("The weather cleared up!");
        }
    }

    if (field.terrainTurns > 0) {
        field.terrainTurns--;
        if (field.terrainTurns == 0) {
            field.terrain = "None";
            log.push_back("The terrain faded away!");
        }
    }

    auto tickSide = [&](SideState& s, const std::string& sideName) {
        if (s.tailwind > 0) { s.tailwind--; if (s.tailwind == 0) log.push_back(sideName + "'s Tailwind petered out!"); }
        if (s.reflect > 0) { s.reflect--; if (s.reflect == 0) log.push_back(sideName + "'s Reflect wore off!"); }
        if (s.lightScreen > 0) { s.lightScreen--; if (s.lightScreen == 0) log.push_back(sideName + "'s Light Screen wore off!"); }
        if (s.auroraVeil > 0) { s.auroraVeil--; if (s.auroraVeil == 0) log.push_back(sideName + "'s Aurora Veil wore off!"); }
        if (s.safeguard > 0) { s.safeguard--; if (s.safeguard == 0) log.push_back(sideName + "'s Safeguard wore off!"); }
        };

    tickSide(playerSide, "Your team");
    tickSide(enemySide, "The opposing team");

    auto postTurn = [&](Pokemon& p, Pokemon& opp) {
        p.lastDamageTaken = 0;
        p.lastDamageCategory = "";
        p.statLoweredThisTurn = false;

        if (p.getCurrentHp() > 0) {
            std::string abil = p.getAbility();
            std::string item = (field.magicRoom > 0) ? "NONE" : normalizeString(p.getHeldItem());
            std::string weather = getActiveWeather(p, opp);

            bool gasActive = false;
            if (opp.getCurrentHp() > 0 && opp.getAbility() == "Neutralizing Gas") gasActive = true;
            if (abil == "Neutralizing Gas") gasActive = true;
            if (gasActive && abil != "Neutralizing Gas" && abil != "Stance Change" && abil != "Multitype" && abil != "Zero to Hero" && abil != "Ice Face" && abil != "Disguise" && abil != "Gulp Missile") {
                abil = "None";
            }

            auto removeItem = [&](Pokemon& pkm) {
                pkm.setHeldItem("None");
                if (pkm.getAbility() == "Unburden" && !pkm.hasVolatile("unburden")) {
                    pkm.addVolatile("unburden", 1);
                    log.push_back(pkm.getNickname() + "'s Unburden doubled its Speed!");
                }
                };

            if (p.hasVolatile("partiallytrapped")) {
                if (p.getVolatileTurns("partiallytrapped") == 1) {
                    log.push_back(p.getNickname() + " was freed from the trap!");
                }
                else {
                    int trapDmg = p.getHp() / 8;
                    if (normalizeString(opp.getHeldItem()) == "BINDINGBAND") trapDmg = p.getHp() / 6;
                    p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, trapDmg)));
                    log.push_back(p.getNickname() + " is hurt by its trap!");
                }
            }
            if (p.hasVolatile("taunt") && p.getVolatileTurns("taunt") == 1) log.push_back(p.getNickname() + "'s taunt wore off!");
            if (p.hasVolatile("encore") && p.getVolatileTurns("encore") == 1) log.push_back(p.getNickname() + "'s encore ended!");
            if (p.hasVolatile("disable") && p.getVolatileTurns("disable") == 1) log.push_back(p.getNickname() + "'s disable ended!");

            if (item == "LEFTOVERS" && p.getCurrentHp() < p.getHp()) {
                p.setCurrentHp(std::min(p.getHp(), p.getCurrentHp() + std::max(1, p.getHp() / 16)));
                log.push_back(p.getNickname() + " restored a little HP using its Leftovers!");
            }
            if (item == "BLACKSLUDGE") {
                if (p.getType1() == "Poison" || p.getType2() == "Poison") {
                    if (p.getCurrentHp() < p.getHp()) {
                        p.setCurrentHp(std::min(p.getHp(), p.getCurrentHp() + std::max(1, p.getHp() / 16)));
                        log.push_back(p.getNickname() + " restored a little HP using its Black Sludge!");
                    }
                }
                else if (abil != "Magic Guard") {
                    p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, p.getHp() / 8)));
                    log.push_back(p.getNickname() + " was hurt by its Black Sludge!");
                }
            }

            if (item == "FLAMEORB" && p.getStatus().empty() && p.getType1() != "Fire" && p.getType2() != "Fire" && abil != "Water Veil" && abil != "Water Bubble" && abil != "Thermal Exchange") {
                p.setStatus("brn");
                log.push_back(p.getNickname() + " was burned by its Flame Orb!");
            }
            if (item == "TOXICORB" && p.getStatus().empty() && p.getType1() != "Poison" && p.getType2() != "Poison" && p.getType1() != "Steel" && p.getType2() != "Steel" && abil != "Immunity" && abil != "Pastel Veil") {
                p.setStatus("tox");
                log.push_back(p.getNickname() + " was badly poisoned by its Toxic Orb!");
            }
            if (item == "STICKYBARB" && abil != "Magic Guard") {
                p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, p.getHp() / 8)));
                log.push_back(p.getNickname() + " is hurt by its Sticky Barb!");
            }

            if (abil == "Harvest" && p.getHeldItem() == "None" && p.hasVolatile("used_berry")) {
                if ((weather == "Sun" || weather == "DesolateLand") || (rand() % 100 < 50)) {
                    log.push_back(p.getNickname() + " harvested a Berry!");
                    p.removeVolatile("used_berry");
                }
            }

            if (abil == "Hunger Switch") {
                if (p.hasVolatile("hangry")) {
                    p.removeVolatile("hangry");
                    log.push_back(p.getNickname() + " returned to its Full Belly Mode!");
                }
                else {
                    p.addVolatile("hangry", 1);
                    log.push_back(p.getNickname() + " changed to Hangry Mode!");
                }
            }

            if (abil == "Speed Boost") {
                p.modifyStat("spe", 1, log);
                log.push_back(p.getNickname() + "'s Speed Boost increased its Speed!");
            }
            if (abil == "Solar Power" && (weather == "Sun" || weather == "DesolateLand")) {
                p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, p.getHp() / 8)));
                log.push_back(p.getNickname() + " was hurt by the sunlight!");
            }
            if (abil == "Rain Dish" && (weather == "Rain" || weather == "PrimordialSea")) {
                p.setCurrentHp(std::min(p.getHp(), p.getCurrentHp() + std::max(1, p.getHp() / 16)));
                log.push_back(p.getNickname() + "'s Rain Dish restored its HP!");
            }
            if (abil == "Ice Body" && (weather == "Snow" || weather == "Hail")) {
                p.setCurrentHp(std::min(p.getHp(), p.getCurrentHp() + std::max(1, p.getHp() / 16)));
                log.push_back(p.getNickname() + "'s Ice Body restored its HP!");
            }

            if (abil == "Shed Skin" && p.getStatus() != "" && (rand() % 100 < 33)) {
                p.setStatus("");
                log.push_back(p.getNickname() + " shed its skin to cure its status!");
            }
            if (abil == "Hydration" && p.getStatus() != "" && (weather == "Rain" || weather == "PrimordialSea")) {
                p.setStatus("");
                log.push_back(p.getNickname() + " cured its status with Hydration!");
            }

            if (abil == "Bad Dreams" && opp.getStatus() == "slp" && opp.getAbility() != "Magic Guard") {
                opp.setCurrentHp(std::max(0, opp.getCurrentHp() - std::max(1, opp.getHp() / 8)));
                log.push_back(opp.getNickname() + " is tormented by Bad Dreams!");
            }

            if (abil == "Moody") {
                std::vector<std::string> stats = { "atk", "def", "spa", "spd", "spe", "accuracy", "evasion" };
                std::string raiseStat = stats[rand() % stats.size()];
                std::string lowerStat = stats[rand() % stats.size()];
                while (raiseStat == lowerStat) {
                    lowerStat = stats[rand() % stats.size()];
                }
                log.push_back(p.getNickname() + "'s Moody activated!");
                p.modifyStat(raiseStat, 2, log);
                p.modifyStat(lowerStat, -1, log);
            }

            if (p.hasVolatile("slowstart")) {
                p.addVolatile("slowstart", p.getVolatileTurns("slowstart") - 1);
                if (p.getVolatileTurns("slowstart") <= 0) {
                    p.removeVolatile("slowstart");
                    log.push_back(p.getNickname() + " finally got its act together!");
                }
            }

            if (abil == "Power Construct" && p.getCurrentHp() <= p.getHp() / 2 && !p.hasVolatile("complete_forme")) {
                p.addVolatile("complete_forme", 1);
                log.push_back(p.getNickname() + " sensed the presence of many cells and assumed its Complete Forme!");
            }

            if (abil == "Shields Down") {
                if (p.getCurrentHp() <= p.getHp() / 2 && !p.hasVolatile("shields_down")) {
                    p.addVolatile("shields_down", 1);
                    log.push_back(p.getNickname() + "'s shields went down! Its core is exposed!");
                }
                else if (p.getCurrentHp() > p.getHp() / 2 && p.hasVolatile("shields_down")) {
                    p.removeVolatile("shields_down");
                    log.push_back(p.getNickname() + " restored its shields!");
                }
            }

            if (abil == "Schooling" && p.getLevel() >= 20) {
                if (p.getCurrentHp() > p.getHp() / 4 && !p.hasVolatile("schooling")) {
                    p.addVolatile("schooling", 1);
                    log.push_back(p.getNickname() + " formed a massive school!");
                }
                else if (p.getCurrentHp() <= p.getHp() / 4 && p.hasVolatile("schooling")) {
                    p.removeVolatile("schooling");
                    log.push_back(p.getNickname() + "'s school scattered!");
                }
            }

            if (abil == "Zen Mode") {
                if (p.getCurrentHp() <= p.getHp() / 2 && !p.hasVolatile("zen_mode")) {
                    p.addVolatile("zen_mode", 1);
                    log.push_back(p.getNickname() + " entered Zen Mode!");
                }
                else if (p.getCurrentHp() > p.getHp() / 2 && p.hasVolatile("zen_mode")) {
                    p.removeVolatile("zen_mode");
                    log.push_back(p.getNickname() + " reverted to its Standard Mode!");
                }
            }

            if (abil == "Poison Heal" && (p.getStatus() == "psn" || p.getStatus() == "tox")) {
                p.setCurrentHp(std::min(p.getHp(), p.getCurrentHp() + std::max(1, p.getHp() / 8)));
                log.push_back(p.getNickname() + "'s Poison Heal restored its HP!");
            }
            else if (abil != "Magic Guard") {
                if (p.getStatus() == "brn" || p.getStatus() == "psn" || p.getStatus() == "tox") {
                    p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, p.getHp() / 8)));
                    log.push_back(p.getNickname() + " is hurt by its status!");
                }
                if (p.hasVolatile("curse")) {
                    p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, p.getHp() / 4)));
                    log.push_back(p.getNickname() + " is afflicted by the curse!");
                }

                if (p.hasVolatile("leechseed")) {
                    int drain = std::max(1, p.getHp() / 8);
                    p.setCurrentHp(std::max(0, p.getCurrentHp() - drain));
                    opp.setCurrentHp(std::min(opp.getHp(), opp.getCurrentHp() + drain));
                    log.push_back(p.getNickname() + "'s health is sapped by Leech Seed!");
                }

                if (abil == "Dry Skin") {
                    if (weather == "Sun" || weather == "DesolateLand") {
                        p.setCurrentHp(std::max(0, p.getCurrentHp() - std::max(1, p.getHp() / 8)));
                        log.push_back(p.getNickname() + " was hurt by the sunlight!");
                    }
                    else if (weather == "Rain" || weather == "PrimordialSea") {
                        p.setCurrentHp(std::min(p.getHp(), p.getCurrentHp() + std::max(1, p.getHp() / 8)));
                        log.push_back(p.getNickname() + " had its HP restored by the rain!");
                    }
                }
            }

            if (field.terrain == "Grassy Terrain" && p.getType1() != "Flying" && p.getType2() != "Flying" && abil != "Levitate" && item != "AIRBALLOON") {
                if (p.getCurrentHp() < p.getHp()) {
                    p.setCurrentHp(std::min(p.getHp(), p.getCurrentHp() + std::max(1, p.getHp() / 16)));
                    log.push_back(p.getNickname() + " restored a little HP from the Grassy Terrain!");
                }
            }

            p.removeVolatile("protect");
            p.removeVolatile("flinch");
            p.removeVolatile("moved_this_turn");
            p.decrementVolatiles();

            if (p.getLockedTurns() > 0) {
                p.decrementLockedTurns();
                if (p.getLockedTurns() == 0) {
                    if (p.getLockedMove() == "Rollout" || p.getLockedMove() == "Ice Ball") {
                        p.removeVolatile("rollout_count");
                    }
                    else {
                        log.push_back(p.getNickname() + " calmed down.");
                        p.addVolatile("confusion", (rand() % 4) + 2);
                        log.push_back(p.getNickname() + " became confused due to fatigue!");
                    }
                }
            }
        }
        };

    postTurn(p1, p2);
    postTurn(p2, p1);
}