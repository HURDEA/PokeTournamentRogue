#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include "../Controller/Controller.h"
#include "../Progression/ProgressionManager.h" // NEW: Progression Tracker

class MainHall : public QMainWindow {
    Q_OBJECT

private:
    Controller& controller;
    ProgressionManager progManager; // NEW: Instance to track unlocks

    // UI Elements
    QWidget* centralWidget;
    QLabel* titleLabel;
    QLabel* subtitleLabel;

    QPushButton* guideButton;
    QPushButton* wildAreaBtn;
    QPushButton* pcManagerBtn;
    QPushButton* shopBtn;
    QPushButton* trainerBtn;
    QPushButton* battlesBtn;
    QPushButton* tournamentBtn;
    QPushButton* exitBtn;

    // Theme Engine
    QComboBox* themeComboBox;
    void applyTheme(const QString& themeName);

    void setupUI();
    void connectSignals();

    // NEW: Core game loop explanation dialog & UI refresher
    void showHowToPlay();
    void refreshProgressionUI();

private slots:
    void onWildAreaClicked();
    void onPCManagerClicked();
    void onShopClicked();
    void onExitClicked();
    void onThemeChanged(const QString& themeName);

public:
    MainHall(Controller& ctrl, QWidget* parent = nullptr);
    ~MainHall() = default;
};