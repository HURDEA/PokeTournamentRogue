#include "ProgressionManager.h"
#include <QSettings>

ProgressionManager::ProgressionManager() {
    // Automatically load the player's saved state on boot
    loadProgress();
}

void ProgressionManager::loadProgress() {
    QSettings settings("MyCompany", "PokemonBoxManager");
    currentGauntletStage = settings.value("gauntletStage", 1).toInt();
    currentGauntletStreak = settings.value("gauntletStreak", 0).toInt();
    isTournamentUnlocked = settings.value("tournamentUnlocked", false).toBool();
}

void ProgressionManager::saveProgress() const {
    QSettings settings("MyCompany", "PokemonBoxManager");
    settings.setValue("gauntletStage", currentGauntletStage);
    settings.setValue("gauntletStreak", currentGauntletStreak);
    settings.setValue("tournamentUnlocked", isTournamentUnlocked);
}

void ProgressionManager::recordGauntletWin() {
    currentGauntletStreak++;
    if (currentGauntletStreak >= 3) {
        currentGauntletStage++;
        currentGauntletStreak = 0;

        if (currentGauntletStage > 3 && !isTournamentUnlocked) {
            unlockTournament();
        }
    }
    // Save immediately after state change
    saveProgress();
}

void ProgressionManager::resetGauntletStreak() {
    currentGauntletStreak = 0;
    saveProgress();
}

void ProgressionManager::unlockTournament() {
    isTournamentUnlocked = true;
    saveProgress();
}

std::vector<std::string> ProgressionManager::getUnlockedBiomes() const {
    std::vector<std::string> biomes = { "Viridian Forest (Grass, Bug, Flying, Normal)" };
    if (currentGauntletStage >= 2) {
        biomes.push_back("Mt. Moon (Rock, Ground, Fighting)");
        biomes.push_back("Power Plant (Electric, Steel)");
    }
    if (currentGauntletStage >= 3) {
        biomes.push_back("Sewers (Poison, Dark)");
        biomes.push_back("Volcano (Ground, Rock, Fire)");
    }
    if (isTournamentUnlocked) {
        biomes.push_back("Tundra (Ice, Ghost)");
        biomes.push_back("Temple (Fighting, Psychic)");
        biomes.push_back("Great Waterfall (Water, Fairy, Dragon)");
    }
    return biomes;
}

std::vector<std::string> ProgressionManager::getUnlockedItems() const {
    return {};
}

void ProgressionManager::resetRun() {
    QSettings settings("MyCompany", "PokemonBoxManager");

    // Overwrite the saved progression values with starting defaults
    settings.setValue("gauntletStage", 1);
    settings.setValue("gauntletStreak", 0);
    settings.setValue("tournamentUnlocked", false);

    // Update the live memory variables (assuming these are your variable names)
    currentGauntletStage = 1;
    currentGauntletStreak = 0;
    isTournamentUnlocked = false;
}