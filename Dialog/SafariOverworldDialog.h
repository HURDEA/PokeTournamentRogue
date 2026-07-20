#pragma once
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <optional>
#include <vector>
#include <string>
#include "../Controller/Controller.h"
#include "../Domain/Pokemon.h"

class SafariOverworldDialog : public QDialog {
    Q_OBJECT

private:
    Controller& controller;
    QString playerAvatarPath;
    QString currentZoneName; // Needed for dynamic coloring
    std::vector<std::string> zoneTypes;

    QLabel* statusLabel;
    QListWidget* logWidget;

    std::vector<std::vector<QLabel*>> grassGrid;
    int currentRustleRow = -1;
    int currentRustleCol = -1;

    QTimer* rustleTimer;
    std::optional<Pokemon> overworldCaughtPokemon;

    void setupUI();
    void connectSignals();
    void log(const QString& msg);
    void applyTheme(const QString& themeName); // Theme Engine
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onRustle();
    void onExitZone();

public:
    SafariOverworldDialog(Controller& ctrl, const QString& avatarPath, const QString& zoneName, const std::vector<std::string>& allowedTypes, QWidget* parent = nullptr);
    ~SafariOverworldDialog() = default;

    std::optional<Pokemon> getCaughtPokemon() const { return overworldCaughtPokemon; }
};