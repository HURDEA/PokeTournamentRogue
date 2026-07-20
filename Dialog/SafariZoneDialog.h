#pragma once
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QComboBox>
#include <optional>
#include <vector>
#include <string>
#include "../Controller/Controller.h"
#include "../Domain/Pokemon.h"

class SafariZoneDialog : public QDialog {
    Q_OBJECT
private:
    Controller& controller;
    Pokemon currentEncounter;
    QString playerAvatarPath;

    int catchBuff;
    int fleeChance;
    int turnCount; // Tracks turns for Quick Ball & Timer Ball

    QLabel* imageLabel;
    QLabel* statusLabel;
    QListWidget* logWidget;

    QPushButton* ballButton;
    QPushButton* berryButton;
    QPushButton* rockButton;
    QPushButton* runButton;

    QComboBox* ballSelector;
    QComboBox* berrySelector;

    std::optional<Pokemon> caughtPokemon;

    void setupUI();
    void connectSignals();
    void log(const QString& msg);
    void resetStats();
    bool checkFlee(int chance);
    void applyTheme(const QString& themeName); // Theme Engine

private slots:
    void onBall();
    void onBerry();
    void onRock();
    void onRun();

public:
    SafariZoneDialog(Controller& ctrl, const QString& playerPath, const std::vector<std::string>& allowedTypes, QWidget* parent = nullptr);
    std::optional<Pokemon> getCaughtPokemon() const { return caughtPokemon; }
};