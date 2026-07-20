#include "SafariZoneDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QTimer>
#include <QInputDialog>
#include <QIcon>
#include <QFile>
#include <QDebug>
#include <QSettings>
#include <QSizePolicy>
#include <cstdlib>
#include <algorithm>

SafariZoneDialog::SafariZoneDialog(Controller& ctrl, const QString& playerPath, const std::vector<std::string>& allowedTypes, QWidget* parent)
    : QDialog(parent), controller(ctrl), playerAvatarPath(playerPath) {

    currentEncounter = controller.getRandomEncounter(allowedTypes);
    if (currentEncounter.getName() == "") {
        currentEncounter = Pokemon(1, 1, "Bulbasaur", "Grass", "Poison", 45, 49, 49, 65, 65, 45, 50, "None");
    }

    turnCount = 1;
    setupUI();

    QSettings settings("MyCompany", "PokemonBoxManager");
    QString savedTheme = settings.value("mainHallTheme", "Pro Dark").toString();
    applyTheme(savedTheme);

    connectSignals();
    resetStats();

    log(QString("Wild %1 appeared!").arg(QString::fromStdString(currentEncounter.getName())));
}

void SafariZoneDialog::setupUI() {
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
    else battleScene->setStyleSheet("background-color: #1a1c23; border-bottom: 2px solid #2d303e;");

    QLabel* enemyBaseLabel = new QLabel(battleScene);
    enemyBaseLabel->setGeometry(310, 110, 160, 60);
    QPixmap enemyBase("CatchPoke/enemy.png");
    if (!enemyBase.isNull()) enemyBaseLabel->setPixmap(enemyBase.scaled(160, 60, Qt::KeepAspectRatio));

    QLabel* playerBaseLabel = new QLabel(battleScene);
    playerBaseLabel->setGeometry(30, 190, 160, 60);
    QPixmap playerBase("CatchPoke/player.png");
    if (!playerBase.isNull()) playerBaseLabel->setPixmap(playerBase.scaled(160, 60, Qt::KeepAspectRatio));

    imageLabel = new QLabel(battleScene);
    imageLabel->setGeometry(330, 20, 120, 120);
    imageLabel->setAlignment(Qt::AlignCenter);

    QString baseName = QString::fromStdString(currentEncounter.getName());
    QString paddedId = QString("%1").arg(currentEncounter.getSpeciesId(), 4, 10, QChar('0'));
    QString iconPath = QString("assets/images/%1.png").arg(paddedId);

    if (baseName.contains("-")) {
        QString specificPath = QString("assets/images/%1-%2.png").arg(paddedId).arg(baseName.mid(baseName.indexOf('-') + 1));
        if (QFile::exists(specificPath)) iconPath = specificPath;
    }
    if (!QFile::exists(iconPath)) iconPath = "Items/000.png";

    QPixmap fullSprite(iconPath);
    if (!fullSprite.isNull()) imageLabel->setPixmap(fullSprite.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* playerAvatarLabel = new QLabel(battleScene);
    playerAvatarLabel->setGeometry(50, 100, 120, 120);
    playerAvatarLabel->setAlignment(Qt::AlignCenter);

    QPixmap playerSheet(playerAvatarPath);
    if (!playerSheet.isNull()) {
        int frameSize = playerSheet.height();
        QPixmap restingFrame = playerSheet.copy(0, 0, frameSize, frameSize);
        QImage flipped = restingFrame.toImage().mirrored(true, false);
        playerAvatarLabel->setPixmap(QPixmap::fromImage(flipped).scaled(120, 120, Qt::KeepAspectRatio));
    }
    mainLayout->addWidget(battleScene, 0, Qt::AlignHCenter);

    QWidget* controlArea = new QWidget(this);
    controlArea->setObjectName("controlArea");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlArea);
    controlLayout->setContentsMargins(15, 15, 15, 15);
    controlLayout->setSpacing(15);

    statusLabel = new QLabel("What will you do?", this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    controlLayout->addWidget(statusLabel);

    QGridLayout* buttonGrid = new QGridLayout();
    buttonGrid->setSpacing(10);

    for (int i = 0; i < 4; i++) {
        buttonGrid->setColumnStretch(i, 1);
    }

    ballSelector = new QComboBox(this);
    ballSelector->setMinimumHeight(35);
    ballButton = new QPushButton("THROW BALL");
    ballButton->setObjectName("actionBtn");
    ballButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ballButton->setMinimumHeight(45);

    std::map<std::string, int> bag = controller.getInventory();
    int totalBalls = 0;

    for (const auto& pair : bag) {
        if (pair.second > 0 && QString::fromStdString(pair.first).contains("BALL")) {
            QString b = QString::fromStdString(pair.first);
            QString path = "Items/" + b + ".png";
            if (!QFile::exists(path)) path = "Items/000.png";
            ballSelector->addItem(QIcon(path), QString("%1 (x%2)").arg(b).arg(pair.second), b);
            totalBalls += pair.second;
        }
    }

    if (totalBalls == 0) {
        controller.addItem("SAFARIBALL", 5);
        QString path = "Items/SAFARIBALL.png";
        if (!QFile::exists(path)) path = "Items/000.png";
        ballSelector->addItem(QIcon(path), "SAFARIBALL (x5)", "SAFARIBALL");
    }

    buttonGrid->addWidget(ballSelector, 0, 0);
    buttonGrid->addWidget(ballButton, 1, 0);

    berrySelector = new QComboBox(this);
    berrySelector->setMinimumHeight(35);
    berryButton = new QPushButton("THROW BERRY");
    berryButton->setObjectName("actionBtn");
    berryButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    berryButton->setMinimumHeight(45);

    int totalBerries = 0;
    for (const auto& pair : bag) {
        if (pair.second > 0 && QString::fromStdString(pair.first).contains("BERRY")) {
            QString b = QString::fromStdString(pair.first);
            QString path = "Items/" + b + ".png";
            if (!QFile::exists(path)) path = "Items/000.png";
            berrySelector->addItem(QIcon(path), QString("%1 (x%2)").arg(b).arg(pair.second), b);
            totalBerries += pair.second;
        }
    }

    if (totalBerries == 0) {
        berryButton->setDisabled(true);
        berrySelector->addItem("No Berries!", "NONE");
    }

    buttonGrid->addWidget(berrySelector, 0, 1);
    buttonGrid->addWidget(berryButton, 1, 1);

    rockButton = new QPushButton("THROW ROCK");
    rockButton->setObjectName("actionBtn");
    rockButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rockButton->setMinimumHeight(45);

    runButton = new QPushButton("RUN");
    runButton->setObjectName("dangerBtn");
    runButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    runButton->setMinimumHeight(45);

    buttonGrid->addWidget(rockButton, 1, 2);
    buttonGrid->addWidget(runButton, 1, 3);

    controlLayout->addLayout(buttonGrid);

    logWidget = new QListWidget(this);
    logWidget->setObjectName("logWidget");
    logWidget->setMinimumHeight(80);
    logWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    controlLayout->addWidget(logWidget);

    mainLayout->addWidget(controlArea);

    setWindowTitle("Safari Zone Encounter - Pokémon Champions");
    setMinimumSize(540, 620);
    resize(540, 650);
}

void SafariZoneDialog::applyTheme(const QString& themeName) {
    QString bg, ctrlBg, textBase, textMuted, accentColor;
    QString btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText;
    QString dangerBorder, dangerHoverBg, dangerHoverText;

    if (themeName == "Pro Dark") {
        bg = "#111318"; ctrlBg = "#0f1115"; textBase = "#ffffff"; textMuted = "#8b949e"; accentColor = "#007acc";
        btnBg = "#1a1c23"; btnText = "#d1d5db"; btnBorder = "#2d303e";
        btnHoverBg = "#242733"; btnHoverBorder = "#007acc"; btnHoverText = "#ffffff";
        dangerBorder = "#e74c3c"; dangerHoverBg = "#2c1618"; dangerHoverText = "#ff6b6b";
    }
    else if (themeName == "Mystic Water") {
        bg = "#b3e5fc"; ctrlBg = "#e1f5fe"; textBase = "#01579b"; textMuted = "#0277bd"; accentColor = "#0288d1";
        btnBg = "#ffffff"; btnText = "#0277bd"; btnBorder = "#81d4fa";
        btnHoverBg = "#b3e5fc"; btnHoverBorder = "#0288d1"; btnHoverText = "#01579b";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
    }
    else if (themeName == "Electric Spark") {
        bg = "#fff9c4"; ctrlBg = "#fffde7"; textBase = "#212121"; textMuted = "#616161"; accentColor = "#f57f17";
        btnBg = "#ffffff"; btnText = "#424242"; btnBorder = "#fff176";
        btnHoverBg = "#fff59d"; btnHoverBorder = "#fbc02d"; btnHoverText = "#212121";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
    }
    else if (themeName == "Leafy Forest") {
        bg = "#dcedc8"; ctrlBg = "#f1f8e9"; textBase = "#1b5e20"; textMuted = "#33691e"; accentColor = "#558b2f";
        btnBg = "#ffffff"; btnText = "#2e7d32"; btnBorder = "#c5e1a5";
        btnHoverBg = "#dcedc8"; btnHoverBorder = "#7cb342"; btnHoverText = "#1b5e20";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
    }
    else if (themeName == "Ember Volcano") {
        bg = "#1a0b0b"; ctrlBg = "#2d1313"; textBase = "#ff4757"; textMuted = "#a36e6e"; accentColor = "#ffa502";
        btnBg = "#2d1313"; btnText = "#ff6b81"; btnBorder = "#ff4757";
        btnHoverBg = "#ff4757"; btnHoverBorder = "#ff4757"; btnHoverText = "#ffffff";
        dangerBorder = "#eccc68"; dangerHoverBg = "#592b2b"; dangerHoverText = "#eccc68";
    }
    else if (themeName == "Psychic Mind") {
        bg = "#190033"; ctrlBg = "#2d1b4e"; textBase = "#e056fd"; textMuted = "#9b7eb5"; accentColor = "#be2edd";
        btnBg = "#2d1b4e"; btnText = "#dcbdf2"; btnBorder = "#8c7ae6";
        btnHoverBg = "#be2edd"; btnHoverBorder = "#e056fd"; btnHoverText = "#ffffff";
        dangerBorder = "#ff4757"; dangerHoverBg = "#4a1c40"; dangerHoverText = "#ff6b81";
    }
    else if (themeName == "Dragon's Den") {
        bg = "#0b0a1a"; ctrlBg = "#15142b"; textBase = "#f5cd79"; textMuted = "#706f8a"; accentColor = "#e66767";
        btnBg = "#15142b"; btnText = "#f5cd79"; btnBorder = "#546de5";
        btnHoverBg = "#546de5"; btnHoverBorder = "#778beb"; btnHoverText = "#ffffff";
        dangerBorder = "#e66767"; dangerHoverBg = "#2b1414"; dangerHoverText = "#ff7979";
    }
    else if (themeName == "Fairy Tale") {
        bg = "#fff0f5"; ctrlBg = "#ffffff"; textBase = "#e84393"; textMuted = "#b78e9b"; accentColor = "#fd79a8";
        btnBg = "#ffffff"; btnText = "#d63031"; btnBorder = "#fab1a0";
        btnHoverBg = "#fd79a8"; btnHoverBorder = "#e84393"; btnHoverText = "#ffffff";
        dangerBorder = "#d63031"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c0392b";
    }
    else if (themeName == "Champion's Gold") {
        bg = "#fdfbfa"; ctrlBg = "#ffffff"; textBase = "#2d3436"; textMuted = "#7f7b71"; accentColor = "#d4af37";
        btnBg = "#ffffff"; btnText = "#2d3436"; btnBorder = "#d4af37";
        btnHoverBg = "#fff6d6"; btnHoverBorder = "#f1c40f"; btnHoverText = "#2d3436";
        dangerBorder = "#e74c3c"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#c0392b";
    }
    else {
        bg = "#e0e0e0"; ctrlBg = "#f4f6f8"; textBase = "#2c3e50"; textMuted = "#7f8c8d"; accentColor = "#3498db";
        btnBg = "#ffffff"; btnText = "#2c3e50"; btnBorder = "#e0e0e0";
        btnHoverBg = "#ecf0f1"; btnHoverBorder = "#3498db"; btnHoverText = "#2c3e50";
        dangerBorder = "#c0392b"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#922b21";
    }

    QString qss = QString(R"(
        QDialog { background-color: %1; font-family: 'Segoe UI', Helvetica, sans-serif; }
        QWidget#controlArea { background-color: %2; border-top: 2px solid %7; }
        
        QLabel#statusLabel { color: %3; font-size: 18px; font-weight: 900; letter-spacing: 1px; }
        
        QListWidget#logWidget { 
            background-color: %6; color: %8; border: 1px solid %7; 
            border-radius: 6px; padding: 6px; font-family: 'Consolas', 'Courier New', monospace; font-size: 12px; 
        }
        
        QComboBox { background-color: %6; color: %3; border: 1px solid %7; border-radius: 4px; padding: 4px 8px; font-weight: 600; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background-color: %6; color: %3; selection-background-color: %9; selection-color: %5; border: 1px solid %7; }

        QPushButton { background-color: %6; color: %8; font-size: 13px; font-weight: 800; padding: 12px 10px; border: 1px solid %7; border-radius: 6px; letter-spacing: 1px; }
        QPushButton#actionBtn:hover { background-color: %9; border: 1px solid %10; color: %11; }
        QPushButton#dangerBtn { border: 1px solid %12; color: %14; }
        QPushButton#dangerBtn:hover { background-color: %13; border: 1px solid %12; color: %14; }
    )").arg(bg, ctrlBg, textBase, textMuted, accentColor, btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText, dangerBorder, dangerHoverBg, dangerHoverText);

    this->setStyleSheet(qss);
}

void SafariZoneDialog::connectSignals() {
    connect(ballButton, &QPushButton::clicked, this, &SafariZoneDialog::onBall);
    connect(berryButton, &QPushButton::clicked, this, &SafariZoneDialog::onBerry);
    connect(rockButton, &QPushButton::clicked, this, &SafariZoneDialog::onRock);
    connect(runButton, &QPushButton::clicked, this, &SafariZoneDialog::onRun);
}

void SafariZoneDialog::log(const QString& msg) { logWidget->insertItem(0, msg); }

void SafariZoneDialog::resetStats() { catchBuff = 0; fleeChance = 20; }

bool SafariZoneDialog::checkFlee(int chance) {
    if (rand() % 100 + 1 <= chance) {
        log(QString("%1 fled!").arg(QString::fromStdString(currentEncounter.getName())));
        statusLabel->setText("The Pokémon got away...");
        ballButton->setDisabled(true); berryButton->setDisabled(true); rockButton->setDisabled(true);
        QTimer::singleShot(1500, this, &QDialog::reject);
        return true;
    }
    return false;
}

void SafariZoneDialog::onBall() {
    if (ballSelector->count() == 0) return;
    QString usedBall = ballSelector->currentData().toString();

    if (!controller.consumeItem(usedBall.toStdString(), 1)) return;

    int remaining = controller.getItemCount(usedBall.toStdString());
    ballSelector->setItemText(ballSelector->currentIndex(), QString("%1 (x%2)").arg(usedBall).arg(remaining));
    if (remaining == 0) ballSelector->removeItem(ballSelector->currentIndex());

    log("You threw a " + usedBall + "!");

    int baseCatch = 15;

    // Guaranteed Catch
    if (usedBall == "MASTERBALL" || usedBall == "PARKBALL") {
        baseCatch = 1000;
    }
    // Standard Modifiers
    else if (usedBall == "POKEBALL" || usedBall == "PREMIERBALL" || usedBall == "LUXURYBALL" ||
        usedBall == "HEALBALL" || usedBall == "FRIENDBALL" || usedBall == "CHERISHBALL" ||
        usedBall == "SPORTBALL" || usedBall == "STRANGEBALL") {
        baseCatch = 15;
    }
    else if (usedBall == "GREATBALL" || usedBall == "SAFARIBALL") {
        baseCatch = 30;
    }
    else if (usedBall == "ULTRABALL") {
        baseCatch = 40;
    }
    // Conditionals
    else if (usedBall == "NETBALL") {
        if (currentEncounter.getType1() == "Water" || currentEncounter.getType2() == "Water" ||
            currentEncounter.getType1() == "Bug" || currentEncounter.getType2() == "Bug") {
            baseCatch = 45;
        }
    }
    else if (usedBall == "DIVEBALL" || usedBall == "LUREBALL") {
        if (currentEncounter.getType1() == "Water" || currentEncounter.getType2() == "Water") { baseCatch = 45; }
    }
    else if (usedBall == "DUSKBALL") {
        if (currentEncounter.getType1() == "Ghost" || currentEncounter.getType2() == "Ghost" ||
            currentEncounter.getType1() == "Dark" || currentEncounter.getType2() == "Dark") {
            baseCatch = 45;
        }
    }
    else if (usedBall == "QUICKBALL") {
        if (turnCount == 1) baseCatch = 60;
        else baseCatch = 10;
    }
    else if (usedBall == "TIMERBALL") {
        baseCatch = std::min(15 + (turnCount * 5), 50);
    }
    else if (usedBall == "REPEATBALL") {
        bool alreadyCaught = false;
        for (const auto& p : controller.getAllPokemon()) {
            if (p.getSpeciesId() == currentEncounter.getSpeciesId()) { alreadyCaught = true; break; }
        }
        if (alreadyCaught) baseCatch = 45;
    }
    else if (usedBall == "FASTBALL") {
        if (currentEncounter.getBaseSpeed() >= 100) baseCatch = 45;
    }
    else if (usedBall == "HEAVYBALL") {
        if (currentEncounter.getWeight() >= 200.0) baseCatch = 45;
    }
    else if (usedBall == "LEVELBALL") {
        baseCatch = 45; // Simulated Safari Zone Advantage
    }
    else if (usedBall == "LOVEBALL" || usedBall == "MOONBALL") {
        baseCatch = 15; // Simplified edge cases
    }
    else if (usedBall == "BEASTBALL") {
        std::vector<std::string> ubs = { "Nihilego", "Buzzwole", "Pheromosa", "Xurkitree", "Celesteela", "Kartana", "Guzzlord", "Poipole", "Naganadel", "Stakataka", "Blacephalon" };
        if (std::find(ubs.begin(), ubs.end(), currentEncounter.getName()) != ubs.end()) {
            baseCatch = 60;
        }
        else {
            baseCatch = 5; // Heavily penalized on normal Pokemon
        }
    }

    int finalCatchRate = baseCatch + catchBuff;

    if (rand() % 100 + 1 <= finalCatchRate) {
        log("Gotcha! The Pokémon was caught!");
        statusLabel->setText("Caught!");

        bool ok;
        QString nickInput = QInputDialog::getText(this, "Catch!", "Gotcha!\nGive it a nickname?", QLineEdit::Normal, QString::fromStdString(currentEncounter.getName()), &ok);
        std::string finalNick = currentEncounter.getName();
        if (ok && !nickInput.trimmed().isEmpty()) finalNick = nickInput.trimmed().toStdString();

        caughtPokemon = currentEncounter;
        caughtPokemon->setId(rand() % 10000);
        caughtPokemon->setLevel(50);
        caughtPokemon->setNickname(finalNick);
        caughtPokemon->setHeldItem("None");

        QTimer::singleShot(500, this, &QDialog::accept);
    }
    else {
        log("Oh no! The Pokémon broke free!");
        turnCount++;
        if (!checkFlee(fleeChance)) resetStats();
    }
}

void SafariZoneDialog::onBerry() {
    if (berrySelector->currentData().toString() == "NONE") return;
    QString usedBerry = berrySelector->currentData().toString();

    if (!controller.consumeItem(usedBerry.toStdString(), 1)) return;

    int remaining = controller.getItemCount(usedBerry.toStdString());
    berrySelector->setItemText(berrySelector->currentIndex(), QString("%1 (x%2)").arg(usedBerry).arg(remaining));
    if (remaining == 0) {
        berrySelector->removeItem(berrySelector->currentIndex());
        if (berrySelector->count() == 0) {
            berrySelector->addItem("No Berries!", "NONE");
            berryButton->setDisabled(true);
        }
    }

    log("You threw an " + usedBerry + "!");

    if (usedBerry == "ORANBERRY") { catchBuff += 0; fleeChance = 10; }
    else if (usedBerry == "RAZZBERRY") { catchBuff += 0; fleeChance = 0; }
    else if (usedBerry == "SITRUSBERRY") { catchBuff += 10; fleeChance = 0; }
    else { catchBuff += 5; fleeChance = 5; }

    log(QString("%1 is eating! It's less likely to flee!").arg(QString::fromStdString(currentEncounter.getName())));
    turnCount++;
}

void SafariZoneDialog::onRock() {
    log("You threw a Rock.");
    catchBuff += 30; fleeChance = 40;
    log(QString("%1 is angry!").arg(QString::fromStdString(currentEncounter.getName())));

    turnCount++;
    checkFlee(fleeChance);
}

void SafariZoneDialog::onRun() {
    log("Got away safely!");
    QTimer::singleShot(500, this, &QDialog::reject);
}