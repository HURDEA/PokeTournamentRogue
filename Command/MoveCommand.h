#pragma once
#include "Command.h"
#include "IRepository.h"
#include "Pokemon.h"

class MoveCommand : public Command {
private:
    IRepository& repo;
    int pokemonId;
    int oldBox;
    int newBox;
public:
    MoveCommand(IRepository& r, int id, int oldB, int newB)
        : repo(r), pokemonId(id), oldBox(oldB), newBox(newB) {}

    void execute() override {
        Pokemon p = repo.getById(pokemonId);
        p.setBoxNumber(newBox);
        repo.update(p);
    }

    void undo() override {
        Pokemon p = repo.getById(pokemonId);
        p.setBoxNumber(oldBox);
        repo.update(p);
    }
};