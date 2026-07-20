#pragma once
#include "Command.h"
#include "IRepository.h"
#include "Pokemon.h"

class ChangeMoveCommand : public Command {
private:
    IRepository& repo;
    Pokemon oldState;
    Pokemon newState;
public:
    ChangeMoveCommand(IRepository& r, const Pokemon& before, const Pokemon& after)
        : repo(r), oldState(before), newState(after) {}

    void execute() override { repo.update(newState); }
    void undo() override { repo.update(oldState); }
};