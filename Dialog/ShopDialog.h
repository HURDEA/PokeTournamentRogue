#pragma once
#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include "../Controller/Controller.h"
#include "../Progression/ProgressionManager.h" // NEW

class ShopDialog : public QDialog {
    Q_OBJECT

private:
    Controller& controller;
    ProgressionManager& progManager; // NEW

    QLabel* moneyLabel;
    QLabel* itemDescLabel;
    QTabWidget* tabs;

    QListWidget* ballsList;
    QListWidget* berriesList;
    QListWidget* stonesList;
    QListWidget* boostsList;
    QListWidget* megasList;
    QListWidget* battleList;

    QPushButton* buyBtn;
    QPushButton* closeBtn;

    void setupUI();
    void populateTabs();
    void updateMoneyDisplay();
    void applyTheme(const QString& themeName);

private slots:
    void onBuyClicked();

public:
    // NEW: Added ProgressionManager& pm
    ShopDialog(Controller& ctrl, ProgressionManager& pm, QWidget* parent = nullptr);
    ~ShopDialog() = default;
};