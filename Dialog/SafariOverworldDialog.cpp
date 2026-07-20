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
#include <cstdlib>

SafariOverworldDialog::SafariOverworldDialog(Controller& ctrl, const QString& avatarPath, const QString& zoneName, const std::vector<std::string>& allowedTypes, QWidget* parent)
    : QDialog(parent), controller(ctrl), playerAvatarPath(avatarPath), currentZoneName(zoneName), zoneTypes(allowedTypes) {

    setupUI();

    // Load the globally selected theme
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

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(10, 10, 10, 10);

    // DYNAMIC BIOME COLORING
    QColor zoneColor = Qt::darkGreen; // Default Forest
    if (currentZoneName.contains("Power")) zoneColor = QColor(218, 165, 32);
    else if (currentZoneName.contains("Sewers")) zoneColor = QColor(75, 0, 130);
    else if (currentZoneName.contains("Volcano")) zoneColor = QColor(178, 34, 34);
    else if (currentZoneName.contains("Tundra")) zoneColor = QColor(175, 238, 238);
    else if (currentZoneName.contains("Temple")) zoneColor = QColor(210, 180, 140);
    else if (currentZoneName.contains("Waterfall")) zoneColor = QColor(65, 105, 225);

    QPixmap tileset("Tilesets/route-field.png");
    QPixmap grassTile;
    if (!tileset.isNull()) {
        grassTile = tileset.copy(0, 672, 32, 32);
    }
    else {
        grassTile = QPixmap(32, 32); grassTile.fill(zoneColor); // Apply fallback biome color
    }

    grassGrid.resize(10, std::vector<QLabel*>(10));
    for (int r = 0; r < 10; ++r) {
        for (int c = 0; c < 10; ++c) {
            QLabel* grassLabel = new QLabel(this);
            grassLabel->setPixmap(grassTile);
            grassLabel->setScaledContents(true);
            grassLabel->setFixedSize(40, 40);
            grassLabel->installEventFilter(this);
            grassLabel->setCursor(Qt::PointingHandCursor);

            grassGrid[r][c] = grassLabel;
            gridLayout->addWidget(grassLabel, r, c);
        }
    }
    mainLayout->addLayout(gridLayout);

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

    // Responsive scaling
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
    else if (themeName == "Electric Spark") {
        bg = "#fffde7"; ctrlBg = "#ffffff"; textBase = "#212121"; textMuted = "#616161"; accentColor = "#f57f17";
        btnBg = "#ffffff"; btnText = "#424242"; btnBorder = "#fff176";
        btnHoverBg = "#fff59d"; btnHoverBorder = "#fbc02d"; btnHoverText = "#212121";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
    }
    else if (themeName == "Leafy Forest") {
        bg = "#f1f8e9"; ctrlBg = "#ffffff"; textBase = "#1b5e20"; textMuted = "#33691e"; accentColor = "#558b2f";
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
    else { // Poké Center
        bg = "#f4f6f8"; ctrlBg = "#ffffff"; textBase = "#2c3e50"; textMuted = "#7f8c8d"; accentColor = "#3498db";
        btnBg = "#ffffff"; btnText = "#2c3e50"; btnBorder = "#e0e0e0";
        btnHoverBg = "#ecf0f1"; btnHoverBorder = "#3498db"; btnHoverText = "#2c3e50";
        dangerBorder = "#c0392b"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#922b21";
    }

    QString qss = QString(R"(
        QDialog { background-color: %1; font-family: 'Segoe UI', Helvetica, sans-serif; }
        QLabel#statusLabel { color: %3; font-size: 16px; font-weight: 900; letter-spacing: 1px; }
        
        QListWidget { background-color: %2; color: %3; border: 1px solid %7; border-radius: 6px; outline: none; font-weight: 600; padding: 4px; }
        
        QPushButton { background-color: %6; color: %8; font-size: 13px; font-weight: 800; padding: 12px 20px; border: 1px solid %7; border-radius: 6px; letter-spacing: 1px; }
        QPushButton#dangerBtn { border: 1px solid %12; color: %14; }
        QPushButton#dangerBtn:hover { background-color: %13; border: 1px solid %12; color: %14; }
    )").arg(bg, ctrlBg, textBase, textMuted, accentColor, btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText, dangerBorder, dangerHoverBg, dangerHoverText);

    this->setStyleSheet(qss);
}

void SafariOverworldDialog::connectSignals() {}

void SafariOverworldDialog::log(const QString& msg) {
    logWidget->insertItem(0, msg);
}

void SafariOverworldDialog::onExitZone() {
    QDialog::reject();
}

void SafariOverworldDialog::onRustle() {
    // Determine static color based on zone again for resetting
    QColor zoneColor = Qt::darkGreen;
    if (currentZoneName.contains("Power")) zoneColor = QColor(218, 165, 32);
    else if (currentZoneName.contains("Sewers")) zoneColor = QColor(75, 0, 130);
    else if (currentZoneName.contains("Volcano")) zoneColor = QColor(178, 34, 34);
    else if (currentZoneName.contains("Tundra")) zoneColor = QColor(175, 238, 238);
    else if (currentZoneName.contains("Temple")) zoneColor = QColor(210, 180, 140);
    else if (currentZoneName.contains("Waterfall")) zoneColor = QColor(65, 105, 225);

    QPixmap tileset("Tilesets/route-field.png");
    QPixmap staticGrass;
    if (!tileset.isNull()) staticGrass = tileset.copy(0, 672, 32, 32);
    else { staticGrass = QPixmap(32, 32); staticGrass.fill(zoneColor); }

    if (currentRustleRow != -1 && !staticGrass.isNull()) {
        grassGrid[currentRustleRow][currentRustleCol]->setPixmap(staticGrass);
    }

    currentRustleRow = rand() % 10;
    currentRustleCol = rand() % 10;

    QPixmap rustleGrass;
    if (!tileset.isNull()) {
        rustleGrass = tileset.copy(0, 640, 32, 32);
    }
    else {
        rustleGrass = QPixmap(32, 32); rustleGrass.fill(Qt::yellow);
    }

    if (!rustleGrass.isNull()) {
        grassGrid[currentRustleRow][currentRustleCol]->setPixmap(rustleGrass);
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