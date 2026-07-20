#include "MainHall.h"
#include "MainWindow.h"
#include "../Dialog/SafariOverworldDialog.h"
#include "../Dialog/ShopDialog.h"
#include "../Dialog/TrainerDialog.h"
#include "../Dialog/BattleDialog.h" 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QApplication>
#include <QSizePolicy>
#include <QScrollArea> 
#include "../Dialog/GauntletDialog.h"
#include "../Dialog/TournamentDialog.h"

MainHall::MainHall(Controller& ctrl, QWidget* parent) : QMainWindow(parent), controller(ctrl) {
    setupUI();
    connectSignals();

    // Load saved theme or default to Pro Dark
    QSettings settings("MyCompany", "PokemonBoxManager");
    QString savedTheme = settings.value("mainHallTheme", "Pro Dark").toString();

    themeComboBox->blockSignals(true);
    themeComboBox->setCurrentText(savedTheme);
    themeComboBox->blockSignals(false);

    applyTheme(savedTheme);
    refreshProgressionUI(); // Check progression states on boot
}

void MainHall::refreshProgressionUI() {
    if (progManager.getTournamentUnlocked()) {
        tournamentBtn->setText("GRAND TOURNAMENT (UNLOCKED)");
        tournamentBtn->setStyleSheet(QString(R"(
            QPushButton {
                background-color: transparent; color: #e74c3c; font-size: 14px; font-weight: 900;
                padding: 10px; border: 2px solid #e74c3c; border-radius: 6px;
                 letter-spacing: 1px; text-align: center;
            }
            QPushButton:hover { background-color: #e74c3c; color: #ffffff; }
        )"));
    }
    else {
        // Reset to locked styling if progression is wiped
        QString loreColor = themeComboBox->currentText() == "Pro Dark" ? "#6e7681" : "#7f8c8d"; // Fallback colors, fully handled by applyTheme
        tournamentBtn->setText("GRAND TOURNAMENT (LOCKED)");
        applyTheme(themeComboBox->currentText()); // Re-apply full theme to ensure correct locked border color
    }
}

void MainHall::setupUI() {
    centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(30, 20, 30, 30);
    mainLayout->setSpacing(15);

    // --- Top Bar: Guide & Theme Selector ---
    QHBoxLayout* topBarLayout = new QHBoxLayout();

    guideButton = new QPushButton("HOW TO PLAY", this);
    guideButton->setObjectName("infoBtn");
    guideButton->setCursor(Qt::PointingHandCursor);
    guideButton->setMinimumHeight(30);
    topBarLayout->addWidget(guideButton);

    topBarLayout->addStretch();

    themeComboBox = new QComboBox();

    // Expanded Brilliant Theme Roster
    themeComboBox->addItems({
        "Pro Dark", "Poké Center", "Mystic Water", "Electric Spark",
        "Leafy Forest", "Ember Volcano", "Psychic Mind",
        "Dragon's Den", "Fairy Tale", "Champion's Gold"
        });

    themeComboBox->setCursor(Qt::PointingHandCursor);
    themeComboBox->setToolTip("Change the launcher theme");
    themeComboBox->setMinimumHeight(30);
    topBarLayout->addWidget(themeComboBox);
    mainLayout->addLayout(topBarLayout);

    // --- Header Typography ---
    titleLabel = new QLabel("POKÉMON CHAMPIONS", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    subtitleLabel = new QLabel("GRAND TOURNAMENT", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addSpacing(5);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(25);

    // --- Buttons ---
    wildAreaBtn = new QPushButton("EXPLORE WILD AREA");
    pcManagerBtn = new QPushButton("BILL'S PC MANAGER");
    shopBtn = new QPushButton("THE POKÉ MART");
    trainerBtn = new QPushButton("TRAINING HUB");
    battlesBtn = new QPushButton("PRACTICE GAUNTLETS");
    tournamentBtn = new QPushButton("GRAND TOURNAMENT (LOCKED)");

    resetRunBtn = new QPushButton("DANGER: NEW RUN");
    resetRunBtn->setObjectName("dangerBtn");

    exitBtn = new QPushButton("EXIT GAME");

    // Centered layout container for the buttons so they don't stretch too wide on huge monitors
    QVBoxLayout* menuLayout = new QVBoxLayout();
    menuLayout->setAlignment(Qt::AlignHCenter);
    menuLayout->setSpacing(0);

    // Helper lambda to create a responsive, centered button + lore text pairing
    auto addMenuOption = [&](QPushButton* btn, const QString& loreText) {
        QVBoxLayout* optLayout = new QVBoxLayout();
        optLayout->setSpacing(5);
        optLayout->setAlignment(Qt::AlignHCenter);

        // Responsive sizing: Minimum width to look good, max width so it doesn't stretch across ultrawides
        btn->setMinimumWidth(380);
        btn->setMaximumWidth(450);
        btn->setMinimumHeight(55); // Enforce height so text never clips
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setCursor(Qt::PointingHandCursor);
        optLayout->addWidget(btn);

        QLabel* lore = new QLabel(loreText);
        lore->setObjectName("loreLabel"); // For global QSS styling
        lore->setWordWrap(true);
        lore->setAlignment(Qt::AlignCenter);
        lore->setMinimumWidth(380);
        lore->setMaximumWidth(450);
        optLayout->addWidget(lore);

        optLayout->addSpacing(15); // Generous space before the next menu option
        menuLayout->addLayout(optLayout);
        };

    // Build the menu with lore
    addMenuOption(wildAreaBtn, "Track and capture wild Pokémon in their natural, untamed habitats.");
    addMenuOption(pcManagerBtn, "Access the centralized network to organize and evaluate your Pokémon.");
    addMenuOption(shopBtn, "Purchase vital supplies, held items, and resources for your journey.");
    addMenuOption(trainerBtn, "Review your trainer card, stats, and customize your public profile.");
    addMenuOption(battlesBtn, "Test your team's strength in simulated combat scenarios to earn funding.");
    addMenuOption(tournamentBtn, "The ultimate challenge awaits. Only elite trainers may enter.");

    menuLayout->addSpacing(10);
    addMenuOption(resetRunBtn, "Wipe your current progress, team, and inventory to start a fresh journey.");

    menuLayout->addSpacing(10);
    addMenuOption(exitBtn, "Save your progress and return to the real world.");

    mainLayout->addLayout(menuLayout);
    mainLayout->addStretch();

    setCentralWidget(centralWidget);
    setWindowTitle("Pokémon Champions - Client");

    // Completely responsive: Sets a healthy minimum, allowing it to stretch cleanly.
    setMinimumSize(550, 950);
}

void MainHall::applyTheme(const QString& themeName) {
    QSettings settings("MyCompany", "PokemonBoxManager");
    settings.setValue("mainHallTheme", themeName);

    // Theme Palette Variables
    QString bg, titleColor, subColor, loreColor;
    QString btnBg, btnText, btnBorder, btnHoverBg, btnHoverBorder, btnHoverText;
    QString warnHoverBorder, warnHoverBg, warnHoverText;

    if (themeName == "Pro Dark") {
        bg = "#0f1115"; titleColor = "#ffffff"; subColor = "#007acc"; loreColor = "#6e7681";
        btnBg = "#1a1c23"; btnText = "#d1d5db"; btnBorder = "#2d303e";
        btnHoverBg = "#242733"; btnHoverBorder = "#007acc"; btnHoverText = "#ffffff";
        warnHoverBorder = "#e74c3c"; warnHoverBg = "#2c1618"; warnHoverText = "#ff6b6b";
    }
    else if (themeName == "Mystic Water") {
        bg = "#e1f5fe"; titleColor = "#01579b"; subColor = "#0288d1"; loreColor = "#0277bd";
        btnBg = "#ffffff"; btnText = "#0277bd"; btnBorder = "#81d4fa";
        btnHoverBg = "#b3e5fc"; btnHoverBorder = "#0288d1"; btnHoverText = "#01579b";
        warnHoverBorder = "#d32f2f"; warnHoverBg = "#ffcdd2"; warnHoverText = "#c62828";
    }
    else if (themeName == "Electric Spark") {
        bg = "#fffde7"; titleColor = "#212121"; subColor = "#f57f17"; loreColor = "#827717";
        btnBg = "#ffffff"; btnText = "#424242"; btnBorder = "#fff59d";
        btnHoverBg = "#fff9c4"; btnHoverBorder = "#fbc02d"; btnHoverText = "#212121";
        warnHoverBorder = "#d32f2f"; warnHoverBg = "#ffcdd2"; warnHoverText = "#c62828";
    }
    else if (themeName == "Leafy Forest") {
        bg = "#f1f8e9"; titleColor = "#33691e"; subColor = "#689f38"; loreColor = "#558b2f";
        btnBg = "#ffffff"; btnText = "#558b2f"; btnBorder = "#c5e1a5";
        btnHoverBg = "#dcedc8"; btnHoverBorder = "#7cb342"; btnHoverText = "#33691e";
        warnHoverBorder = "#d32f2f"; warnHoverBg = "#ffcdd2"; warnHoverText = "#c62828";
    }
    else if (themeName == "Ember Volcano") {
        bg = "#1a0b0b"; titleColor = "#ff4757"; subColor = "#ffa502"; loreColor = "#a36e6e";
        btnBg = "#2d1313"; btnText = "#ff6b81"; btnBorder = "#ff4757";
        btnHoverBg = "#ff4757"; btnHoverBorder = "#ff4757"; btnHoverText = "#ffffff";
        warnHoverBorder = "#eccc68"; warnHoverBg = "#592b2b"; warnHoverText = "#eccc68";
    }
    else if (themeName == "Psychic Mind") {
        bg = "#190033"; titleColor = "#e056fd"; subColor = "#be2edd"; loreColor = "#9b7eb5";
        btnBg = "#2d1b4e"; btnText = "#dcbdf2"; btnBorder = "#8c7ae6";
        btnHoverBg = "#be2edd"; btnHoverBorder = "#e056fd"; btnHoverText = "#ffffff";
        warnHoverBorder = "#ff4757"; warnHoverBg = "#4a1c40"; warnHoverText = "#ff6b81";
    }
    else if (themeName == "Dragon's Den") {
        bg = "#0b0a1a"; titleColor = "#e66767"; subColor = "#f5cd79"; loreColor = "#706f8a";
        btnBg = "#15142b"; btnText = "#f5cd79"; btnBorder = "#546de5";
        btnHoverBg = "#546de5"; btnHoverBorder = "#778beb"; btnHoverText = "#ffffff";
        warnHoverBorder = "#e66767"; warnHoverBg = "#2b1414"; warnHoverText = "#ff7979";
    }
    else if (themeName == "Fairy Tale") {
        bg = "#fff0f5"; titleColor = "#e84393"; subColor = "#fd79a8"; loreColor = "#b78e9b";
        btnBg = "#ffffff"; btnText = "#d63031"; btnBorder = "#fab1a0";
        btnHoverBg = "#fd79a8"; btnHoverBorder = "#e84393"; btnHoverText = "#ffffff";
        warnHoverBorder = "#d63031"; warnHoverBg = "#ffcdd2"; warnHoverText = "#c0392b";
    }
    else if (themeName == "Champion's Gold") {
        bg = "#fdfbfa"; titleColor = "#d4af37"; subColor = "#f1c40f"; loreColor = "#7f7b71";
        btnBg = "#ffffff"; btnText = "#2d3436"; btnBorder = "#d4af37";
        btnHoverBg = "#fff6d6"; btnHoverBorder = "#f1c40f"; btnHoverText = "#2d3436";
        warnHoverBorder = "#e74c3c"; warnHoverBg = "#f2d7d5"; warnHoverText = "#c0392b";
    }
    else {
        // Poké Center
        bg = "#f4f6f8"; titleColor = "#2c3e50"; subColor = "#e74c3c"; loreColor = "#7f8c8d";
        btnBg = "#ffffff"; btnText = "#2c3e50"; btnBorder = "#e0e0e0";
        btnHoverBg = "#fff5f5"; btnHoverBorder = "#e74c3c"; btnHoverText = "#e74c3c";
        warnHoverBorder = "#c0392b"; warnHoverBg = "#f2d7d5"; warnHoverText = "#922b21";
    }

    // Apply Global Background
    centralWidget->setStyleSheet(QString("QWidget { background-color: %1; font-family: 'Segoe UI', Helvetica, sans-serif; }").arg(bg));

    // Apply Header & Lore Styling
    titleLabel->setStyleSheet(QString("font-size: 28px; font-weight: 900; color: %1; letter-spacing: 2px;").arg(titleColor));
    subtitleLabel->setStyleSheet(QString("font-size: 13px; font-weight: bold; color: %1; letter-spacing: 4px; margin-bottom: 5px;").arg(subColor));

    // Globally styled lore labels (Ensures no margin clipping)
    this->setStyleSheet(QString("QLabel#loreLabel { color: %1; font-size: 11px; padding: 0px 5px; margin-bottom: 2px; }").arg(loreColor));

    // Combobox styling
    themeComboBox->setStyleSheet(QString(R"(
        QComboBox {
             background: %1; color: %2; border: 1px solid %3;
             border-radius: 4px; padding: 6px 12px; font-weight: bold;
         }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: %1; color: %2; selection-background-color: %4;
            selection-color: %5; border: 1px solid %3; outline: none;
        }
    )").arg(btnBg, btnText, btnBorder, btnHoverBg, btnHoverText));

    // Modern Button Style Template (Responsive flex-friendly)
    QString baseBtnQSS = QString(R"(
        QPushButton {
            background-color: %1; color: %2; font-size: 14px; font-weight: 800;
            padding: 10px; border: 2px solid %3; border-radius: 6px;
             letter-spacing: 1px; text-align: center;
        }
        QPushButton:hover { background-color: %4; border: 2px solid %5; color: %6; }
        QPushButton:pressed { background-color: %5; color: #ffffff; }
    )").arg(btnBg, btnText, btnBorder, btnHoverBg, btnHoverBorder, btnHoverText);

    wildAreaBtn->setStyleSheet(baseBtnQSS);
    pcManagerBtn->setStyleSheet(baseBtnQSS);
    shopBtn->setStyleSheet(baseBtnQSS);
    trainerBtn->setStyleSheet(baseBtnQSS);
    battlesBtn->setStyleSheet(baseBtnQSS);

    // Tournament Remains Locked (Dashed border, muted text) unless unlocked
    if (!progManager.getTournamentUnlocked()) {
        tournamentBtn->setStyleSheet(QString(R"(
            QPushButton {
                background-color: transparent; color: %1; font-size: 14px; font-weight: 800;
                padding: 10px; border: 2px dashed %2; border-radius: 6px;
                 letter-spacing: 1px; text-align: center;
            }
        )").arg(loreColor, btnBorder));
    }

    // Exit & Reset Button Warning Colors
    QString exitBtnQSS = QString(R"(
        QPushButton {
            background-color: %1; color: %2; font-size: 14px; font-weight: 800;
            padding: 10px; border: 2px solid %3; border-radius: 6px;
             letter-spacing: 1px; text-align: center;
        }
        QPushButton:hover { background-color: %4; border: 2px solid %5; color: %6; }
    )").arg(btnBg, btnText, btnBorder, warnHoverBg, warnHoverBorder, warnHoverText);

    exitBtn->setStyleSheet(exitBtnQSS);
    resetRunBtn->setStyleSheet(exitBtnQSS);

    // Guide Button Colors
    QString infoBtnQSS = QString(R"(
        QPushButton#infoBtn {
            background-color: transparent; color: %1; border: 2px solid %2; border-radius: 6px; font-weight: bold; padding: 5px 15px; font-size: 11px;
        }
        QPushButton#infoBtn:hover {
            background-color: %2; color: #ffffff;
        }
    )").arg(titleColor, subColor);

    guideButton->setStyleSheet(infoBtnQSS);
}

void MainHall::onThemeChanged(const QString& themeName) {
    applyTheme(themeName);
}

void MainHall::connectSignals() {
    connect(themeComboBox, &QComboBox::currentTextChanged, this, &MainHall::onThemeChanged);
    connect(guideButton, &QPushButton::clicked, this, &MainHall::showHowToPlay);
    connect(wildAreaBtn, &QPushButton::clicked, this, &MainHall::onWildAreaClicked);
    connect(pcManagerBtn, &QPushButton::clicked, this, &MainHall::onPCManagerClicked);
    connect(shopBtn, &QPushButton::clicked, this, &MainHall::onShopClicked);
    connect(resetRunBtn, &QPushButton::clicked, this, &MainHall::onResetRunClicked);
    connect(exitBtn, &QPushButton::clicked, this, &MainHall::onExitClicked);

    connect(trainerBtn, &QPushButton::clicked, this, [this]() {
        TrainerDialog dialog(controller, this);
        dialog.exec();
        });

    connect(battlesBtn, &QPushButton::clicked, this, [this]() {
        GauntletDialog gauntlet(controller, progManager, this);
        this->hide();
        gauntlet.exec();
        this->show();

        // Refresh UI in case they unlocked the tournament while in the Gauntlet!
        refreshProgressionUI();
        });

    connect(tournamentBtn, &QPushButton::clicked, this, [this]() {
        if (!progManager.getTournamentUnlocked()) {
            QMessageBox::warning(this, "Locked", "You must conquer all Practice Gauntlet stages to earn an invitation to the Grand Tournament!");
            return;
        }

        TournamentDialog tournament(controller, this);
        this->hide();
        tournament.exec();
        this->show();
        });
}

void MainHall::showHowToPlay() {
    QDialog guideDialog(this);
    guideDialog.setWindowTitle("Trainer's Guide");
    guideDialog.setFixedSize(500, 600);

    // Copy the current theme style to the popup
    guideDialog.setStyleSheet(this->styleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&guideDialog);

    QLabel* title = new QLabel("HOW TO PLAY");
    title->setObjectName("titleText");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 24px; font-weight: 900; letter-spacing: 2px;"); // Ensure title stands out
    layout->addWidget(title);

    QScrollArea* scroll = new QScrollArea(&guideDialog);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("background: transparent;");

    QWidget* scrollWidget = new QWidget();
    scrollWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

    QLabel* rulesText = new QLabel();
    rulesText->setObjectName("loreLabel");
    rulesText->setWordWrap(true);
    rulesText->setStyleSheet("font-size: 14px; line-height: 1.5;"); // Boosted size for readability
    rulesText->setText(
        "<b>Welcome to Pokémon Champions!</b><br><br>"
        "This is a Rogue-lite Pokémon Simulator. Your ultimate goal is to conquer the Champion's Tournament, but you must build your team, optimize your strategies, and unlock resources to get there.<br><br>"
        "<b>1. Building Your Team</b><br>"
        "Head to the <i>Wild Area</i> to catch wild Pokémon. Catch rates depend heavily on the type of Poké Ball used and current battle conditions.<br><br>"
        "<b>2. Training & Evolution</b><br>"
        "Access <i>Bill's PC</i> to manage your roster. For strict competitive balance, <b>all Pokémon are hard-capped at Level 50.</b><br>"
        "You can manipulate EVs, change Natures, and adjust movesets in the Training Hub.<br>"
        "<i>Evolution:</i> Use standard stones from the Mart, or use the custom <b>Link Cable</b> (for trade evolutions) and the <b>Universal Stone</b> (which overrides complicated friendship/location/time requirements).<br><br>"
        "<b>3. The Practice Gauntlets</b><br>"
        "Before entering the tournament, you must train in the Gauntlet. Fight randomized trainers of increasing difficulty to earn Pokédollars and test your teams.<br><br>"
        "<b>4. The Champion's Tournament</b><br>"
        "Once you have unlocked all features, the Tournament opens. This is a punishing Boss Rush where you must defeat Gym Leaders, Elite Four members, and the Champion consecutively.<br>"
        "<i>Warning:</i> You cannot back down once you enter. If you lose, your run ends, and you must start the tournament over."
    );

    scrollLayout->addWidget(rulesText);
    scroll->setWidget(scrollWidget);
    layout->addWidget(scroll);

    QPushButton* closeBtn = new QPushButton("I'M READY");
    closeBtn->setMinimumHeight(45);
    // Mimic the primary button style
    closeBtn->setStyleSheet("QPushButton { font-size: 14px; font-weight: 800; border-radius: 6px; letter-spacing: 1.5px; border: none; padding: 10px; background-color: #007acc; color: #ffffff;} QPushButton:hover{background-color: #3498db;}");
    connect(closeBtn, &QPushButton::clicked, &guideDialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    guideDialog.exec();
}

void MainHall::onWildAreaClicked() {
    // Dynamically pull unlocked biomes from the progression manager
    std::vector<std::string> rawBiomes = progManager.getUnlockedBiomes();
    QStringList zones;
    for (const auto& b : rawBiomes) {
        zones << QString::fromStdString(b);
    }

    if (zones.isEmpty()) {
        QMessageBox::warning(this, "No Areas Unlocked", "You have no Wild Areas unlocked yet!");
        return;
    }

    bool ok;
    QString choice = QInputDialog::getItem(this, "Select Wild Area", "Choose an unlocked zone to explore:", zones, 0, false, &ok);
    if (!ok) return;

    std::vector<std::string> allowedTypes;
    if (choice.contains("Forest")) allowedTypes = { "Grass", "Bug", "Flying", "Normal" };
    else if (choice.contains("Power")) allowedTypes = { "Electric", "Steel" };
    else if (choice.contains("Sewers")) allowedTypes = { "Poison", "Dark" };
    else if (choice.contains("Volcano")) allowedTypes = { "Ground", "Rock", "Fire" };
    else if (choice.contains("Tundra")) allowedTypes = { "Ice", "Ghost" };
    else if (choice.contains("Temple")) allowedTypes = { "Fighting", "Psychic" };
    else if (choice.contains("Waterfall")) allowedTypes = { "Water", "Fairy", "Dragon" };
    else if (choice.contains("Mt. Moon")) allowedTypes = { "Rock", "Ground", "Fighting" };

    QSettings settings("MyCompany", "PokemonBoxManager");
    int avatarId = settings.value("trainerId", 0).toInt();
    QString trainerPath = QString("Trainers/trainer%1.png").arg(avatarId, 3, 10, QChar('0'));

    SafariOverworldDialog dialog(controller, trainerPath, choice, allowedTypes, this);
    this->hide();
    if (dialog.exec() == QDialog::Accepted) {
        auto caught = dialog.getCaughtPokemon();
        if (caught.has_value()) {
            Pokemon p = caught.value();
            int partyCount = 0;
            for (const auto& poke : controller.getAllPokemon()) {
                if (poke.getBoxNumber() == 0) partyCount++;
            }
            if (partyCount < 6) p.setBoxNumber(0);
            else p.setBoxNumber(1);

            controller.catchPokemon(p);
        }
    }
    this->show();
}

void MainHall::onPCManagerClicked() {
    MainWindow pcDialog(controller, this);
    pcDialog.exec();
}

void MainHall::onShopClicked() {
    // Pass the ProgressionManager to the ShopDialog
    ShopDialog shop(controller, progManager, this);
    shop.exec();
}

void MainHall::onResetRunClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::critical(this, "FACTORY RESET",
        "Are you absolutely sure you want to start a New Run?\n\nThis will permanently delete ALL your Pokémon, items, money, and Gauntlet progress. This CANNOT be undone!",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 1. Wipe the Database
        controller.factoryReset();

        // 2. Wipe the Progression
        progManager.resetRun();

        // 3. Notify the player and refresh the Hub
        QMessageBox::information(this, "Run Reset", "Your save has been wiped. Good luck on your new run!");

        // Refresh UI so the Tournament button re-locks and updates visually
        refreshProgressionUI();
    }
}

void MainHall::onExitClicked() {
    QApplication::quit();
}