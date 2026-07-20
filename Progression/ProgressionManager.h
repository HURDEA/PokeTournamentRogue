#pragma once
#include <vector>
#include <string>

class ProgressionManager {
private:
    int currentGauntletStage;
    int currentGauntletStreak;
    bool isTournamentUnlocked;

public:
    ProgressionManager();

    int getGauntletStage() const { return currentGauntletStage; }
    int getGauntletStreak() const { return currentGauntletStreak; }
    bool getTournamentUnlocked() const { return isTournamentUnlocked; }

    void recordGauntletWin();
    void resetGauntletStreak();
    void unlockTournament();

    std::vector<std::string> getUnlockedBiomes() const;
    std::vector<std::string> getUnlockedItems() const;

    // --- State Persistence ---
    void loadProgress();
    void saveProgress() const;
};