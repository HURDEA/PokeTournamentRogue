#pragma once
#include "IRepository.h"
#include <map>
#include <vector>
#include <string>

class CSVRepository : public IRepository {
public:
    CSVRepository(const std::string& filepath);

    void add(const Pokemon& pokemon) override;
    void remove(int id) override;
    void update(const Pokemon& newPokemon) override;
    std::vector<Pokemon> getAll() const override;
    Pokemon getById(int id) const override;

    int getPokedexSize() const override;
    Pokemon getSpeciesData(int index) const override;
    Pokemon getSpeciesDataByName(const std::string& name) const override;
    Pokemon getRandomEncounter(const std::vector<std::string>& allowedTypes) const override;

    std::vector<std::string> getLearnset(const std::string& speciesName) const override;
    std::string getItemDescription(const std::string& itemName) const override;
    MoveData getMoveData(const std::string& moveId) const override;

    int getMoney() const override;
    void setMoney(int amount) override;
    int getItemCount(const std::string& itemName) const override;
    void setItemCount(const std::string& itemName, int count) override;
    std::map<std::string, int> getInventory() const override;

    void moveWithinBox(int id, int newIndex) override;

    std::string getMoveName(const std::string& moveId) const;
    std::string getMoveCategory(const std::string& moveId) const;
    std::string getMoveType(const std::string& moveId) const;
    int getMovePower(const std::string& moveId) const;

    // --- EVOLUTION ENGINE ---
    std::vector<std::string> getPossibleEvolutions(const std::string& species) const override;
    EvoRule getEvoRule(const std::string& species) const override;

private:
    void loadDatabase(const std::string& filepath);
    void loadShowdownData();
    void loadItemData();
    void loadSaveFile();
    void loadTrainerSave();
    void saveTrainerSave() const;
    void saveToFile() const;

    std::vector<Pokemon> pokedexDB;
    std::vector<Pokemon> caughtParty;

    std::map<std::string, MoveData> moveDB;
    std::map<std::string, std::string> moveNamesDB;
    std::map<std::string, std::string> moveCategoryDB;
    std::map<std::string, std::string> moveTypeDB;
    std::map<std::string, int> movePowerDB;
    std::map<std::string, std::vector<std::string>> learnsetDB;
    std::map<std::string, std::string> itemDescriptionsDB;

    // Maps a base species (e.g. "bulbasaur") to its direct evolutions (e.g. ["Ivysaur"])
    std::map<std::string, std::vector<std::string>> evosDB;
    // Maps an evolved species (e.g. "ivysaur") to the rule needed to reach it
    std::map<std::string, EvoRule> evoRequirementsDB;

    int money = 0;
    std::map<std::string, int> inventory;
};