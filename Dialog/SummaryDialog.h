#pragma once
#include <QDialog>
#include "../Controller/Controller.h"

class SummaryDialog : public QDialog {
    Q_OBJECT
private:
    Controller& controller;
    int partyId;

    void setupUI();

public:
    SummaryDialog(Controller& ctrl, int pId, QWidget* parent = nullptr);
    ~SummaryDialog() = default;
};