#pragma once
#include "Command.h"
#include "IRepository.h"
#include "Pokemon.h"
#include <string>

class NicknameCommand : public Command {
private:
    IRepository& repo;
    int pokemonId;
    std::string oldName;
    std::string newName;

public:
    NicknameCommand(IRepository& r, int id, const std::string& prev, const std::string& up)
        : repo(r), pokemonId(id), oldName(prev), newName(up) {}

    void execute() override {
        Pokemon p = repo.getById(pokemonId);
        p.setNickname(newName);
        repo.update(p);
    }

    void undo() override {
        Pokemon p = repo.getById(pokemonId);
        p.setNickname(oldName);
        repo.update(p);
    }
};