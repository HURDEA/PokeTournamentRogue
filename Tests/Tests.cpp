#include "Tests.h"
#include "../Domain/Pokemon.h"
#include "../Controller/Controller.h"
#include "../Filter/IPokemonFilter.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <map>

class TestMemoryRepo : public IRepository {
private:
    std::vector<Pokemon> data;
public:
    // Add inside TestMemoryRepo class in Tests.cpp
    std::string getMoveCategory(const std::string& moveId) const override { return "Physical"; }
    std::string getMoveType(const std::string& moveId) const override { return "Normal"; }
    int getMovePower(const std::string& moveId) const override { return 80; } // NEW STUB
    void add(const Pokemon& p) override { data.push_back(p); }
    void remove(int id) override {
        data.erase(std::remove_if(data.begin(), data.end(), [id](const Pokemon& p) { return p.getId() == id; }), data.end());
    }
    void update(const Pokemon& newP) override {
        for (auto& p : data) { if (p.getId() == newP.getId()) { p = newP; return; } }
    }
    Pokemon getById(int id) const override {
        for (const auto& p : data) { if (p.getId() == id) return p; }
        throw std::runtime_error("Not found");
    }
    std::vector<Pokemon> getAll() const override { return data; }

    int getPokedexSize() const override { return 0; }
    Pokemon getSpeciesData(int index) const override { return Pokemon(); }

    // NEW: Dummy method to satisfy the new Mega Evolution lookup in IRepository
    Pokemon getSpeciesDataByName(const std::string& name) const override { return Pokemon(); }

    Pokemon getRandomEncounter(const std::vector<std::string>& allowedTypes) const override { return Pokemon(); }

    std::vector<std::string> getLearnset(const std::string& speciesName) const override { return { "tackle" }; }
    std::string getMoveName(const std::string& moveId) const override { return "Tackle"; }
    MoveData getMoveData(const std::string& moveId) const override {
        MoveData md;
        md.id = moveId;
        md.name = "Tackle";
        md.category = "Physical";
        md.type = "Normal";
        md.basePower = 40;
        return md;
    }
    std::string getItemDescription(const std::string& itemName) const override { return "Test Item"; }

    // Inventory Memory Stubs
    int getMoney() const override { return 0; }
    void setMoney(int) override {}
    int getItemCount(const std::string&) const override { return 0; }
    void setItemCount(const std::string&, int) override {}
    std::map<std::string, int> getInventory() const override { return {}; }
};

void Tests::runAllTests() {
    std::cout << "--- Starting Automated Test Suite ---" << std::endl;
    testDomainAndRepository();
    testControllerAndMemoryTimeline();
    testAdvancedFiltering();
    std::cout << "--- All Tests Passed Successfully! ---" << std::endl;
}

void Tests::testDomainAndRepository() {
    TestMemoryRepo repo;

    // Default arguments from Pokemon.h will safely fill in the missing EVs, Natures, etc.
    Pokemon p1(1, 25, "Pikachu", "Electric", "None", 35, 55, 40, 50, 50, 90, 10, "None", 1, "Sparky");
    repo.add(p1);
    assert(repo.getAll().size() == 1);
    assert(repo.getById(1).getNickname() == "Sparky");

    Pokemon pUpdated(1, 25, "Raichu", "Electric", "None", 60, 90, 55, 90, 80, 110, 20, "LightBall", 1, "Raichu");
    repo.update(pUpdated);
    assert(repo.getById(1).getNickname() == "Raichu");
    assert(repo.getById(1).getHeldItem() == "LightBall");

    repo.remove(1);
    assert(repo.getAll().empty());
}

void Tests::testControllerAndMemoryTimeline() {
    auto repo = std::make_unique<TestMemoryRepo>();
    Controller ctrl(std::move(repo));
    Pokemon p(10, 1, "Bulbasaur", "Grass", "Poison", 45, 49, 49, 65, 65, 45, 5, "None", 1, "Bulba");

    ctrl.catchPokemon(p);
    assert(ctrl.getAllPokemon().size() == 1);

    ctrl.undo();
    assert(ctrl.getAllPokemon().empty());

    ctrl.redo();
    assert(ctrl.getAllPokemon().size() == 1);

    ctrl.updatePokemonItem(10, "MiracleSeed");
    assert(ctrl.getAllPokemon()[0].getHeldItem() == "MiracleSeed");

    ctrl.undo();
    assert(ctrl.getAllPokemon()[0].getHeldItem() == "None");
}

void Tests::testAdvancedFiltering() {
    Pokemon p1(1, 79, "Slowpoke", "Water", "Psychic", 90, 65, 65, 40, 40, 15, 10, "None", 1, "Slowpoke");
    Pokemon p2(2, 4, "Charmander", "Fire", "None", 39, 52, 43, 60, 50, 65, 10, "None", 1, "Charmander");
    Pokemon p3(3, 121, "Starmie", "Water", "Psychic", 60, 75, 85, 100, 85, 115, 10, "None", 1, "Starmie");

    TypeFilter isWater("Water");
    TypeFilter isPsychic("Psychic");
    TypeFilter isFire("Fire");

    assert(isWater.satisfies(p1) == true);
    assert(isWater.satisfies(p2) == false);

    AndFilter waterAndPsychic(isWater, isPsychic);
    assert(waterAndPsychic.satisfies(p1) == true);
    assert(waterAndPsychic.satisfies(p3) == true);
    assert(waterAndPsychic.satisfies(p2) == false);

    OrFilter fireOrPsychic(isFire, isPsychic);
    assert(fireOrPsychic.satisfies(p1) == true);
    assert(fireOrPsychic.satisfies(p2) == true);
    assert(fireOrPsychic.satisfies(p3) == true);
}