#include "Controller.h"
#include "../Command/ItemCommand.h"
#include "../Command/MoveCommand.h"
#include "../Command/CatchCommand.h"
#include "../Command/ReleaseCommand.h"
#include "../Command/EvolveCommand.h"
#include "../Command/ChangeMoveCommand.h"
#include "../Command/UpdateCommand.h"

Controller::Controller(std::unique_ptr<IRepository> repo) : repository(std::move(repo)) {}

void Controller::recordCommand(std::unique_ptr<Command> cmd) {
    undoStack.push_back(std::move(cmd));
    redoStack.clear();
    if (undoStack.size() > MAX_HISTORY) undoStack.pop_front();
}
void Controller::executeCommand(std::unique_ptr<Command> cmd) {
    cmd->execute();
    recordCommand(std::move(cmd));
}
void Controller::undo() {
    if (!undoStack.empty()) {
        auto cmd = std::move(undoStack.back()); undoStack.pop_back();
        cmd->undo(); redoStack.push_back(std::move(cmd));
    }
}
void Controller::redo() {
    if (!redoStack.empty()) {
        auto cmd = std::move(redoStack.back()); redoStack.pop_back();
        cmd->execute(); undoStack.push_back(std::move(cmd));
    }
}

void Controller::catchPokemon(const Pokemon& p) {
    // Create a copy so we can modify the box number
    Pokemon newlyCaught = p;

    // Automatically find the next available box (30 limit)
    newlyCaught.setBoxNumber(getFirstAvailableBox());

    auto cmd = std::make_unique<CatchCommand>(*repository, newlyCaught);
    executeCommand(std::move(cmd));
}

void Controller::releasePokemon(int id) {
    Pokemon p = repository->getById(id);
    auto cmd = std::make_unique<ReleaseCommand>(*repository, p);
    executeCommand(std::move(cmd));
}
void Controller::updatePokemonItem(int id, const std::string& newItem) {
    Pokemon p = repository->getById(id);
    auto cmd = std::make_unique<ItemCommand>(*repository, id, p.getHeldItem(), newItem);
    executeCommand(std::move(cmd));
}
bool Controller::movePokemonBox(int id, int newBox) {
    Pokemon p = repository->getById(id);
    if (p.getBoxNumber() == newBox) return true; // no-op, considered successful

    // Prevent moving into a full box. Box 0 (active party) can hold at most 6 Pokemon.
    if (newBox == 0) {
        auto all = repository->getAll();
        int countInBox0 = 0;
        for (const auto& pp : all) if (pp.getBoxNumber() == 0) ++countInBox0;
        if (countInBox0 >= 6) return false; // do not perform the move
    }

    auto cmd = std::make_unique<MoveCommand>(*repository, id, p.getBoxNumber(), newBox);
    executeCommand(std::move(cmd));
    return true;
}
void Controller::reorderPartyMember(int id, int newIndex) {
    // Direct repository call - not undoable for now
    repository->moveWithinBox(id, newIndex);
}

void Controller::updatePokemonMoves(int partyId, const std::vector<std::string>& newMoves) {
    Pokemon oldP = repository->getById(partyId);
    Pokemon newP = oldP;
    newP.setMoves(newMoves);

    // NEW: Retain old PP if keeping the move, otherwise initialize Max PP
    for (const auto& m : newMoves) {
        if (oldP.getMaxMovePP(m) > 0) {
            newP.setMaxMovePP(m, oldP.getMaxMovePP(m));
            newP.setMovePP(m, oldP.getMovePP(m));
        }
        else {
            MoveData md = repository->getMoveData(m);
            newP.setMaxMovePP(m, md.pp);
            newP.setMovePP(m, md.pp);
        }
    }

    auto cmd = std::make_unique<ChangeMoveCommand>(*repository, oldP, newP);
    executeCommand(std::move(cmd));
}

void Controller::updatePokemon(const Pokemon& updatedPokemon) {
    Pokemon oldP = repository->getById(updatedPokemon.getId());
    auto cmd = std::make_unique<UpdateCommand>(*repository, oldP, updatedPokemon);
    executeCommand(std::move(cmd));
}

std::vector<Pokemon> Controller::getAllPokemon() const { return repository->getAll(); }
std::vector<Pokemon> Controller::filterPokemon(const IPokemonFilter& filter) const {
    std::vector<Pokemon> all = repository->getAll();
    std::vector<Pokemon> result;
    for (const auto& p : all) { if (filter.satisfies(p)) result.push_back(p); }
    return result;
}

int Controller::getPokedexSize() const { return repository->getPokedexSize(); }
Pokemon Controller::getSpeciesData(int index) const { return repository->getSpeciesData(index); }
Pokemon Controller::getSpeciesDataByName(const std::string& name) const { return repository->getSpeciesDataByName(name); }
Pokemon Controller::getSpeciesDataById(int id) const {
    int size = repository->getPokedexSize();
    for (int i = 0; i < size; ++i) {
        Pokemon p = repository->getSpeciesData(i);
        if (p.getSpeciesId() == id) return p;
    }
    return Pokemon();
}
Pokemon Controller::getPokemonById(int partyId) const { return repository->getById(partyId); }
Pokemon Controller::getRandomEncounter(const std::vector<std::string>& allowedTypes) const { return repository->getRandomEncounter(allowedTypes); }

std::vector<std::string> Controller::getLearnset(const std::string& speciesName) const { return repository->getLearnset(speciesName); }
std::string Controller::getItemDescription(const std::string& itemName) const { return repository->getItemDescription(itemName); }

MoveData Controller::getMoveData(const std::string& moveId) const {
    return repository->getMoveData(moveId);
}

int Controller::getMoney() const { return repository->getMoney(); }
void Controller::addMoney(int amount) { repository->setMoney(repository->getMoney() + amount); }
bool Controller::spendMoney(int amount) {
    if (repository->getMoney() >= amount) {
        repository->setMoney(repository->getMoney() - amount);
        return true;
    }
    return false;
}
int Controller::getItemCount(const std::string& itemName) const { return repository->getItemCount(itemName); }
void Controller::addItem(const std::string& itemName, int count) { repository->setItemCount(itemName, repository->getItemCount(itemName) + count); }
bool Controller::consumeItem(const std::string& itemName, int count) {
    int current = repository->getItemCount(itemName);
    if (current >= count) {
        repository->setItemCount(itemName, current - count);
        return true;
    }
    return false;
}
std::map<std::string, int> Controller::getInventory() const { return repository->getInventory(); }

std::string Controller::getMoveName(const std::string& moveId) const {
    return repository->getMoveData(moveId).name;
}

std::string Controller::getMoveCategory(const std::string& moveId) const {
    return repository->getMoveData(moveId).category;
}

std::string Controller::getMoveType(const std::string& moveId) const {
    return repository->getMoveData(moveId).type;
}

int Controller::getMovePower(const std::string& moveId) const {
    return repository->getMoveData(moveId).basePower;
}

int Controller::getFirstAvailableBox() const {
    std::vector<Pokemon> all = repository->getAll();
    std::map<int, int> boxCounts;

    // Tally up how many Pokemon are in each box
    for (const auto& p : all) {
        boxCounts[p.getBoxNumber()]++;
    }

    // Box 0 is the active party (max 6)
    if (boxCounts[0] < 6) {
        return 0;
    }

    // Boxes 1 and up are PC boxes (max 30)
    int box = 1;
    while (boxCounts[box] >= 30) {
        box++;
    }

    return box;
}


std::string Controller::checkEvolution(int partyId, const std::string& usedItem) const {
    Pokemon p = repository->getById(partyId);
    auto evos = repository->getPossibleEvolutions(p.getName());

    std::string cleanUsedItem = "";
    for (char c : usedItem) { if (std::isalnum(c)) cleanUsedItem += std::toupper(c); }

    for (const auto& target : evos) {
        EvoRule rule = repository->getEvoRule(target);

        if (cleanUsedItem == "UNIVERSALSTONE") {
            if (rule.evoType == "levelFriendship" || rule.evoType == "levelMove" || rule.evoType == "levelExtra" || rule.evoType == "levelHold" || !rule.evoCondition.empty()) {
                return target;
            }
        }
        if (rule.evoType == "useItem") {
            if (!usedItem.empty()) {
                std::string cleanRuleItem = "";
                for (char c : rule.evoItem) { if (std::isalnum(c)) cleanRuleItem += std::toupper(c); }
                if (cleanRuleItem == cleanUsedItem) return target;
            }
        }
        else if (rule.evoType == "trade") {
            if (cleanUsedItem == "LINKCABLE" && rule.evoItem.empty()) return target;
            if (!usedItem.empty() && !rule.evoItem.empty()) {
                std::string cleanRuleItem = "";
                for (char c : rule.evoItem) { if (std::isalnum(c)) cleanRuleItem += std::toupper(c); }
                if (cleanRuleItem == cleanUsedItem) return target;
            }
        }
        else {
            // Standard Level Up: If it's a level-based evo, allow it to trigger regardless of the 
            // specific level requirement, since we are hard-capping at Level 50.
            if (usedItem.empty() && rule.evoLevel > 0 && rule.evoCondition.empty() && rule.evoType != "levelHold") {
                return target;
            }
            if (usedItem.empty() && rule.evoType == "levelFriendship" && p.getHappiness() >= 220) {
                return target;
            }
        }
    }
    return "";
}

void Controller::evolvePokemon(int partyId, int newSpeciesId) {
    Pokemon oldP = repository->getById(partyId);
    Pokemon baseStats = getSpeciesDataById(newSpeciesId);
    if (baseStats.getName() == "") return;

    std::string newNick = oldP.getNickname();
    if (oldP.getNickname() == oldP.getName()) newNick = baseStats.getName();

    Pokemon evolved(oldP.getId(), baseStats.getSpeciesId(), baseStats.getName(),
        baseStats.getType1(), baseStats.getType2(),
        baseStats.getBaseHp(), baseStats.getBaseAttack(), baseStats.getBaseDefense(),
        baseStats.getBaseSpAttack(), baseStats.getBaseSpDefense(), baseStats.getBaseSpeed(),
        oldP.getLevel(), oldP.getHeldItem(), oldP.getBoxNumber(), newNick, oldP.getMoves(),
        oldP.getNature(), baseStats.getAbility(), oldP.getEvHp(), oldP.getEvAtk(), oldP.getEvDef(),
        oldP.getEvSpA(), oldP.getEvSpD(), oldP.getEvSpe(), baseStats.getWeight(), baseStats.getHappiness());

    auto cmd = std::make_unique<EvolveCommand>(*repository, oldP, evolved);
    executeCommand(std::move(cmd));
}

void Controller::factoryReset() {
    // 1. Release every single Pokémon in the database
    std::vector<Pokemon> allPokes = getAllPokemon();
    for (const auto& p : allPokes) {
        releasePokemon(p.getId());
    }

    // 2. Drain all Money
    int currentMoney = getMoney();
    if (currentMoney > 0) {
        spendMoney(currentMoney);
    }

    // 3. Trash all Items
    std::map<std::string, int> bag = getInventory();
    for (const auto& item : bag) {
        if (item.second > 0) {
            consumeItem(item.first, item.second);
        }
    }

    // Optional: If your undo/redo stacks are accessible here, clear them 
    // so the player can't "Undo" the factory reset and get their team back!
    // undoStack = std::stack<std::unique_ptr<Command>>();
    // redoStack = std::stack<std::unique_ptr<Command>>();
}