#include "SafariOverworldDialog.h"
#include "SafariZoneDialog.h" 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QSettings>
#include <QSizePolicy>
#include <QFile>
#include <cstdlib>
#include <ctime>

SafariOverworldDialog::SafariOverworldDialog(Controller& ctrl, const QString& avatarPath, const QString& zoneName, const std::vector<std::string>& allowedTypes, QWidget* parent)
    : QDialog(parent), controller(ctrl), playerAvatarPath(avatarPath), currentZoneName(zoneName), zoneTypes(allowedTypes) {

    srand(static_cast<unsigned int>(time(nullptr)));

    setupUI();

    QSettings settings("MyCompany", "PokemonBoxManager");
    QString savedTheme = settings.value("mainHallTheme", "Pro Dark").toString();
    applyTheme(savedTheme);

    rustleTimer = new QTimer(this);
    connect(rustleTimer, &QTimer::timeout, this, &SafariOverworldDialog::onRustle);
    rustleTimer->start(3000);

    log(QString("Entered %1! Watch the terrain...").arg(currentZoneName.split("(")[0].trimmed()));
}

void SafariOverworldDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    statusLabel = new QLabel("Observe the terrain...", this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // --- NEW: Cohesive Scene Frame instead of individual tiles ---
    QFrame* sceneFrame = new QFrame(this);
    sceneFrame->setObjectName("sceneFrame");
    sceneFrame->setFixedSize(400, 400);

    QGridLayout* gridLayout = new QGridLayout(sceneFrame);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    grassGrid.resize(10, std::vector<QLabel*>(10));
    for (int r = 0; r < 10; ++r) {
        for (int c = 0; c < 10; ++c) {
            QLabel* cellLabel = new QLabel(sceneFrame);
            cellLabel->setFixedSize(40, 40);

            // Cells are now invisible, acting only as clickable coordinate zones
            cellLabel->setStyleSheet("background: transparent; border: none;");
            cellLabel->installEventFilter(this);
            cellLabel->setCursor(Qt::PointingHandCursor);

            grassGrid[r][c] = cellLabel;
            gridLayout->addWidget(cellLabel, r, c);
        }
    }
    mainLayout->addWidget(sceneFrame, 0, Qt::AlignHCenter);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(15);

    logWidget = new QListWidget(this);
    logWidget->setMinimumHeight(60);
    logWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QPushButton* exitButton = new QPushButton("LEAVE AREA");
    exitButton->setObjectName("dangerBtn");
    exitButton->setMinimumHeight(60);
    connect(exitButton, &QPushButton::clicked, this, &SafariOverworldDialog::onExitZone);

    bottomLayout->addWidget(logWidget, 2);
    bottomLayout->addWidget(exitButton, 1);
    mainLayout->addLayout(bottomLayout);

    setWindowTitle("Wild Area Exploration - Pokémon Champions");
    setMinimumSize(480, 620);
    resize(480, 650);
}

void SafariOverworldDialog::applyTheme(const QString& themeName) {
    QString bg, ctrlBg, textBase, textMuted, accentColor;
    QString btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText;
    QString dangerBorder, dangerHoverBg, dangerHoverText;

    if (themeName == "Pro Dark") {
        bg = "#0f1115"; ctrlBg = "#1a1c23"; textBase = "#ffffff"; textMuted = "#6e7681"; accentColor = "#007acc";
        btnBg = "#1a1c23"; btnText = "#d1d5db"; btnBorder = "#2d303e";
        btnHoverBg = "#242733"; btnHoverBorder = "#007acc"; btnHoverText = "#ffffff";
        dangerBorder = "#e74c3c"; dangerHoverBg = "#2c1618"; dangerHoverText = "#ff6b6b";
    }
    else if (themeName == "Mystic Water") {
        bg = "#e1f5fe"; ctrlBg = "#ffffff"; textBase = "#01579b"; textMuted = "#0277bd"; accentColor = "#0288d1";
        btnBg = "#ffffff"; btnText = "#0277bd"; btnBorder = "#81d4fa";
        btnHoverBg = "#b3e5fc"; btnHoverBorder = "#0288d1"; btnHoverText = "#01579b";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
    }
    else if (themeName == "Ember Volcano") {
        bg = "#1a0b0b"; ctrlBg = "#2d1313"; textBase = "#ff4757"; textMuted = "#a36e6e"; accentColor = "#ffa502";
        btnBg = "#2d1313"; btnText = "#ff6b81"; btnBorder = "#ff4757";
        btnHoverBg = "#ff4757"; btnHoverBorder = "#ff4757"; btnHoverText = "#ffffff";
        dangerBorder = "#eccc68"; dangerHoverBg = "#592b2b"; dangerHoverText = "#eccc68";
    }
    else {
        bg = "#f4f6f8"; ctrlBg = "#ffffff"; textBase = "#2c3e50"; textMuted = "#7f8c8d"; accentColor = "#3498db";
        btnBg = "#ffffff"; btnText = "#2c3e50"; btnBorder = "#e0e0e0";
        btnHoverBg = "#ecf0f1"; btnHoverBorder = "#3498db"; btnHoverText = "#2c3e50";
        dangerBorder = "#c0392b"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#922b21";
    }

    // --- NEW: Dynamic Biome Backgrounds ---
    QString bgName = "forest";
    QString gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #1e3c27, stop:1 #2d5a27)"; // Default Forest

    if (currentZoneName.contains("Power")) {
        bgName = "power"; gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #2c3e50, stop:1 #f39c12)";
    }
    else if (currentZoneName.contains("Sewers")) {
        bgName = "sewers"; gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #1a252f, stop:1 #16a085)";
    }
    else if (currentZoneName.contains("Volcano")) {
        bgName = "volcano"; gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #7f8c8d, stop:1 #c0392b)";
    }
    else if (currentZoneName.contains("Tundra")) {
        bgName = "tundra"; gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #ecf0f1, stop:1 #74b9ff)";
    }
    else if (currentZoneName.contains("Temple")) {
        bgName = "temple"; gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #d35400, stop:1 #f39c12)";
    }
    else if (currentZoneName.contains("Waterfall")) {
        bgName = "waterfall"; gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #0984e3, stop:1 #00cec9)";
    }
    else if (currentZoneName.contains("Mt. Moon")) {
        bgName = "mtmoon"; gradient = "qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #2d3436, stop:1 #636e72)";
    }

    QString bgPath = QString("assets/WildAreas/%1.png").arg(bgName);

    QString sceneStyle = QString("QFrame#sceneFrame { border: 4px solid %1; border-radius: 8px; ").arg(accentColor);
    if (QFile::exists(bgPath)) {
        sceneStyle += QString("background-image: url(%1); background-position: center; }").arg(bgPath);
    }
    else {
        sceneStyle += QString("background: %1; }").arg(gradient);
    }
    // ----------------------------------------

    QString qss = QString(R"(
        QDialog { background-color: %1; font-family: 'Segoe UI', Helvetica, sans-serif; }
        QLabel#statusLabel { color: %3; font-size: 16px; font-weight: 900; letter-spacing: 1px; }
        
        QListWidget { background-color: %2; color: %3; border: 1px solid %7; border-radius: 6px; outline: none; font-weight: 600; padding: 4px; }
        
        QPushButton { background-color: %6; color: %8; font-size: 13px; font-weight: 800; padding: 12px 20px; border: 1px solid %7; border-radius: 6px; letter-spacing: 1px; }
        QPushButton#dangerBtn { border: 1px solid %12; color: %14; }
        QPushButton#dangerBtn:hover { background-color: %13; border: 1px solid %12; color: %14; }
    )").arg(bg, ctrlBg, textBase, textMuted, accentColor, btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText, dangerBorder, dangerHoverBg, dangerHoverText);

    this->setStyleSheet(qss + sceneStyle);
}

void SafariOverworldDialog::connectSignals() {}

void SafariOverworldDialog::log(const QString& msg) {
    logWidget->insertItem(0, msg);
}

void SafariOverworldDialog::onExitZone() {
    QDialog::reject();
}

void SafariOverworldDialog::onRustle() {
    // Clear previous rustle
    if (currentRustleRow != -1 && currentRustleCol != -1) {
        grassGrid[currentRustleRow][currentRustleCol]->setPixmap(QPixmap());
        grassGrid[currentRustleRow][currentRustleCol]->setStyleSheet("background: transparent; border: none;");
    }

    currentRustleRow = rand() % 10;
    currentRustleCol = rand() % 10;

    // Show a distinct visual marker for the rustling spot over the background
    QPixmap rustleIcon("assets/Others/rustle.png");
    if (!rustleIcon.isNull()) {
        grassGrid[currentRustleRow][currentRustleCol]->setPixmap(rustleIcon.scaled(40, 40, Qt::KeepAspectRatio));
    }
    else {
        // Fallback: A pulsing glowing yellow circle
        grassGrid[currentRustleRow][currentRustleCol]->setStyleSheet("background-color: rgba(241, 196, 15, 0.7); border: 2px solid #f39c12; border-radius: 20px;");
    }

    statusLabel->setText("Something moved! Click it!");
    log("Movement detected!");
}

bool SafariOverworldDialog::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel* clickedLabel = qobject_cast<QLabel*>(obj);
        if (clickedLabel && rustleTimer->isActive()) {
            if (currentRustleRow != -1 && clickedLabel == grassGrid[currentRustleRow][currentRustleCol]) {
                log("Encounter triggered!");
                statusLabel->setText("Wild Pokémon appeared!");

                rustleTimer->stop();

                // Clear the marker before battle
                grassGrid[currentRustleRow][currentRustleCol]->setPixmap(QPixmap());
                grassGrid[currentRustleRow][currentRustleCol]->setStyleSheet("background: transparent; border: none;");
                currentRustleRow = -1;

                SafariZoneDialog battleDialog(controller, playerAvatarPath, zoneTypes, this);

                if (battleDialog.exec() == QDialog::Accepted) {
                    overworldCaughtPokemon = battleDialog.getCaughtPokemon();
                    QDialog::accept();
                }
                else {
                    log("Pokémon fled. Watch the terrain...");
                    statusLabel->setText("Observe the terrain...");
                    rustleTimer->start(3000);
                }
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}