#pragma once
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <vector>
#include "../Controller/Controller.h"
#include "../Progression/Trainer.h"

class TournamentDialog : public QDialog {
    Q_OBJECT
private:
    Controller& controller;

    // CRITICAL: We must track the round to prevent out-of-bounds array crashes
    int currentRound;

    std::vector<TrainerTeam> gymPool;
    std::vector<TrainerTeam> e4Pool;

    QLabel* bracketLabel;
    QPushButton* nextMatchBtn;
    QPushButton* forfeitBtn;

    void setupUI();
    void updateBracketUI();

private slots:
    void onNextMatchClicked();
    void onForfeitClicked();

public:
    TournamentDialog(Controller& ctrl, QWidget* parent = nullptr);
    ~TournamentDialog() = default;
};