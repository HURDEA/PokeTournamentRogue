#pragma once
#include "Command.h"
#include "IRepository.h"
#include "Pokemon.h"

class ReleaseCommand : public Command {
private:
    IRepository& repo;
    Pokemon p;
public:
    ReleaseCommand(IRepository& r, const Pokemon& poke) : repo(r), p(poke) {}

    void execute() override {
        repo.remove(p.getId());
    }

    void undo() override {
        repo.add(p);
    }
};