#pragma once
#include "Command.h"
#include "IRepository.h"
#include "Pokemon.h"
#include <string>

class ItemCommand : public Command {
private:
    IRepository& repo;
    int pokemonId;
    std::string oldItem;
    std::string newItem;
public:
    ItemCommand(IRepository& r, int id, const std::string& oldI, const std::string& newI)
        : repo(r), pokemonId(id), oldItem(oldI), newItem(newI) {}

    void execute() override {
        Pokemon p = repo.getById(pokemonId);
        p.setHeldItem(newItem);
        repo.update(p);
    }

    void undo() override {
        Pokemon p = repo.getById(pokemonId);
        p.setHeldItem(oldItem);
        repo.update(p);
    }
};