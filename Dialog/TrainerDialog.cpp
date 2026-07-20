#include "TrainerDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QSizePolicy>
#include <QSettings>
#include <QVariant>
#include <QFile> // Added for Sprite Loading
#include <algorithm>

extern std::map<std::string, std::vector<std::string>> speciesAbilitiesDB;

TrainerDialog::TrainerDialog(Controller& ctrl, QWidget* parent) : QDialog(parent), controller(ctrl) {
    setupUI();

    QSettings settings("MyCompany", "PokemonBoxManager");
    QString savedTheme = settings.value("mainHallTheme", "Pro Dark").toString();
    applyTheme(savedTheme);

    refreshPartyList();
}

void TrainerDialog::setupUI() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);

    // ================= LEFT PANEL =================
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(6);

    QLabel* lLabel = new QLabel("SELECT POKÉMON");
    lLabel->setObjectName("sectionHeader");
    leftLayout->addWidget(lLabel);

    QLabel* lLore = new QLabel("Choose a team member to evaluate.");
    lLore->setObjectName("loreText");
    leftLayout->addWidget(lLore);

    partyList = new QListWidget();
    partyList->setMinimumWidth(250);
    partyList->setIconSize(QSize(48, 48)); // Boosted icon size for high-res sprites
    partyList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    leftLayout->addWidget(partyList);

    mainLayout->addLayout(leftLayout, 1);

    // ================= RIGHT PANEL =================
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(6);

    // --- NATURE & ABILITY ---
    QLabel* natAbilHeader = new QLabel("GENETICS & TRAITS");
    natAbilHeader->setObjectName("sectionHeader");
    rightLayout->addWidget(natAbilHeader);

    QGridLayout* natAbilLayout = new QGridLayout();
    natAbilLayout->setVerticalSpacing(8);
    natAbilLayout->setHorizontalSpacing(10);

    natAbilLayout->addWidget(new QLabel("Nature:"), 0, 0);
    natureBox = new QComboBox();
    natureBox->addItems({ "Hardy", "Adamant", "Jolly", "Timid", "Modest", "Bold", "Calm", "Impish", "Careful", "Brave", "Quiet" });
    natureBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    natAbilLayout->addWidget(natureBox, 0, 1);

    natAbilLayout->addWidget(new QLabel("Ability:"), 1, 0);
    abilityBox = new QComboBox();
    abilityBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    natAbilLayout->addWidget(abilityBox, 1, 1);

    applyNatureAbilityBtn = new QPushButton(QString("APPLY TRAITS ($%1)").arg(TRAINING_COST));
    applyNatureAbilityBtn->setObjectName("actionBtn");
    applyNatureAbilityBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    natAbilLayout->addWidget(applyNatureAbilityBtn, 2, 0, 1, 2);

    rightLayout->addLayout(natAbilLayout);
    rightLayout->addSpacing(5);

    // --- EV TRAINING ---
    QLabel* evHeader = new QLabel("EFFORT VALUES (EVs)");
    evHeader->setObjectName("sectionHeader");
    rightLayout->addWidget(evHeader);

    QLabel* evLore = new QLabel(QString("Max %1 Total | Max %2 per Stat").arg(EV_TOTAL_MAX).arg(EV_STAT_MAX));
    evLore->setObjectName("loreText");
    rightLayout->addWidget(evLore);

    QGridLayout* evLayout = new QGridLayout();
    evLayout->setVerticalSpacing(8);
    evLayout->setHorizontalSpacing(10);

    auto setupEv = [&](QSpinBox*& sb, const QString& label, int row, int col) {
        evLayout->addWidget(new QLabel(label), row, col);
        sb = new QSpinBox();
        sb->setRange(0, EV_STAT_MAX);
        sb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), this, &TrainerDialog::updateEvTotal);
        evLayout->addWidget(sb, row, col + 1);
        };

    setupEv(evHp, "HP:", 0, 0); setupEv(evAtk, "Atk:", 0, 2); setupEv(evDef, "Def:", 0, 4);
    setupEv(evSpA, "SpA:", 1, 0); setupEv(evSpD, "SpD:", 1, 2); setupEv(evSpe, "Spe:", 1, 4);

    evTotalLabel = new QLabel(QString("Total Allocated: 0 / %1").arg(EV_TOTAL_MAX));
    evTotalLabel->setAlignment(Qt::AlignCenter);
    evLayout->addWidget(evTotalLabel, 2, 0, 1, 6);

    applyEvBtn = new QPushButton(QString("APPLY EVs ($%1)").arg(TRAINING_COST));
    applyEvBtn->setObjectName("actionBtn");
    applyEvBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    evLayout->addWidget(applyEvBtn, 3, 0, 1, 6);

    rightLayout->addLayout(evLayout);
    rightLayout->addSpacing(5);

    // --- MOVESET ---
    QLabel* cMoveLabel = new QLabel("ACTIVE MOVESET");
    cMoveLabel->setObjectName("sectionHeader");
    rightLayout->addWidget(cMoveLabel);

    currentMovesList = new QListWidget();
    currentMovesList->setMinimumHeight(75);
    currentMovesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightLayout->addWidget(currentMovesList);

    forgetBtn = new QPushButton("FORGET SELECTED MOVE");
    forgetBtn->setObjectName("dangerBtn");
    rightLayout->addWidget(forgetBtn);

    rightLayout->addSpacing(5);

    QLabel* aMoveLabel = new QLabel("LEARNABLE MOVES");
    aMoveLabel->setObjectName("sectionHeader");
    rightLayout->addWidget(aMoveLabel);

    learnableMovesList = new QListWidget();
    learnableMovesList->setMinimumHeight(75);
    learnableMovesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightLayout->addWidget(learnableMovesList);

    learnBtn = new QPushButton(QString("LEARN SELECTED MOVE ($%1)").arg(TRAINING_COST));
    learnBtn->setObjectName("successBtn");
    rightLayout->addWidget(learnBtn);

    rightLayout->addSpacing(5);

    QPushButton* closeBtn = new QPushButton("CLOSE HUB");
    rightLayout->addWidget(closeBtn);

    mainLayout->addLayout(rightLayout, 2);

    connect(partyList, &QListWidget::itemSelectionChanged, this, &TrainerDialog::onPartySelectionChanged);
    connect(applyNatureAbilityBtn, &QPushButton::clicked, this, &TrainerDialog::onApplyNatureAbilityClicked);
    connect(applyEvBtn, &QPushButton::clicked, this, &TrainerDialog::onApplyEvClicked);
    connect(forgetBtn, &QPushButton::clicked, this, &TrainerDialog::onForgetMoveClicked);
    connect(learnBtn, &QPushButton::clicked, this, &TrainerDialog::onLearnMoveClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    setWindowTitle("Training Hub - Pokémon Champions");

    setMinimumSize(800, 650);
    resize(850, 700);
}

void TrainerDialog::applyTheme(const QString& themeName) {
    QString bg, ctrlBg, textBase, textMuted, accentColor;
    QString btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText;
    QString dangerBorder, dangerHoverBg, dangerHoverText;
    QString successBorder, successHoverBg, successHoverText;

    if (themeName == "Pro Dark") {
        bg = "#0f1115"; ctrlBg = "#1a1c23"; textBase = "#ffffff"; textMuted = "#6e7681"; accentColor = "#007acc";
        btnBg = "#1a1c23"; btnText = "#d1d5db"; btnBorder = "#2d303e";
        btnHoverBg = "#242733"; btnHoverBorder = "#007acc"; btnHoverText = "#ffffff";
        dangerBorder = "#e74c3c"; dangerHoverBg = "#2c1618"; dangerHoverText = "#ff6b6b";
        successBorder = "#27ae60"; successHoverBg = "#142b1c"; successHoverText = "#2ecc71";
    }
    else if (themeName == "Mystic Water") {
        bg = "#e1f5fe"; ctrlBg = "#ffffff"; textBase = "#01579b"; textMuted = "#0277bd"; accentColor = "#0288d1";
        btnBg = "#ffffff"; btnText = "#0277bd"; btnBorder = "#81d4fa";
        btnHoverBg = "#b3e5fc"; btnHoverBorder = "#0288d1"; btnHoverText = "#01579b";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
        successBorder = "#388e3c"; successHoverBg = "#c8e6c9"; successHoverText = "#2e7d32";
    }
    else if (themeName == "Electric Spark") {
        bg = "#fffde7"; ctrlBg = "#ffffff"; textBase = "#212121"; textMuted = "#616161"; accentColor = "#f57f17";
        btnBg = "#ffffff"; btnText = "#424242"; btnBorder = "#fff176";
        btnHoverBg = "#fff59d"; btnHoverBorder = "#fbc02d"; btnHoverText = "#212121";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
        successBorder = "#388e3c"; successHoverBg = "#c8e6c9"; successHoverText = "#2e7d32";
    }
    else if (themeName == "Leafy Forest") {
        bg = "#f1f8e9"; ctrlBg = "#ffffff"; textBase = "#1b5e20"; textMuted = "#33691e"; accentColor = "#558b2f";
        btnBg = "#ffffff"; btnText = "#2e7d32"; btnBorder = "#c5e1a5";
        btnHoverBg = "#dcedc8"; btnHoverBorder = "#7cb342"; btnHoverText = "#1b5e20";
        dangerBorder = "#d32f2f"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c62828";
        successBorder = "#388e3c"; successHoverBg = "#c8e6c9"; successHoverText = "#2e7d32";
    }
    else if (themeName == "Ember Volcano") {
        bg = "#1a0b0b"; ctrlBg = "#2d1313"; textBase = "#ff4757"; textMuted = "#a36e6e"; accentColor = "#ffa502";
        btnBg = "#2d1313"; btnText = "#ff6b81"; btnBorder = "#ff4757";
        btnHoverBg = "#ff4757"; btnHoverBorder = "#ff4757"; btnHoverText = "#ffffff";
        dangerBorder = "#eccc68"; dangerHoverBg = "#592b2b"; dangerHoverText = "#eccc68";
        successBorder = "#ff4757"; successHoverBg = "#4a1c1c"; successHoverText = "#ff6b81";
    }
    else if (themeName == "Psychic Mind") {
        bg = "#190033"; ctrlBg = "#2d1b4e"; textBase = "#e056fd"; textMuted = "#9b7eb5"; accentColor = "#be2edd";
        btnBg = "#2d1b4e"; btnText = "#dcbdf2"; btnBorder = "#8c7ae6";
        btnHoverBg = "#be2edd"; btnHoverBorder = "#e056fd"; btnHoverText = "#ffffff";
        dangerBorder = "#ff4757"; dangerHoverBg = "#4a1c40"; dangerHoverText = "#ff6b81";
        successBorder = "#e056fd"; successHoverBg = "#3c1a40"; successHoverText = "#f3a6ff";
    }
    else if (themeName == "Dragon's Den") {
        bg = "#0b0a1a"; ctrlBg = "#15142b"; textBase = "#f5cd79"; textMuted = "#706f8a"; accentColor = "#e66767";
        btnBg = "#15142b"; btnText = "#f5cd79"; btnBorder = "#546de5";
        btnHoverBg = "#546de5"; btnHoverBorder = "#778beb"; btnHoverText = "#ffffff";
        dangerBorder = "#e66767"; dangerHoverBg = "#2b1414"; dangerHoverText = "#ff7979";
        successBorder = "#f5cd79"; successHoverBg = "#332d16"; successHoverText = "#ffeba3";
    }
    else if (themeName == "Fairy Tale") {
        bg = "#fff0f5"; ctrlBg = "#ffffff"; textBase = "#e84393"; textMuted = "#b78e9b"; accentColor = "#fd79a8";
        btnBg = "#ffffff"; btnText = "#d63031"; btnBorder = "#fab1a0";
        btnHoverBg = "#fd79a8"; btnHoverBorder = "#e84393"; btnHoverText = "#ffffff";
        dangerBorder = "#d63031"; dangerHoverBg = "#ffcdd2"; dangerHoverText = "#c0392b";
        successBorder = "#e84393"; successHoverBg = "#fce4ec"; successHoverText = "#d81b60";
    }
    else if (themeName == "Champion's Gold") {
        bg = "#fdfbfa"; ctrlBg = "#ffffff"; textBase = "#2d3436"; textMuted = "#7f7b71"; accentColor = "#d4af37";
        btnBg = "#ffffff"; btnText = "#2d3436"; btnBorder = "#d4af37";
        btnHoverBg = "#fff6d6"; btnHoverBorder = "#f1c40f"; btnHoverText = "#2d3436";
        dangerBorder = "#e74c3c"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#c0392b";
        successBorder = "#d4af37"; successHoverBg = "#fef9e7"; successHoverText = "#b8860b";
    }
    else { // Poké Center
        bg = "#f4f6f8"; ctrlBg = "#ffffff"; textBase = "#2c3e50"; textMuted = "#7f8c8d"; accentColor = "#3498db";
        btnBg = "#ffffff"; btnText = "#2c3e50"; btnBorder = "#e0e0e0";
        btnHoverBg = "#ecf0f1"; btnHoverBorder = "#3498db"; btnHoverText = "#2c3e50";
        dangerBorder = "#c0392b"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#922b21";
        successBorder = "#27ae60"; successHoverBg = "#d5f5e3"; successHoverText = "#1d8348";
    }

    evTotalLabel->setProperty("themeAccent", accentColor);
    evTotalLabel->setProperty("themeError", dangerBorder);

    QString qss = QString(R"(
        QDialog { background-color: %1; font-family: 'Segoe UI', Helvetica, sans-serif; }
        QLabel { color: %3; font-size: 13px; }
        QLabel#sectionHeader { color: %5; font-size: 14px; font-weight: 800; letter-spacing: 1px; }
        QLabel#loreText { color: %4; font-size: 11px; font-style: italic; }
        
        QListWidget { background-color: %2; color: %3; border: 1px solid %7; border-radius: 6px; outline: none; font-weight: 600; padding: 4px; }
        QListWidget::item { padding: 6px; border-radius: 4px; margin-bottom: 2px; }
        QListWidget::item:selected { background-color: %5; color: #ffffff; }
        
        QComboBox, QSpinBox { background-color: %2; color: %3; border: 1px solid %7; border-radius: 4px; padding: 4px 8px; font-weight: 600; min-height: 24px; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background-color: %2; color: %3; selection-background-color: %9; selection-color: %5; border: 1px solid %7; }
        
        QPushButton { background-color: %6; color: %8; font-size: 12px; font-weight: 800; padding: 8px 15px; border: 1px solid %7; border-radius: 6px; letter-spacing: 1px; }
        QPushButton:hover { background-color: %9; border: 1px solid %10; color: %11; }
        QPushButton:pressed { background-color: %10; color: #ffffff; }
        
        QPushButton#actionBtn:hover { border: 1px solid %5; color: %5; }
        QPushButton#dangerBtn:hover { border: 1px solid %12; color: %14; background-color: %13; }
        QPushButton#successBtn:hover { border: 1px solid %15; color: %17; background-color: %16; }
    )").arg(bg, ctrlBg, textBase, textMuted, accentColor, btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText, dangerBorder, dangerHoverBg, dangerHoverText, successBorder, successHoverBg, successHoverText);

    this->setStyleSheet(qss);
    updateEvTotal();
}

void TrainerDialog::updateEvTotal() {
    int total = evHp->value() + evAtk->value() + evDef->value() + evSpA->value() + evSpD->value() + evSpe->value();
    evTotalLabel->setText(QString("Total Allocated: %1 / %2").arg(total).arg(EV_TOTAL_MAX));

    QString color = (total > EV_TOTAL_MAX) ? evTotalLabel->property("themeError").toString() : evTotalLabel->property("themeAccent").toString();
    evTotalLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px;").arg(color));
}

void TrainerDialog::refreshPartyList() {
    partyList->clear();
    for (const auto& p : controller.getAllPokemon()) {
        QString baseName = QString::fromStdString(p.getName());
        QString paddedId = QString("%1").arg(p.getSpeciesId(), 4, 10, QChar('0'));
        QString iconPath = QString("assets/images/%1.png").arg(paddedId);

        if (baseName.contains("-")) {
            QString formSuffix = baseName.mid(baseName.indexOf('-') + 1);
            QString specificPath = QString("assets/images/%1-%2.png").arg(paddedId).arg(formSuffix);
            if (QFile::exists(specificPath)) {
                iconPath = specificPath;
            }
        }

        if (!QFile::exists(iconPath)) iconPath = "Items/000.png";

        // Embed the Pokémon ID into the UserRole for clean logic mapping, and show the Icon/Name
        QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), QString(" ID: %1 - %2").arg(p.getId()).arg(QString::fromStdString(p.getNickname())));
        item->setData(Qt::UserRole, p.getId());
        partyList->addItem(item);
    }
}

void TrainerDialog::onPartySelectionChanged() {
    currentMovesList->clear(); learnableMovesList->clear(); abilityBox->clear();
    if (!partyList->currentItem()) return;

    selectedPartyId = partyList->currentItem()->data(Qt::UserRole).toInt();
    Pokemon p;
    try { p = controller.getPokemonById(selectedPartyId); }
    catch (...) { return; }

    natureBox->setCurrentText(QString::fromStdString(p.getNature()));

    if (speciesAbilitiesDB.find(p.getName()) != speciesAbilitiesDB.end()) {
        for (const auto& ab : speciesAbilitiesDB[p.getName()]) {
            if (!ab.empty()) abilityBox->addItem(QString::fromStdString(ab));
        }
    }
    else {
        abilityBox->addItem(QString::fromStdString(p.getAbility()));
    }
    abilityBox->setCurrentText(QString::fromStdString(p.getAbility()));

    evHp->setValue(p.getEvHp()); evAtk->setValue(p.getEvAtk()); evDef->setValue(p.getEvDef());
    evSpA->setValue(p.getEvSpA()); evSpD->setValue(p.getEvSpD()); evSpe->setValue(p.getEvSpe());

    for (const auto& moveId : p.getMoves()) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(controller.getMoveName(moveId)));
        item->setData(Qt::UserRole, QString::fromStdString(moveId));
        currentMovesList->addItem(item);
    }

    std::vector<std::string> validMoves = controller.getLearnset(p.getName());
    for (const auto& moveId : validMoves) {
        if (std::find(p.getMoves().begin(), p.getMoves().end(), moveId) == p.getMoves().end()) {
            QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(controller.getMoveName(moveId)));
            item->setData(Qt::UserRole, QString::fromStdString(moveId));
            learnableMovesList->addItem(item);
        }
    }
}

void TrainerDialog::onApplyNatureAbilityClicked() {
    if (selectedPartyId == -1) return;
    if (!controller.spendMoney(TRAINING_COST)) { QMessageBox::warning(this, "Funds", "Not enough money!"); return; }

    Pokemon p = controller.getPokemonById(selectedPartyId);
    p.setNature(natureBox->currentText().toStdString());
    p.setAbility(abilityBox->currentText().toStdString());

    controller.updatePokemon(p);
}

void TrainerDialog::onApplyEvClicked() {
    if (selectedPartyId == -1) return;
    int total = evHp->value() + evAtk->value() + evDef->value() + evSpA->value() + evSpD->value() + evSpe->value();
    if (total > EV_TOTAL_MAX) { QMessageBox::warning(this, "EV Limit", "Total EVs cannot exceed limit!"); return; }
    if (!controller.spendMoney(TRAINING_COST)) { QMessageBox::warning(this, "Funds", "Not enough money!"); return; }

    Pokemon p = controller.getPokemonById(selectedPartyId);
    p.setEvs(evHp->value(), evAtk->value(), evDef->value(), evSpA->value(), evSpD->value(), evSpe->value());

    controller.updatePokemon(p);
}

void TrainerDialog::onForgetMoveClicked() {
    if (!currentMovesList->currentItem() || selectedPartyId == -1) return;
    Pokemon p = controller.getPokemonById(selectedPartyId);
    std::vector<std::string> moves = p.getMoves();
    if (moves.size() <= 1) { QMessageBox::warning(this, "Rules", "Must have at least 1 move!"); return; }

    QString moveIdToRemove = currentMovesList->currentItem()->data(Qt::UserRole).toString();
    moves.erase(std::remove(moves.begin(), moves.end(), moveIdToRemove.toStdString()), moves.end());
    controller.updatePokemonMoves(selectedPartyId, moves);
    onPartySelectionChanged();
}

void TrainerDialog::onLearnMoveClicked() {
    if (!learnableMovesList->currentItem() || selectedPartyId == -1) return;
    if (!controller.spendMoney(TRAINING_COST)) { QMessageBox::warning(this, "Funds", "Not enough money!"); return; }

    Pokemon p = controller.getPokemonById(selectedPartyId);
    std::vector<std::string> moves = p.getMoves();
    if (moves.size() >= 4) { QMessageBox::warning(this, "Rules", "Cannot have more than 4 moves!"); return; }

    QString moveIdToAdd = learnableMovesList->currentItem()->data(Qt::UserRole).toString();
    moves.push_back(moveIdToAdd.toStdString());
    controller.updatePokemonMoves(selectedPartyId, moves);
    onPartySelectionChanged();
}