#include "GauntletDialog.h"
#include "BattleDialog.h"
#include "../Progression/TrainerRoster.h"
#include <QVBoxLayout>
#include <QMessageBox>

GauntletDialog::GauntletDialog(Controller& ctrl, ProgressionManager& pm, QWidget* parent)
    : QDialog(parent), controller(ctrl), progManager(pm) {
    setupUI();

    if (parent) this->setStyleSheet(parent->styleSheet());
}

void GauntletDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel* title = new QLabel("PRACTICE GAUNTLET", this);
    title->setObjectName("titleText");
    title->setAlignment(Qt::AlignCenter);

    stageLabel = new QLabel(this);
    streakLabel = new QLabel(this);
    stageLabel->setAlignment(Qt::AlignCenter);
    streakLabel->setAlignment(Qt::AlignCenter);
    stageLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    streakLabel->setStyleSheet("font-size: 14px; font-style: italic; color: #8b949e;");

    updateUI();

    startBattleBtn = new QPushButton("FIND OPPONENT", this);
    startBattleBtn->setObjectName("warningBtn");
    startBattleBtn->setMinimumHeight(50);

    leaveBtn = new QPushButton("LEAVE GAUNTLET", this);
    leaveBtn->setObjectName("primaryBtn");
    leaveBtn->setMinimumHeight(50);

    layout->addWidget(title);
    layout->addWidget(stageLabel);
    layout->addWidget(streakLabel);
    layout->addStretch();
    layout->addWidget(startBattleBtn);
    layout->addWidget(leaveBtn);

    connect(leaveBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(startBattleBtn, &QPushButton::clicked, this, &GauntletDialog::onStartBattleClicked);

    setWindowTitle("Gauntlet Matchmaking");
    setFixedSize(350, 300);
}

void GauntletDialog::updateUI() {
    int stage = progManager.getGauntletStage();
    QString stageName = (stage == 1) ? "Novice" : (stage == 2) ? "Intermediate" : (stage == 3) ? "Advanced" : "Expert";

    stageLabel->setText(QString("Gauntlet Stage %1: %2").arg(stage).arg(stageName));
    streakLabel->setText(QString("Current Win Streak: %1 / 3\n(3 Wins required to advance)").arg(progManager.getGauntletStreak()));
}

void GauntletDialog::onStartBattleClicked() {
    std::vector<Pokemon> playerParty;
    for (const auto& p : controller.getAllPokemon()) {
        if (p.getBoxNumber() == 0) playerParty.push_back(p);
    }

    if (playerParty.empty()) {
        QMessageBox::warning(this, "Empty Party", "You cannot battle without Pokémon in your Active Party (Box 0)!");
        return;
    }

    int currentStage = progManager.getGauntletStage();
    std::vector<TrainerTeam> possibleTeams = TrainerRoster::getGauntletTrainers(currentStage);
    TrainerTeam enemyTrainer = possibleTeams[rand() % possibleTeams.size()];

    std::vector<Pokemon> enemyParty;
    for (const auto& ep : enemyTrainer.team) {
        Pokemon baseData = controller.getSpeciesDataByName(ep.species);
        if (baseData.getName().empty()) continue;

        Pokemon battleMon(
            rand() % 10000 + 20000, baseData.getSpeciesId(), baseData.getName(),
            baseData.getType1(), baseData.getType2(), baseData.getBaseHp(),
            baseData.getBaseAttack(), baseData.getBaseDefense(), baseData.getBaseSpAttack(),
            baseData.getBaseSpDefense(), baseData.getBaseSpeed(),
            50, ep.heldItem, -1, baseData.getName(), ep.moves,
            "Hardy", ep.ability, ep.evHp, ep.evAtk, ep.evDef, ep.evSpA, ep.evSpD, ep.evSpe,
            baseData.getWeight(), baseData.getHappiness()
        );
        enemyParty.push_back(battleMon);
    }

    int battleResult;

    // Create scope so BattleDialog is completely destroyed before the popups
    {
        // Parent is strictly 'this', and we DO NOT hide the GauntletDialog
        BattleDialog battleDialog(controller, playerParty, enemyParty, this);
        battleResult = battleDialog.exec();
    }

    if (battleResult == QDialog::Accepted) {
        progManager.recordGauntletWin();
        int payout = 1000 * currentStage;
        controller.addMoney(payout);

        if (progManager.getGauntletStage() > currentStage) {
            QMessageBox::information(this, "Stage Cleared!",
                QString("You defeated %1 %2!\nYou earned $%3.\n\nCONGRATULATIONS! You have advanced to Gauntlet Stage %4! New Wild Areas have been unlocked!").arg(QString::fromStdString(enemyTrainer.trainerClass), QString::fromStdString(enemyTrainer.trainerName)).arg(payout).arg(progManager.getGauntletStage()));
        }
        else {
            QMessageBox::information(this, "Victory",
                QString("You defeated %1 %2!\nYou earned $%3.").arg(QString::fromStdString(enemyTrainer.trainerClass), QString::fromStdString(enemyTrainer.trainerName)).arg(payout));
        }
    }
    else {
        progManager.resetGauntletStreak();
        QMessageBox::critical(this, "Defeat", "Your win streak has been broken. You must start this stage over.");
    }

    updateUI();
}