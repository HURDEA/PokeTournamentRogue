#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include "../Controller/Controller.h"
#include "../Progression/ProgressionManager.h"

class GauntletDialog : public QDialog {
    Q_OBJECT

private:
    Controller& controller;
    ProgressionManager& progManager;

    QLabel* stageLabel;
    QLabel* streakLabel;
    QPushButton* startBattleBtn;
    QPushButton* leaveBtn;

    void setupUI();
    void updateUI();

private slots:
    void onStartBattleClicked();

public:
    GauntletDialog(Controller& ctrl, ProgressionManager& pm, QWidget* parent = nullptr);
    ~GauntletDialog() = default;
};