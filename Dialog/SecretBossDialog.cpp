#include "SecretBossDialog.h"
#include "SummaryDialog.h"
#include "../Engine/BattleEngine.h"
#include "../Engine/AI/AiDetector.h"
#include "../Progression/TrainerRoster.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QTimer>
#include <QIcon>
#include <QFile>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>
#include <QStringList>
#include <algorithm>
#include <random>

// =========================================================
// CUSTOM OOP EXAM POP-UP
// =========================================================
struct OOPQuestion {
    QString question;
    QStringList answers; // Index 0 is ALWAYS the correct answer in this definition
};

class OOPQuizDialog : public QDialog {
public:
    OOPQuizDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Dialog);
        setFixedSize(500, 350);

        // Inherit the intense boss theme
        if (parent) setStyleSheet(parent->styleSheet());

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(25, 25, 25, 25);
        layout->setSpacing(15);

        QLabel* header = new QLabel("INTERRUPT: POP QUIZ!");
        header->setStyleSheet("font-size: 20px; font-weight: 900; color: #ff4757; border: none; background: transparent;");
        header->setAlignment(Qt::AlignCenter);
        layout->addWidget(header);

        std::vector<OOPQuestion> pool = {
            {"Which OOP principle hides internal states and requires all interaction to be performed through an object's methods?",
             {"Encapsulation", "Polymorphism", "Inheritance", "Abstraction"}},
            {"What is the concept of a derived class redefining a method from its base class?",
             {"Overriding", "Overloading", "Encapsulation", "Virtualization"}},
            {"In C++, what happens if a class contains at least one pure virtual function?",
             {"It becomes an abstract class", "It fails to compile", "It becomes a concrete class", "It throws a runtime error"}},
            {"What type of binding is achieved when using virtual functions in C++?",
             {"Dynamic / Late Binding", "Static / Early Binding", "Compile-time Binding", "Lexical Binding"}},
            {"Which keyword is used in C++ to prevent a class from being inherited?",
             {"final", "const", "static", "sealed"}},
            {"What is a 'memory leak' in the context of C++?",
             {"Failing to deallocate dynamically allocated memory", "Exceeding the array bounds", "A pointer pointing to a null address", "A stack overflow from deep recursion"}},
            {"What does the 'O' in the S.O.L.I.D. principles stand for?",
             {"Open/Closed Principle", "Object-Oriented Principle", "Overloading Principle", "Override Principle"}}
        };

        OOPQuestion q = pool[rand() % pool.size()];

        QLabel* questionLabel = new QLabel(q.question);
        questionLabel->setWordWrap(true);
        questionLabel->setStyleSheet("font-size: 15px; font-weight: bold; color: #ffffff; border: none; background: transparent;");
        questionLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(questionLabel);

        // Map buttons to answers and shuffle them so the correct answer isn't always first
        std::vector<int> indices = { 0, 1, 2, 3 };
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(indices.begin(), indices.end(), g);

        for (int i : indices) {
            QPushButton* btn = new QPushButton(q.answers[i]);
            btn->setMinimumHeight(40);
            if (i == 0) {
                // Correct answer triggers Accept
                connect(btn, &QPushButton::clicked, this, &QDialog::accept);
            }
            else {
                // Wrong answer triggers Reject
                connect(btn, &QPushButton::clicked, this, &QDialog::reject);
            }
            layout->addWidget(btn);
        }
    }
};

// =========================================================

SecretBossDialog::SecretBossDialog(Controller& ctrl, const std::vector<Pokemon>& pParty, QWidget* parent)
    : QDialog(parent), controller(ctrl), playerParty(pParty),
    currentPlayerIndex(0), currentEnemyIndex(0), lastPlayerHp(-1), lastEnemyHp(-1),
    isPlayerMegaEvolved(false), isEnemyMegaEvolved(false), megaEvolveNextMove(false),
    isFaintSwitch(false), isMidTurnSwitch(false), currentPhase(TurnPhase::Idle),
    turnCounter(1), isNewTurn(true), hasShownHalfwayDialogue(false), hasShownLastMonDialogue(false) {

    for (auto& p : playerParty) {
        for (const auto& m : p.getMoves()) {
            if (!m.empty() && m != "None" && p.getMaxMovePP(m) <= 0) {
                int pp = controller.getMoveData(m).pp;
                p.setMaxMovePP(m, pp);
                p.setMovePP(m, pp);
            }
        }
        p.fullyHeal();
    }

    // --- GENERATE LEVEL 100 SECRET BOSS TEAM ---
    TrainerTeam gabi = TrainerRoster::getSecretBoss();
    for (const auto& ep : gabi.team) {
        Pokemon baseData = controller.getSpeciesDataByName(ep.species);
        if (baseData.getName().empty()) continue;

        Pokemon battleMon(
            rand() % 10000 + 30000, baseData.getSpeciesId(), baseData.getName(),
            baseData.getType1(), baseData.getType2(), baseData.getBaseHp(),
            baseData.getBaseAttack(), baseData.getBaseDefense(), baseData.getBaseSpAttack(),
            baseData.getBaseSpDefense(), baseData.getBaseSpeed(),
            100, // <--- FORCED LEVEL 100
            ep.heldItem, -1, baseData.getName(), ep.moves,
            "Serious", ep.ability, 31, 31, 31, 31, 31, 31, // Max DVs
            baseData.getWeight(), baseData.getHappiness()
        );

        for (const auto& m : battleMon.getMoves()) {
            if (!m.empty() && m != "None") {
                int pp = controller.getMoveData(m).pp;
                battleMon.setMaxMovePP(m, pp);
                battleMon.setMovePP(m, pp);
            }
        }
        battleMon.fullyHeal();
        enemyParty.push_back(battleMon);
    }
    // -------------------------------------------

    for (size_t i = 0; i < playerParty.size(); i++) {
        if (playerParty[i].getCurrentHp() > 0) { currentPlayerIndex = i; break; }
    }

    setupUI();
    connectSignals();
    applyBossTheme();

    engine = std::make_unique<BattleEngine>(controller);
    engine->setFaintedCount(true, 0);
    engine->setFaintedCount(false, 0);

    updateBattleDisplay();

    engine->onSwitchIn(playerParty[currentPlayerIndex], true, currentLog, &enemyParty[currentEnemyIndex]);
    engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
    engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
    updateFieldDisplay();

    animateSwitchIn(true);
    animateSwitchIn(false);

    // --- CUSTOM INTRO DIALOGUE ---
    currentLog.push_back("Immortal Gabi Mircea: Ah, a student. Let's see if your algorithms are optimized.");
    currentLog.push_back("Immortal Gabi Mircea: Welcome to the true UBB FMI final exam!");
    flushLog(false);
}

SecretBossDialog::~SecretBossDialog() = default;

void SecretBossDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget* battleScene = new QWidget(this);
    battleScene->setFixedSize(520, 270);
    battleScene->setObjectName("battleScene");

    QPixmap bgImg("CatchPoke/bg.png");
    if (!bgImg.isNull()) {
        QPalette palette;
        palette.setBrush(QPalette::Window, QBrush(bgImg.scaled(520, 270, Qt::IgnoreAspectRatio)));
        battleScene->setAutoFillBackground(true);
        battleScene->setPalette(palette);
    }

    QLabel* enemyBaseLabel = new QLabel(battleScene);
    enemyBaseLabel->setGeometry(310, 110, 160, 60);
    QPixmap enemyBase("CatchPoke/enemy.png");
    if (!enemyBase.isNull()) enemyBaseLabel->setPixmap(enemyBase.scaled(160, 60, Qt::KeepAspectRatio));

    enemySprite = new QLabel(battleScene);
    enemySprite->setGeometry(330, 20, 120, 120);
    enemySprite->setAlignment(Qt::AlignCenter);

    QWidget* enemyHud = new QWidget(battleScene);
    enemyHud->setGeometry(20, 20, 210, 70);
    enemyHud->setObjectName("enemyHud");
    QVBoxLayout* eHudLayout = new QVBoxLayout(enemyHud);
    eHudLayout->setSpacing(2); eHudLayout->setContentsMargins(10, 8, 10, 8);

    enemyNameLabel = new QLabel();
    enemyNameLabel->setObjectName("hudName");
    enemyHpBar = new QProgressBar();
    enemyHpBar->setFixedHeight(10);
    enemyHpBar->setTextVisible(false);
    enemyHpText = new QLabel();
    enemyHpText->setObjectName("hudHp");
    eHudLayout->addWidget(enemyNameLabel);
    eHudLayout->addWidget(enemyHpBar);
    eHudLayout->addWidget(enemyHpText);

    QLabel* playerBaseLabel = new QLabel(battleScene);
    playerBaseLabel->setGeometry(30, 190, 160, 60);
    QPixmap playerBase("CatchPoke/player.png");
    if (!playerBase.isNull()) playerBaseLabel->setPixmap(playerBase.scaled(160, 60, Qt::KeepAspectRatio));

    playerSprite = new QLabel(battleScene);
    playerSprite->setGeometry(50, 100, 120, 120);
    playerSprite->setAlignment(Qt::AlignCenter);

    QWidget* playerHud = new QWidget(battleScene);
    playerHud->setGeometry(290, 180, 210, 70);
    playerHud->setObjectName("playerHud");
    QVBoxLayout* pHudLayout = new QVBoxLayout(playerHud);
    pHudLayout->setSpacing(2); pHudLayout->setContentsMargins(10, 8, 10, 8);

    playerNameLabel = new QLabel();
    playerNameLabel->setObjectName("hudName");
    playerHpBar = new QProgressBar();
    playerHpBar->setFixedHeight(10);
    playerHpBar->setTextVisible(false);
    playerHpText = new QLabel();
    playerHpText->setObjectName("hudHp");
    pHudLayout->addWidget(playerNameLabel);
    pHudLayout->addWidget(playerHpBar);
    pHudLayout->addWidget(playerHpText);

    mainLayout->addWidget(battleScene, 0, Qt::AlignHCenter);

    QWidget* controlArea = new QWidget(this);
    controlArea->setObjectName("controlArea");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlArea);
    controlLayout->setContentsMargins(15, 15, 15, 15);
    controlLayout->setSpacing(12);

    fieldInfoBtn = new QPushButton("FIELD IS CLEAR (CLICK FOR DETAILS)", this);
    fieldInfoBtn->setObjectName("fieldInfoBtn");
    fieldInfoBtn->setCursor(Qt::PointingHandCursor);
    controlLayout->addWidget(fieldInfoBtn);

    logWidget = new QListWidget(this);
    logWidget->setObjectName("logWidget");
    logWidget->setMinimumHeight(120);
    logWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    controlLayout->addWidget(logWidget);

    actionPanel = new QWidget();
    QGridLayout* actionGrid = new QGridLayout(actionPanel);
    actionGrid->setContentsMargins(0, 0, 0, 0);
    actionGrid->setSpacing(10);

    fightBtn = new QPushButton("FIGHT");
    fightBtn->setObjectName("actionBtn");
    fightBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    fightBtn->setMinimumHeight(45);

    switchBtn = new QPushButton("POKÉMON");
    switchBtn->setObjectName("actionBtn");
    switchBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    switchBtn->setMinimumHeight(45);

    megaBtn = new QPushButton(" MEGA EVOLVE");
    megaBtn->setObjectName("actionBtn");
    megaBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    megaBtn->setMinimumHeight(45);
    megaBtn->setIcon(QIcon("assets/Others/Original/Mega-Evolution-Sigil.png"));
    megaBtn->setIconSize(QSize(18, 18));

    runBtn = new QPushButton("RUN");
    runBtn->setObjectName("dangerBtn");
    runBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    runBtn->setMinimumHeight(45);

    actionGrid->addWidget(fightBtn, 0, 0); actionGrid->addWidget(switchBtn, 0, 1);
    actionGrid->addWidget(megaBtn, 1, 0); actionGrid->addWidget(runBtn, 1, 1);
    controlLayout->addWidget(actionPanel);

    movePanel = new QWidget();
    QGridLayout* moveGrid = new QGridLayout(movePanel);
    moveGrid->setContentsMargins(0, 0, 0, 0);
    moveGrid->setSpacing(10);

    auto styleMoveBtn = [](QPushButton* btn) {
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        btn->setMinimumHeight(45);
        };

    move1Btn = new QPushButton("-"); styleMoveBtn(move1Btn);
    move2Btn = new QPushButton("-"); styleMoveBtn(move2Btn);
    move3Btn = new QPushButton("-"); styleMoveBtn(move3Btn);
    move4Btn = new QPushButton("-"); styleMoveBtn(move4Btn);

    backToActionsBtn = new QPushButton("BACK");
    backToActionsBtn->setObjectName("neutralBtn");
    styleMoveBtn(backToActionsBtn);

    moveGrid->addWidget(move1Btn, 0, 0); moveGrid->addWidget(move2Btn, 0, 1);
    moveGrid->addWidget(move3Btn, 1, 0); moveGrid->addWidget(move4Btn, 1, 1);
    moveGrid->addWidget(backToActionsBtn, 2, 0, 1, 2);
    controlLayout->addWidget(movePanel);
    movePanel->hide();

    switchPanel = new QWidget();
    QVBoxLayout* switchLayout = new QVBoxLayout(switchPanel);
    switchLayout->setContentsMargins(0, 0, 0, 0);
    switchLayout->setSpacing(10);

    partyListWidget = new QListWidget();
    partyListWidget->setObjectName("logWidget");
    partyListWidget->setMinimumHeight(100);
    partyListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout* switchBtns = new QHBoxLayout();
    switchBtns->setSpacing(10);

    confirmSwitchBtn = new QPushButton("SEND OUT");
    confirmSwitchBtn->setObjectName("successBtn");
    confirmSwitchBtn->setMinimumHeight(40);

    cancelSwitchBtn = new QPushButton("CANCEL");
    cancelSwitchBtn->setObjectName("dangerBtn");
    cancelSwitchBtn->setMinimumHeight(40);

    switchBtns->addWidget(confirmSwitchBtn); switchBtns->addWidget(cancelSwitchBtn);
    switchLayout->addWidget(partyListWidget);
    switchLayout->addLayout(switchBtns);
    controlLayout->addWidget(switchPanel);
    switchPanel->hide();

    mainLayout->addWidget(controlArea);
    setWindowTitle("FINAL EXAM - UBB FMI");

    setMinimumSize(540, 680);
    resize(540, 720);
}

void SecretBossDialog::applyBossTheme() {
    // Intense Hacker / Abyssal Boss Theme
    QString bg = "#050000";
    QString ctrlBg = "#0a0101";
    QString textBase = "#ff3333";
    QString textMuted = "#992222";
    QString fieldColor = "#ff0000";
    QString hudBg = "rgba(10, 1, 1, 230)";
    QString hudBorderP = "#ff3333";
    QString hudBorderE = "#ffffff";
    QString btnBg = "#110202";
    QString btnBorder = "#aa0000";
    QString btnText = "#ff6666";
    QString btnHoverBg = "#440000";
    QString btnHoverBorder = "#ff0000";
    QString dangerBorder = "#eccc68";
    QString dangerHoverBg = "#592b2b";
    QString dangerHoverText = "#eccc68";
    QString successBorder = "#ff4757";
    QString successHoverBg = "#4a1c1c";
    QString successHoverText = "#ff6b81";

    this->setProperty("themeBtnBg", btnBg);
    this->setProperty("themeBtnBorder", btnBorder);
    this->setProperty("themeBtnTextMuted", textMuted);

    QString qss = QString(R"(
        QDialog { background-color: %1; font-family: 'Consolas', monospace; }
        QWidget#controlArea { background-color: %2; border-top: 2px solid %8; }
        QWidget#playerHud { background-color: %6; border: 2px solid %7; border-radius: 6px; }
        QWidget#enemyHud { background-color: %6; border: 2px solid %16; border-radius: 6px; }
        QLabel#hudName { font-weight: 800; font-size: 13px; color: %3; background: transparent; }
        QLabel#hudHp { font-size: 11px; font-weight: bold; color: %4; background: transparent; }
        QPushButton#fieldInfoBtn { background-color: %10; color: %5; font-size: 13px; font-weight: 800; border: 1px solid %5; border-radius: 6px; padding: 8px; }
        QPushButton#fieldInfoBtn:hover { background-color: %5; color: %2; }
        QListWidget#logWidget { background-color: %9; color: %11; border: 1px solid %8; border-radius: 6px; padding: 6px; font-family: 'Consolas', monospace; font-size: 12px; }
        QListWidget::item:selected { background-color: %12; color: #ffffff; }
        QPushButton { background-color: %9; color: %11; font-size: 13px; font-weight: 800; padding: 10px; border: 1px solid %8; border-radius: 6px; }
        QPushButton:hover { background-color: %10; border: 1px solid %12; color: %3; }
        QPushButton#actionBtn:hover { background-color: %10; border: 1px solid %12; }
        QPushButton#neutralBtn:hover { background-color: %10; border: 1px solid %4; color: %4; }
        QPushButton#dangerBtn:hover { border: 1px solid %13; background-color: %14; color: %17; }
        QPushButton#successBtn:hover { border: 1px solid %18; background-color: %19; color: %20; }
    )").arg(bg, ctrlBg, textBase, textMuted, fieldColor, hudBg, hudBorderP, btnBorder, btnBg, btnHoverBg, btnText, btnHoverBorder, dangerBorder, dangerHoverBg, "6px", hudBorderE, dangerHoverText, successBorder, successHoverBg, successHoverText);

    this->setStyleSheet(qss);
}

void SecretBossDialog::connectSignals() {
    connect(fightBtn, &QPushButton::clicked, this, &SecretBossDialog::onFightClicked);
    connect(switchBtn, &QPushButton::clicked, this, &SecretBossDialog::onSwitchClicked);
    connect(megaBtn, &QPushButton::clicked, this, &SecretBossDialog::onMegaClicked);
    connect(runBtn, &QPushButton::clicked, this, &SecretBossDialog::onRunClicked);
    connect(backToActionsBtn, &QPushButton::clicked, [this]() { movePanel->hide(); actionPanel->show(); });
    connect(cancelSwitchBtn, &QPushButton::clicked, this, &SecretBossDialog::onCancelSwitchClicked);
    connect(confirmSwitchBtn, &QPushButton::clicked, this, &SecretBossDialog::onConfirmSwitchClicked);
    connect(partyListWidget, &QListWidget::itemDoubleClicked, this, &SecretBossDialog::onPartyItemDoubleClicked);
    connect(fieldInfoBtn, &QPushButton::clicked, this, &SecretBossDialog::onFieldInfoClicked);

    connect(move1Btn, &QPushButton::clicked, [this]() { onMoveClicked(0); });
    connect(move2Btn, &QPushButton::clicked, [this]() { onMoveClicked(1); });
    connect(move3Btn, &QPushButton::clicked, [this]() { onMoveClicked(2); });
    connect(move4Btn, &QPushButton::clicked, [this]() { onMoveClicked(3); });
}

// =========================================================
// CUSTOM BOSS TRIGGERS & QUIZ LOGIC
// =========================================================
void SecretBossDialog::triggerOOPQuiz() {
    OOPQuizDialog quiz(this);
    int result = quiz.exec();

    std::vector<std::string> stats = { "atk", "def", "spa", "spd", "spe" };
    std::string chosenStat1 = stats[rand() % stats.size()];
    std::string chosenStat2 = stats[rand() % stats.size()];
    Pokemon& p = playerParty[currentPlayerIndex];

    if (result == QDialog::Accepted) {
        currentLog.push_back("Immortal Gabi Mircea: Correct! Your object allocation is flawless.");
        p.modifyStat(chosenStat1, 2, currentLog);
        p.modifyStat(chosenStat2, 2, currentLog);
    }
    else {
        currentLog.push_back("Immortal Gabi Mircea: Wrong! Did you even read the documentation?!");
        p.modifyStat(chosenStat1, -2, currentLog);
    }

    // --- SPRITE REFRESH FIX ---
    playerSprite->show();
    enemySprite->show();
    updateBattleDisplay();
    // --------------------------
}

void SecretBossDialog::checkBossTriggers() {
    int aliveCount = 0;
    for (const auto& ep : enemyParty) {
        if (ep.getCurrentHp() > 0) aliveCount++;
    }

    if (aliveCount <= 3 && !hasShownHalfwayDialogue) {
        hasShownHalfwayDialogue = true;
        currentLog.push_back("Immortal Gabi Mircea: Your code is inefficient! Memory leak detected!");
    }

    if (aliveCount == 1 && !hasShownLastMonDialogue) {
        hasShownLastMonDialogue = true;
        currentLog.push_back("Immortal Gabi Mircea: Segmentation fault (core dumped)... I will manually override your runtime!");
    }
}

// =========================================================
// BATTLE ENGINE ROUTING
// =========================================================

void SecretBossDialog::flushLog(bool endOfTurn) {
    if (currentLog.empty()) return;

    QString block;
    if (isNewTurn) {
        block += QString("=== Turn %1 ===\n").arg(turnCounter);
        isNewTurn = false;
    }
    else if (isMidTurnSwitch) {
        block += QString("=== Turn %1 (Cont.) ===\n").arg(turnCounter);
    }

    for (const auto& s : currentLog) block += QString::fromStdString(s) + "\n";

    QListWidgetItem* item = new QListWidgetItem(block.trimmed());
    logWidget->insertItem(0, item);
    currentLog.clear();

    if (endOfTurn) { turnCounter++; isNewTurn = true; }
}

QString SecretBossDialog::getTypeColor(const std::string& type) {
    if (type == "Normal") return "#A8A77A"; if (type == "Fire") return "#EE8130";
    if (type == "Water") return "#6390F0"; if (type == "Electric") return "#F7D02C";
    if (type == "Grass") return "#7AC74C"; if (type == "Ice") return "#96D9D6";
    if (type == "Fighting") return "#C22E28"; if (type == "Poison") return "#A33EA1";
    if (type == "Ground") return "#E2BF65"; if (type == "Flying") return "#A98FF3";
    if (type == "Psychic") return "#F95587"; if (type == "Bug") return "#A6B91A";
    if (type == "Rock") return "#B6A136"; if (type == "Ghost") return "#735797";
    if (type == "Dragon") return "#6F35FC"; if (type == "Dark") return "#705848";
    if (type == "Steel") return "#B7B7CE"; if (type == "Fairy") return "#D685AD";
    return "#34495e";
}

void SecretBossDialog::updateFieldDisplay() {
    const FieldState& fs = engine->getFieldState();
    Pokemon& p = playerParty[currentPlayerIndex];
    Pokemon& e = enemyParty[currentEnemyIndex];
    std::string activeW = engine->getActiveWeather(p, e);

    QString txt = "";
    if (fs.weather != "None") {
        if (activeW == "None") txt += QString::fromStdString(fs.weather).toUpper() + " (SUPPRESSED) | ";
        else txt += QString::fromStdString(fs.weather).toUpper() + " | ";
    }
    if (fs.terrain != "None") txt += QString::fromStdString(fs.terrain).toUpper() + " | ";

    if (txt.isEmpty()) txt = "FIELD: CLEAR";
    else { txt.chop(3); txt = "FIELD: " + txt; }

    fieldInfoBtn->setText(txt + " (CLICK FOR DETAILS)");
}

void SecretBossDialog::onFieldInfoClicked() {
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Dialog);
    dialog.setFixedSize(450, 400);
    dialog.setStyleSheet(this->styleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    QLabel* title = new QLabel("ACTIVE BATTLE EFFECTS");
    title->setStyleSheet("font-size: 16px; font-weight: 900; color: " + this->property("themeBtnBorder").toString() + "; border: none;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QListWidget* list = new QListWidget();
    list->setObjectName("logWidget");
    list->setSelectionMode(QAbstractItemView::NoSelection);
    list->setStyleSheet("QListWidget { font-size: 14px; font-weight: bold; }");

    const FieldState& fs = engine->getFieldState();
    const SideState& ps = engine->getPlayerSide();
    const SideState& es = engine->getEnemySide();
    Pokemon& p = playerParty[currentPlayerIndex];
    Pokemon& e = enemyParty[currentEnemyIndex];
    std::string activeW = engine->getActiveWeather(p, e);

    auto addItem = [&](const QString& text, const QString& colorHex) {
        QListWidgetItem* item = new QListWidgetItem(text);
        if (!colorHex.isEmpty()) item->setForeground(QColor(colorHex));
        list->addItem(item);
        };

    QString textBase = this->property("themeBtnTextMuted").toString();

    if (fs.weather != "None") {
        QString wStr = QString("Weather: %1").arg(QString::fromStdString(fs.weather));
        if (activeW == "None") wStr += " (Suppressed by Cloud Nine/Air Lock)";
        addItem(wStr, "#e67e22");
    }
    if (fs.terrain != "None") addItem(QString("Terrain: %1").arg(QString::fromStdString(fs.terrain)), "#27ae60");
    if (fs.trickRoom > 0) addItem(QString("Trick Room (%1 turns left)").arg(fs.trickRoom), "#9b59b6");
    if (fs.magicRoom > 0) addItem(QString("Magic Room (%1 turns left)").arg(fs.magicRoom), "#9b59b6");
    if (fs.wonderRoom > 0) addItem(QString("Wonder Room (%1 turns left)").arg(fs.wonderRoom), "#9b59b6");
    if (fs.gravity > 0) addItem(QString("Gravity (%1 turns left)").arg(fs.gravity), "#34495e");

    QString pHaz = "Your Side: ";
    bool pHasHaz = false;
    if (ps.stealthRock) { pHaz += "Stealth Rock, "; pHasHaz = true; }
    if (ps.spikes > 0) { pHaz += QString("Spikes (x%1), ").arg(ps.spikes); pHasHaz = true; }
    if (ps.toxicSpikes > 0) { pHaz += QString("Toxic Spikes (x%1), ").arg(ps.toxicSpikes); pHasHaz = true; }
    if (ps.stickyWeb) { pHaz += "Sticky Web, "; pHasHaz = true; }
    if (pHasHaz) { pHaz.chop(2); addItem(pHaz, "#3498db"); }

    QString eHaz = "Enemy Side: ";
    bool eHasHaz = false;
    if (es.stealthRock) { eHaz += "Stealth Rock, "; eHasHaz = true; }
    if (es.spikes > 0) { eHaz += QString("Spikes (x%1), ").arg(es.spikes); eHasHaz = true; }
    if (es.toxicSpikes > 0) { eHaz += QString("Toxic Spikes (x%1), ").arg(es.toxicSpikes); eHasHaz = true; }
    if (es.stickyWeb) { eHaz += "Sticky Web, "; eHasHaz = true; }
    if (eHasHaz) { eHaz.chop(2); addItem(eHaz, "#e74c3c"); }

    if (list->count() == 0) addItem("The field is completely clear.", textBase);

    layout->addWidget(list);

    QPushButton* closeBtn = new QPushButton("CLOSE");
    closeBtn->setObjectName("neutralBtn");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    dialog.exec();
}

void SecretBossDialog::updateBattleDisplay() {
    Pokemon& p = playerParty[currentPlayerIndex];
    Pokemon& e = enemyParty[currentEnemyIndex];

    playerNameLabel->setText(QString("%1 Lv.%2").arg(QString::fromStdString(p.getNickname())).arg(p.getLevel()));

    playerHpBar->setMaximum(p.getHp());
    if (lastPlayerHp == -1) {
        lastPlayerHp = p.getCurrentHp();
        animateHpBar(true, lastPlayerHp, lastPlayerHp, p.getHp(), p.getStatus());
    }
    else if (lastPlayerHp != p.getCurrentHp()) {
        animateHpBar(true, lastPlayerHp, p.getCurrentHp(), p.getHp(), p.getStatus());
        lastPlayerHp = p.getCurrentHp();
    }
    else animateHpBar(true, p.getCurrentHp(), p.getCurrentHp(), p.getHp(), p.getStatus());

    QString pId = QString("%1").arg(p.getSpeciesId(), 4, 10, QChar('0'));
    QString pBase = QString::fromStdString(p.getName());
    QString pPath = QString("assets/images/%1.png").arg(pId);
    if (pBase.contains("-")) {
        QString specific = QString("assets/images/%1-%2.png").arg(pId).arg(pBase.mid(pBase.indexOf('-') + 1));
        if (QFile::exists(specific)) pPath = specific;
    }
    if (!QFile::exists(pPath)) pPath = "Items/000.png";
    QPixmap pSprite(pPath);
    if (!pSprite.isNull()) playerSprite->setPixmap(QPixmap::fromImage(pSprite.toImage().mirrored(true, false)).scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    enemyNameLabel->setText(QString("%1 Lv.%2").arg(QString::fromStdString(e.getName())).arg(e.getLevel()));
    enemyHpBar->setMaximum(e.getHp());
    if (lastEnemyHp == -1) {
        lastEnemyHp = e.getCurrentHp();
        animateHpBar(false, lastEnemyHp, lastEnemyHp, e.getHp(), e.getStatus());
    }
    else if (lastEnemyHp != e.getCurrentHp()) {
        animateHpBar(false, lastEnemyHp, e.getCurrentHp(), e.getHp(), e.getStatus());
        lastEnemyHp = e.getCurrentHp();
    }
    else animateHpBar(false, e.getCurrentHp(), e.getCurrentHp(), e.getHp(), e.getStatus());

    QString eId = QString("%1").arg(e.getSpeciesId(), 4, 10, QChar('0'));
    QString eBase = QString::fromStdString(e.getName());
    QString ePath = QString("assets/images/%1.png").arg(eId);
    if (eBase.contains("-")) {
        QString specific = QString("assets/images/%1-%2.png").arg(eId).arg(eBase.mid(eBase.indexOf('-') + 1));
        if (QFile::exists(specific)) ePath = specific;
    }
    if (!QFile::exists(ePath)) ePath = "Items/000.png";
    QPixmap eSprite(ePath);
    if (!eSprite.isNull()) enemySprite->setPixmap(eSprite.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    auto moves = p.getMoves();
    int validCount = 0;
    QString btnBg = this->property("themeBtnBg").toString();
    QString btnBorder = this->property("themeBtnBorder").toString();
    QString textMuted = this->property("themeBtnTextMuted").toString();

    auto updateMoveBtn = [&](QPushButton* btn, const std::string& moveId, bool forceValid = false) {
        if (!moveId.empty() && moveId != "None") {
            MoveData md = controller.getMoveData(moveId);

            // --- FIX: Normalize strings for proper comparison ---
            std::string normLastMove = "";
            for (char c : p.lastMoveUsed) { if (c != ' ' && c != '-' && c != '\'') normLastMove += std::tolower(c); }
            std::string normMoveId = "";
            for (char c : moveId) { if (c != ' ' && c != '-' && c != '\'') normMoveId += std::tolower(c); }

            bool disabled = false;
            if (p.getMovePP(moveId) <= 0) disabled = true;
            if (p.hasVolatile("torment") && normLastMove == normMoveId) disabled = true;

            if (e.hasVolatile("imprison")) {
                for (auto& em : e.getMoves()) {
                    std::string normEm = "";
                    for (char c : em) { if (c != ' ' && c != '-' && c != '\'') normEm += std::tolower(c); }
                    if (normEm == normMoveId) disabled = true;
                }
            }

            std::string rawItem = p.getHeldItem();
            std::string normalizedItem = "";
            for (char c : rawItem) { if (c != ' ') normalizedItem += std::tolower(c); }

            if (normalizedItem == "assaultvest" && md.category == "Status") {
                disabled = true;
            }

            // Choice Item Lock
            if ((normalizedItem == "choiceband" || normalizedItem == "choicespecs" || normalizedItem == "choicescarf") &&
                !p.lastMoveUsed.empty() && normMoveId != normLastMove) {
                disabled = true;
            }
            // ----------------------------------------------------

            if (forceValid) disabled = false;

            btn->setIcon(QIcon("assets/Others/damage-category-icons/1x/" + QString::fromStdString(md.category) + "-white.png"));
            btn->setIconSize(QSize(24, 12));

            if (forceValid && moveId == "struggle") {
                btn->setText("  STRUGGLE\n  PP: --/--");
            }
            else {
                btn->setText("  " + QString::fromStdString(md.name).toUpper() + "\n  PP: " + QString::number(p.getMovePP(moveId)) + "/" + QString::number(p.getMaxMovePP(moveId)));
            }

            if (disabled) {
                btn->setStyleSheet(QString("QPushButton { background-color: %1; color: %2; border: 1px solid %3; opacity: 0.6; }").arg(btnBg, textMuted, btnBorder));
                btn->setEnabled(false);
            }
            else {
                QString tColor = getTypeColor(md.type);
                btn->setStyleSheet(QString(R"(
                    QPushButton { border: 2px solid %1; color: %1; }
                    QPushButton:hover { background-color: %1; color: #ffffff; }
                    QPushButton:pressed { background-color: %1; color: #ffffff; }
                )").arg(tColor));
                btn->setEnabled(true);
                validCount++;
            }
        }
        else {
            btn->setIcon(QIcon());
            btn->setText("-");
            btn->setStyleSheet(QString("QPushButton { background: transparent; color: transparent; border: 1px dashed %1; }").arg(btnBorder));
            btn->setEnabled(false);
        }
        };

    updateMoveBtn(move1Btn, moves.size() > 0 ? moves[0] : "");
    updateMoveBtn(move2Btn, moves.size() > 1 ? moves[1] : "");
    updateMoveBtn(move3Btn, moves.size() > 2 ? moves[2] : "");
    updateMoveBtn(move4Btn, moves.size() > 3 ? moves[3] : "");

    if (validCount == 0 && moves.size() > 0) {
        updateMoveBtn(move1Btn, "struggle", true);
        move1Btn->setProperty("struggle_override", true);
    }
    else {
        move1Btn->setProperty("struggle_override", false);
    }
}

void SecretBossDialog::onFightClicked() {
    Pokemon& p = playerParty[currentPlayerIndex];
    if (!p.getChargingMove().empty()) {
        currentLog.push_back(p.getNickname() + " is preparing its attack!");
        beginTurn(p.getChargingMove());
    }
    else if (p.getLockedTurns() > 0) {
        currentLog.push_back(p.getNickname() + " is locked in!");
        beginTurn(p.getLockedMove());
    }
    else {
        actionPanel->hide();
        movePanel->show();
    }
}

void SecretBossDialog::onSwitchClicked() {
    Pokemon& p = playerParty[currentPlayerIndex];
    if (!p.getChargingMove().empty() || p.getLockedTurns() > 0) {
        currentLog.push_back(p.getNickname() + " is locked in and cannot switch!");
        flushLog(false);
        return;
    }
    partyListWidget->clear();
    for (size_t i = 0; i < playerParty.size(); i++) {
        const Pokemon& poke = playerParty[i];
        QString status = poke.getCurrentHp() <= 0 ? "[FNT] " : "";
        int cur = poke.getCurrentHp();
        int max = poke.getHp();
        int pct = (max > 0) ? (cur * 100) / max : 0;
        QString text = QString("%1%2 - %3 (HP: %4/%5 - %6%)").arg(status).arg(QString::fromStdString(poke.getNickname())).arg(QString::fromStdString(poke.getName())).arg(cur).arg(max).arg(pct);
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, static_cast<int>(i));
        if (i == currentPlayerIndex || playerParty[i].getCurrentHp() <= 0) item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        partyListWidget->addItem(item);
    }
    actionPanel->hide();
    switchPanel->show();
}

void SecretBossDialog::onPartyItemDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    SummaryDialog dialog(controller, playerParty[item->data(Qt::UserRole).toInt()].getId(), this);
    dialog.exec();
}

void SecretBossDialog::onCancelSwitchClicked() { switchPanel->hide(); actionPanel->show(); }

void SecretBossDialog::onConfirmSwitchClicked() {
    if (!partyListWidget->currentItem()) return;
    int targetIdx = partyListWidget->currentItem()->data(Qt::UserRole).toInt();
    switchPanel->hide();
    actionPanel->show();
    executeSwitch(targetIdx);
}

void SecretBossDialog::onMegaClicked() {
    Pokemon& p = playerParty[currentPlayerIndex];
    if (!p.getChargingMove().empty() || p.getLockedTurns() > 0) {
        currentLog.push_back(p.getNickname() + " is locked in and cannot Mega Evolve now!");
        flushLog(false);
        return;
    }

    if (isPlayerMegaEvolved) { currentLog.push_back("You have already Mega Evolved a Pokémon!"); flushLog(false); return; }

    std::string item = playerParty[currentPlayerIndex].getHeldItem();
    bool hasDragonAscent = false;
    for (const auto& m : p.getMoves()) { if (m == "dragonascent") hasDragonAscent = true; }

    if (QString::fromStdString(item).contains("ITE", Qt::CaseInsensitive) || (p.getName() == "Rayquaza" && hasDragonAscent)) {
        megaEvolveNextMove = true;
        megaBtn->setStyleSheet(R"(QPushButton { background-color: #8e44ad; color: #ffffff; font-size: 13px; font-weight: 800; padding: 10px; border: 1px solid #6c3483; border-radius: 6px; letter-spacing: 1px; })");
        if (p.getName() == "Rayquaza") currentLog.push_back("A fervent wish has reached " + p.getNickname() + "!");
        else currentLog.push_back("Your Mega Bracelet is reacting to " + item + "!");
        flushLog(false);
    }
    else {
        currentLog.push_back(p.getNickname() + " cannot Mega Evolve right now!");
        flushLog(false);
    }
}

void SecretBossDialog::onRunClicked() {
    currentLog.push_back("Immortal Gabi Mircea: There are no 'break' statements in my exam! You cannot flee!");
    flushLog(false);
}

void SecretBossDialog::onMoveClicked(int moveIndex) {
    movePanel->hide();
    actionPanel->show();
    Pokemon& p = playerParty[currentPlayerIndex];
    if (moveIndex == 0 && move1Btn->property("struggle_override").toBool()) { beginTurn("struggle"); return; }
    if (moveIndex >= p.getMoves().size()) return;
    beginTurn(p.getMoves()[moveIndex]);
}

void SecretBossDialog::beginTurn(const std::string& playerMove) {
    queuedPlayerMove = playerMove;
    Pokemon& p = playerParty[currentPlayerIndex];
    Pokemon& e = enemyParty[currentEnemyIndex];
    currentLog.clear();

    auto handleMegaForm = [&](Pokemon& pkmn, bool isPlayer) {
        std::string megaName = pkmn.getName() + "-Mega";
        std::string rawItem = pkmn.getHeldItem();
        std::string item = "";
        for (char c : rawItem) { if (c != ' ' && c != '-' && c != '\'') item += std::toupper(c); }

        if (item == "CHARIZARDITEX") megaName = "Charizard-Mega-X";
        else if (item == "CHARIZARDITEY") megaName = "Charizard-Mega-Y";
        else if (item == "MEWTWONITEX") megaName = "Mewtwo-Mega-X";
        else if (item == "MEWTWONITEY") megaName = "Mewtwo-Mega-Y";
        else if (item == "ABSOLITEZ") megaName = "Absol-Mega-Z";
        else if (item == "GARCHOMPITEZ") megaName = "Garchomp-Mega-Z";
        else if (item == "LUCARIONITEZ") megaName = "Lucario-Mega-Z";

        if (pkmn.getName() == "Rayquaza") megaName = "Rayquaza-Mega";

        Pokemon megaForm = controller.getSpeciesDataByName(megaName);
        if (megaForm.getName() != "") {
            pkmn.megaEvolve(megaForm);
            currentLog.push_back(pkmn.getNickname() + " Mega Evolved into " + megaForm.getName() + "!");

            animateMegaEvolution(isPlayer);

            if (isPlayer) isPlayerMegaEvolved = true;
            else isEnemyMegaEvolved = true;

            engine->onSwitchIn(pkmn, isPlayer, currentLog, isPlayer ? &enemyParty[currentEnemyIndex] : &playerParty[currentPlayerIndex]);
            engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
            updateFieldDisplay();
        }
        else if (isPlayer) {
            currentLog.push_back("The Mega Stone failed to react! Form missing from DB.");
        }
        };

    if (megaEvolveNextMove) {
        handleMegaForm(p, true);
        megaEvolveNextMove = false;
        megaBtn->setStyleSheet("");
        megaBtn->setDisabled(true);
        updateBattleDisplay();
    }

    AI::AiDetector smartAI(controller);
    std::string decisionToken = smartAI.chooseBestAction(
        enemyParty, currentEnemyIndex, playerParty, currentPlayerIndex,
        !isEnemyMegaEvolved, !isPlayerMegaEvolved
    );

    if (decisionToken.rfind("switch:", 0) == 0) {
        int targetIndex = std::stoi(decisionToken.substr(7));
        Pokemon& oldEnemy = enemyParty[currentEnemyIndex];
        oldEnemy.isSwitchingOut = false;
        if (oldEnemy.hasVolatile("transform")) oldEnemy.fullyHeal();
        if (!oldEnemy.isBatonPassing) oldEnemy.resetStatStages();
        oldEnemy.isBatonPassing = false;

        currentEnemyIndex = targetIndex;
        currentLog.push_back("Immortal Gabi Mircea withdrew " + oldEnemy.getName() + "!");
        currentLog.push_back("Immortal Gabi Mircea sent out " + enemyParty[currentEnemyIndex].getName() + "!");
        lastEnemyHp = -1;

        engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
        engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);

        updateBattleDisplay();
        animateSwitchIn(false);
        queuedEnemyMove = "SKIP_TURN";
    }
    else {
        std::string eMove = decisionToken;
        if (decisionToken.rfind("mega:", 0) == 0) {
            handleMegaForm(e, false);
            eMove = decisionToken.substr(5);
        }
        else if (decisionToken.rfind("move:", 0) == 0) {
            eMove = decisionToken.substr(5);
        }

        // --- FIX: Choice Item Lock for AI ---
        std::string eRawItem = e.getHeldItem();
        std::string eItem = "";
        for (char c : eRawItem) { if (c != ' ') eItem += std::tolower(c); }

        if ((eItem == "choiceband" || eItem == "choicespecs" || eItem == "choicescarf") && !e.lastMoveUsed.empty()) {
            std::string normLastMove = "";
            for (char c : e.lastMoveUsed) { if (c != ' ' && c != '-' && c != '\'') normLastMove += std::tolower(c); }
            eMove = normLastMove;
            if (e.getMovePP(eMove) <= 0) eMove = "struggle";
        }
        // ------------------------------------

        queuedEnemyMove = eMove;
    }

    MoveData m1 = controller.getMoveData(queuedPlayerMove);
    MoveData m2 = controller.getMoveData(queuedEnemyMove);

    int p1Pri = m1.priority;
    int p2Pri = m2.priority;
    if (p.getAbility() == "Prankster" && m1.category == "Status") p1Pri++;
    if (e.getAbility() == "Prankster" && m2.category == "Status") p2Pri++;
    if (p.getAbility() == "Gale Wings" && m1.type == "Flying" && p.getCurrentHp() == p.getHp()) p1Pri++;
    if (e.getAbility() == "Gale Wings" && m2.type == "Flying" && e.getCurrentHp() == e.getHp()) p2Pri++;

    if (p1Pri > p2Pri) playerGoesFirst = true;
    else if (p2Pri > p1Pri) playerGoesFirst = false;
    else {
        int p1Speed = engine->getEffectiveSpeed(p, e, true);
        int p2Speed = engine->getEffectiveSpeed(e, p, false);
        if (engine->getFieldState().trickRoom > 0) {
            if (p1Speed < p2Speed) playerGoesFirst = true;
            else if (p2Speed < p1Speed) playerGoesFirst = false;
            else playerGoesFirst = (rand() % 2 == 0);
        }
        else {
            if (p1Speed > p2Speed) playerGoesFirst = true;
            else if (p2Speed > p1Speed) playerGoesFirst = false;
            else playerGoesFirst = (rand() % 2 == 0);
        }
    }

    if (playerGoesFirst && !m2.isProtectMove) e.resetProtectCounter();
    if (!playerGoesFirst && !m1.isProtectMove) p.resetProtectCounter();

    currentPhase = TurnPhase::FirstMove;
    executeNextPhase();
}

void SecretBossDialog::executeNextPhase() {
    Pokemon& p = playerParty[currentPlayerIndex];
    Pokemon& e = enemyParty[currentEnemyIndex];

    auto handleForcedRandomSwitch = [&](Pokemon& target, bool targetIsPlayer) {
        target.isForcedRandomSwitch = false;
        target.isSwitchingOut = false;

        auto& party = targetIsPlayer ? playerParty : enemyParty;
        int& currentIndex = targetIsPlayer ? currentPlayerIndex : currentEnemyIndex;

        bool hasMore = false;
        std::vector<int> alive;
        for (size_t i = 0; i < party.size(); i++) {
            if (i != currentIndex && party[i].getCurrentHp() > 0) {
                hasMore = true;
                alive.push_back(i);
            }
        }

        if (hasMore) {
            int randIdx = alive[rand() % alive.size()];

            if (target.hasVolatile("transform")) target.fullyHeal();
            target.resetStatStages();
            target.isBatonPassing = false;

            currentIndex = randIdx;

            if (targetIsPlayer) {
                currentLog.push_back(target.getNickname() + " was dragged out! Go! " + party[currentIndex].getNickname() + "!");
                engine->onSwitchIn(party[currentIndex], true, currentLog, &enemyParty[currentEnemyIndex]);
                lastPlayerHp = -1;
                queuedPlayerMove = "SKIP_TURN";
                updateBattleDisplay();
                animateSwitchIn(true);
            }
            else {
                currentLog.push_back(target.getName() + " was forced out! Immortal Gabi Mircea sent out " + party[currentIndex].getName() + "!");
                engine->onSwitchIn(party[currentIndex], false, currentLog, &playerParty[currentPlayerIndex]);
                lastEnemyHp = -1;
                queuedEnemyMove = "SKIP_TURN";
                updateBattleDisplay();
                animateSwitchIn(false);
            }
            engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
            updateFieldDisplay();
        }
        };

    auto handleManualSwitch = [&](Pokemon& target, bool targetIsPlayer) {
        if (targetIsPlayer) {
            bool hasMore = false;
            for (size_t i = 0; i < playerParty.size(); i++) {
                if (i != currentPlayerIndex && playerParty[i].getCurrentHp() > 0) { hasMore = true; break; }
            }
            if (hasMore) {
                engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
                flushLog(false);
                updateBattleDisplay();
                actionPanel->hide();
                onSwitchClicked();
                cancelSwitchBtn->setDisabled(true);
                isMidTurnSwitch = true;
                queuedPlayerMove = "SKIP_TURN";
                return true;
            }
            else {
                target.isSwitchingOut = false;
            }
        }
        else {
            executeEnemySwitch();
            queuedEnemyMove = "SKIP_TURN";
        }
        return false;
        };

    if (currentPhase == TurnPhase::FirstMove) {
        currentPhase = TurnPhase::SecondMove;
        Pokemon& attacker = playerGoesFirst ? p : e;
        Pokemon& defender = playerGoesFirst ? e : p;
        std::string move = playerGoesFirst ? queuedPlayerMove : queuedEnemyMove;
        std::string defMove = playerGoesFirst ? queuedEnemyMove : queuedPlayerMove;
        bool isPlayer = playerGoesFirst;

        if (move != "SKIP_TURN" && attacker.getCurrentHp() > 0 && defender.getCurrentHp() > 0) {
            MoveData md = controller.getMoveData(move);
            auto animCallback = [this, md](bool isP) { this->animateMoveEffect(isP, md.type, md.category); };
            engine->executeMove(attacker, defender, move, isPlayer, currentLog, defMove, animCallback);
            updateFieldDisplay();

            bool paused = false;
            if (defender.isSwitchingOut) {
                if (defender.isForcedRandomSwitch) handleForcedRandomSwitch(defender, !isPlayer);
                else paused = handleManualSwitch(defender, !isPlayer);
            }
            if (!paused && attacker.isSwitchingOut) {
                if (attacker.isForcedRandomSwitch) handleForcedRandomSwitch(attacker, isPlayer);
                else paused = handleManualSwitch(attacker, isPlayer);
            }
            if (paused) return;
        }
        executeNextPhase();
    }
    else if (currentPhase == TurnPhase::SecondMove) {
        currentPhase = TurnPhase::EndTurn;
        Pokemon& attacker = playerGoesFirst ? e : p;
        Pokemon& defender = playerGoesFirst ? p : e;
        std::string move = playerGoesFirst ? queuedEnemyMove : queuedPlayerMove;
        std::string defMove = playerGoesFirst ? queuedPlayerMove : queuedEnemyMove;
        bool isPlayer = !playerGoesFirst;

        if (move != "SKIP_TURN" && attacker.getCurrentHp() > 0 && defender.getCurrentHp() > 0) {
            MoveData md = controller.getMoveData(move);
            auto animCallback = [this, md](bool isP) { this->animateMoveEffect(isP, md.type, md.category); };
            engine->executeMove(attacker, defender, move, isPlayer, currentLog, defMove, animCallback);
            updateFieldDisplay();

            bool paused = false;
            if (defender.isSwitchingOut) {
                if (defender.isForcedRandomSwitch) handleForcedRandomSwitch(defender, !isPlayer);
                else paused = handleManualSwitch(defender, !isPlayer);
            }
            if (!paused && attacker.isSwitchingOut) {
                if (attacker.isForcedRandomSwitch) handleForcedRandomSwitch(attacker, isPlayer);
                else paused = handleManualSwitch(attacker, isPlayer);
            }
            if (paused) return;
        }
        executeNextPhase();
    }
    else if (currentPhase == TurnPhase::EndTurn) {
        engine->runEndOfTurn(p, e, currentLog);
        checkBossTriggers();

        // --- NEW: MID-BATTLE OOP EXAM ---
        if (turnCounter > 0 && turnCounter % 3 == 0) {
            triggerOOPQuiz();
        }
        // ---------------------------------

        currentPhase = TurnPhase::Idle;
        isMidTurnSwitch = false;
        flushLog(true);
        updateBattleDisplay();
        checkFaint();
    }
}

void SecretBossDialog::executeSwitch(int newIndex) {
    currentPlayerIndex = newIndex;
    currentLog.push_back("Go! " + playerParty[newIndex].getNickname() + "!");
    engine->onSwitchIn(playerParty[newIndex], true, currentLog, &enemyParty[currentEnemyIndex]);
    lastPlayerHp = -1;
    updateBattleDisplay();
    animateSwitchIn(true);
}

void SecretBossDialog::executeEnemySwitch() {
    for (size_t i = 0; i < enemyParty.size(); i++) {
        if (enemyParty[i].getCurrentHp() > 0 && i != currentEnemyIndex) {
            currentEnemyIndex = i;
            currentLog.push_back("Immortal Gabi Mircea sent out " + enemyParty[currentEnemyIndex].getName() + "!");
            engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
            lastEnemyHp = -1;
            updateBattleDisplay();
            animateSwitchIn(false);
            break;
        }
    }
}

bool SecretBossDialog::checkFaint() {
    if (enemyParty[currentEnemyIndex].getCurrentHp() <= 0) {
        currentLog.push_back("Opponent " + enemyParty[currentEnemyIndex].getName() + " fainted!");
        flushLog(false);
        animateFaint(false);

        bool enemyHasMore = false;
        for (const auto& ep : enemyParty) if (ep.getCurrentHp() > 0) enemyHasMore = true;

        if (!enemyHasMore) {
            QTimer::singleShot(1000, this, [this]() {
                currentLog.push_back("Immortal Gabi Mircea: 10/10. Your code compiled flawlessly. You pass.");
                flushLog(false);
                QMessageBox::information(this, "Victory!", "10/10. Your code compiled flawlessly. You pass.");
                this->accept();
                });
        }
        else {
            QTimer::singleShot(600, this, [this]() { executeEnemySwitch(); });
        }
        return true;
    }

    if (playerParty[currentPlayerIndex].getCurrentHp() <= 0) {
        currentLog.push_back(playerParty[currentPlayerIndex].getNickname() + " fainted!");
        flushLog(false);
        animateFaint(true);

        bool playerHasMore = false;
        for (const auto& poke : playerParty) if (poke.getCurrentHp() > 0) playerHasMore = true;

        if (!playerHasMore) {
            QTimer::singleShot(1000, this, [this]() {
                currentLog.push_back("Immortal Gabi Mircea: Compilation failed. See you at the autumn retake session.");
                flushLog(false);
                QMessageBox::critical(this, "Defeat", "You have failed the final exam.");
                this->reject();
                });
        }
        else {
            QTimer::singleShot(600, this, [this]() {
                actionPanel->hide();
                onSwitchClicked();
                cancelSwitchBtn->setDisabled(true);
                });
        }
        return true;
    }
    return false;
}

void SecretBossDialog::animateMoveEffect(bool isPlayerAttacker, const std::string& moveType, const std::string& category) {
    QLabel* attackerSprite = isPlayerAttacker ? playerSprite : enemySprite;
    QLabel* defenderSprite = isPlayerAttacker ? enemySprite : playerSprite;

    QPoint attackerBasePos = isPlayerAttacker ? QPoint(50, 100) : QPoint(330, 20);
    QPoint defenderBasePos = isPlayerAttacker ? QPoint(330, 20) : QPoint(50, 100);

    QPoint lungePos = isPlayerAttacker ? attackerBasePos + QPoint(30, -30) : attackerBasePos + QPoint(-30, 30);

    QPropertyAnimation* lungeAnim = new QPropertyAnimation(attackerSprite, "pos", this);
    lungeAnim->setDuration(350);
    lungeAnim->setStartValue(attackerBasePos);
    lungeAnim->setKeyValueAt(0.5, lungePos);
    lungeAnim->setEndValue(attackerBasePos);
    lungeAnim->setEasingCurve(QEasingCurve::InOutQuad);
    lungeAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QColor effectColor = QColor(200, 200, 200);
    if (moveType == "Fire") effectColor = QColor(255, 69, 0);
    else if (moveType == "Water") effectColor = QColor(52, 152, 219);
    else if (moveType == "Grass") effectColor = QColor(46, 204, 113);
    else if (moveType == "Electric") effectColor = QColor(241, 196, 15);
    else if (moveType == "Ice") effectColor = QColor(0, 255, 255);
    else if (moveType == "Psychic") effectColor = QColor(155, 89, 182);
    else if (moveType == "Dark" || moveType == "Ghost") effectColor = QColor(44, 62, 80);
    else if (moveType == "Fairy") effectColor = QColor(253, 121, 168);
    else if (moveType == "Poison") effectColor = QColor(142, 68, 173);
    else if (moveType == "Dragon") effectColor = QColor(52, 31, 151);
    else if (moveType == "Ground" || moveType == "Rock") effectColor = QColor(205, 133, 63);
    else if (moveType == "Flying") effectColor = QColor(135, 206, 235);
    else if (moveType == "Bug") effectColor = QColor(164, 196, 0);
    else if (moveType == "Fighting") effectColor = QColor(192, 48, 40);
    else if (moveType == "Steel") effectColor = QColor(183, 183, 206);

    QTimer::singleShot(150, [this, defenderSprite, effectColor]() {
        QGraphicsColorizeEffect* colorEffect = new QGraphicsColorizeEffect(defenderSprite);
        colorEffect->setColor(effectColor);
        colorEffect->setStrength(0.0);
        defenderSprite->setGraphicsEffect(colorEffect);

        QPropertyAnimation* flashAnim = new QPropertyAnimation(colorEffect, "strength", this);
        flashAnim->setDuration(450);
        flashAnim->setStartValue(0.0);
        flashAnim->setKeyValueAt(0.3, 0.85);
        flashAnim->setEndValue(0.0);

        connect(flashAnim, &QPropertyAnimation::finished, [defenderSprite, colorEffect]() {
            if (defenderSprite->graphicsEffect() == colorEffect) {
                defenderSprite->setGraphicsEffect(nullptr);
            }
            });
        flashAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });

    if (category == "Physical" || moveType == "Rock" || moveType == "Ground" || moveType == "Fighting") {
        QTimer::singleShot(170, [this, defenderSprite, defenderBasePos]() {
            QPropertyAnimation* shakeAnim = new QPropertyAnimation(defenderSprite, "pos", this);
            shakeAnim->setDuration(350);
            shakeAnim->setStartValue(defenderBasePos);
            shakeAnim->setKeyValueAt(0.2, defenderBasePos + QPoint(12, 0));
            shakeAnim->setKeyValueAt(0.4, defenderBasePos + QPoint(-12, 0));
            shakeAnim->setKeyValueAt(0.6, defenderBasePos + QPoint(8, 0));
            shakeAnim->setKeyValueAt(0.8, defenderBasePos + QPoint(-8, 0));
            shakeAnim->setEndValue(defenderBasePos);
            shakeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            });
    }
}

void SecretBossDialog::animateSwitchIn(bool isPlayer) {
    QLabel* sprite = isPlayer ? playerSprite : enemySprite;
    QPoint basePos = isPlayer ? QPoint(50, 100) : QPoint(330, 20);

    QPoint startPos = isPlayer ? basePos + QPoint(-200, 0) : basePos + QPoint(200, 0);

    QPropertyAnimation* anim = new QPropertyAnimation(sprite, "pos", this);
    anim->setDuration(400);
    anim->setStartValue(startPos);
    anim->setEndValue(basePos);
    anim->setEasingCurve(QEasingCurve::OutBack);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SecretBossDialog::animateHpBar(bool isPlayer, int startHp, int endHp, int maxHp, const std::string& status) {
    QProgressBar* bar = isPlayer ? playerHpBar : enemyHpBar;
    QLabel* textLabel = isPlayer ? playerHpText : enemyHpText;

    if (maxHp < 0) maxHp = 0;
    startHp = qBound(0, startHp, maxHp);
    endHp = qBound(0, endHp, maxHp);

    if (startHp == endHp) {
        bar->setValue(endHp);

        int pct = maxHp > 0 ? (endHp * 100) / maxHp : 0;
        QString color = pct > 50 ? "#2ecc71" : (pct > 20 ? "#f1c40f" : "#e74c3c");
        bar->setStyleSheet(QString(R"(
            QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid rgba(0,0,0,0.5); border-radius: 4px; }
            QProgressBar::chunk { background-color: %1; border-radius: 3px; }
        )").arg(color));

        QString statStr = status.empty() ? "" : QString(" [%1]").arg(QString::fromStdString(status).toUpper());
        textLabel->setText(QString("HP: %1 / %2%3").arg(endHp).arg(maxHp).arg(statStr));
        return;
    }

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(500);
    anim->setStartValue(startHp);
    anim->setEndValue(endHp);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QVariantAnimation::valueChanged, [bar, textLabel, maxHp, status](const QVariant& value) {
        int current = value.toInt();
        bar->setValue(current);

        int pct = maxHp > 0 ? (current * 100) / maxHp : 0;
        QString color = pct > 50 ? "#2ecc71" : (pct > 20 ? "#f1c40f" : "#e74c3c");

        bar->setStyleSheet(QString(R"(
            QProgressBar { background: rgba(0,0,0,0.2); border: 1px solid rgba(0,0,0,0.5); border-radius: 4px; }
            QProgressBar::chunk { background-color: %1; border-radius: 3px; }
        )").arg(color));

        QString statStr = status.empty() ? "" : QString(" [%1]").arg(QString::fromStdString(status).toUpper());
        textLabel->setText(QString("HP: %1 / %2%3").arg(current).arg(maxHp).arg(statStr));
        });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SecretBossDialog::animateMegaEvolution(bool isPlayer) {
    QLabel* sprite = isPlayer ? playerSprite : enemySprite;

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(sprite);
    sprite->setGraphicsEffect(effect);

    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(250);
    anim->setStartValue(1.0);
    anim->setKeyValueAt(0.5, 0.2);
    anim->setEndValue(1.0);
    anim->setLoopCount(4);

    connect(anim, &QPropertyAnimation::finished, [sprite, effect]() {
        if (sprite->graphicsEffect() == effect) {
            sprite->setGraphicsEffect(nullptr);
        }
        });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SecretBossDialog::animateFaint(bool isPlayer) {
    QLabel* sprite = isPlayer ? playerSprite : enemySprite;

    QPoint basePos = sprite->pos();
    QPoint dropPos = basePos + QPoint(0, 40);

    QPropertyAnimation* posAnim = new QPropertyAnimation(sprite, "pos", this);
    posAnim->setDuration(500);
    posAnim->setStartValue(basePos);
    posAnim->setEndValue(dropPos);
    posAnim->setEasingCurve(QEasingCurve::InCubic);

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(sprite);
    sprite->setGraphicsEffect(effect);

    QPropertyAnimation* opAnim = new QPropertyAnimation(effect, "opacity", this);
    opAnim->setDuration(500);
    opAnim->setStartValue(1.0);
    opAnim->setEndValue(0.0);

    posAnim->start(QAbstractAnimation::DeleteWhenStopped);
    opAnim->start(QAbstractAnimation::DeleteWhenStopped);

    connect(opAnim, &QPropertyAnimation::finished, [sprite, effect]() {
        if (sprite->graphicsEffect() == effect) {
            sprite->setGraphicsEffect(nullptr);
            sprite->hide();
        }
        });
}