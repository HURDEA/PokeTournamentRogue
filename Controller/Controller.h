#pragma once
#include "../Repository/IRepository.h"
#include "../Command/Command.h"
#include "../Filter/IPokemonFilter.h"
#include <memory>
#include <deque>
#include <vector>
#include <map>
#include <string>

class Controller {
private:
    std::unique_ptr<IRepository> repository;

    std::deque<std::unique_ptr<Command>> undoStack;
    std::deque<std::unique_ptr<Command>> redoStack;
    const size_t MAX_HISTORY = 10;

    void recordCommand(std::unique_ptr<Command> cmd);

public:
    explicit Controller(std::unique_ptr<IRepository> repo);

    void catchPokemon(const Pokemon& p);
    void releasePokemon(int id);
    void updatePokemonItem(int id, const std::string& newItem);
    bool movePokemonBox(int id, int newBox);
    void reorderPartyMember(int id, int newIndex);

    // Extensively modified evolution system
    std::string checkEvolution(int partyId, const std::string& usedItem = "") const;
    void evolvePokemon(int partyId, int newSpeciesId);

    void updatePokemonMoves(int partyId, const std::vector<std::string>& newMoves);
    void updatePokemon(const Pokemon& updatedPokemon);

    void executeCommand(std::unique_ptr<Command> cmd);
    void undo();
    void redo();

    std::vector<Pokemon> filterPokemon(const IPokemonFilter& filter) const;
    std::vector<Pokemon> getAllPokemon() const;

    int getFirstAvailableBox() const;
    int getPokedexSize() const;
    Pokemon getSpeciesData(int index) const;
    Pokemon getSpeciesDataByName(const std::string& name) const;
    Pokemon getSpeciesDataById(int id) const;
    Pokemon getPokemonById(int partyId) const;
    Pokemon getRandomEncounter(const std::vector<std::string>& allowedTypes) const;

    std::vector<std::string> getLearnset(const std::string& speciesName) const;
    MoveData getMoveData(const std::string& moveId) const;
    std::string getItemDescription(const std::string& itemName) const;
    std::string getMoveName(const std::string& moveId) const;
    std::string getMoveCategory(const std::string& moveId) const;
    std::string getMoveType(const std::string& moveId) const;
    int getMovePower(const std::string& moveId) const;

    int getMoney() const;
    void addMoney(int amount);
    bool spendMoney(int amount);
    int getItemCount(const std::string& itemName) const;
    void addItem(const std::string& itemName, int count);
    bool consumeItem(const std::string& itemName, int count);
    std::map<std::string, int> getInventory() const;
};