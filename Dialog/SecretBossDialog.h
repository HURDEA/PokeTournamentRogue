#pragma once
#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QListWidget>
#include <memory>
#include <vector>
#include "../Controller/Controller.h"
#include "../Domain/Pokemon.h"
#include "BattleDialog.h" // Needed for the TurnPhase enum

class BattleEngine;

class SecretBossDialog : public QDialog {
    Q_OBJECT
private:
    Controller& controller;
    std::unique_ptr<BattleEngine> engine;

    std::vector<Pokemon> playerParty;
    std::vector<Pokemon> enemyParty;

    int currentPlayerIndex;
    int currentEnemyIndex;
    int lastPlayerHp;
    int lastEnemyHp;

    bool isPlayerMegaEvolved;
    bool isEnemyMegaEvolved;
    bool megaEvolveNextMove;

    bool isFaintSwitch;
    bool isMidTurnSwitch;
    TurnPhase currentPhase;
    bool playerGoesFirst;
    std::string queuedPlayerMove;
    std::string queuedEnemyMove;

    int turnCounter;
    bool isNewTurn;
    std::vector<std::string> currentLog;

    // --- BOSS MECHANICS ---
    bool hasShownHalfwayDialogue;
    bool hasShownLastMonDialogue;
    void checkBossTriggers();
    void triggerOOPQuiz(); // NEW: Pop-up quiz method

    // UI Elements
    QLabel* playerNameLabel;
    QProgressBar* playerHpBar;
    QLabel* playerHpText;
    QLabel* playerSprite;

    QLabel* enemyNameLabel;
    QProgressBar* enemyHpBar;
    QLabel* enemyHpText;
    QLabel* enemySprite;

    QPushButton* fieldInfoBtn;
    QListWidget* logWidget;

    QWidget* actionPanel;
    QPushButton* fightBtn;
    QPushButton* switchBtn;
    QPushButton* megaBtn;
    QPushButton* runBtn;

    QWidget* movePanel;
    QPushButton* move1Btn;
    QPushButton* move2Btn;
    QPushButton* move3Btn;
    QPushButton* move4Btn;
    QPushButton* backToActionsBtn;

    QWidget* switchPanel;
    QListWidget* partyListWidget;
    QPushButton* confirmSwitchBtn;
    QPushButton* cancelSwitchBtn;

    void setupUI();
    void applyBossTheme();
    void connectSignals();

    void updateBattleDisplay();
    void updateFieldDisplay();
    void flushLog(bool endOfTurn);
    QString getTypeColor(const std::string& type);

    void beginTurn(const std::string& playerMove);
    void executeNextPhase();
    void executeSwitch(int newIndex);
    void executeEnemySwitch();
    bool checkFaint();

    void animateMoveEffect(bool isPlayerAttacker, const std::string& moveType, const std::string& moveCategory);
    void animateSwitchIn(bool isPlayer);
    void animateHpBar(bool isPlayer, int startHp, int endHp, int maxHp, const std::string& status);
    void animateMegaEvolution(bool isPlayer);
    void animateFaint(bool isPlayer);

private slots:
    void onFightClicked();
    void onSwitchClicked();
    void onPartyItemDoubleClicked(QListWidgetItem* item);
    void onCancelSwitchClicked();
    void onConfirmSwitchClicked();
    void onMegaClicked();
    void onRunClicked();
    void onMoveClicked(int moveIndex);
    void onFieldInfoClicked();

public:
    SecretBossDialog(Controller& ctrl, const std::vector<Pokemon>& pParty, QWidget* parent = nullptr);
    ~SecretBossDialog();
};