#include "BattleDialog.h"
#include "SummaryDialog.h"
#include "../Engine/BattleEngine.h"
#include "../Engine/AI/AiDetector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QTimer>
#include <QInputDialog>
#include <QIcon>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QSizePolicy>
#include <QSettings>
#include <QVariant>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>

class LoadingDialog : public QDialog {
private:
    QLabel* tipTitle;
    QLabel* tipDesc;
    std::vector<std::pair<QString, QString>> tips = {
        {"Intimidate", "Lowers the opponent's Attack stat upon entering battle."},
        {"Levitate", "Gives full immunity to all Ground-type moves."},
        {"Overgrow", "Powers up Grass-type moves when the Pokémon's HP is low."},
        {"Sturdy", "It cannot be knocked out with one hit as long as its HP is full."},
        {"Static", "The Pokémon creates a static charge that may cause paralysis if hit by a physical move."},
        {"Synchronize", "Passes poison, paralyze, or burn to the Pokémon that inflicted it."},
        {"Inner Focus", "The Pokémon's intensely focused, and that protects the Pokémon from flinching."},
        {"Huge Power", "Doubles the Pokémon's Attack stat in battle."},
        {"Speed Boost", "Its Speed stat is gradually boosted every turn."},
        {"Protean", "Changes the Pokémon's type to the type of the move it's about to use."}
    };

public:
    LoadingDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Dialog);
        setFixedSize(400, 220);

        setStyleSheet(R"(
            QDialog { background-color: #0f1115; border: 2px solid #2d303e; border-radius: 8px; }
            QLabel#title { color: #ffffff; font-weight: 900; font-size: 18px; letter-spacing: 1px; border: none; }
            QLabel#tipTitle { color: #007acc; font-weight: 800; font-size: 14px; border: none; }
            QLabel#tipDesc { color: #8b949e; font-size: 12px; font-style: italic; border: none; }
            QPushButton { 
                background-color: #1a1c23; color: #d1d5db; font-weight: 800; font-size: 13px;
                padding: 12px 20px; border-radius: 6px; border: 1px solid #2d303e; letter-spacing: 1px;
            }
            QPushButton:hover { background-color: #242733; border: 1px solid #007acc; color: #ffffff; }
        )");

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(25, 25, 25, 25);
        layout->setSpacing(10);

        QLabel* title = new QLabel("PREPARING BATTLE...");
        title->setObjectName("title");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);

        tipTitle = new QLabel();
        tipTitle->setObjectName("tipTitle");
        tipTitle->setAlignment(Qt::AlignCenter);
        layout->addWidget(tipTitle);

        tipDesc = new QLabel();
        tipDesc->setObjectName("tipDesc");
        tipDesc->setWordWrap(true);
        tipDesc->setAlignment(Qt::AlignCenter);
        layout->addWidget(tipDesc);

        layout->addStretch();

        QPushButton* nextBtn = new QPushButton("START BATTLE");
        connect(nextBtn, &QPushButton::clicked, this, &QDialog::accept);
        layout->addWidget(nextBtn, 0, Qt::AlignCenter);

        loadRandomTip();
    }
    void loadRandomTip() {
        auto tip = tips[rand() % tips.size()];
        tipTitle->setText("ABILITY INSIGHT: " + tip.first.toUpper());
        tipDesc->setText(tip.second);
    }
};

BattleDialog::BattleDialog(Controller& ctrl, const std::vector<Pokemon>& pParty, const std::vector<Pokemon>& eParty, QWidget* parent)
    : QDialog(parent), controller(ctrl), playerParty(pParty), enemyParty(eParty),
    currentPlayerIndex(0), currentEnemyIndex(0), lastPlayerHp(-1), lastEnemyHp(-1),
    isPlayerMegaEvolved(false), isEnemyMegaEvolved(false), megaEvolveNextMove(false),
    isFaintSwitch(false), isMidTurnSwitch(false), currentPhase(TurnPhase::Idle),
    turnCounter(1), isNewTurn(true) {

    for (auto& p : playerParty) p.fullyHeal();
    for (auto& e : enemyParty) {
        if (e.getMoves().empty()) e.setMoves({ "tackle", "growl" });
        e.fullyHeal();
    }

    for (size_t i = 0; i < playerParty.size(); i++) {
        if (playerParty[i].getCurrentHp() > 0) { currentPlayerIndex = i; break; }
    }
    for (size_t i = 0; i < enemyParty.size(); i++) {
        if (enemyParty[i].getCurrentHp() > 0) { currentEnemyIndex = i; break; }
    }

    setupUI();
    connectSignals();

    QSettings settings("MyCompany", "PokemonBoxManager");
    QString savedTheme = settings.value("mainHallTheme", "Pro Dark").toString();
    applyTheme(savedTheme);

    LoadingDialog loading(this);
    loading.exec();

    engine = std::make_unique<BattleEngine>(controller);

    int pFaints = 0; for (const auto& pk : playerParty) if (pk.getCurrentHp() <= 0) pFaints++;
    int eFaints = 0; for (const auto& pk : enemyParty) if (pk.getCurrentHp() <= 0) eFaints++;
    engine->setFaintedCount(true, pFaints);
    engine->setFaintedCount(false, eFaints);

    updateBattleDisplay();

    engine->onSwitchIn(playerParty[currentPlayerIndex], true, currentLog, &enemyParty[currentEnemyIndex]);
    engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
    engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
    updateFieldDisplay();
    flushLog(false);

    animateSwitchIn(true);
    animateSwitchIn(false);

    currentLog.push_back("Trainer challenges you to a battle!");
    flushLog(false);
}

BattleDialog::~BattleDialog() = default;

void BattleDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- BATTLE SCENE (STRICTLY FIXED 520x270 FOR ABSOLUTE POSITIONING) ---
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

    // --- CONTROL AREA (RESPONSIVE) ---
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

    // ACTION PANEL
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

    // MOVE PANEL
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

    // SWITCH PANEL
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
    setWindowTitle("Pokémon Champions - Battle Screen");

    setMinimumSize(540, 680);
    resize(540, 720);
}

void BattleDialog::applyTheme(const QString& themeName) {
    QString bg, ctrlBg, textBase, textMuted, fieldColor;
    QString hudBg, hudBorderP, hudBorderE;
    QString btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder;
    QString dangerBorder, dangerHoverBg, dangerHoverText;
    QString successBorder, successHoverBg, successHoverText;
    QString radius = "6px";

    if (themeName == "Pro Dark") {
        bg = "#111318"; ctrlBg = "#0f1115"; textBase = "#ffffff"; textMuted = "#8b949e"; fieldColor = "#007acc";
        hudBg = "rgba(15, 17, 21, 230)"; hudBorderP = "#007acc"; hudBorderE = "#e74c3c";
        btnBg = "#1a1c23"; btnBorder = "#2d303e"; btnText = "#d1d5db"; btnHoverBg = "#242733"; btnHoverBorder = "#007acc";
        dangerBorder = "#e74c3c"; dangerHoverBg = "#2c1618"; dangerHoverText = "#ff6b6b";
        successBorder = "#27ae60"; successHoverBg = "#142b1c"; successHoverText = "#2ecc71";
    }
    else if (themeName == "Mystic Water") {
        bg = "#b3e5fc"; ctrlBg = "#e1f5fe"; textBase = "#01579b"; textMuted = "#0277bd"; fieldColor = "#0288d1";
        hudBg = "rgba(225, 245, 254, 230)"; hudBorderP = "#0288d1"; hudBorderE = "#e53935";
        btnBg = "#ffffff"; btnBorder = "#81d4fa"; btnText = "#01579b"; btnHoverBg = "#b3e5fc"; btnHoverBorder = "#0288d1";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
        successBorder = "#388e3c"; successHoverBg = "#c8e6c9"; successHoverText = "#2e7d32";
    }
    else if (themeName == "Electric Spark") {
        bg = "#fff9c4"; ctrlBg = "#fffde7"; textBase = "#212121"; textMuted = "#616161"; fieldColor = "#f57f17";
        hudBg = "rgba(255, 253, 231, 230)"; hudBorderP = "#fbc02d"; hudBorderE = "#d32f2f";
        btnBg = "#ffffff"; btnBorder = "#fff176"; btnText = "#424242"; btnHoverBg = "#fff59d"; btnHoverBorder = "#fbc02d";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
        successBorder = "#388e3c"; successHoverBg = "#c8e6c9"; successHoverText = "#2e7d32";
    }
    else if (themeName == "Leafy Forest") {
        bg = "#dcedc8"; ctrlBg = "#f1f8e9"; textBase = "#1b5e20"; textMuted = "#33691e"; fieldColor = "#558b2f";
        hudBg = "rgba(241, 248, 233, 230)"; hudBorderP = "#689f38"; hudBorderE = "#e53935";
        btnBg = "#ffffff"; btnBorder = "#c5e1a5"; btnText = "#2e7d32"; btnHoverBg = "#dcedc8"; btnHoverBorder = "#7cb342";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
        successBorder = "#388e3c"; successHoverBg = "#c8e6c9"; successHoverText = "#2e7d32";
    }
    else if (themeName == "Ember Volcano") {
        bg = "#1a0b0b"; ctrlBg = "#2d1313"; textBase = "#ff4757"; textMuted = "#a36e6e"; fieldColor = "#ffa502";
        hudBg = "rgba(45, 19, 19, 230)"; hudBorderP = "#ffa502"; hudBorderE = "#eccc68";
        btnBg = "#2d1313"; btnBorder = "#ff4757"; btnText = "#ff6b81"; btnHoverBg = "#ff4757"; btnHoverBorder = "#ff4757";
        dangerBorder = "#eccc68"; dangerHoverBg = "#592b2b"; dangerHoverText = "#eccc68";
        successBorder = "#ff4757"; successHoverBg = "#4a1c1c"; successHoverText = "#ff6b81";
    }
    else if (themeName == "Psychic Mind") {
        bg = "#190033"; ctrlBg = "#2d1b4e"; textBase = "#e056fd"; textMuted = "#9b7eb5"; fieldColor = "#be2edd";
        hudBg = "rgba(45, 27, 78, 230)"; hudBorderP = "#be2edd"; hudBorderE = "#ff4757";
        btnBg = "#2d1b4e"; btnBorder = "#8c7ae6"; btnText = "#dcbdf2"; btnHoverBg = "#be2edd"; btnHoverBorder = "#e056fd";
        dangerBorder = "#ff4757"; dangerHoverBg = "#4a1c40"; dangerHoverText = "#ff6b81";
        successBorder = "#e056fd"; successHoverBg = "#3c1a40"; successHoverText = "#f3a6ff";
    }
    else if (themeName == "Dragon's Den") {
        bg = "#0b0a1a"; ctrlBg = "#15142b"; textBase = "#f5cd79"; textMuted = "#706f8a"; fieldColor = "#e66767";
        hudBg = "rgba(21, 20, 43, 230)"; hudBorderP = "#e66767"; hudBorderE = "#ff7979";
        btnBg = "#15142b"; btnBorder = "#546de5"; btnText = "#f5cd79"; btnHoverBg = "#546de5"; btnHoverBorder = "#778beb";
        dangerBorder = "#e66767"; dangerHoverBg = "#2b1414"; dangerHoverText = "#ff7979";
        successBorder = "#f5cd79"; successHoverBg = "#332d16"; successHoverText = "#ffeba3";
    }
    else if (themeName == "Fairy Tale") {
        bg = "#fff0f5"; ctrlBg = "#ffffff"; textBase = "#e84393"; textMuted = "#b78e9b"; fieldColor = "#fd79a8";
        hudBg = "rgba(255, 255, 255, 230)"; hudBorderP = "#fd79a8"; hudBorderE = "#d63031";
        btnBg = "#ffffff"; btnBorder = "#fab1a0"; btnText = "#d63031"; btnHoverBg = "#fd79a8"; btnHoverBorder = "#e84393";
        dangerBorder = "#d63031"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c0392b";
        successBorder = "#e84393"; successHoverBg = "#fce4ec"; successHoverText = "#d81b60";
    }
    else if (themeName == "Champion's Gold") {
        bg = "#fdfbfa"; ctrlBg = "#ffffff"; textBase = "#2d3436"; textMuted = "#7f7b71"; fieldColor = "#d4af37";
        hudBg = "rgba(255, 255, 255, 230)"; hudBorderP = "#d4af37"; hudBorderE = "#e74c3c";
        btnBg = "#ffffff"; btnBorder = "#d4af37"; btnText = "#2d3436"; btnHoverBg = "#fff6d6"; btnHoverBorder = "#f1c40f";
        dangerBorder = "#e74c3c"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#c0392b";
        successBorder = "#d4af37"; successHoverBg = "#fef9e7"; successHoverText = "#b8860b";
    }
    else {
        bg = "#e0e0e0"; ctrlBg = "#f4f6f8"; textBase = "#2c3e50"; textMuted = "#7f8c8d"; fieldColor = "#e74c3c";
        hudBg = "rgba(255, 255, 255, 230)"; hudBorderP = "#3498db"; hudBorderE = "#e74c3c";
        btnBg = "#ffffff"; btnBorder = "#bdc3c7"; btnText = "#2c3e50"; btnHoverBg = "#ecf0f1"; btnHoverBorder = "#3498db";
        dangerBorder = "#c0392b"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#922b21";
        successBorder = "#27ae60"; successHoverBg = "#d5f5e3"; successHoverText = "#1d8348";
    }

    this->setProperty("themeBtnBg", btnBg);
    this->setProperty("themeBtnBorder", btnBorder);
    this->setProperty("themeBtnTextMuted", textMuted);

    QString qss = QString(R"(
        QDialog { background-color: %1; font-family: 'Segoe UI', Helvetica, sans-serif; }
        QWidget#controlArea { background-color: %2; border-top: 2px solid %8; }
        
        QWidget#playerHud { background-color: %6; border: 2px solid %7; border-radius: %15; }
        QWidget#enemyHud { background-color: %6; border: 2px solid %16; border-radius: %15; }
        
        QLabel#hudName { font-weight: 800; font-size: 13px; color: %3; background: transparent; }
        QLabel#hudHp { font-size: 11px; font-weight: bold; color: %4; background: transparent; }
        
        QPushButton#fieldInfoBtn {
            background-color: %10;
            color: %5;
            font-size: 13px; font-weight: 800;
            border: 1px solid %5;
            border-radius: %15;
            padding: 8px;
            text-align: center;
        }
        QPushButton#fieldInfoBtn:hover {
            background-color: %5;
            color: %2;
        }
        
        QListWidget#logWidget { 
            background-color: %9; color: %11; border: 1px solid %8; 
            border-radius: %15; padding: 6px; font-family: 'Consolas', 'Courier New', monospace; font-size: 12px; 
        }
        QListWidget::item { padding: 4px; border-radius: 4px; }
        QListWidget::item:selected { background-color: %12; color: #ffffff; }

        QPushButton { 
            background-color: %9; color: %11; font-size: 13px; font-weight: 800; 
            padding: 10px; border: 1px solid %8; border-radius: %15; letter-spacing: 1px; 
        }
        QPushButton:hover { background-color: %10; border: 1px solid %12; color: %3; }
        
        QPushButton#actionBtn:hover { background-color: %10; border: 1px solid %12; }
        QPushButton#neutralBtn:hover { background-color: %10; border: 1px solid %4; color: %4; }
        QPushButton#dangerBtn:hover { border: 1px solid %13; background-color: %14; color: %17; }
        QPushButton#successBtn:hover { border: 1px solid %18; background-color: %19; color: %20; }
    )").arg(bg, ctrlBg, textBase, textMuted, fieldColor, hudBg, hudBorderP, btnBorder, btnBg, btnHoverBg, btnText, btnHoverBorder, dangerBorder, dangerHoverBg, radius, hudBorderE, dangerHoverText, successBorder, successHoverBg, successHoverText);

    this->setStyleSheet(qss);
}

void BattleDialog::connectSignals() {
    connect(fightBtn, &QPushButton::clicked, this, &BattleDialog::onFightClicked);
    connect(switchBtn, &QPushButton::clicked, this, &BattleDialog::onSwitchClicked);
    connect(megaBtn, &QPushButton::clicked, this, &BattleDialog::onMegaClicked);
    connect(runBtn, &QPushButton::clicked, this, &BattleDialog::onRunClicked);
    connect(backToActionsBtn, &QPushButton::clicked, [this]() { movePanel->hide(); actionPanel->show(); });
    connect(cancelSwitchBtn, &QPushButton::clicked, this, &BattleDialog::onCancelSwitchClicked);
    connect(confirmSwitchBtn, &QPushButton::clicked, this, &BattleDialog::onConfirmSwitchClicked);
    connect(partyListWidget, &QListWidget::itemDoubleClicked, this, &BattleDialog::onPartyItemDoubleClicked);
    connect(fieldInfoBtn, &QPushButton::clicked, this, &BattleDialog::onFieldInfoClicked);

    connect(move1Btn, &QPushButton::clicked, [this]() { onMoveClicked(0); });
    connect(move2Btn, &QPushButton::clicked, [this]() { onMoveClicked(1); });
    connect(move3Btn, &QPushButton::clicked, [this]() { onMoveClicked(2); });
    connect(move4Btn, &QPushButton::clicked, [this]() { onMoveClicked(3); });
}

void BattleDialog::flushLog(bool endOfTurn) {
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

    if (endOfTurn) {
        turnCounter++;
        isNewTurn = true;
    }
}

QString BattleDialog::getTypeColor(const std::string& type) {
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

void BattleDialog::updateFieldDisplay() {
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
    else {
        txt.chop(3);
        txt = "FIELD: " + txt;
    }

    fieldInfoBtn->setText(txt + " (CLICK FOR DETAILS)");
}

void BattleDialog::onFieldInfoClicked() {
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

    // Weather
    if (fs.weather != "None") {
        QString wStr = QString("Weather: %1").arg(QString::fromStdString(fs.weather));
        if (activeW == "None") wStr += " (Suppressed by Cloud Nine/Air Lock)";
        addItem(wStr, "#e67e22");
    }

    // Terrain
    if (fs.terrain != "None") addItem(QString("Terrain: %1").arg(QString::fromStdString(fs.terrain)), "#27ae60");

    // Rooms
    if (fs.trickRoom > 0) addItem(QString("Trick Room (%1 turns left)").arg(fs.trickRoom), "#9b59b6");
    if (fs.magicRoom > 0) addItem(QString("Magic Room (%1 turns left)").arg(fs.magicRoom), "#9b59b6");
    if (fs.wonderRoom > 0) addItem(QString("Wonder Room (%1 turns left)").arg(fs.wonderRoom), "#9b59b6");
    if (fs.gravity > 0) addItem(QString("Gravity (%1 turns left)").arg(fs.gravity), "#34495e");

    // Player Hazards
    QString pHaz = "Your Side: ";
    bool pHasHaz = false;
    if (ps.stealthRock) { pHaz += "Stealth Rock, "; pHasHaz = true; }
    if (ps.spikes > 0) { pHaz += QString("Spikes (x%1), ").arg(ps.spikes); pHasHaz = true; }
    if (ps.toxicSpikes > 0) { pHaz += QString("Toxic Spikes (x%1), ").arg(ps.toxicSpikes); pHasHaz = true; }
    if (ps.stickyWeb) { pHaz += "Sticky Web, "; pHasHaz = true; }
    if (pHasHaz) { pHaz.chop(2); addItem(pHaz, "#3498db"); }

    // Enemy Hazards
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

void BattleDialog::updateBattleDisplay() {
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
    else {
        animateHpBar(true, p.getCurrentHp(), p.getCurrentHp(), p.getHp(), p.getStatus());
    }

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
    else {
        animateHpBar(false, e.getCurrentHp(), e.getCurrentHp(), e.getHp(), e.getStatus());
    }

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

            bool disabled = false;
            if (p.hasVolatile("torment") && p.lastMoveUsed == moveId) disabled = true;
            if (e.hasVolatile("imprison")) {
                for (auto& em : e.getMoves()) if (em == moveId) disabled = true;
            }
            std::string rawItem = p.getHeldItem();
            std::string normalizedItem = "";
            for (char c : rawItem) { if (c != ' ') normalizedItem += std::tolower(c); }
            if (normalizedItem == "assaultvest" && md.category == "Status") {
                disabled = true;
            }

            if (forceValid) disabled = false;

            btn->setIcon(QIcon("assets/Others/damage-category-icons/1x/" + QString::fromStdString(md.category) + "-white.png"));
            btn->setIconSize(QSize(24, 12));
            btn->setText("  " + QString::fromStdString(md.name).toUpper());

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
        updateMoveBtn(move2Btn, "");
        updateMoveBtn(move3Btn, "");
        updateMoveBtn(move4Btn, "");
        move1Btn->setProperty("struggle_override", true);
    }
    else {
        move1Btn->setProperty("struggle_override", false);
    }

    updateFieldDisplay();
}

void BattleDialog::onFightClicked() {
    Pokemon& p = playerParty[currentPlayerIndex];
    if (p.getLockedTurns() > 0) {
        currentLog.push_back(p.getNickname() + " is locked in!");
        beginTurn(p.getLockedMove());
    }
    else {
        actionPanel->hide();
        movePanel->show();
    }
}

void BattleDialog::onSwitchClicked() {
    partyListWidget->clear();
    for (size_t i = 0; i < playerParty.size(); i++) {
        const Pokemon& poke = playerParty[i];
        QString status = poke.getCurrentHp() <= 0 ? "[FNT] " : "";
        int cur = poke.getCurrentHp();
        int max = poke.getHp();
        int pct = (max > 0) ? (cur * 100) / max : 0;

        QString text = QString("%1%2 - %3 (HP: %4/%5 - %6%)").arg(status)
            .arg(QString::fromStdString(poke.getNickname()))
            .arg(QString::fromStdString(poke.getName()))
            .arg(cur).arg(max).arg(pct);

        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, static_cast<int>(i));

        if (i == currentPlayerIndex || playerParty[i].getCurrentHp() <= 0) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        }
        partyListWidget->addItem(item);
    }
    actionPanel->hide();
    switchPanel->show();
}

void BattleDialog::onPartyItemDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    int idx = item->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= (int)playerParty.size()) return;
    int partyId = playerParty[idx].getId();
    SummaryDialog dialog(controller, partyId, this);
    dialog.exec();
}

void BattleDialog::onCancelSwitchClicked() { switchPanel->hide(); actionPanel->show(); }

void BattleDialog::onConfirmSwitchClicked() {
    if (!partyListWidget->currentItem()) return;
    int targetIdx = partyListWidget->currentItem()->data(Qt::UserRole).toInt();
    switchPanel->hide();
    actionPanel->show();
    executeSwitch(targetIdx);
}

void BattleDialog::onMegaClicked() {
    if (isPlayerMegaEvolved) { currentLog.push_back("You have already Mega Evolved a Pokémon!"); flushLog(false); return; }
    std::string item = playerParty[currentPlayerIndex].getHeldItem();
    if (QString::fromStdString(item).contains("ITE", Qt::CaseInsensitive) || item == "REDORB" || item == "BLUEORB") {
        megaEvolveNextMove = true;
        megaBtn->setStyleSheet(R"(
            QPushButton { background-color: #8e44ad; color: #ffffff; font-size: 13px; font-weight: 800; padding: 10px; border: 1px solid #6c3483; border-radius: 6px; letter-spacing: 1px; }
        )");
        currentLog.push_back("Your Mega Bracelet is reacting to " + item + "!");
        flushLog(false);
    }
    else {
        currentLog.push_back(playerParty[currentPlayerIndex].getNickname() + " isn't holding a valid Mega Stone!");
        flushLog(false);
    }
}

void BattleDialog::onRunClicked() {
    currentLog.push_back("Got away safely!");
    flushLog(false);
    QTimer::singleShot(1000, this, &QDialog::reject);
}

void BattleDialog::onMoveClicked(int moveIndex) {
    movePanel->hide();
    actionPanel->show();

    Pokemon& p = playerParty[currentPlayerIndex];

    if (moveIndex == 0 && move1Btn->property("struggle_override").toBool()) {
        beginTurn("struggle");
        return;
    }

    if (moveIndex >= p.getMoves().size()) return;
    beginTurn(p.getMoves()[moveIndex]);
}

void BattleDialog::beginTurn(const std::string& playerMove) {
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
        else if (item == "REDORB") megaName = "Groudon-Primal";
        else if (item == "BLUEORB") megaName = "Kyogre-Primal";
        else if (item == "ABSOLITEZ") megaName = "Absol-Mega-Z";
        else if (item == "GARCHOMPITEZ") megaName = "Garchomp-Mega-Z";
        else if (item == "LUCARIONITEZ") megaName = "Lucario-Mega-Z";

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
        enemyParty, currentEnemyIndex,
        playerParty, currentPlayerIndex,
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
        currentLog.push_back("Trainer withdrew " + oldEnemy.getName() + "!");
        currentLog.push_back("Trainer sent out " + enemyParty[currentEnemyIndex].getName() + "!");

        lastEnemyHp = -1;

        engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
        engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);

        updateBattleDisplay();
        animateSwitchIn(false);

        queuedEnemyMove = "SKIP_TURN";
    }
    else {
        if (decisionToken.rfind("mega:", 0) == 0) {
            handleMegaForm(e, false);
            queuedEnemyMove = decisionToken.substr(5);
        }
        else if (decisionToken.rfind("move:", 0) == 0) {
            queuedEnemyMove = decisionToken.substr(5);
        }
        else {
            queuedEnemyMove = decisionToken;
        }
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

void BattleDialog::executeNextPhase() {
    Pokemon& p = playerParty[currentPlayerIndex];
    Pokemon& e = enemyParty[currentEnemyIndex];

    // Helper 1: Resolves Roar, Whirlwind, Dragon Tail, Circle Throw, and Red Card
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
                queuedPlayerMove = "SKIP_TURN"; // Ensure dragged out mon loses its move
                updateBattleDisplay();
                animateSwitchIn(true);
            }
            else {
                currentLog.push_back(target.getName() + " was forced out! Trainer sent out " + party[currentIndex].getName() + "!");
                engine->onSwitchIn(party[currentIndex], false, currentLog, &playerParty[currentPlayerIndex]);
                lastEnemyHp = -1;
                queuedEnemyMove = "SKIP_TURN"; // Ensure dragged out mon loses its move
                updateBattleDisplay();
                animateSwitchIn(false);
            }
            engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
            updateFieldDisplay();
        }
        };

    // Helper 2: Resolves U-Turn, Volt Switch, Flip Turn, Parting Shot, Eject Pack, and Eject Button
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
                return true; // Indicates the turn sequence must pause
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

            // 1. Check if the Defender was forced out (Roar, Dragon Tail, Eject Button)
            if (defender.isSwitchingOut) {
                if (defender.isForcedRandomSwitch) handleForcedRandomSwitch(defender, !isPlayer);
                else paused = handleManualSwitch(defender, !isPlayer);
            }

            // 2. Check if the Attacker forced themselves out (U-Turn, Red Card, Eject Pack)
            if (!paused && attacker.isSwitchingOut) {
                if (attacker.isForcedRandomSwitch) handleForcedRandomSwitch(attacker, isPlayer);
                else paused = handleManualSwitch(attacker, isPlayer);
            }

            if (paused) return; // Halt turn progression until player chooses a new Pokémon
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
        engine->checkExtremeWeather(p, e, currentLog);
        currentPhase = TurnPhase::Idle;
        isMidTurnSwitch = false;

        flushLog(true);
        updateBattleDisplay();
        checkFaint();
    }
}

void BattleDialog::executeSwitch(int newIndex) {
    Pokemon& oldPoke = playerParty[currentPlayerIndex];
    Pokemon& newPoke = playerParty[newIndex];

    if (oldPoke.hasVolatile("transform")) oldPoke.fullyHeal();

    if (oldPoke.isBatonPassing) {
        newPoke.inheritBatonPass(oldPoke);
        currentLog.push_back(newPoke.getNickname() + " inherited the Baton Pass stats!");
    }
    else {
        oldPoke.resetStatStages();
    }

    oldPoke.isSwitchingOut = false;
    oldPoke.isBatonPassing = false;

    currentLog.push_back("Come back " + oldPoke.getNickname() + "!");

    currentPlayerIndex = newIndex;
    megaEvolveNextMove = false;

    megaBtn->setStyleSheet("");
    cancelSwitchBtn->setEnabled(true);

    currentLog.push_back("Go! " + newPoke.getNickname() + "!");

    engine->onSwitchIn(newPoke, true, currentLog, &enemyParty[currentEnemyIndex]);
    engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
    updateFieldDisplay();

    lastPlayerHp = -1;

    playerSprite->show();

    updateBattleDisplay();
    animateSwitchIn(true);

    if (isMidTurnSwitch) {
        isMidTurnSwitch = false;
        executeNextPhase();
    }
    else if (isFaintSwitch) {
        isFaintSwitch = false;
        flushLog(false);
    }
    else {
        Pokemon& e = enemyParty[currentEnemyIndex];

        AI::AiDetector smartAI(controller);
        std::string decisionToken = smartAI.chooseBestAction(
            enemyParty, currentEnemyIndex,
            playerParty, currentPlayerIndex,
            !isEnemyMegaEvolved, !isPlayerMegaEvolved
        );

        if (decisionToken.rfind("switch:", 0) == 0) {
            int targetIndex = std::stoi(decisionToken.substr(7));
            Pokemon& oldEnemy = enemyParty[currentEnemyIndex];
            if (oldEnemy.hasVolatile("transform")) oldEnemy.fullyHeal();
            if (!oldEnemy.isBatonPassing) oldEnemy.resetStatStages();
            oldEnemy.isBatonPassing = false;

            currentEnemyIndex = targetIndex;
            currentLog.push_back("Trainer withdrew " + oldEnemy.getName() + "!");
            currentLog.push_back("Trainer sent out " + enemyParty[currentEnemyIndex].getName() + "!");

            engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
            engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
            updateFieldDisplay();
        }
        else {
            std::string eMove = decisionToken;
            if (decisionToken.rfind("mega:", 0) == 0) {
                eMove = decisionToken.substr(5);

                std::string megaName = e.getName() + "-Mega";
                std::string rawItem = e.getHeldItem();
                std::string item = "";
                for (char c : rawItem) { if (c != ' ') item += std::toupper(c); }

                if (item == "CHARIZARDITEX") megaName = "Charizard-Mega-X";
                else if (item == "CHARIZARDITEY") megaName = "Charizard-Mega-Y";
                else if (item == "MEWTWONITEX") megaName = "Mewtwo-Mega-X";
                else if (item == "MEWTWONITEY") megaName = "Mewtwo-Mega-Y";
                else if (item == "REDORB") megaName = "Groudon-Primal";
                else if (item == "BLUEORB") megaName = "Kyogre-Primal";
                else if (item == "ABSOLITEZ") megaName = "Absol-Mega-Z";
                else if (item == "GARCHOMPITEZ") megaName = "Garchomp-Mega-Z";
                else if (item == "LUCARIONITEZ") megaName = "Lucario-Mega-Z";

                Pokemon megaForm = controller.getSpeciesDataByName(megaName);
                if (megaForm.getName() != "") {
                    e.megaEvolve(megaForm);
                    currentLog.push_back(e.getNickname() + " Mega Evolved into " + megaForm.getName() + "!");
                    isEnemyMegaEvolved = true;

                    engine->onSwitchIn(e, false, currentLog, &playerParty[currentPlayerIndex]);
                    engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
                    updateFieldDisplay();
                }
            }
            else if (decisionToken.rfind("move:", 0) == 0) {
                eMove = decisionToken.substr(5);
            }

            MoveData md = controller.getMoveData(eMove);
            auto animCallback = [this, md](bool isP) { this->animateMoveEffect(isP, md.type, md.category); };
            engine->executeMove(e, playerParty[currentPlayerIndex], eMove, false, currentLog, "switch", animCallback);
            updateFieldDisplay();
        }

        flushLog(true);
        updateBattleDisplay();
        checkFaint();
    }
}

void BattleDialog::executeEnemySwitch() {
    Pokemon& oldEnemy = enemyParty[currentEnemyIndex];
    oldEnemy.isSwitchingOut = false;

    bool enemyHasMore = false;
    int nextEnemy = -1;
    for (size_t i = 0; i < enemyParty.size(); i++) {
        if (enemyParty[i].getCurrentHp() > 0 && i != currentEnemyIndex) {
            enemyHasMore = true;
            nextEnemy = i;
            break;
        }
    }
    if (enemyHasMore) {
        if (oldEnemy.hasVolatile("transform")) oldEnemy.fullyHeal();
        if (!oldEnemy.isBatonPassing) oldEnemy.resetStatStages();
        oldEnemy.isBatonPassing = false;

        currentEnemyIndex = nextEnemy;
        currentLog.push_back("Trainer sent out " + enemyParty[currentEnemyIndex].getName() + "!");

        engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
        engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
        updateFieldDisplay();

        lastEnemyHp = -1;
        updateBattleDisplay();
        animateSwitchIn(false);
    }
}

bool BattleDialog::checkFaint() {
    if (enemyParty[currentEnemyIndex].getCurrentHp() <= 0) {
        engine->setFaintedCount(false, engine->getEnemySide().faintedCount + 1);

        currentLog.push_back("Opponent " + enemyParty[currentEnemyIndex].getName() + " fainted!");
        flushLog(false);

        animateFaint(false);

        bool enemyHasMore = false;
        int nextIdx = -1;
        for (size_t i = 0; i < enemyParty.size(); i++) {
            if (enemyParty[i].getCurrentHp() > 0) {
                nextIdx = i;
                enemyHasMore = true;
                break;
            }
        }

        if (!enemyHasMore) {
            QTimer::singleShot(1000, this, [this]() {
                QMessageBox::information(this, "Victory!", "You won the battle!");
                this->accept();
                });
        }
        else {
            QTimer::singleShot(600, this, [this, nextIdx]() {
                currentEnemyIndex = nextIdx;
                currentLog.push_back("Trainer sent out " + enemyParty[currentEnemyIndex].getName() + "!");

                engine->onSwitchIn(enemyParty[currentEnemyIndex], false, currentLog, &playerParty[currentPlayerIndex]);
                engine->checkExtremeWeather(playerParty[currentPlayerIndex], enemyParty[currentEnemyIndex], currentLog);
                flushLog(false);

                enemySprite->show();

                updateBattleDisplay();
                animateSwitchIn(false);
                });
        }
        return true;
    }

    if (playerParty[currentPlayerIndex].getCurrentHp() <= 0) {
        engine->setFaintedCount(true, engine->getPlayerSide().faintedCount + 1);

        currentLog.push_back(playerParty[currentPlayerIndex].getNickname() + " fainted!");
        flushLog(false);

        animateFaint(true);

        bool playerHasMore = false;
        for (const auto& poke : playerParty) {
            if (poke.getCurrentHp() > 0) { playerHasMore = true; break; }
        }

        if (!playerHasMore) {
            currentLog.push_back("You have no more Pokémon left!");
            currentLog.push_back("You blacked out...");
            flushLog(false);

            QTimer::singleShot(1000, this, [this]() {
                QMessageBox::critical(this, "Defeat", "You lost the battle...");
                this->reject();
                });
        }
        else {
            QTimer::singleShot(600, this, [this]() {
                isFaintSwitch = true;
                actionPanel->hide();
                onSwitchClicked();
                cancelSwitchBtn->setDisabled(true);
                });
        }
        return true;
    }
    return false;
}

void BattleDialog::animateMoveEffect(bool isPlayerAttacker, const std::string& moveType, const std::string& category) {
    QLabel* attackerSprite = isPlayerAttacker ? playerSprite : enemySprite;
    QLabel* defenderSprite = isPlayerAttacker ? enemySprite : playerSprite;

    // FIX: Hardcoded absolute coordinates guarantee sprites never drift off-screen
    QPoint attackerBasePos = isPlayerAttacker ? QPoint(50, 100) : QPoint(330, 20);
    QPoint defenderBasePos = isPlayerAttacker ? QPoint(330, 20) : QPoint(50, 100);

    // 1. Attacker Lunge (Slower duration)
    QPoint lungePos = isPlayerAttacker ? attackerBasePos + QPoint(30, -30) : attackerBasePos + QPoint(-30, 30);

    QPropertyAnimation* lungeAnim = new QPropertyAnimation(attackerSprite, "pos", this);
    lungeAnim->setDuration(350);
    lungeAnim->setStartValue(attackerBasePos);
    lungeAnim->setKeyValueAt(0.5, lungePos);
    lungeAnim->setEndValue(attackerBasePos);
    lungeAnim->setEasingCurve(QEasingCurve::InOutQuad);
    lungeAnim->start(QAbstractAnimation::DeleteWhenStopped);

    // 2. Determine Elemental Color
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

    // 3. Apply Defender Color Flash (Slower duration)
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

    // 4. Physical / Heavy Move Screen Shake (Slower duration, absolute reference)
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

void BattleDialog::animateSwitchIn(bool isPlayer) {
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

void BattleDialog::animateHpBar(bool isPlayer, int startHp, int endHp, int maxHp, const std::string& status) {
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

void BattleDialog::animateMegaEvolution(bool isPlayer) {
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

void BattleDialog::animateFaint(bool isPlayer) {
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