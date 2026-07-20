#include "TournamentDialog.h"
#include "BattleDialog.h"
#include "../Progression/TrainerRoster.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <algorithm>
#include <random>

// FIXED: currentRound(1) is now explicitly initialized to prevent garbage-memory crashes
TournamentDialog::TournamentDialog(Controller& ctrl, QWidget* parent)
    : QDialog(parent), controller(ctrl), currentRound(1) {

    setupUI();
    if (parent) this->setStyleSheet(parent->styleSheet());

    // Initialize the bracket pools for the current run
    gymPool = TrainerRoster::getTournamentGymLeaders();
    e4Pool = TrainerRoster::getTournamentEliteFour();

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(gymPool.begin(), gymPool.end(), g);
    std::shuffle(e4Pool.begin(), e4Pool.end(), g);
}

void TournamentDialog::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    QLabel* title = new QLabel("THE GRAND TOURNAMENT", this);
    title->setObjectName("titleText");
    title->setAlignment(Qt::AlignCenter);

    bracketLabel = new QLabel(this);
    bracketLabel->setAlignment(Qt::AlignCenter);
    bracketLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");

    updateBracketUI();

    nextMatchBtn = new QPushButton("ENTER ARENA", this);
    nextMatchBtn->setObjectName("dangerBtn");
    nextMatchBtn->setMinimumHeight(50);

    forfeitBtn = new QPushButton("FORFEIT RUN", this);
    forfeitBtn->setObjectName("primaryBtn");
    forfeitBtn->setMinimumHeight(50);

    layout->addWidget(title);
    layout->addWidget(bracketLabel);
    layout->addStretch();
    layout->addWidget(nextMatchBtn);
    layout->addWidget(forfeitBtn);

    connect(forfeitBtn, &QPushButton::clicked, this, &TournamentDialog::onForfeitClicked);
    connect(nextMatchBtn, &QPushButton::clicked, this, &TournamentDialog::onNextMatchClicked);

    setWindowTitle("Pokémon League");
    setFixedSize(400, 350);
}

void TournamentDialog::updateBracketUI() {
    if (currentRound <= 4) {
        bracketLabel->setText(QString("Quarter-Finals\n\nRound %1 of 4\n(Opponent: Gym Leader)").arg(currentRound));
    }
    else if (currentRound <= 6) {
        bracketLabel->setText(QString("Semi-Finals\n\nRound %1 of 2\n(Opponent: Elite Four)").arg(currentRound - 4));
    }
    else if (currentRound == 7) {
        bracketLabel->setText("Grand Finals\n\n(Opponent: The Champion)");
    }
    else {
        bracketLabel->setText("HALL OF FAME\n\nYou are the Pokémon Champion!");
        nextMatchBtn->setText("VIEW HALL OF FAME");
    }
}

void TournamentDialog::onNextMatchClicked() {
    if (currentRound > 7) {
        QMessageBox::information(this, "Champion", "You have already conquered the Tournament! (Hall of Fame UI coming soon)");
        accept();
        return;
    }

    std::vector<Pokemon> playerParty;
    for (const auto& p : controller.getAllPokemon()) {
        if (p.getBoxNumber() == 0) playerParty.push_back(p);
    }

    if (playerParty.empty()) {
        QMessageBox::warning(this, "Empty Party", "You cannot battle without Pokémon in your Active Party!");
        return;
    }

    TrainerTeam enemyTrainer;
    if (currentRound <= 4) {
        enemyTrainer = gymPool[currentRound - 1];
    }
    else if (currentRound <= 6) {
        enemyTrainer = e4Pool[currentRound - 5];
    }
    else {
        enemyTrainer = TrainerRoster::getTournamentChampion();
    }

    std::vector<Pokemon> enemyParty;
    for (const auto& ep : enemyTrainer.team) {
        Pokemon baseData = controller.getSpeciesDataByName(ep.species);
        if (baseData.getName().empty()) continue;

        Pokemon battleMon(
            rand() % 10000 + 30000, baseData.getSpeciesId(), baseData.getName(),
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

    {
        // Parent is strictly 'this', and we DO NOT hide the TournamentDialog
        BattleDialog battleDialog(controller, playerParty, enemyParty, this);
        battleResult = battleDialog.exec();
    }

    if (battleResult == QDialog::Accepted) {
        currentRound++;
        if (currentRound > 7) {
            QMessageBox::information(this, "CHAMPION!", "You defeated Cynthia!\n\nYou have conquered the rogue-lite simulation and become the Pokémon Champion!");
        }
        else {
            QMessageBox::information(this, "Victory", "You won the match! Proceeding to the next round of the bracket.");
        }
        updateBracketUI();
    }
    else {
        QMessageBox::critical(this, "Eliminated", "You were defeated! Your tournament run is over. You must start from the Quarter-Finals again.");
        reject();
    }
}

void TournamentDialog::onForfeitClicked() {
    if (QMessageBox::question(this, "Forfeit Run", "Are you sure you want to forfeit? You will lose all current tournament progress and have to restart from Round 1!") == QMessageBox::Yes) {
        reject();
    }
}