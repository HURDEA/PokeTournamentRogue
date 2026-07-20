#pragma once
#include "Command.h"
#include "IRepository.h"
#include "Pokemon.h"

class CatchCommand : public Command {
private:
    IRepository& repo;
    Pokemon p;
public:
    CatchCommand(IRepository& r, const Pokemon& poke) : repo(r), p(poke) {}

    void execute() override {
        repo.add(p);
    }

    void undo() override {
        repo.remove(p.getId());
    }
};