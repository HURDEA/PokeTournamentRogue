#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QString>
#include "../Controller/Controller.h"

class MainWindow : public QDialog {
    Q_OBJECT

private:
    Controller& controller;
    QTabWidget* tabs;
    QWidget* managerTab;
    QWidget* statsTab;

    int currentBox = 0;
    QPushButton* prevBoxButton;
    QPushButton* nextBoxButton;
    QLabel* boxLabel;

    QTableWidget* boxTable;

    QLineEdit* nicknameInput;
    QSpinBox* boxInput;

    QComboBox* filterTypeBox;
    QComboBox* filterTypeBox2;
    QComboBox* logicalOperatorBox;
    QPushButton* applyFilterButton;
    QPushButton* clearFilterButton;

    QPushButton* renameButton;  // NEW: Rename feature
    QPushButton* levelUpButton;
    QPushButton* releaseButton;
    QPushButton* giveItemButton;
    QPushButton* moveBoxButton;
    QPushButton* moveUpButton;
    QPushButton* moveDownButton;
    QPushButton* undoButton;
    QPushButton* redoButton;

    QLabel* totalCountLabel;
    QLabel* avgLevelLabel;
    QListWidget* actionLogList;
    QLabel* trainerImageLabel;
    QSpinBox* trainerIdInput;
    QPushButton* changeTrainerButton;

    QString currentTheme;
    QComboBox* themeComboBox;
    QPushButton* editCustomThemeBtn;

    QString customBg;
    QString customAltBg;
    QString customText;
    QString customAccent;
    QString customBorder;

    QString generateQSS(QString bg, QString altBg, QString text, QString accent, QString border);
    void applyTheme(const QString& themeName);
    void configureCustomTheme();

    void setupUI();
    void connectSignalsAndSlots();
    void updateBoxView(const std::vector<Pokemon>& dataToDisplay);
    void refreshDashboard();
    void logAction(const QString& action);
    void updateBoxLabel();

private slots:
    void onPokemonDoubleClicked(int row, int column);
    void onRenameClicked(); // NEW: Rename Slot
    void onLevelUpClicked();
    void onReleaseClicked();
    void onGiveItemClicked();
    void onMoveBoxClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onUndoClicked();
    void onRedoClicked();
    void onApplyFilterClicked();
    void onClearFilterClicked();
    void onPrevBox();
    void onNextBox();
    void onChangeTrainerClicked();
    void onThemeChanged(const QString& themeName);

public:
    MainWindow(Controller& ctrl, QWidget* parent = nullptr);
    ~MainWindow() = default;
};