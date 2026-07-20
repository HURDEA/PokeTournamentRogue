#include "Simulator.h"
#include "../BattleEngine.h"
#include "../../Controller/Controller.h"
#include <algorithm>
#include <limits>
#include <cctype>

using namespace AI;

std::vector<std::string> Simulator::generateValidActions(const Pokemon& activePkmn, const std::vector<Pokemon>& party, size_t activeIdx, bool allowMega, Controller& ctrl) {
    std::vector<std::string> actions;

    if (activePkmn.getCurrentHp() <= 0) {
        for (size_t i = 0; i < party.size(); ++i) {
            if (i != activeIdx && party[i].getCurrentHp() > 0) {
                actions.push_back("switch:" + std::to_string(i));
            }
        }
        return actions;
    }

    if (activePkmn.getLockedTurns() > 0) {
        actions.push_back("move:" + activePkmn.getLockedMove());
        return actions;
    }

    auto moves = activePkmn.getMoves();
    for (const auto& mId : moves) {
        if (!mId.empty() && mId != "None") {
            MoveData md = ctrl.getMoveData(mId);
            // STALL PREVENTION: Forbid the AI from attempting consecutive Protects/Endures.
            if (md.isProtectMove && activePkmn.getProtectCounter() > 0) {
                continue;
            }
            actions.push_back("move:" + mId);
        }
    }
    if (actions.empty()) actions.push_back("move:struggle");

    if (allowMega) {
        std::string item = activePkmn.getHeldItem();
        std::string normItem = "";
        for (char c : item) if (c != ' ' && c != '-' && c != '\'') normItem += std::toupper(c);

        if (normItem.find("ITE") != std::string::npos || normItem == "REDORB" || normItem == "BLUEORB") {
            for (const auto& mId : moves) {
                if (!mId.empty() && mId != "None") {
                    MoveData md = ctrl.getMoveData(mId);
                    if (md.isProtectMove && activePkmn.getProtectCounter() > 0) continue;
                    actions.push_back("mega:" + mId);
                }
            }
        }
    }

    for (size_t i = 0; i < party.size(); ++i) {
        if (i != activeIdx && party[i].getCurrentHp() > 0) {
            actions.push_back("switch:" + std::to_string(i));
        }
    }

    return actions;
}

double Simulator::evaluateActionBranches(
    const std::vector<Pokemon>& aiParty, size_t activeAiIdx,
    const std::vector<Pokemon>& playerParty, size_t activePlayerIdx,
    const std::string& forcedAiAction, Controller& ctrl, int depth,
    bool aiCanMega, bool playerCanMega,
    double alpha, double beta
) {
    if (depth == 0 || aiParty[activeAiIdx].getCurrentHp() <= 0 || playerParty[activePlayerIdx].getCurrentHp() <= 0) {
        return evaluateBoardState(aiParty, activeAiIdx, playerParty, activePlayerIdx, ctrl);
    }

    std::vector<std::string> playerActions = generateValidActions(playerParty[activePlayerIdx], playerParty, activePlayerIdx, playerCanMega, ctrl);
    if (playerActions.empty()) playerActions.push_back("move:struggle");

    double worstCaseScore = std::numeric_limits<double>::infinity();

    for (const std::string& pAct : playerActions) {
        std::vector<Pokemon> simAiParty = aiParty;
        std::vector<Pokemon> simPlayerParty = playerParty;
        size_t simAiIdx = activeAiIdx;
        size_t simPlayerIdx = activePlayerIdx;

        BattleEngine sandbox(ctrl);
        std::vector<std::string> dummyLog;

        int aiFaints = 0; for (const auto& p : simAiParty) if (p.getCurrentHp() <= 0) aiFaints++;
        int pFaints = 0; for (const auto& p : simPlayerParty) if (p.getCurrentHp() <= 0) pFaints++;
        sandbox.setFaintedCount(false, aiFaints);
        sandbox.setFaintedCount(true, pFaints);

        bool aiSwitched = false;
        bool playerSwitched = false;

        if (forcedAiAction.rfind("switch:", 0) == 0) {
            simAiIdx = std::stoi(forcedAiAction.substr(7));
            sandbox.onSwitchIn(simAiParty[simAiIdx], false, dummyLog, &simPlayerParty[simPlayerIdx]);
            aiSwitched = true;
        }
        if (pAct.rfind("switch:", 0) == 0) {
            simPlayerIdx = std::stoi(pAct.substr(7));
            sandbox.onSwitchIn(simPlayerParty[simPlayerIdx], true, dummyLog, &simAiParty[simAiIdx]);
            playerSwitched = true;
        }

        auto handleMegaForm = [&](Pokemon& pkmn, const std::string& action, bool isPlayer) {
            if (action.rfind("mega:", 0) == 0) {
                std::string megaName = pkmn.getName() + "-Mega";
                std::string rawItem = pkmn.getHeldItem();
                std::string item = "";
                for (char c : rawItem) { if (c != ' ' && c != '-' && c != '\'') item += std::toupper(c); }

                if (item == "CHARIZARDITEX") megaName = "Charizard-Mega-X";
                else if (item == "CHARIZARDITEY") megaName = "Charizard-Mega-Y";
                else if (item == "MEWTWONITEX") megaName = "Mewtwo-Mega-X";
                else if (item == "MEWTWONITEY") megaName = "Mewtwo-Mega-Y";
                else if (item == "REDORB") megaName = "Groudon-Primal";
                else if (item == "BLUEORB") megaName = "Kyogre-Primal";
                else if (item == "ABSOLITEZ") megaName = "Absol-Mega-Z";
                else if (item == "GARCHOMPITEZ") megaName = "Garchomp-Mega-Z";
                else if (item == "LUCARIONITEZ") megaName = "Lucario-Mega-Z";

                Pokemon megaData = ctrl.getSpeciesDataByName(megaName);
                if (!megaData.getName().empty()) {
                    pkmn.megaEvolve(megaData);
                    sandbox.onSwitchIn(pkmn, isPlayer, dummyLog, isPlayer ? &simAiParty[simAiIdx] : &simPlayerParty[simPlayerIdx]);
                }
            }
            };

        bool nextAiMegaAvailable = aiCanMega;
        bool nextPlayerMegaAvailable = playerCanMega;

        if (!aiSwitched) {
            handleMegaForm(simAiParty[simAiIdx], forcedAiAction, false);
            if (forcedAiAction.rfind("mega:", 0) == 0) nextAiMegaAvailable = false;
        }
        if (!playerSwitched) {
            handleMegaForm(simPlayerParty[simPlayerIdx], pAct, true);
            if (pAct.rfind("mega:", 0) == 0) nextPlayerMegaAvailable = false;
        }

        if (!aiSwitched || !playerSwitched) {
            std::string aiMoveId = (forcedAiAction.rfind("move:", 0) == 0) ? forcedAiAction.substr(5) : ((forcedAiAction.rfind("mega:", 0) == 0) ? forcedAiAction.substr(5) : "");
            std::string pMoveId = (pAct.rfind("move:", 0) == 0) ? pAct.substr(5) : ((pAct.rfind("mega:", 0) == 0) ? pAct.substr(5) : "");

            int aiPriority = 0;
            if (!aiMoveId.empty()) {
                MoveData md = ctrl.getMoveData(aiMoveId);
                aiPriority = md.priority;
                if (simAiParty[simAiIdx].getAbility() == "Prankster" && md.category == "Status") aiPriority++;
                if (simAiParty[simAiIdx].getAbility() == "Gale Wings" && md.type == "Flying" && simAiParty[simAiIdx].getCurrentHp() == simAiParty[simAiIdx].getHp()) aiPriority++;
            }

            int pPriority = 0;
            if (!pMoveId.empty()) {
                MoveData md = ctrl.getMoveData(pMoveId);
                pPriority = md.priority;
                if (simPlayerParty[simPlayerIdx].getAbility() == "Prankster" && md.category == "Status") pPriority++;
                if (simPlayerParty[simPlayerIdx].getAbility() == "Gale Wings" && md.type == "Flying" && simPlayerParty[simPlayerIdx].getCurrentHp() == simPlayerParty[simPlayerIdx].getHp()) pPriority++;
            }

            bool aiGoesFirst = false;
            if (aiSwitched && !playerSwitched) aiGoesFirst = true;
            else if (playerSwitched && !aiSwitched) aiGoesFirst = false;
            else if (aiPriority > pPriority) aiGoesFirst = true;
            else if (pPriority > aiPriority) aiGoesFirst = false;
            else {
                int aiSpeed = sandbox.getEffectiveSpeed(simAiParty[simAiIdx], simPlayerParty[simPlayerIdx], false);
                int pSpeed = sandbox.getEffectiveSpeed(simPlayerParty[simPlayerIdx], simAiParty[simAiIdx], true);
                aiGoesFirst = sandbox.getFieldState().trickRoom > 0 ? (aiSpeed < pSpeed) : (aiSpeed >= pSpeed);
            }

            if (aiGoesFirst) {
                if (!aiSwitched) applyActionPhase(simAiParty[simAiIdx], simPlayerParty[simPlayerIdx], forcedAiAction, pAct, false, sandbox, dummyLog);
                if (!playerSwitched && simPlayerParty[simPlayerIdx].getCurrentHp() > 0) {
                    applyActionPhase(simPlayerParty[simPlayerIdx], simAiParty[simAiIdx], pAct, forcedAiAction, true, sandbox, dummyLog);
                }
            }
            else {
                if (!playerSwitched) applyActionPhase(simPlayerParty[simPlayerIdx], simAiParty[simAiIdx], pAct, forcedAiAction, true, sandbox, dummyLog);
                if (!aiSwitched && simAiParty[simAiIdx].getCurrentHp() > 0) {
                    applyActionPhase(simAiParty[simAiIdx], simPlayerParty[simPlayerIdx], forcedAiAction, pAct, false, sandbox, dummyLog);
                }
            }
        }

        sandbox.runEndOfTurn(simAiParty[simAiIdx], simPlayerParty[simPlayerIdx], dummyLog);

        double score = runSimTurn(simAiParty, simAiIdx, simPlayerParty, simPlayerIdx, ctrl, depth - 1, "", nextAiMegaAvailable, nextPlayerMegaAvailable, alpha, beta);

        if (aiSwitched) score -= 5.0;

        if (score < worstCaseScore) {
            worstCaseScore = score;
        }

        if (worstCaseScore < beta) {
            beta = worstCaseScore;
        }
        if (beta <= alpha) {
            break;
        }
    }

    return worstCaseScore;
}

double Simulator::runSimTurn(const std::vector<Pokemon>& aiParty, size_t activeAiIdx, const std::vector<Pokemon>& playerParty, size_t activePlayerIdx, Controller& ctrl, int depth, const std::string& forcedAiAct, bool aiCanMega, bool playerCanMega, double alpha, double beta) {
    if (depth == 0 || aiParty[activeAiIdx].getCurrentHp() <= 0 || playerParty[activePlayerIdx].getCurrentHp() <= 0) {
        return evaluateBoardState(aiParty, activeAiIdx, playerParty, activePlayerIdx, ctrl);
    }

    std::vector<std::string> aiActions;
    if (!forcedAiAct.empty()) aiActions.push_back(forcedAiAct);
    else aiActions = generateValidActions(aiParty[activeAiIdx], aiParty, activeAiIdx, aiCanMega, ctrl);

    double bestMaximin = -std::numeric_limits<double>::infinity();
    for (const auto& aiAct : aiActions) {
        double score = evaluateActionBranches(aiParty, activeAiIdx, playerParty, activePlayerIdx, aiAct, ctrl, depth, aiCanMega, playerCanMega, alpha, beta);

        if (score > bestMaximin) {
            bestMaximin = score;
        }

        if (bestMaximin > alpha) {
            alpha = bestMaximin;
        }
        if (beta <= alpha) {
            break;
        }
    }
    return bestMaximin;
}

void Simulator::applyActionPhase(Pokemon& activeAttacker, Pokemon& activeDefender, const std::string& action, const std::string& defenderAction, bool isPlayerSide, BattleEngine& sandbox, std::vector<std::string>& log) {
    if (action.rfind("switch:", 0) == 0) return;

    std::string moveId = action.substr(action.find(':') + 1);

    std::string defMoveId = "";
    if (defenderAction.rfind("move:", 0) == 0) defMoveId = defenderAction.substr(5);
    else if (defenderAction.rfind("mega:", 0) == 0) defMoveId = defenderAction.substr(5);
    else defMoveId = defenderAction;

    sandbox.executeMove(activeAttacker, activeDefender, moveId, isPlayerSide, log, defMoveId);
}

// AI REWORK: Exact Damage Projections instead of Type-Chart Guessing
double Simulator::evaluateBoardState(const std::vector<Pokemon>& aiParty, size_t aiIdx, const std::vector<Pokemon>& playerParty, size_t playerIdx, Controller& ctrl) {
    double aiScore = 0.0;
    double playerScore = 0.0;
    int aiAlive = 0;
    int pAlive = 0;

    for (size_t i = 0; i < aiParty.size(); ++i) {
        const auto& p = aiParty[i];
        if (p.getCurrentHp() > 0) {
            aiAlive++;
            double hpPct = (static_cast<double>(p.getCurrentHp()) / p.getHp()) * 100.0;
            double value = hpPct;

            if (i == aiIdx) {
                value += p.getStageAtk() * 10.0;
                value += p.getStageSpA() * 10.0;
                value += p.getStageSpe() * 10.0;
                value += p.getStageDef() * 5.0;
                value += p.getStageSpD() * 5.0;
            }

            if (p.getStatus() == "slp" || p.getStatus() == "frz") value -= 25.0;
            else if (!p.getStatus().empty()) value -= 10.0;

            aiScore += std::max(1.0, value);
        }
    }

    for (size_t i = 0; i < playerParty.size(); ++i) {
        const auto& p = playerParty[i];
        if (p.getCurrentHp() > 0) {
            pAlive++;
            double hpPct = (static_cast<double>(p.getCurrentHp()) / p.getHp()) * 100.0;
            double value = hpPct;

            if (i == playerIdx) {
                value += p.getStageAtk() * 10.0;
                value += p.getStageSpA() * 10.0;
                value += p.getStageSpe() * 10.0;
                value += p.getStageDef() * 5.0;
                value += p.getStageSpD() * 5.0;
            }

            if (p.getStatus() == "slp" || p.getStatus() == "frz") value -= 25.0;
            else if (!p.getStatus().empty()) value -= 10.0;

            playerScore += std::max(1.0, value);
        }
    }

    if (aiAlive == 0) return -99999.0;
    if (pAlive == 0) return 99999.0;

    double matchupScore = 0.0;
    const Pokemon& aiActive = aiParty[aiIdx];
    const Pokemon& pActive = playerParty[playerIdx];

    if (aiActive.getCurrentHp() > 0 && pActive.getCurrentHp() > 0) {
        BattleEngine dummy(ctrl);

        int aiFaints = 0; for (const auto& p : aiParty) if (p.getCurrentHp() <= 0) aiFaints++;
        int pFaints = 0; for (const auto& p : playerParty) if (p.getCurrentHp() <= 0) pFaints++;
        dummy.setFaintedCount(false, aiFaints);
        dummy.setFaintedCount(true, pFaints);

        int aiSpeed = dummy.getEffectiveSpeed(aiActive, pActive, false);
        int pSpeed = dummy.getEffectiveSpeed(pActive, aiActive, true);

        if (dummy.getFieldState().trickRoom > 0) {
            if (aiSpeed < pSpeed) matchupScore += 15.0;
            else if (pSpeed < aiSpeed) matchupScore -= 15.0;
        }
        else {
            if (aiSpeed > pSpeed) matchupScore += 15.0;
            else if (pSpeed > aiSpeed) matchupScore -= 15.0;
        }

        double maxAiDamagePct = 0.0;
        for (const auto& mId : aiActive.getMoves()) {
            if (mId == "None" || mId.empty()) continue;
            MoveData md = ctrl.getMoveData(mId);
            if (md.basePower == 0 && md.ohko == false) continue;

            Pokemon aiCopy = aiActive;
            Pokemon pCopy = pActive;
            std::vector<std::string> dummyLog;

            int dmg = dummy.calculateDamageInternal(aiCopy, pCopy, md, false, dummyLog, true, false);
            double pct = static_cast<double>(dmg) / std::max(1, pActive.getHp());
            if (pct > maxAiDamagePct) maxAiDamagePct = pct;
        }

        double maxPlayerDamagePct = 0.0;
        for (const auto& mId : pActive.getMoves()) {
            if (mId == "None" || mId.empty()) continue;
            MoveData md = ctrl.getMoveData(mId);
            if (md.basePower == 0 && md.ohko == false) continue;

            Pokemon aiCopy = aiActive;
            Pokemon pCopy = pActive;
            std::vector<std::string> dummyLog;

            int dmg = dummy.calculateDamageInternal(pCopy, aiCopy, md, false, dummyLog, true, true);
            double pct = static_cast<double>(dmg) / std::max(1, aiActive.getHp());
            if (pct > maxPlayerDamagePct) maxPlayerDamagePct = pct;
        }

        maxAiDamagePct = std::min(1.5, maxAiDamagePct);
        maxPlayerDamagePct = std::min(1.5, maxPlayerDamagePct);

        matchupScore += (maxAiDamagePct - maxPlayerDamagePct) * 50.0;
    }
    else if (aiActive.getCurrentHp() <= 0) {
        matchupScore -= 50.0;
    }

    return (aiScore - playerScore) * 2.0 + matchupScore;
}