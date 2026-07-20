#include "CSVRepository.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <algorithm>
#include <cstdlib>
#include <random>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QString toShowdownId(const std::string& name) {
    QString s = QString::fromStdString(name).toLower();
    s.remove(QRegularExpression("[^a-z0-9]"));
    return s;
}

void CSVRepository::moveWithinBox(int id, int newIndex) {
    int targetPos = -1;
    for (size_t i = 0; i < caughtParty.size(); ++i) {
        if (caughtParty[i].getId() == id) { targetPos = (int)i; break; }
    }
    if (targetPos == -1) return;

    int box = caughtParty[targetPos].getBoxNumber();

    std::vector<int> boxIndices;
    for (size_t i = 0; i < caughtParty.size(); ++i) if (caughtParty[i].getBoxNumber() == box) boxIndices.push_back((int)i);
    int boxIndex = -1;
    for (size_t i = 0; i < boxIndices.size(); ++i) if (boxIndices[i] == targetPos) { boxIndex = (int)i; break; }
    if (boxIndex == -1) return;

    if (newIndex < 0) newIndex = 0;
    if (newIndex >= (int)boxIndices.size()) newIndex = (int)boxIndices.size() - 1;
    if (newIndex == boxIndex) return;

    Pokemon temp = caughtParty[targetPos];
    caughtParty.erase(caughtParty.begin() + targetPos);

    boxIndices.clear();
    for (size_t i = 0; i < caughtParty.size(); ++i) if (caughtParty[i].getBoxNumber() == box) boxIndices.push_back((int)i);

    int insertGlobalIndex;
    if (newIndex >= (int)boxIndices.size()) insertGlobalIndex = (int)caughtParty.size();
    else insertGlobalIndex = boxIndices[newIndex];

    caughtParty.insert(caughtParty.begin() + insertGlobalIndex, temp);
    saveToFile();
}

std::map<std::string, std::vector<std::string>> speciesAbilitiesDB;

CSVRepository::CSVRepository(const std::string& filepath) {
    loadDatabase(filepath);
    loadShowdownData();
    loadItemData();
    loadSaveFile();
    loadTrainerSave();
}

void CSVRepository::loadItemData() {
    QStringList paths = { "ShowdownData/items.js", "ShowdownData/items.txt", "ShowdownData/items.json" };
    QString content;
    for (const auto& path : paths) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            content = file.readAll();
            break;
        }
    }
    if (content.isEmpty()) return;

    QRegularExpression rx("([a-zA-Z0-9]+)\\s*:\\s*\\{.*?(?:desc|shortDesc)\\s*:\\s*\"([^\"]+)\"");
    rx.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator i = rx.globalMatch(content);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        itemDescriptionsDB[match.captured(1).toLower().toStdString()] = match.captured(2).toStdString();
    }
}

void CSVRepository::loadTrainerSave() {
    QFile file("trainer_save.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        money = 5000;
        inventory["POKEBALL"] = 5;
        inventory["SAFARIBALL"] = 5;
        saveTrainerSave(); // Force creation on first run
        return;
    }
    QTextStream in(&file);
    QString moneyLine = in.readLine();
    if (moneyLine.isNull() || moneyLine.isEmpty()) {
        money = 5000;
    }
    else {
        QStringList moneyFields = moneyLine.split(',');
        if (moneyFields.size() >= 2) money = moneyFields[1].toInt();
        else money = 5000;
    }

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        if (fields.size() == 2) {
            inventory[fields[0].toStdString()] = fields[1].toInt();
        }
    }
}

void CSVRepository::saveTrainerSave() const {
    QFile file("trainer_save.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    out << "MONEY," << money << "\n";
    for (const auto& pair : inventory) {
        if (pair.second > 0) {
            out << QString::fromStdString(pair.first) << "," << pair.second << "\n";
        }
    }
}

int CSVRepository::getMoney() const { return money; }
void CSVRepository::setMoney(int amount) { money = amount; saveTrainerSave(); }
int CSVRepository::getItemCount(const std::string& itemName) const {
    auto it = inventory.find(itemName);
    if (it != inventory.end()) return it->second;
    return 0;
}
void CSVRepository::setItemCount(const std::string& itemName, int count) {
    inventory[itemName] = count;
    saveTrainerSave();
}
std::map<std::string, int> CSVRepository::getInventory() const { return inventory; }

void CSVRepository::loadShowdownData() {
    QFile movesFile("ShowdownData/moves.json");
    if (movesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonObject movesObj = QJsonDocument::fromJson(movesFile.readAll()).object();
        for (auto it = movesObj.begin(); it != movesObj.end(); ++it) {
            QJsonObject mObj = it.value().toObject();

            MoveData md;
            md.id = it.key().toLower().toStdString();
            md.name = mObj["name"].toString().toStdString();
            md.category = mObj["category"].toString().toStdString();
            md.type = mObj["type"].toString().toStdString();
            md.basePower = mObj.contains("basePower") ? mObj["basePower"].toInt() : 0;
            md.priority = mObj.contains("priority") ? mObj["priority"].toInt() : 0;
            md.pp = mObj.contains("pp") ? mObj["pp"].toInt() : 10; // NEW: Parse PP from JSON

            if (mObj["accuracy"].isBool()) md.accuracy = 101;
            else md.accuracy = mObj["accuracy"].toInt();

            if (mObj.contains("ohko")) md.ohko = true;

            if (mObj.contains("overrideOffensiveStat")) md.overrideOffensiveStat = mObj["overrideOffensiveStat"].toString().toStdString();
            if (mObj.contains("overrideDefensiveStat")) md.overrideDefensiveStat = mObj["overrideDefensiveStat"].toString().toStdString();
            if (mObj.contains("overrideOffensivePokemon")) md.overrideOffensivePokemon = mObj["overrideOffensivePokemon"].toString().toStdString();

            if (mObj.contains("recoil") && mObj["recoil"].isArray()) {
                QJsonArray arr = mObj["recoil"].toArray();
                md.recoilNum = arr[0].toInt();
                md.recoilDen = arr[1].toInt();
            }

            md.minHits = 1;
            md.maxHits = 1;
            if (mObj.contains("multihit")) {
                if (mObj["multihit"].isArray()) {
                    QJsonArray arr = mObj["multihit"].toArray();
                    md.minHits = arr[0].toInt();
                    md.maxHits = arr[1].toInt();
                }
                else {
                    md.minHits = mObj["multihit"].toInt();
                    md.maxHits = md.minHits;
                }
            }

            if (mObj.contains("heal") && mObj["heal"].isArray()) {
                QJsonArray arr = mObj["heal"].toArray();
                md.healNum = arr[0].toInt();
                md.healDen = arr[1].toInt();
            }

            if (md.name == "Protect" || md.name == "Detect" || md.name == "Spiky Shield" || md.name == "King's Shield" || md.name == "Baneful Bunker") {
                md.isProtectMove = true;
            }

            if (mObj.contains("flags") && mObj["flags"].isObject()) {
                QJsonObject flags = mObj["flags"].toObject();
                md.isContact = flags.contains("contact");
                md.isCharge = flags.contains("charge");
                if (flags.contains("protect")) md.flagsProtect = true;
                else md.flagsProtect = false;
            }

            if (mObj.contains("secondary") && mObj["secondary"].isObject()) {
                QJsonObject sec = mObj["secondary"].toObject();
                md.effectChance = sec["chance"].toInt();
                if (sec.contains("status")) md.status = sec["status"].toString().toStdString();
                if (sec.contains("volatileStatus")) md.volatileStatus = sec["volatileStatus"].toString().toStdString();

                if (sec.contains("boosts")) {
                    QJsonObject b = sec["boosts"].toObject();
                    for (auto k : b.keys()) md.boosts[k.toStdString()] = b[k].toInt();
                }
            }
            else if (mObj.contains("status")) {
                md.status = mObj["status"].toString().toStdString();
                md.effectChance = 100;
            }
            else if (mObj.contains("volatileStatus")) {
                md.volatileStatus = mObj["volatileStatus"].toString().toStdString();
                md.effectChance = 100;
            }

            if (mObj.contains("self") && mObj["self"].isObject()) {
                QJsonObject selfObj = mObj["self"].toObject();
                if (selfObj.contains("volatileStatus")) {
                    if (selfObj["volatileStatus"].toString() == "lockedmove") md.volatileStatus = "lockedmove";
                }
                if (selfObj.contains("boosts")) {
                    QJsonObject b = selfObj["boosts"].toObject();
                    for (auto k : b.keys()) md.selfBoosts[k.toStdString()] = b[k].toInt();
                }
            }
            if (mObj.contains("boosts")) {
                QJsonObject b = mObj["boosts"].toObject();
                for (auto k : b.keys()) md.selfBoosts[k.toStdString()] = b[k].toInt();
            }

            moveDB[md.id] = md;
        }
    }

    QFile learnsetsFile("ShowdownData/learnsets.json");
    if (learnsetsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonObject lsObj = QJsonDocument::fromJson(learnsetsFile.readAll()).object();
        for (auto it = lsObj.begin(); it != lsObj.end(); ++it) {
            QJsonObject pData = it.value().toObject();
            QJsonObject learnsetObj = pData["learnset"].toObject();
            std::vector<std::string> moves;
            for (auto moveIt = learnsetObj.begin(); moveIt != learnsetObj.end(); ++moveIt) {
                moves.push_back(moveIt.key().toLower().toStdString());
            }
            learnsetDB[it.key().toLower().toStdString()] = moves;
        }
    }

    QFile pokedexFile("ShowdownData/pokedex.json");
    if (pokedexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonObject dexObj = QJsonDocument::fromJson(pokedexFile.readAll()).object();
        for (auto it = dexObj.begin(); it != dexObj.end(); ++it) {
            QJsonObject monObj = it.value().toObject();
            std::string key = it.key().toLower().toStdString();
            std::string realName = monObj["name"].toString().toStdString();

            if (monObj.contains("evos")) {
                QJsonArray evosArr = monObj["evos"].toArray();
                for (int i = 0; i < evosArr.size(); ++i) {
                    evosDB[key].push_back(evosArr[i].toString().toStdString());
                }
            }
            if (monObj.contains("prevo")) {
                EvoRule rule;
                rule.prevo = monObj["prevo"].toString().toStdString();
                if (monObj.contains("evoLevel")) rule.evoLevel = monObj["evoLevel"].toInt();
                if (monObj.contains("evoType")) rule.evoType = monObj["evoType"].toString().toStdString();
                if (monObj.contains("evoItem")) rule.evoItem = monObj["evoItem"].toString().toStdString();
                if (monObj.contains("evoCondition")) rule.evoCondition = monObj["evoCondition"].toString().toStdString();
                evoRequirementsDB[key] = rule;
            }

            if (monObj.contains("abilities") && monObj["abilities"].isObject()) {
                QJsonObject abObj = monObj["abilities"].toObject();
                std::vector<std::string> abilities;
                if (abObj.contains("0")) abilities.push_back(abObj["0"].toString().toStdString());
                if (abObj.contains("1")) abilities.push_back(abObj["1"].toString().toStdString());
                if (abObj.contains("H")) abilities.push_back(abObj["H"].toString().toStdString());
                if (abObj.contains("S")) abilities.push_back(abObj["S"].toString().toStdString());
                speciesAbilitiesDB[realName] = abilities;
            }
        }
    }

    // Retroactively heal all base Pokedex objects loaded from CSV
    // Ensure they have proper JSON abilities instead of "None"
    for (auto& p : pokedexDB) {
        if (speciesAbilitiesDB.find(p.getName()) != speciesAbilitiesDB.end() && !speciesAbilitiesDB[p.getName()].empty()) {
            p.setAbility(speciesAbilitiesDB[p.getName()][0]);
        }
    }
}

MoveData CSVRepository::getMoveData(const std::string& moveId) const {
    if (moveDB.find(moveId) != moveDB.end()) return moveDB.at(moveId);

    MoveData fallback;
    fallback.id = moveId;
    fallback.name = (moveId == "struggle") ? "Struggle" : moveId;
    fallback.category = "Physical";
    fallback.type = "Normal";
    fallback.basePower = (moveId == "struggle") ? 50 : 40;
    return fallback;
}

void CSVRepository::loadDatabase(const std::string& filepath) {
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    if (!in.atEnd()) in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        if (fields.size() < 16) continue;
        try {
            int id = fields[0].toInt();
            auto idExists = [&](int candidate) {
                for (const auto& cp : caughtParty) if (cp.getId() == candidate) return true;
                return false;
                };
            if (id == 0 || idExists(id)) {
                int newId = 0; int attempts = 0;
                do {
                    newId = 1000 + (std::rand() % 9000);
                    attempts++;
                } while (idExists(newId) && attempts < 10000);
                if (newId != 0) id = newId;
            }
            std::string name = fields[1].toStdString();
            std::string type1 = fields[2].toStdString();
            std::string type2 = fields[3].isEmpty() ? "None" : fields[3].toStdString();

            double weight = (fields.size() > 12 && !fields[12].isEmpty()) ? fields[12].toDouble() : 10.0;
            int happiness = (fields.size() > 26 && !fields[26].isEmpty()) ? fields[26].toInt() : 70;

            Pokemon species(id, id, name, type1, type2, fields[4].toInt(), fields[5].toInt(), fields[6].toInt(), fields[7].toInt(), fields[8].toInt(), fields[9].toInt(), 50, "None", 1, "", {}, "Hardy", "None", 0, 0, 0, 0, 0, 0, weight, happiness);
            pokedexDB.push_back(species);
        }
        catch (...) {}
    }
}

void CSVRepository::loadSaveFile() {
    QFile file("party_save.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        saveToFile();
        return;
    }

    QTextStream in(&file);
    if (!in.atEnd()) in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        if (fields.size() < 15) continue;
        try {
            int id = fields[0].toInt();
            int speciesId = fields[1].toInt();
            std::string name = fields[2].toStdString();

            std::string nature = "Hardy";
            std::string ability = "None";
            int ev[6] = { 0,0,0,0,0,0 };
            std::vector<std::string> savedMoves;
            std::vector<int> savedPPs; // NEW

            if (fields.size() >= 31) { // NEW: Safe load for 31 fields (Current PP System)
                nature = fields[15].trimmed().toStdString();
                ability = fields[16].trimmed().toStdString();
                for (int i = 0; i < 6; i++) ev[i] = fields[17 + i].toInt();
                for (int i = 23; i < 27; i++) if (fields[i].trimmed() != "None" && !fields[i].isEmpty()) savedMoves.push_back(fields[i].trimmed().toStdString());
                for (int i = 27; i < 31; i++) savedPPs.push_back(fields[i].toInt());
            }
            else if (fields.size() >= 27) { // Old system
                nature = fields[15].trimmed().toStdString();
                ability = fields[16].trimmed().toStdString();
                for (int i = 0; i < 6; i++) ev[i] = fields[17 + i].toInt();
                for (int i = 23; i < 27; i++) if (fields[i].trimmed() != "None" && !fields[i].isEmpty()) savedMoves.push_back(fields[i].trimmed().toStdString());
            }
            else if (fields.size() >= 19) {
                for (int i = 15; i < 19; i++) if (fields[i].trimmed() != "None" && !fields[i].isEmpty()) savedMoves.push_back(fields[i].trimmed().toStdString());
            }
            if (savedMoves.empty()) savedMoves.push_back("tackle");

            // HEAL BROKEN OR MISSING ABILITIES FROM OLD SAVES
            if (ability == "None" || ability.empty()) {
                if (speciesAbilitiesDB.find(name) != speciesAbilitiesDB.end() && !speciesAbilitiesDB[name].empty()) {
                    ability = speciesAbilitiesDB[name][0];
                }
            }

            double weight = 10.0; int happiness = 70;
            for (const auto& s : pokedexDB) {
                if (s.getSpeciesId() == speciesId) { weight = s.getWeight(); happiness = s.getHappiness(); break; }
            }

            Pokemon p(id, speciesId, name, fields[3].trimmed().toStdString(), fields[4].trimmed().toStdString(), fields[5].toInt(), fields[6].toInt(), fields[7].toInt(), fields[8].toInt(), fields[9].toInt(), fields[10].toInt(), fields[11].toInt(), fields[12].trimmed().toStdString(), fields[13].toInt(), fields[14].trimmed().toStdString(), savedMoves, nature, ability, ev[0], ev[1], ev[2], ev[3], ev[4], ev[5], weight, happiness);

            // NEW: Integrate the loaded or default PPs
            for (size_t i = 0; i < savedMoves.size(); i++) {
                MoveData md = getMoveData(savedMoves[i]);
                p.setMaxMovePP(savedMoves[i], md.pp);
                if (!savedPPs.empty() && i < savedPPs.size()) {
                    p.setMovePP(savedMoves[i], savedPPs[i]);
                }
                else {
                    p.setMovePP(savedMoves[i], md.pp);
                }
            }

            caughtParty.push_back(p);
        }
        catch (...) {}
    }
}

void CSVRepository::saveToFile() const {
    QFile file("party_save.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    // NEW: Expanded headers
    out << "id,speciesId,name,type1,type2,basehp,baseatk,basedef,basespa,basespd,basespe,level,heldItem,boxNumber,nickname,nature,ability,evhp,evatk,evdef,evspa,evspd,evspe,m1,m2,m3,m4,pp1,pp2,pp3,pp4\n";
    for (const auto& p : caughtParty) {
        QString cleanNick = QString::fromStdString(p.getNickname()).replace(",", " ");
        out << p.getId() << "," << p.getSpeciesId() << "," << QString::fromStdString(p.getName()).replace(",", "") << ","
            << QString::fromStdString(p.getType1()) << "," << QString::fromStdString(p.getType2()) << ","
            << p.getBaseHp() << "," << p.getBaseAttack() << "," << p.getBaseDefense() << ","
            << p.getBaseSpAttack() << "," << p.getBaseSpDefense() << "," << p.getBaseSpeed() << ","
            << p.getLevel() << "," << QString::fromStdString(p.getHeldItem()).replace(",", "") << "," << p.getBoxNumber() << "," << cleanNick << ","
            << QString::fromStdString(p.getNature()) << "," << QString::fromStdString(p.getAbility()) << ","
            << p.getEvHp() << "," << p.getEvAtk() << "," << p.getEvDef() << "," << p.getEvSpA() << "," << p.getEvSpD() << "," << p.getEvSpe() << ",";

        auto moves = p.getMoves();
        out << (moves.size() > 0 ? QString::fromStdString(moves[0]) : "None") << ",";
        out << (moves.size() > 1 ? QString::fromStdString(moves[1]) : "None") << ",";
        out << (moves.size() > 2 ? QString::fromStdString(moves[2]) : "None") << ",";
        out << (moves.size() > 3 ? QString::fromStdString(moves[3]) : "None") << ",";

        // NEW: Save current PP
        out << p.getMovePP(moves.size() > 0 ? moves[0] : "") << ",";
        out << p.getMovePP(moves.size() > 1 ? moves[1] : "") << ",";
        out << p.getMovePP(moves.size() > 2 ? moves[2] : "") << ",";
        out << p.getMovePP(moves.size() > 3 ? moves[3] : "") << "\n";
    }
}

void CSVRepository::add(const Pokemon& p) {
    Pokemon copy = p;
    std::vector<int> existingIds;
    for (const auto& cp : caughtParty) existingIds.push_back(cp.getId());

    auto idExists = [&](int id) {
        return std::find(existingIds.begin(), existingIds.end(), id) != existingIds.end();
        };

    if (copy.getId() == 0 || idExists(copy.getId())) {
        int newId = 0;
        int attempts = 0;
        do {
            newId = 1000 + (std::rand() % 9000);
            attempts++;
        } while (idExists(newId) && attempts < 10000);
        copy.setId(newId == 0 ? (int)caughtParty.size() + 1 : newId);
    }

    caughtParty.push_back(copy);
    saveToFile();
}

void CSVRepository::remove(int id) {
    auto it = std::remove_if(caughtParty.begin(), caughtParty.end(), [id](const Pokemon& p) { return p.getId() == id; });
    if (it != caughtParty.end()) { caughtParty.erase(it, caughtParty.end()); saveToFile(); }
}

void CSVRepository::update(const Pokemon& newP) {
    for (auto& p : caughtParty) { if (p.getId() == newP.getId()) { p = newP; saveToFile(); return; } }
}

Pokemon CSVRepository::getById(int id) const {
    for (const auto& p : caughtParty) { if (p.getId() == id) return p; }
    throw RepositoryException("Pokemon not in party");
}

std::vector<Pokemon> CSVRepository::getAll() const { return caughtParty; }
int CSVRepository::getPokedexSize() const { return pokedexDB.size(); }

Pokemon CSVRepository::getSpeciesData(int index) const {
    if (index >= 0 && index < pokedexDB.size()) return pokedexDB[index];
    return Pokemon();
}

Pokemon CSVRepository::getSpeciesDataByName(const std::string& name) const {
    QString searchName = QString::fromStdString(name).toLower();
    for (const auto& p : pokedexDB) {
        if (QString::fromStdString(p.getName()).toLower() == searchName) {
            return p;
        }
    }
    return Pokemon();
}

std::vector<std::string> CSVRepository::getLearnset(const std::string& speciesName) const {
    std::string key = toShowdownId(speciesName).toStdString();
    if (learnsetDB.find(key) != learnsetDB.end()) return learnsetDB.at(key);
    size_t dashPos = speciesName.find('-');
    if (dashPos != std::string::npos) {
        std::string baseKey = toShowdownId(speciesName.substr(0, dashPos)).toStdString();
        if (learnsetDB.find(baseKey) != learnsetDB.end()) return learnsetDB.at(baseKey);
    }
    return { "tackle", "growl" };
}

std::string CSVRepository::getMoveName(const std::string& moveId) const {
    if (moveNamesDB.find(moveId) != moveNamesDB.end()) return moveNamesDB.at(moveId);
    return moveId;
}

std::string CSVRepository::getItemDescription(const std::string& itemName) const {
    std::string key = toShowdownId(itemName).toStdString();
    if (itemDescriptionsDB.find(key) != itemDescriptionsDB.end()) return itemDescriptionsDB.at(key);
    return "";
}

std::string CSVRepository::getMoveCategory(const std::string& moveId) const {
    if (moveCategoryDB.find(moveId) != moveCategoryDB.end()) return moveCategoryDB.at(moveId);
    return "Physical";
}

std::string CSVRepository::getMoveType(const std::string& moveId) const {
    if (moveTypeDB.find(moveId) != moveTypeDB.end()) return moveTypeDB.at(moveId);
    return "Normal";
}

Pokemon CSVRepository::getRandomEncounter(const std::vector<std::string>& allowedTypes) const {
    std::vector<Pokemon> validEncounters;
    std::vector<int> seenIds;
    for (const auto& p : pokedexDB) {
        if (p.getSpeciesId() >= 10000 || std::find(seenIds.begin(), seenIds.end(), p.getSpeciesId()) != seenIds.end()) continue;
        seenIds.push_back(p.getSpeciesId());
        bool typeMatch = false;
        for (const auto& allowed : allowedTypes) {
            if (p.getType1() == allowed || p.getType2() == allowed) { typeMatch = true; break; }
        }
        if (typeMatch) validEncounters.push_back(p);
    }
    if (validEncounters.empty()) return Pokemon();

    Pokemon encounter = validEncounters[rand() % validEncounters.size()];
    std::vector<std::string> availableMoves = getLearnset(encounter.getName());
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(availableMoves.begin(), availableMoves.end(), g);

    std::vector<std::string> selectedMoves;
    for (size_t i = 0; i < 4 && i < availableMoves.size(); ++i) selectedMoves.push_back(availableMoves[i]);
    if (selectedMoves.empty()) selectedMoves.push_back("tackle");
    encounter.setMoves(selectedMoves);

    std::vector<std::string> natures = { "Adamant", "Jolly", "Timid", "Modest", "Bold", "Calm", "Impish", "Careful", "Brave", "Quiet", "Hardy" };
    encounter.setNature(natures[rand() % natures.size()]);

    if (speciesAbilitiesDB.find(encounter.getName()) != speciesAbilitiesDB.end()) {
        auto abs = speciesAbilitiesDB[encounter.getName()];
        if (!abs.empty()) encounter.setAbility(abs[rand() % abs.size()]);
    }

    // NEW: Initialize max PP for random encounters
    for (const auto& m : selectedMoves) {
        MoveData md = getMoveData(m);
        encounter.setMaxMovePP(m, md.pp);
        encounter.setMovePP(m, md.pp);
    }

    return encounter;
}

int CSVRepository::getMovePower(const std::string& moveId) const {
    if (movePowerDB.find(moveId) != movePowerDB.end()) return movePowerDB.at(moveId);
    return 0;
}

std::vector<std::string> CSVRepository::getPossibleEvolutions(const std::string& species) const {
    std::string key = toShowdownId(species).toStdString();
    if (evosDB.find(key) != evosDB.end()) {
        return evosDB.at(key);
    }
    return {};
}

EvoRule CSVRepository::getEvoRule(const std::string& species) const {
    std::string key = toShowdownId(species).toStdString();
    if (evoRequirementsDB.find(key) != evoRequirementsDB.end()) {
        return evoRequirementsDB.at(key);
    }
    return EvoRule();
}