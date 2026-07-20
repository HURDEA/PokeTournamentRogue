#include "MainWindow.h"
#include "../Dialog/SummaryDialog.h"
#include <QDebug>
#include <QFile>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QIcon>
#include <QPixmap>
#include <QRect>
#include <QPainter>
#include <QSettings>
#include <QDialog>
#include <QListWidget>
#include <QColorDialog> // Required for custom theme builder

MainWindow::MainWindow(Controller& ctrl, QWidget* parent)
    : QDialog(parent), controller(ctrl) {

    QSettings settings("MyCompany", "PokemonBoxManager");

    // Load theme settings
    currentTheme = settings.value("currentTheme", "Classic Light").toString();
    customBg = settings.value("customBg", "#202225").toString();
    customAltBg = settings.value("customAltBg", "#2f3136").toString();
    customText = settings.value("customText", "#dcddde").toString();
    customAccent = settings.value("customAccent", "#7289da").toString();
    customBorder = settings.value("customBorder", "#202225").toString();

    setupUI();
    connectSignalsAndSlots();

    // Sync combobox without triggering signal prematurely
    themeComboBox->blockSignals(true);
    themeComboBox->setCurrentText(currentTheme);
    themeComboBox->blockSignals(false);

    applyTheme(currentTheme);

    updateBoxLabel();
    updateBoxView(controller.getAllPokemon());
    refreshDashboard();

    trainerIdInput->setValue(settings.value("trainerId", 0).toInt());
    onChangeTrainerClicked();
}

QString MainWindow::generateQSS(QString bg, QString altBg, QString text, QString accent, QString border) {
    QString qss = R"(
        QWidget { background-color: %BG%; color: %TEXT%; font-family: 'Segoe UI', Helvetica, sans-serif; }
        QDialog { background-color: %ALTBG%; }
        QPushButton { 
            background-color: %ALTBG%; border: 1px solid %BORDER%; 
            border-radius: 6px; padding: 6px 12px; font-weight: bold; 
        }
        QPushButton:hover { border: 1px solid %ACCENT%; background-color: %BG%; }
        QPushButton:pressed { background-color: %ACCENT%; color: #ffffff; }
        QComboBox, QSpinBox, QLineEdit { 
            background-color: %ALTBG%; border: 1px solid %BORDER%; 
            border-radius: 4px; padding: 5px; 
        }
        QComboBox::drop-down { border-left: 1px solid %BORDER%; }
        QTableWidget { 
            background-color: %BG%; alternate-background-color: %ALTBG%; 
            border: 1px solid %BORDER%; border-radius: 8px; outline: none;
        }
        QTableWidget::item:selected { background-color: %ACCENT%; color: #ffffff; border-radius: 4px; }
        QTabWidget::pane { border: 1px solid %BORDER%; border-radius: 6px; background-color: %BG%; }
        QTabBar::tab { 
            background: %ALTBG%; padding: 8px 16px; margin-right: 2px;
            border-top-left-radius: 6px; border-top-right-radius: 6px; border: 1px solid %BORDER%; border-bottom: none;
        }
        QTabBar::tab:selected { background: %BG%; color: %ACCENT%; border-top: 3px solid %ACCENT%; }
    )";

    return qss.replace("%BG%", bg)
        .replace("%ALTBG%", altBg)
        .replace("%TEXT%", text)
        .replace("%ACCENT%", accent)
        .replace("%BORDER%", border);
}

void MainWindow::applyTheme(const QString& themeName) {
    currentTheme = themeName;
    QSettings settings("MyCompany", "PokemonBoxManager");
    settings.setValue("currentTheme", currentTheme);

    QString qss;

    if (themeName == "Modern Dark") {
        qss = generateQSS("#1e1e1e", "#252526", "#e0e0e0", "#007acc", "#3e3e42");
    }
    else if (themeName == "Dracula") {
        qss = generateQSS("#282a36", "#44475a", "#f8f8f2", "#bd93f9", "#6272a4");
    }
    else if (themeName == "Monokai") {
        qss = generateQSS("#272822", "#3e3d32", "#f8f8f2", "#f92672", "#75715e");
    }
    else if (themeName == "Solarized Dark") {
        qss = generateQSS("#002b36", "#073642", "#839496", "#2aa198", "#586e75");
    }
    else if (themeName == "Custom") {
        qss = generateQSS(customBg, customAltBg, customText, customAccent, customBorder);
    }
    else {
        // Default Classic Light
        qss = generateQSS("#f3f3f3", "#ffffff", "#1e1e1e", "#1890ff", "#cccccc");
    }

    this->setStyleSheet(qss);
    updateBoxLabel();

    // Enable/Disable custom builder button visually based on selection
    editCustomThemeBtn->setEnabled(themeName == "Custom");
}

void MainWindow::onThemeChanged(const QString& themeName) {
    applyTheme(themeName);
}

void MainWindow::configureCustomTheme() {
    QDialog dialog(this);
    dialog.setWindowTitle("Custom Theme Builder");
    dialog.setFixedSize(300, 250);
    QFormLayout layout(&dialog);

    // C++11 Lambda to generate color picking rows dynamically
    auto makePicker = [&](const QString& labelText, QString& colorVal) {
        QPushButton* btn = new QPushButton(colorVal);
        btn->setStyleSheet(QString("background-color: %1; font-weight: bold; color: palette(text);").arg(colorVal));

        QObject::connect(btn, &QPushButton::clicked, [btn, &colorVal, &dialog]() {
            QColor c = QColorDialog::getColor(QColor(colorVal), &dialog, "Pick Color");
            if (c.isValid()) {
                colorVal = c.name();
                btn->setText(colorVal);
                btn->setStyleSheet(QString("background-color: %1; font-weight: bold; color: palette(text);").arg(colorVal));
            }
            });
        layout.addRow(labelText, btn);
        };

    // Temporaries in case user cancels
    QString tBg = customBg, tAlt = customAltBg, tText = customText, tAcc = customAccent, tBord = customBorder;

    makePicker("Main Background:", tBg);
    makePicker("Widget Background:", tAlt);
    makePicker("Text Color:", tText);
    makePicker("Accent Color:", tAcc);
    makePicker("Border Color:", tBord);

    QPushButton* saveBtn = new QPushButton("Save & Apply");
    saveBtn->setStyleSheet("margin-top: 15px; padding: 10px;");
    layout.addRow(saveBtn);
    QObject::connect(saveBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        customBg = tBg; customAltBg = tAlt; customText = tText; customAccent = tAcc; customBorder = tBord;

        QSettings settings("MyCompany", "PokemonBoxManager");
        settings.setValue("customBg", customBg);
        settings.setValue("customAltBg", customAltBg);
        settings.setValue("customText", customText);
        settings.setValue("customAccent", customAccent);
        settings.setValue("customBorder", customBorder);

        applyTheme("Custom");
    }
}

void MainWindow::updateBoxLabel() {
    if (currentBox == 0) {
        boxLabel->setText("Active Party (Box 0)");
        boxLabel->setStyleSheet("color: #e53935; font-weight: bold; font-size: 14px;");
    }
    else {
        boxLabel->setText(QString("PC Box %1").arg(currentBox));
        boxLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    }
}

void MainWindow::setupUI() {
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    tabs = new QTabWidget(this);

    managerTab = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(managerTab);
    QVBoxLayout* leftLayout = new QVBoxLayout();

    QHBoxLayout* filterLayout = new QHBoxLayout();

    themeComboBox = new QComboBox();
    themeComboBox->addItems({ "Classic Light", "Modern Dark", "Dracula", "Monokai", "Solarized Dark", "Custom" });
    editCustomThemeBtn = new QPushButton("⚙️ Edit Custom");

    filterTypeBox = new QComboBox();
    filterTypeBox->addItems({ "All Types", "Water", "Fire", "Grass", "Electric", "Normal", "Poison", "Bug", "Flying", "Ground", "Rock", "Fighting", "Psychic", "Ghost", "Ice", "Dragon" });

    logicalOperatorBox = new QComboBox();
    logicalOperatorBox->addItems({ "AND", "OR" });

    filterTypeBox2 = new QComboBox();
    filterTypeBox2->addItems({ "None", "Water", "Fire", "Grass", "Electric", "Normal", "Poison", "Bug", "Flying", "Ground", "Rock", "Fighting", "Psychic", "Ghost", "Ice", "Dragon" });

    applyFilterButton = new QPushButton("Search");
    clearFilterButton = new QPushButton("Clear Filters");

    filterLayout->addWidget(themeComboBox);
    filterLayout->addWidget(editCustomThemeBtn);
    filterLayout->addWidget(filterTypeBox);
    filterLayout->addWidget(logicalOperatorBox);
    filterLayout->addWidget(filterTypeBox2);
    filterLayout->addWidget(applyFilterButton);
    filterLayout->addWidget(clearFilterButton);

    QHBoxLayout* boxNavLayout = new QHBoxLayout();
    prevBoxButton = new QPushButton("< Prev Box");
    nextBoxButton = new QPushButton("Next Box >");
    boxLabel = new QLabel("Active Party (Box 0)");
    boxLabel->setAlignment(Qt::AlignCenter);

    boxNavLayout->addWidget(prevBoxButton);
    boxNavLayout->addWidget(boxLabel, 1);
    boxNavLayout->addWidget(nextBoxButton);

    boxTable = new QTableWidget(5, 6);
    boxTable->horizontalHeader()->setVisible(false);
    boxTable->verticalHeader()->setVisible(false);
    boxTable->setSelectionMode(QAbstractItemView::SingleSelection);
    boxTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    boxTable->setIconSize(QSize(64, 64));
    boxTable->setShowGrid(false);

    for (int i = 0; i < 5; ++i) boxTable->setRowHeight(i, 70);
    for (int j = 0; j < 6; ++j) boxTable->setColumnWidth(j, 70);
    boxTable->setFixedSize(6 * 70 + 20, 5 * 70 + 20);

    leftLayout->addLayout(filterLayout);
    leftLayout->addLayout(boxNavLayout);
    leftLayout->addWidget(boxTable, 0, Qt::AlignCenter);
    leftLayout->addStretch();

    mainLayout->addLayout(leftLayout, 2);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    QFormLayout* formLayout = new QFormLayout();

    nicknameInput = new QLineEdit();
    nicknameInput->setPlaceholderText("Enter new name...");
    boxInput = new QSpinBox();
    boxInput->setRange(0, 30);

    formLayout->addRow("Nickname:", nicknameInput);
    formLayout->addRow("Target Box:", boxInput);
    rightLayout->addLayout(formLayout);

    renameButton = new QPushButton("Rename Pokémon");
    levelUpButton = new QPushButton("Level Up (+1)");
    levelUpButton->setObjectName("successBtn");

    releaseButton = new QPushButton("Release (Remove)");
    giveItemButton = new QPushButton("Give/Take Item");
    moveBoxButton = new QPushButton("Move to Box");
    moveUpButton = new QPushButton("Move Up in Party");
    moveDownButton = new QPushButton("Move Down in Party");
    undoButton = new QPushButton("Undo Timeline");
    redoButton = new QPushButton("Redo Timeline");

    rightLayout->addWidget(renameButton);
    rightLayout->addWidget(levelUpButton);
    rightLayout->addWidget(releaseButton);
    rightLayout->addWidget(giveItemButton);
    rightLayout->addWidget(moveBoxButton);
    rightLayout->addWidget(moveUpButton);
    rightLayout->addWidget(moveDownButton);
    rightLayout->addWidget(undoButton);
    rightLayout->addWidget(redoButton);
    rightLayout->addStretch();

    mainLayout->addLayout(rightLayout, 1);

    statsTab = new QWidget();
    QHBoxLayout* statsMainLayout = new QHBoxLayout(statsTab);

    QVBoxLayout* trainerLayout = new QVBoxLayout();
    QLabel* profileTitle = new QLabel("Trainer Profile");
    profileTitle->setStyleSheet("font-weight: bold; font-size: 16px;");

    trainerImageLabel = new QLabel();
    trainerImageLabel->setAlignment(Qt::AlignCenter);
    trainerImageLabel->setFixedSize(150, 150);
    trainerImageLabel->setStyleSheet("border: 2px dashed #aaa;");

    QHBoxLayout* avatarSelectLayout = new QHBoxLayout();
    trainerIdInput = new QSpinBox();
    trainerIdInput->setRange(0, 139);
    changeTrainerButton = new QPushButton("Set Avatar ID");
    avatarSelectLayout->addWidget(trainerIdInput);
    avatarSelectLayout->addWidget(changeTrainerButton);

    trainerLayout->addWidget(profileTitle);
    trainerLayout->addWidget(trainerImageLabel);
    trainerLayout->addLayout(avatarSelectLayout);
    trainerLayout->addStretch();

    QVBoxLayout* statsLayout = new QVBoxLayout();
    totalCountLabel = new QLabel("Total Caught: 0");
    avgLevelLabel = new QLabel("Average Level: 0");
    QFont titleFont = totalCountLabel->font(); titleFont.setPointSize(14); titleFont.setBold(true);
    totalCountLabel->setFont(titleFont); avgLevelLabel->setFont(titleFont);
    actionLogList = new QListWidget();

    statsLayout->addWidget(totalCountLabel);
    statsLayout->addWidget(avgLevelLabel);
    statsLayout->addWidget(new QLabel("System Action Log:"));
    statsLayout->addWidget(actionLogList);

    statsMainLayout->addLayout(trainerLayout, 1);
    statsMainLayout->addLayout(statsLayout, 2);

    tabs->addTab(managerTab, "PC Box Manager");
    tabs->addTab(statsTab, "Trainer Card & Logs");

    rootLayout->addWidget(tabs);
    setLayout(rootLayout);

    setWindowTitle("Bill's PC - Box Manager");
    resize(950, 600);
}

void MainWindow::connectSignalsAndSlots() {
    connect(boxTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onPokemonDoubleClicked);

    connect(renameButton, &QPushButton::clicked, this, &MainWindow::onRenameClicked); // NEW
    connect(levelUpButton, &QPushButton::clicked, this, &MainWindow::onLevelUpClicked);
    connect(releaseButton, &QPushButton::clicked, this, &MainWindow::onReleaseClicked);
    connect(giveItemButton, &QPushButton::clicked, this, &MainWindow::onGiveItemClicked);
    connect(moveBoxButton, &QPushButton::clicked, this, &MainWindow::onMoveBoxClicked);
    connect(moveUpButton, &QPushButton::clicked, this, &MainWindow::onMoveUpClicked);
    connect(moveDownButton, &QPushButton::clicked, this, &MainWindow::onMoveDownClicked);
    connect(undoButton, &QPushButton::clicked, this, &MainWindow::onUndoClicked);
    connect(redoButton, &QPushButton::clicked, this, &MainWindow::onRedoClicked);
    connect(applyFilterButton, &QPushButton::clicked, this, &MainWindow::onApplyFilterClicked);
    connect(clearFilterButton, &QPushButton::clicked, this, &MainWindow::onClearFilterClicked);
    connect(prevBoxButton, &QPushButton::clicked, this, &MainWindow::onPrevBox);
    connect(nextBoxButton, &QPushButton::clicked, this, &MainWindow::onNextBox);
    connect(changeTrainerButton, &QPushButton::clicked, this, &MainWindow::onChangeTrainerClicked);

    connect(themeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::onThemeChanged);
    connect(editCustomThemeBtn, &QPushButton::clicked, this, &MainWindow::configureCustomTheme);
}


void MainWindow::onLevelUpClicked() {
    QTableWidgetItem* selected = boxTable->currentItem();
    if (!selected || selected->data(Qt::UserRole).isNull()) {
        QMessageBox::warning(this, "Error", "Select a Pokémon to evolve/level up!");
        return;
    }
    int id = selected->data(Qt::UserRole).toInt();
    Pokemon p = controller.getPokemonById(id);

    // Hardcap at 50 for Game Balance
    if (p.getLevel() != 50) {
        Pokemon newP = p;
        newP.setLevel(50);
        controller.updatePokemon(newP);
        logAction(QString("Leveled up %1 to Max Level (50)!").arg(QString::fromStdString(p.getNickname())));
    }

    std::string evoTarget = controller.checkEvolution(id, "");
    if (!evoTarget.empty()) {
        QMessageBox::StandardButton res = QMessageBox::question(this, "Evolution!",
            QString("What? %1 is evolving!\nAllow evolution into %2?")
            .arg(QString::fromStdString(p.getNickname()), QString::fromStdString(evoTarget)));

        if (res == QMessageBox::Yes) {
            Pokemon baseStats = controller.getSpeciesDataByName(evoTarget);
            if (baseStats.getSpeciesId() != 0) {
                controller.evolvePokemon(id, baseStats.getSpeciesId());
                QMessageBox::information(this, "Evolution Success!",
                    QString("Congratulations! Your %1 evolved into %2!")
                    .arg(QString::fromStdString(p.getNickname()), QString::fromStdString(evoTarget)));
                logAction(QString("%1 evolved into %2!").arg(QString::fromStdString(p.getNickname()), QString::fromStdString(evoTarget)));
            }
        }
    }
    else {
        if (p.getLevel() == 50) {
            QMessageBox::information(this, "Max Level", QString("%1 is at Max Level (50) and has no available level-up evolutions right now.").arg(QString::fromStdString(p.getNickname())));
        }
    }
    updateBoxView(controller.getAllPokemon());
    refreshDashboard();
}

void MainWindow::onMoveUpClicked() {
    QTableWidgetItem* selected = boxTable->currentItem();
    if (!selected || selected->data(Qt::UserRole).isNull()) { QMessageBox::warning(this, "Error", "Select a Pokémon to move!"); return; }
    int id = selected->data(Qt::UserRole).toInt();

    std::vector<Pokemon> party;
    for (const auto& p : controller.getAllPokemon()) if (p.getBoxNumber() == currentBox) party.push_back(p);
    int idx = -1;
    for (size_t i = 0; i < party.size(); ++i) if (party[i].getId() == id) { idx = (int)i; break; }
    if (idx == -1) return;
    if (idx == 0) return; // already top

    controller.reorderPartyMember(id, idx - 1);
    updateBoxView(controller.getAllPokemon());
    logAction(QString("Moved Pokémon ID %1 up in Box %2").arg(id).arg(currentBox));
}

void MainWindow::onMoveDownClicked() {
    QTableWidgetItem* selected = boxTable->currentItem();
    if (!selected || selected->data(Qt::UserRole).isNull()) { QMessageBox::warning(this, "Error", "Select a Pokémon to move!"); return; }
    int id = selected->data(Qt::UserRole).toInt();

    std::vector<Pokemon> party;
    for (const auto& p : controller.getAllPokemon()) if (p.getBoxNumber() == currentBox) party.push_back(p);
    int idx = -1;
    for (size_t i = 0; i < party.size(); ++i) if (party[i].getId() == id) { idx = (int)i; break; }
    if (idx == -1) return;
    if (idx >= (int)party.size() - 1) return; // already bottom

    controller.reorderPartyMember(id, idx + 1);
    updateBoxView(controller.getAllPokemon());
    logAction(QString("Moved Pokémon ID %1 down in Box %2").arg(id).arg(currentBox));
}

void MainWindow::onPokemonDoubleClicked(int row, int column) {
    QTableWidgetItem* item = boxTable->item(row, column);
    if (!item || item->data(Qt::UserRole).isNull()) return;

    int partyId = item->data(Qt::UserRole).toInt();
    SummaryDialog dialog(controller, partyId, this);
    dialog.exec();
}

void MainWindow::onPrevBox() {
    if (currentBox > 0) {
        currentBox--;
        boxInput->setValue(currentBox);
        updateBoxLabel();
        updateBoxView(controller.getAllPokemon());
    }
}

void MainWindow::onNextBox() {
    if (currentBox < 30) {
        currentBox++;
        boxInput->setValue(currentBox);
        updateBoxLabel();
        updateBoxView(controller.getAllPokemon());
    }
}

void MainWindow::onChangeTrainerClicked() {
    int id = trainerIdInput->value();
    QSettings settings("MyCompany", "PokemonBoxManager");
    settings.setValue("trainerId", id);

    QString fileName = QString("trainer%1.png").arg(id, 3, 10, QChar('0'));
    QString fullPath = "Trainers/" + fileName;
    QPixmap sprite(fullPath);
    if (!sprite.isNull()) {
        trainerImageLabel->setPixmap(sprite.scaled(150, 150, Qt::KeepAspectRatio));
        logAction(QString("Trainer Avatar changed to ID %1").arg(id));
    }
    else {
        QMessageBox::warning(this, "Missing File", "Could not find: " + fullPath);
    }
}

void MainWindow::updateBoxView(const std::vector<Pokemon>& dataToDisplay) {
    boxTable->clearContents();

    int row = 0;
    int col = 0;

    for (const auto& p : dataToDisplay) {
        if (row >= 5) break;
        if (p.getBoxNumber() != currentBox) continue;

        QString baseName = QString::fromStdString(p.getName());
        QString paddedId = QString("%1").arg(p.getSpeciesId(), 4, 10, QChar('0'));

        // SWITCHED: Now points to the lighter 'images' folder
        QString iconPath = QString("assets/images/%1.png").arg(paddedId);

        if (baseName.contains("-")) {
            QString formSuffix = baseName.mid(baseName.indexOf('-') + 1);
            // SWITCHED: Now points to the lighter 'images' folder
            QString specificPath = QString("assets/images/%1-%2.png").arg(paddedId).arg(formSuffix);
            if (QFile::exists(specificPath)) {
                iconPath = specificPath;
            }
        }

        if (!QFile::exists(iconPath)) iconPath = "Items/000.png";

        QPixmap fullSprite(iconPath);
        QTableWidgetItem* item = new QTableWidgetItem();

        if (!fullSprite.isNull()) {
            QPixmap singleFrame = fullSprite;

            if (p.getHeldItem() != "None" && !p.getHeldItem().empty()) {
                QString itemSpritePath = QString("Items/%1.png").arg(QString::fromStdString(p.getHeldItem()));
                if (!QFile::exists(itemSpritePath)) itemSpritePath = "Items/000.png";

                QPixmap itemSprite(itemSpritePath);
                if (!itemSprite.isNull()) {
                    QPixmap miniItem = itemSprite.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    QPainter painter(&singleFrame);
                    painter.drawPixmap(singleFrame.width() - miniItem.width() - 2, singleFrame.height() - miniItem.height() - 2, miniItem);
                    painter.end();
                }
            }
            item->setIcon(QIcon(singleFrame.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        }
        else {
            item->setText(QString::fromStdString(p.getNickname()));
        }

        item->setData(Qt::UserRole, p.getId());
        item->setToolTip(QString("ID: %1 | %2 (Lv. %3)\nDouble-Click for Summary")
            .arg(p.getId()).arg(QString::fromStdString(p.getNickname())).arg(p.getLevel()));

        if (row < boxTable->rowCount() && col < boxTable->columnCount())
            boxTable->setItem(row, col, item);
        else {
            delete item;
        }

        col++;
        if (col >= 6) {
            col = 0;
            row++;
            if (row >= 5) break;
        }
    }
}
void MainWindow::onMoveBoxClicked() {
    QTableWidgetItem* selected = boxTable->currentItem();
    if (!selected || selected->data(Qt::UserRole).isNull()) { QMessageBox::warning(this, "Error", "Select a Pokémon to move!"); return; }

    int id = selected->data(Qt::UserRole).toInt();
    int targetBox = boxInput->value();

    // Box Capacity Check
    int maxCapacity = (targetBox == 0) ? 6 : 30;
    int currentCount = 0;
    for (const auto& p : controller.getAllPokemon()) {
        if (p.getBoxNumber() == targetBox) {
            currentCount++;
        }
    }

    if (currentCount >= maxCapacity) {
        QMessageBox::warning(this, "Box Full",
            (targetBox == 0) ? "Your Active Party is full (Max 6)!" : QString("Box %1 is full (Max 30)!").arg(targetBox));
        return;
    }

    bool moved = controller.movePokemonBox(id, targetBox);
    if (!moved) {
        QMessageBox::warning(this, "Move Failed", "Could not move Pokémon: target box is full or move is invalid.");
        return;
    }
    updateBoxView(controller.getAllPokemon());
    logAction(QString("Moved Pokémon ID %1 to Box %2").arg(id).arg(targetBox));
}

void MainWindow::onGiveItemClicked() {
    QTableWidgetItem* selected = boxTable->currentItem();
    if (!selected || selected->data(Qt::UserRole).isNull()) {
        QMessageBox::warning(this, "Error", "Select a Pokémon to give an item to!");
        return;
    }

    int partyId = selected->data(Qt::UserRole).toInt();
    Pokemon p = controller.getPokemonById(partyId);

    QDialog inventoryDialog(this);
    inventoryDialog.setWindowTitle("Your Bag");
    inventoryDialog.setFixedSize(350, 400);
    QVBoxLayout* layout = new QVBoxLayout(&inventoryDialog);

    QListWidget* list = new QListWidget();
    list->setIconSize(QSize(24, 24));
    layout->addWidget(new QLabel("Select an item to give:"));

    QListWidgetItem* removeOption = new QListWidgetItem("Take current item (Return to Bag)");
    removeOption->setData(Qt::UserRole, "None");
    list->addItem(removeOption);

    std::map<std::string, int> bag = controller.getInventory();
    for (const auto& item : bag) {
        if (item.second > 0) {
            QString itemPath = "Items/" + QString::fromStdString(item.first) + ".png";
            if (!QFile::exists(itemPath)) itemPath = "Items/000.png";

            QListWidgetItem* listItem = new QListWidgetItem(QIcon(itemPath), QString("%1 (x%2)").arg(QString::fromStdString(item.first)).arg(item.second));
            listItem->setData(Qt::UserRole, QString::fromStdString(item.first));
            list->addItem(listItem);
        }
    }
    layout->addWidget(list);

    QPushButton* confirmBtn = new QPushButton("Confirm");
    layout->addWidget(confirmBtn);
    connect(confirmBtn, &QPushButton::clicked, &inventoryDialog, &QDialog::accept);

    if (inventoryDialog.exec() == QDialog::Accepted) {
        if (!list->currentItem()) return;

        QString selectedItem = list->currentItem()->data(Qt::UserRole).toString();
        std::string oldItem = p.getHeldItem();

        // CHECK FOR ITEM-BASED EVOLUTION FIRST
        std::string evoTarget = controller.checkEvolution(partyId, selectedItem.toStdString());
        if (!evoTarget.empty()) {
            QMessageBox::StandardButton res = QMessageBox::question(this, "Evolution!",
                QString("Using %1 will evolve %2 into %3. Proceed?")
                .arg(selectedItem, QString::fromStdString(p.getNickname()), QString::fromStdString(evoTarget)));

            if (res == QMessageBox::Yes) {
                if (controller.consumeItem(selectedItem.toStdString(), 1)) {
                    Pokemon baseStats = controller.getSpeciesDataByName(evoTarget);
                    if (baseStats.getSpeciesId() != 0) {
                        controller.evolvePokemon(partyId, baseStats.getSpeciesId());
                        QMessageBox::information(this, "Evolution!", "Evolution successful!");
                        logAction(QString("Used %1 to evolve %2 into %3.").arg(selectedItem, QString::fromStdString(p.getNickname()), QString::fromStdString(evoTarget)));
                    }
                }
            }
            updateBoxView(controller.getAllPokemon());
            refreshDashboard();
            return; // Return early so they don't hold the stone
        }

        // Standard hold-item logic
        if (selectedItem == "None") {
            if (oldItem != "None") {
                controller.addItem(oldItem, 1);
                controller.updatePokemonItem(partyId, "None");
                logAction("Returned " + QString::fromStdString(oldItem) + " to bag.");
            }
        }
        else {
            if (controller.consumeItem(selectedItem.toStdString(), 1)) {
                if (oldItem != "None") controller.addItem(oldItem, 1);
                controller.updatePokemonItem(partyId, selectedItem.toStdString());
                logAction("Gave " + selectedItem + " to Pokémon.");
            }
        }
        updateBoxView(controller.getAllPokemon());
    }
}



void MainWindow::refreshDashboard() {
    std::vector<Pokemon> all = controller.getAllPokemon();
    totalCountLabel->setText(QString("Total Caught: %1").arg(all.size()));
    if (all.empty()) { avgLevelLabel->setText("Average Level: 0"); return; }
    int totalLevel = 0;
    for (const auto& p : all) totalLevel += p.getLevel();
    avgLevelLabel->setText(QString("Average Level: %1").arg(totalLevel / all.size()));
}

void MainWindow::logAction(const QString& action) { actionLogList->insertItem(0, action); }

void MainWindow::onReleaseClicked() {
    QTableWidgetItem* selected = boxTable->currentItem();
    if (!selected || selected->data(Qt::UserRole).isNull()) { QMessageBox::warning(this, "Error", "Select a Pokémon to release!"); return; }

    if (QMessageBox::question(this, "Confirm Release", "Are you sure?") == QMessageBox::Yes) {
        int id = selected->data(Qt::UserRole).toInt();
        controller.releasePokemon(id);
        updateBoxView(controller.getAllPokemon());
        refreshDashboard();
        logAction(QString("Released Pokemon (ID: %1)").arg(id));
    }
}

void MainWindow::onUndoClicked() { controller.undo(); updateBoxView(controller.getAllPokemon()); refreshDashboard(); }
void MainWindow::onRedoClicked() { controller.redo(); updateBoxView(controller.getAllPokemon()); refreshDashboard(); }

void MainWindow::onApplyFilterClicked() {
    std::string type1 = filterTypeBox->currentText().toStdString();
    std::string type2 = filterTypeBox2->currentText().toStdString();
    std::string op = logicalOperatorBox->currentText().toStdString();

    TypeFilter f1(type1); TypeFilter f2(type2);
    if (type2 != "None") {
        if (op == "AND") updateBoxView(controller.filterPokemon(AndFilter(f1, f2)));
        else updateBoxView(controller.filterPokemon(OrFilter(f1, f2)));
    }
    else updateBoxView(controller.filterPokemon(f1));
}

void MainWindow::onClearFilterClicked() {
    filterTypeBox->setCurrentIndex(0); filterTypeBox2->setCurrentIndex(0); logicalOperatorBox->setCurrentIndex(0);
    updateBoxView(controller.getAllPokemon());
}

void MainWindow::onRenameClicked() {
    QTableWidgetItem* selected = boxTable->currentItem();
    if (!selected || selected->data(Qt::UserRole).isNull()) {
        QMessageBox::warning(this, "Error", "Select a Pokémon to rename!");
        return;
    }

    QString newName = nicknameInput->text().trimmed();
    if (newName.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a valid nickname in the input box!");
        return;
    }

    int id = selected->data(Qt::UserRole).toInt();
    Pokemon p = controller.getPokemonById(id);

    p.setNickname(newName.toStdString());
    controller.updatePokemon(p);

    updateBoxView(controller.getAllPokemon());
    logAction(QString("Renamed Pokémon to %1").arg(newName));
    nicknameInput->clear();
}