#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include "../Controller/Controller.h"

class TrainerDialog : public QDialog {
    Q_OBJECT
private:
    Controller& controller;
    QListWidget* partyList;

    QListWidget* currentMovesList;
    QListWidget* learnableMovesList;
    QPushButton* forgetBtn;
    QPushButton* learnBtn;

    QComboBox* natureBox;
    QComboBox* abilityBox;
    QPushButton* applyNatureAbilityBtn;

    // EV Spinboxes
    QSpinBox* evHp, * evAtk, * evDef, * evSpA, * evSpD, * evSpe;
    QLabel* evTotalLabel;
    QPushButton* applyEvBtn;

    int selectedPartyId = -1;
    const int EV_TOTAL_MAX = 66;
    const int EV_STAT_MAX = 32; // <--- Corrected limit
    const int TRAINING_COST = 500; // Cost per session

    void setupUI();
    void refreshPartyList();
    void updateEvTotal();
    void applyTheme(const QString& themeName); // Dynamically applies the Main Hall theme

private slots:
    void onPartySelectionChanged();
    void onForgetMoveClicked();
    void onLearnMoveClicked();
    void onApplyNatureAbilityClicked();
    void onApplyEvClicked();

public:
    TrainerDialog(Controller& ctrl, QWidget* parent = nullptr);
    ~TrainerDialog() = default;
};