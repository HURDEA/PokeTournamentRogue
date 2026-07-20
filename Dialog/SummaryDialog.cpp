#include "SummaryDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QFrame>
#include <QFile>
#include <QColorDialog>
#include <QPushButton>
#include <QSettings>
// --- New Includes for Premium UI ---
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>

SummaryDialog::SummaryDialog(Controller& ctrl, int pId, QWidget* parent)
    : QDialog(parent), controller(ctrl), partyId(pId) {
    setupUI();
}

void SummaryDialog::setupUI() {
    Pokemon p = controller.getPokemonById(partyId);

    // --- Premium UI: Smooth Fade-In Animation ---
    this->setWindowOpacity(0.0);
    QPropertyAnimation* fadeAnim = new QPropertyAnimation(this, "windowOpacity");
    fadeAnim->setDuration(350); // 350ms fade
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutQuad);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    // --- Left Panel: Visuals ---
    QVBoxLayout* leftLayout = new QVBoxLayout();

    QString baseName = QString::fromStdString(p.getName());
    QString paddedId = QString("%1").arg(p.getSpeciesId(), 4, 10, QChar('0'));
    QString iconPath = QString("assets/images/%1.png").arg(paddedId);

    if (baseName.contains("-")) {
        QString formSuffix = baseName.mid(baseName.indexOf('-') + 1);
        QString specificPath = QString("assets/images/%1-%2.png").arg(paddedId).arg(formSuffix);
        if (QFile::exists(specificPath)) iconPath = specificPath;
    }
    if (!QFile::exists(iconPath)) iconPath = "Items/000.png";

    QLabel* imgLabel = new QLabel();
    QPixmap sprite(iconPath);
    if (!sprite.isNull()) imgLabel->setPixmap(sprite.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    leftLayout->addWidget(imgLabel, 0, Qt::AlignCenter);

    QLabel* nameLabel = new QLabel(QString("<b>%1</b><br>Lv. %2").arg(QString::fromStdString(p.getNickname())).arg(p.getLevel()));
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setObjectName("nameLabel"); // Tagged for the theme engine
    leftLayout->addWidget(nameLabel);

    QHBoxLayout* typeLayout = new QHBoxLayout();
    QString t1 = QString::fromStdString(p.getType1()).toLower();
    QLabel* t1Icon = new QLabel();
    t1Icon->setPixmap(QPixmap("assets/Others/type-icons/png/" + t1 + ".png").scaled(50, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    typeLayout->addWidget(t1Icon);

    if (p.getType2() != "None" && !p.getType2().empty()) {
        QString t2 = QString::fromStdString(p.getType2()).toLower();
        QLabel* t2Icon = new QLabel();
        t2Icon->setPixmap(QPixmap("assets/Others/type-icons/png/" + t2 + ".png").scaled(50, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        typeLayout->addWidget(t2Icon);
    }
    typeLayout->setAlignment(Qt::AlignCenter);
    leftLayout->addLayout(typeLayout);

    // Held Item Box 
    QFrame* itemBox = new QFrame();
    itemBox->setFrameShape(QFrame::StyledPanel);
    itemBox->setObjectName("dataPanel"); // Tagged for theme engine
    QHBoxLayout* itemLayout = new QHBoxLayout(itemBox);

    QString heldItem = QString::fromStdString(p.getHeldItem());
    QString itemIconPath = "Items/" + heldItem + ".png";
    if (!QFile::exists(itemIconPath)) itemIconPath = "Items/000.png";

    QLabel* itemIconLabel = new QLabel();
    itemIconLabel->setPixmap(QPixmap(itemIconPath).scaled(32, 32, Qt::KeepAspectRatio));

    QString itemDesc = QString::fromStdString(controller.getItemDescription(heldItem.toLower().toStdString()));
    if (heldItem == "None" || itemDesc.isEmpty()) itemDesc = "No effect.";

    QLabel* itemDescLabel = new QLabel(QString("<b>%1</b><br><small>%2</small>").arg(heldItem).arg(itemDesc));
    itemDescLabel->setWordWrap(true);

    itemLayout->addWidget(itemIconLabel);
    itemLayout->addWidget(itemDescLabel, 1);
    leftLayout->addWidget(itemBox);

    // --- Right Panel: Data ---
    QVBoxLayout* rightLayout = new QVBoxLayout();

    // Stat Box
    QFrame* statBox = new QFrame();
    statBox->setFrameShape(QFrame::StyledPanel);
    statBox->setObjectName("dataPanel");
    QGridLayout* statsGrid = new QGridLayout(statBox);

    statsGrid->addWidget(new QLabel("<b>HP:</b>"), 0, 0);   statsGrid->addWidget(new QLabel(QString::number(p.getHp())), 0, 1);
    statsGrid->addWidget(new QLabel("<b>Atk:</b>"), 1, 0);  statsGrid->addWidget(new QLabel(QString::number(p.getAttack())), 1, 1);
    statsGrid->addWidget(new QLabel("<b>Def:</b>"), 2, 0);  statsGrid->addWidget(new QLabel(QString::number(p.getDefense())), 2, 1);
    statsGrid->addWidget(new QLabel("<b>Sp.A:</b>"), 0, 2); statsGrid->addWidget(new QLabel(QString::number(p.getSpAttack())), 0, 3);
    statsGrid->addWidget(new QLabel("<b>Sp.D:</b>"), 1, 2); statsGrid->addWidget(new QLabel(QString::number(p.getSpDefense())), 1, 3);
    statsGrid->addWidget(new QLabel("<b>Spe:</b>"), 2, 2);  statsGrid->addWidget(new QLabel(QString::number(p.getSpeed())), 2, 3);
    rightLayout->addWidget(statBox);
    rightLayout->addSpacing(10);

    // Moves Box
    QFrame* movesBox = new QFrame();
    movesBox->setFrameShape(QFrame::StyledPanel);
    movesBox->setObjectName("dataPanel");
    QVBoxLayout* movesLayout = new QVBoxLayout(movesBox);
    movesLayout->addWidget(new QLabel("<b style='color:#3498db;'>Known Moves:</b>"));

    for (const auto& moveId : p.getMoves()) {
        QString moveName = QString::fromStdString(controller.getMoveName(moveId));
        QString moveType = QString::fromStdString(controller.getMoveType(moveId)).toLower();
        QString moveCat = QString::fromStdString(controller.getMoveCategory(moveId));

        QHBoxLayout* rowLayout = new QHBoxLayout();
        rowLayout->setContentsMargins(0, 0, 0, 0);

        QLabel* mTypeIcon = new QLabel();
        mTypeIcon->setPixmap(QPixmap("assets/Others/type-icons/png/" + moveType + ".png").scaled(40, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QLabel* mCatIcon = new QLabel();
        mCatIcon->setPixmap(QPixmap("assets/Others/damage-category-icons/1x/" + moveCat + "-white.png").scaled(30, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QLabel* mNameLbl = new QLabel(" " + moveName);
        mNameLbl->setStyleSheet("font-size: 13px; font-weight: bold;");

        rowLayout->addWidget(mTypeIcon);
        rowLayout->addWidget(mCatIcon);
        rowLayout->addWidget(mNameLbl, 1);
        movesLayout->addLayout(rowLayout);
    }
    rightLayout->addWidget(movesBox);
    rightLayout->addStretch();

    // --- Premium UI: Drop Shadows ---
    // A quick lambda to add shadows to our panels
    auto applyShadow = [](QWidget* w) {
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(15);
        shadow->setOffset(0, 5);
        shadow->setColor(QColor(0, 0, 0, 60)); // Soft, semi-transparent black
        w->setGraphicsEffect(shadow);
        };
    applyShadow(itemBox);
    applyShadow(statBox);
    applyShadow(movesBox);

    // --- Theme Selector UI ---
    QHBoxLayout* themeLayout = new QHBoxLayout();
    QLabel* themeLabel = new QLabel("<b>Theme:</b>");
    QComboBox* themeCombo = new QComboBox();
    themeCombo->addItems({ "Light", "Dark", "Dracula", "Late At Night", "Custom" });
    themeCombo->setCursor(Qt::PointingHandCursor);

    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(themeCombo, 1);
    rightLayout->addLayout(themeLayout);

    // --- The Theme Engine (Lambda) ---
    auto applyTheme = [this](const QString& theme, const QColor& customBg) {
        QString qss;
        // Base structure: QDialog is the background, #dataPanel handles the boxes, #nameLabel handles the title
        if (theme == "Light") {
            qss = "QDialog { background-color: #ecf0f1; } "
                "QFrame#dataPanel { background-color: #ffffff; border-radius: 8px; padding: 6px; } "
                "QLabel { color: #2c3e50; } "
                "QLabel#nameLabel { font-size: 18px; color: #2c3e50; }";
        }
        else if (theme == "Dark") {
            qss = "QDialog { background-color: #121212; } "
                "QFrame#dataPanel { background-color: #1e1e1e; border-radius: 8px; padding: 6px; } "
                "QLabel { color: #e0e0e0; } "
                "QLabel#nameLabel { font-size: 18px; color: #ffffff; }";
        }
        else if (theme == "Dracula") {
            qss = "QDialog { background-color: #282a36; } "
                "QFrame#dataPanel { background-color: #44475a; border-radius: 8px; padding: 6px; } "
                "QLabel { color: #f8f8f2; } "
                "QLabel#nameLabel { font-size: 18px; color: #50fa7b; }"; // Dracula Green for the name
        }
        else if (theme == "Late At Night") {
            qss = "QDialog { background-color: #1a1423; } " // Deep warm purple/black
                "QFrame#dataPanel { background-color: #2d223c; border-radius: 8px; padding: 6px; } "
                "QLabel { color: #f4ece6; } "
                "QLabel#nameLabel { font-size: 18px; color: #f1c40f; }"; // Warm yellow for the name
        }
        else if (theme == "Custom") {
            // Dark semi-transparent panels ensure text is readable regardless of the chosen custom background color
            qss = QString("QDialog { background-color: %1; } "
                "QFrame#dataPanel { background-color: rgba(0, 0, 0, 160); border-radius: 8px; padding: 6px; } "
                "QLabel { color: #ffffff; } "
                "QLabel#nameLabel { font-size: 18px; color: #ffffff; }").arg(customBg.name());
        }
        this->setStyleSheet(qss);
        };

    // --- Load Saved Theme ---
    QSettings settings("MyApp", "Settings");
    QString savedTheme = settings.value("Summary_Theme", "Light").toString();
    QColor savedCustomBg = QColor(settings.value("Summary_CustomColor", "#3498db").toString());

    themeCombo->setCurrentText(savedTheme);
    applyTheme(savedTheme, savedCustomBg);

    // --- Handle Theme Changes ---
    connect(themeCombo, &QComboBox::currentTextChanged, this, [=](const QString& text) {
        QSettings s("MyApp", "Settings");
        QColor activeCustomBg = QColor(s.value("Summary_CustomColor", "#3498db").toString());

        if (text == "Custom") {
            QColor color = QColorDialog::getColor(activeCustomBg, this, "Choose Custom Background");
            if (color.isValid()) {
                activeCustomBg = color;
                s.setValue("Summary_CustomColor", color.name());
            }
            else {
                // User cancelled the color picker, revert the combobox visually to previous theme without triggering loops
                themeCombo->blockSignals(true);
                themeCombo->setCurrentText(s.value("Summary_Theme", "Light").toString());
                themeCombo->blockSignals(false);
                return;
            }
        }

        s.setValue("Summary_Theme", text);
        applyTheme(text, activeCustomBg);
        });

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 1);

    setWindowTitle(QString("%1's Summary").arg(QString::fromStdString(p.getNickname())));
    setFixedSize(550, 360);
}