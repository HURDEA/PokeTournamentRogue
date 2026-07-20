#pragma once
#include "../Domain/Pokemon.h"
#include <vector>
#include <string>
#include <map>
#include <stdexcept>

class RepositoryException : public std::runtime_error {
public:
    RepositoryException(const std::string& message) : std::runtime_error(message) {}
};

struct EvoRule {
    std::string prevo;
    int evoLevel = 0;
    std::string evoType = "";
    std::string evoItem = "";
    std::string evoCondition = "";
};

class IRepository {
public:
    virtual ~IRepository() = default;

    virtual void add(const Pokemon& pokemon) = 0;
    virtual void remove(int id) = 0;
    virtual void update(const Pokemon& newPokemon) = 0;
    virtual std::vector<Pokemon> getAll() const = 0;
    virtual Pokemon getById(int id) const = 0;

    virtual int getPokedexSize() const = 0;
    virtual Pokemon getSpeciesData(int index) const = 0;
    virtual Pokemon getSpeciesDataByName(const std::string& name) const = 0;
    virtual Pokemon getRandomEncounter(const std::vector<std::string>& allowedTypes) const = 0;

    virtual std::vector<std::string> getLearnset(const std::string& speciesName) const = 0;
    virtual std::string getItemDescription(const std::string& itemName) const = 0;

    virtual MoveData getMoveData(const std::string& moveId) const = 0;

    virtual std::string getMoveName(const std::string& moveId) const = 0;
    virtual std::string getMoveCategory(const std::string& moveId) const = 0;
    virtual std::string getMoveType(const std::string& moveId) const = 0;
    virtual int getMovePower(const std::string& moveId) const = 0;

    virtual int getMoney() const = 0;
    virtual void setMoney(int amount) = 0;
    virtual int getItemCount(const std::string& itemName) const = 0;
    virtual void setItemCount(const std::string& itemName, int count) = 0;
    virtual std::map<std::string, int> getInventory() const = 0;

    virtual void moveWithinBox(int id, int newIndex) = 0;

    // --- EVOLUTION ENGINE ---
    virtual std::vector<std::string> getPossibleEvolutions(const std::string& species) const = 0;
    virtual EvoRule getEvoRule(const std::string& species) const = 0;
};