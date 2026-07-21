#include "ShopDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFile>
#include <QSizePolicy>
#include <QSettings>

ShopDialog::ShopDialog(Controller& ctrl, ProgressionManager& pm, QWidget* parent)
    : QDialog(parent), controller(ctrl), progManager(pm) {
    setupUI();

    QSettings settings("MyCompany", "PokemonBoxManager");
    QString savedTheme = settings.value("mainHallTheme", "Pro Dark").toString();
    applyTheme(savedTheme);

    populateTabs();
    updateMoneyDisplay();
}

void ShopDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QLabel* title = new QLabel("THE POKÉ MART", this);
    title->setObjectName("title");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    moneyLabel = new QLabel(this);
    moneyLabel->setObjectName("money");
    moneyLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(moneyLabel);

    tabs = new QTabWidget(this);
    tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto createList = [](QListWidget*& list) {
        list = new QListWidget();
        list->setIconSize(QSize(32, 32));
        list->setStyleSheet("border: none; background-color: transparent; outline: none;");
        };

    createList(ballsList);
    createList(berriesList);
    createList(stonesList);
    createList(boostsList);
    createList(megasList);
    createList(battleList);

    tabs->addTab(ballsList, "Poké Balls");
    tabs->addTab(berriesList, "Berries");
    tabs->addTab(stonesList, "Evo & Fossils");
    tabs->addTab(boostsList, "Boosts & Plates");
    tabs->addTab(megasList, "Mega Stones");
    tabs->addTab(battleList, "Battle Items");

    mainLayout->addWidget(tabs, 1);

    itemDescLabel = new QLabel("Select an item to view its properties and effects.", this);
    itemDescLabel->setObjectName("desc");
    itemDescLabel->setWordWrap(true);
    itemDescLabel->setMinimumHeight(60);
    itemDescLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(itemDescLabel);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    closeBtn = new QPushButton("LEAVE SHOP");
    closeBtn->setObjectName("dangerBtn");
    closeBtn->setMinimumHeight(45);
    closeBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    buyBtn = new QPushButton("PURCHASE SELECTED ITEM");
    buyBtn->setObjectName("successBtn");
    buyBtn->setMinimumHeight(45);
    buyBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    btnLayout->addWidget(closeBtn, 1);
    btnLayout->addWidget(buyBtn, 2);
    mainLayout->addLayout(btnLayout);

    connect(buyBtn, &QPushButton::clicked, this, &ShopDialog::onBuyClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto connectList = [&](QListWidget* list) {
        connect(list, &QListWidget::itemSelectionChanged, this, [this, list]() {
            if (list->currentItem()) {
                QString itemName = list->currentItem()->data(Qt::UserRole).toString();
                QString desc;

                // Custom descriptions for Simulator-Exclusive Items
                if (itemName == "LINKCABLE") {
                    desc = "A high-tech cable that induces evolution in Pokémon that normally require trading.";
                }
                else if (itemName == "UNIVERSALSTONE") {
                    desc = "A bizarre, omni-elemental stone that forces evolution in Pokémon with highly complex or specific growth requirements (e.g., high Friendship, knowing specific moves, specific locations).";
                }
                else {
                    desc = QString::fromStdString(controller.getItemDescription(itemName.toLower().toStdString()));
                }

                if (desc.isEmpty()) desc = "No description available in Showdown data.";
                itemDescLabel->setText(desc);
            }
            });
        };

    connectList(ballsList); connectList(berriesList); connectList(stonesList);
    connectList(boostsList); connectList(megasList); connectList(battleList);

    setWindowTitle("The Poké Mart - Pokémon Champions");

    setMinimumSize(600, 650);
    resize(650, 700);
}

void ShopDialog::applyTheme(const QString& themeName) {
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
    else {
        bg = "#f4f6f8"; ctrlBg = "#ffffff"; textBase = "#2c3e50"; textMuted = "#7f8c8d"; accentColor = "#3498db";
        btnBg = "#ffffff"; btnText = "#2c3e50"; btnBorder = "#e0e0e0";
        btnHoverBg = "#ecf0f1"; btnHoverBorder = "#3498db"; btnHoverText = "#2c3e50";
        dangerBorder = "#c0392b"; dangerHoverBg = "#f2d7d5"; dangerHoverText = "#922b21";
        successBorder = "#27ae60"; successHoverBg = "#d5f5e3"; successHoverText = "#1d8348";
    }

    moneyLabel->setProperty("themeSuccess", successBorder);

    QString qss = QString(R"(
        QDialog { background-color: %1; font-family: 'Segoe UI', Helvetica, sans-serif; }
        
        QLabel#title { font-size: 24px; font-weight: 900; color: %3; letter-spacing: 2px; margin-bottom: 2px; }
        QLabel#money { font-size: 15px; font-weight: 800; color: %18; letter-spacing: 1px; margin-bottom: 10px; }
        
        QLabel#desc { 
            font-style: italic; color: %4; background-color: %2; 
            padding: 15px; border: 1px solid %7; border-radius: 6px; 
            font-size: 13px; 
        }
        
        QTabWidget::pane { 
            border: 1px solid %7; background-color: %2; 
            border-radius: 6px; border-top-left-radius: 0px; top: -1px; 
        }
        QTabBar::tab { 
            background: %1; color: %4; font-weight: bold; font-size: 13px;
            padding: 10px 16px; border: 1px solid %7; border-bottom: none; 
            border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 2px; 
        }
        QTabBar::tab:selected { background: %2; color: %3; border-top: 3px solid %5; }
        QTabBar::tab:hover:!selected { background: %2; color: %8; }
        
        QListWidget { 
            background-color: transparent; color: %8; 
            font-weight: 600; font-size: 14px; border: none; 
            outline: none; padding: 5px; 
        }
        QListWidget::item { padding: 8px; border-radius: 4px; margin-bottom: 2px; }
        QListWidget::item:selected { background-color: %5; color: #ffffff; }
        
        QPushButton { 
            background-color: %6; color: %8; font-size: 13px; font-weight: 800; 
            padding: 12px 20px; border: 1px solid %7; 
            border-radius: 6px; letter-spacing: 1px; 
        }
        QPushButton:hover { background-color: %9; border: 1px solid %10; color: %11; }
        QPushButton:pressed { background-color: %10; color: #ffffff; }
        
        QPushButton#successBtn { color: %18; border: 1px solid %7; }
        QPushButton#successBtn:hover { border: 1px solid %18; background-color: %19; color: %20; }
        
        QPushButton#dangerBtn:hover { border: 1px solid %12; color: %14; background-color: %13; }
    )").arg(bg, ctrlBg, textBase, textMuted, accentColor, btnBg, btnBorder, btnText, btnHoverBg, btnHoverBorder, btnHoverText, dangerBorder, dangerHoverBg, dangerHoverText, successBorder, successHoverBg, successHoverText, successBorder, successHoverBg, successHoverText);

    this->setStyleSheet(qss);
    updateMoneyDisplay();
}

void ShopDialog::updateMoneyDisplay() {
    QString color = moneyLabel->property("themeSuccess").toString();
    moneyLabel->setStyleSheet(QString("color: %1;").arg(color));
    moneyLabel->setText(QString("WALLET: $%1").arg(controller.getMoney()));
}

void ShopDialog::populateTabs() {
    auto addItems = [&](QListWidget* list, const std::vector<std::pair<QString, int>>& items) {
        for (const auto& item : items) {
            QString itemPath = "Items/" + item.first + ".png";
            if (!QFile::exists(itemPath)) itemPath = "Items/000.png";

            QListWidgetItem* listItem = new QListWidgetItem(QIcon(itemPath), QString("%1  -  $%2").arg(item.first).arg(item.second));
            listItem->setData(Qt::UserRole, item.first);
            listItem->setData(Qt::UserRole + 1, item.second);
            list->addItem(listItem);
        }
        };

    int stage = progManager.getGauntletStage();
    bool tournament = progManager.getTournamentUnlocked();

    // Poké Balls Progression
    std::vector<std::pair<QString, int>> balls = { {"POKEBALL", 200}, {"HEALBALL", 300}, {"PREMIERBALL", 200} };
    if (stage >= 2) {
        balls.insert(balls.end(), { {"GREATBALL", 600}, {"SAFARIBALL", 500}, {"FASTBALL", 500}, {"FRIENDBALL", 500}, {"HEAVYBALL", 500}, {"LEVELBALL", 500}, {"LOVEBALL", 500}, {"LUREBALL", 500}, {"MOONBALL", 500} });
    }
    if (stage >= 3) {
        balls.insert(balls.end(), { {"ULTRABALL", 1200}, {"NETBALL", 1000}, {"DIVEBALL", 1000}, {"DUSKBALL", 1000}, {"TIMERBALL", 1000}, {"QUICKBALL", 1000}, {"REPEATBALL", 1000}, {"NESTBALL", 1000}, {"LUXURYBALL", 1000} });
    }
    if (tournament) {
        balls.insert(balls.end(), { {"MASTERBALL", 100000}, {"BEASTBALL", 1000}, {"CHERISHBALL", 2000}, {"PARKBALL", 2000}, {"SPORTBALL", 2000} });
    }
    addItems(ballsList, balls);

    // Berries Progression
    std::vector<std::pair<QString, int>> berries = { {"ORANBERRY", 100}, {"PECHABERRY", 200}, {"RAWSTBERRY", 200}, {"CHERIBERRY", 200}, {"CHESTOBERRY", 200}, {"PERSIMBERRY", 200}, {"ASPEARBERRY", 200} };
    if (stage >= 2) {
        berries.insert(berries.end(), { {"SITRUSBERRY", 300}, {"LUMBERRY", 400}, {"LEPPABERRY", 500} });
    }
    if (stage >= 3) {
        berries.insert(berries.end(), { {"FIGYBERRY", 400}, {"WIKIBERRY", 400}, {"MAGOBERRY", 400}, {"AGUAVBERRY", 400}, {"IAPAPABERRY", 400}, {"LIECHIBERRY", 800}, {"SALACBERRY", 800}, {"PETAYABERRY", 800}, {"APICOTBERRY", 800}, {"LANSATBERRY", 800}, {"STARFBERRY", 800}, {"KEEBERRY", 800}, {"MARANGABERRY", 800} });
    }
    if (tournament) {
        berries.insert(berries.end(), { {"OCCABERRY", 600}, {"PASSHOBERRY", 600}, {"WACANBERRY", 600}, {"YACHEBERRY", 600}, {"CHOPLEBERRY", 600}, {"SHUCABERRY", 600}, {"ROSELIBERRY", 600}, {"ENIGMABERRY", 1000}, {"MICLEBERRY", 1000}, {"CUSTAPBERRY", 1000}, {"JABOCABERRY", 1000}, {"ROWAPBERRY", 1000}, {"RINDOBERRY", 600}, {"KEBIABERRY", 600}, {"COBABERRY", 600}, {"PAYAPABERRY", 600}, {"TANGABERRY", 600}, {"CHARTIBERRY", 600}, {"KASIBBERRY", 600}, {"HABANBERRY", 600}, {"COLBURBERRY", 600}, {"BABIRIBERRY", 600}, {"CHILANBERRY", 600} });
    }
    addItems(berriesList, berries);

    // Evo Stones Progression
    std::vector<std::pair<QString, int>> stones;
    if (stage >= 2) {
        stones.insert(stones.end(), { {"FIRESTONE", 2000}, {"WATERSTONE", 2000}, {"THUNDERSTONE", 2000}, {"LEAFSTONE", 2000}, {"MOONSTONE", 2000}, {"SUNSTONE", 2000}, {"OVALSTONE", 2000}, {"SWEETAPPLE", 2000}, {"TARTAPPLE", 2000}, {"SYRUPYAPPLE", 2000} });
    }
    if (stage >= 3) {
        stones.insert(stones.end(), { {"DAWNSTONE", 2000}, {"DUSKSTONE", 2000}, {"SHINYSTONE", 2000}, {"ICESTONE", 2000}, {"LINKCABLE", 2000}, {"UPGRADE", 2000}, {"METALALLOY", 2000}, {"DRAGONSCALE", 2000}, {"KINGSROCK", 2000}, {"PRISMSCALE", 2000}, {"DEEPSEATOOTH", 2000}, {"DEEPSEASCALE", 2000}, {"SACHET", 2000}, {"WHIPPEDDREAM", 2000}, {"GALARICACUFF", 2000}, {"GALARICAWREATH", 2000}, {"AUSPICIOUSARMOR", 2000}, {"MALICIOUSARMOR", 2000}, {"CRACKEDPOT", 2000}, {"CHIPPEDPOT", 2000}, {"UNREMARKABLETEACUP", 2000}, {"MASTERPIECETEACUP", 2000} });
    }
    if (tournament) {
        stones.insert(stones.end(), { {"UNIVERSALSTONE", 10000}, {"DUBIOUSDISC", 2000}, {"ELECTIRIZER", 2000}, {"MAGMARIZER", 2000}, {"PROTECTOR", 2000}, {"REAPERCLOTH", 2000}, {"ARMORFOSSIL", 1000}, {"CLAWFOSSIL", 1000}, {"COVERFOSSIL", 1000}, {"DOMEFOSSIL", 1000}, {"FOSSILIZEDBIRD", 1000}, {"FOSSILIZEDDINO", 1000}, {"FOSSILIZEDDRAKE", 1000}, {"FOSSILIZEDFISH", 1000}, {"HELIXFOSSIL", 1000}, {"JAWFOSSIL", 1000}, {"PLUMEFOSSIL", 1000}, {"ROOTFOSSIL", 1000}, {"SAILFOSSIL", 1000}, {"SKULLFOSSIL", 1000}, {"OLDAMBER", 1000} });
    }
    addItems(stonesList, stones);

    // Boosts & Plates Progression
    std::vector<std::pair<QString, int>> boosts = { {"SILKSCARF", 1000}, {"MYSTICWATER", 1000}, {"CHARCOAL", 1000}, {"MIRACLESEED", 1000} };
    if (stage >= 2) {
        boosts.insert(boosts.end(), { {"MAGNET", 1000}, {"HARDSTONE", 1000}, {"BLACKBELT", 1000}, {"BLACKGLASSES", 1000}, {"NEVERMELTICE", 1000}, {"POISONBARB", 1000}, {"SHARPBEAK", 1000}, {"SPELLTAG", 1000}, {"TWISTEDSPOON", 1000}, {"METALCOAT", 1000}, {"DRAGONFANG", 1000}, {"SOFTSAND", 1000}, {"FAIRYFEATHER", 1000}, {"MUSCLEBAND", 1000}, {"WISEGLASSES", 1000} });
    }
    if (stage >= 3) {
        boosts.insert(boosts.end(), { {"FLAMEPLATE", 1500}, {"SPLASHPLATE", 1500}, {"MEADOWPLATE", 1500}, {"ZAPPLATE", 1500}, {"PIXIEPLATE", 1500}, {"DREADPLATE", 1500}, {"IRONPLATE", 1500}, {"EARTHPLATE", 1500}, {"SKYPLATE", 1500}, {"MINDPLATE", 1500}, {"INSECTPLATE", 1500}, {"STONEPLATE", 1500}, {"SPOOKYPLATE", 1500}, {"DRACOPLATE", 1500}, {"ICICLEPLATE", 1500}, {"FISTPLATE", 1500}, {"TOXICPLATE", 1500} });
    }
    if (tournament) {
        boosts.insert(boosts.end(), { {"NORMALGEM", 1000}, {"FIREGEM", 1000}, {"WATERGEM", 1000}, {"ELECTRICGEM", 1000}, {"GRASSGEM", 1000}, {"DRAGONGEM", 1000}, {"FAIRYGEM", 1000}, {"ICEGEM", 1000}, {"FIGHTINGGEM", 1000}, {"POISONGEM", 1000}, {"GROUNDGEM", 1000}, {"FLYINGGEM", 1000}, {"PSYCHICGEM", 1000}, {"BUGGEM", 1000}, {"ROCKGEM", 1000}, {"GHOSTGEM", 1000}, {"DARKGEM", 1000}, {"STEELGEM", 1000} });
    }
    addItems(boostsList, boosts);

    // Battle Items Progression
    std::vector<std::pair<QString, int>> battle = { {"ROCKYHELMET", 2000}, {"QUICKCLAW", 1500}, {"LIGHTBALL", 1500}, {"THICKCLUB", 1500}, {"BERRYJUICE", 500} };
    if (stage >= 2) {
        battle.insert(battle.end(), { {"LEFTOVERS", 2000}, {"BLACKSLUDGE", 2000}, {"EVIOLITE", 2000}, {"EXPERTBELT", 2000}, {"LIGHTCLAY", 1500}, {"DAMPROCK", 1500}, {"HEATROCK", 1500}, {"SMOOTHROCK", 1500}, {"ICYROCK", 1500}, {"TERRAINEXTENDER", 1500}, {"BIGROOT", 1500}, {"BINDINGBAND", 1500}, {"GRIPCLAW", 1500}, {"DESTINYKNOT", 1500}, {"METRONOME", 1500}, {"SCOPELENS", 1500}, {"WIDELENS", 1500}, {"ZOOMLENS", 1500} });
    }
    if (stage >= 3) {
        battle.insert(battle.end(), { {"LIFEORB", 2500}, {"CHOICEBAND", 2500}, {"CHOICESPECS", 2500}, {"CHOICESCARF", 2500}, {"HEAVYDUTYBOOTS", 2000}, {"FOCUSSASH", 2000}, {"ASSAULTVEST", 2000}, {"EJECTBUTTON", 2000}, {"EJECTPACK", 2000}, {"REDCARD", 2000}, {"ROOMSERVICE", 2000}, {"BLUNDERPOLICY", 2000}, {"PUNCHINGGLOVE", 2000}, {"PROTECTIVEPADS", 2000}, {"SAFETYGOGGLES", 2000}, {"SHEDSHELL", 2000}, {"UTILITYUMBRELLA", 2000}, {"IRONBALL", 1500}, {"MACHOBRACE", 1500}, {"POWERANKLET", 1500}, {"POWERBAND", 1500}, {"POWERBELT", 1500}, {"POWERBRACER", 1500}, {"POWERLENS", 1500}, {"POWERWEIGHT", 1500} });
    }
    if (tournament) {
        battle.insert(battle.end(), { {"TOXICORB", 1500}, {"FLAMEORB", 1500}, {"WEAKNESSPOLICY", 2000}, {"WHITEHERB", 1500}, {"MENTALHERB", 1500}, {"POWERHERB", 1500}, {"CLEARAMULET", 2000}, {"COVERTCLOAK", 2000}, {"LOADEDDICE", 2000}, {"BOOSTERENERGY", 2500}, {"ABILITYSHIELD", 2000}, {"ABSORBBULB", 1500}, {"CELLBATTERY", 1500}, {"LUMINOUSMOSS", 1500}, {"SNOWBALL", 1500}, {"ADRENALINEORB", 1500}, {"AIRBALLOON", 2000}, {"ELECTRICSEED", 1500}, {"GRASSYSEED", 1500}, {"MISTYSEED", 1500}, {"PSYCHICSEED", 1500}, {"MIRRORHERB", 2000}, {"THROATSPRAY", 1500}, {"ADAMANTCRYSTAL", 2500}, {"ADAMANTORB", 2500}, {"LUSTROUSGLOBE", 2500}, {"LUSTROUSORB", 2500}, {"GRISEOUSCORE", 2500}, {"GRISEOUSORB", 2500}, {"RUSTEDSHIELD", 2500}, {"RUSTEDSWORD", 2500}, {"BURNDRIVE", 2000}, {"CHILLDRIVE", 2000}, {"DOUSEDRIVE", 2000}, {"SHOCKDRIVE", 2000}, {"CORNERSTONEMASK", 2500}, {"HEARTHFLAMEMASK", 2500}, {"WELLSPRINGMASK", 2500}, {"BUGMEMORY", 2000}, {"DARKMEMORY", 2000}, {"DRAGONMEMORY", 2000}, {"ELECTRICMEMORY", 2000}, {"FAIRYMEMORY", 2000}, {"FIGHTINGMEMORY", 2000}, {"FIREMEMORY", 2000}, {"FLYINGMEMORY", 2000}, {"GHOSTMEMORY", 2000}, {"GRASSMEMORY", 2000}, {"GROUNDMEMORY", 2000}, {"ICEMEMORY", 2000}, {"POISONMEMORY", 2000}, {"PSYCHICMEMORY", 2000}, {"ROCKMEMORY", 2000}, {"STEELMEMORY", 2000}, {"WATERMEMORY", 2000}, {"VILEVIAL", 2500} });
    }
    addItems(battleList, battle);

    // Mega Stones Progression (All locked until Tournament)
    std::vector<std::pair<QString, int>> megas;
    if (tournament) {
        megas = {
            {"ABOMASITE", 2500}, {"ABSOLITE", 2500}, {"ABSOLITEZ", 2500}, {"AERODACTYLITE", 2500}, {"AGGRONITE", 2500},
            {"ALAKAZITE", 2500}, {"ALTARIANITE", 2500}, {"AMPHAROSITE", 2500}, {"AUDINITE", 2500}, {"BANETTITE", 2500},
            {"BARBARACITE", 2500}, {"BAXCALIBRITE", 2500}, {"BEEDRILLITE", 2500}, {"BLASTOISINITE", 2500}, {"BLAZIKENITE", 2500},
            {"BLUEORB", 2500}, {"CAMERUPTITE", 2500}, {"CHANDELURITE", 2500}, {"CHARIZARDITEX", 2500}, {"CHARIZARDITEY", 2500},
            {"CHESNAUGHTITE", 2500}, {"CHIMECHITE", 2500}, {"CLEFABLITE", 2500}, {"CRABOMINITE", 2500}, {"CRUCIBELLITE", 2500},
            {"DARKRANITE", 2500}, {"DELPHOXITE", 2500}, {"DIANCITE", 2500}, {"DRAGALGITE", 2500}, {"DRAGONINITE", 2500},
            {"DRAMPANITE", 2500}, {"EELEKTROSSITE", 2500}, {"EMBOARITE", 2500}, {"EXCADRITE", 2500}, {"FALINKSITE", 2500},
            {"FERALIGITE", 2500}, {"FLOETTITE", 2500}, {"FROSLASSITE", 2500}, {"GALLADITE", 2500}, {"GARCHOMPITE", 2500},
            {"GARCHOMPITEZ", 2500}, {"GARDEVOIRITE", 2500}, {"GENGARITE", 2500}, {"GLALITITE", 2500}, {"GLIMMORANITE", 2500},
            {"GOLISOPITE", 2500}, {"GOLURKITE", 2500}, {"GRENINJITE", 2500}, {"GYARADOSITE", 2500}, {"HAWLUCHANITE", 2500},
            {"HEATRANITE", 2500}, {"HERACRONITE", 2500}, {"HOUNDOOMINITE", 2500}, {"KANGASKHANITE", 2500}, {"LATIASITE", 2500},
            {"LATIOSITE", 2500}, {"LOPUNNITE", 2500}, {"LUCARIONITE", 2500}, {"LUCARIONITEZ", 2500}, {"MAGEARNITE", 2500},
            {"MALAMARITE", 2500}, {"MANECTITE", 2500}, {"MAWILITE", 2500}, {"MEDICHAMITE", 2500}, {"MEGANIUMITE", 2500},
            {"MEOWSTICITE", 2500}, {"METAGROSSITE", 2500}, {"MEWTWONITEX", 2500}, {"MEWTWONITEY", 2500}, {"PIDGEOTITE", 2500},
            {"PINSIRITE", 2500}, {"PYROARITE", 2500}, {"RAICHUNITEX", 2500}, {"RAICHUNITEY", 2500}, {"REDORB", 2500},
            {"SABLENITE", 2500}, {"SALAMENCITE", 2500}, {"SCEPTILITE", 2500}, {"SCIZORITE", 2500}, {"SCOLIPITE", 2500},
            {"SCOVILLAINITE", 2500}, {"SCRAFTINITE", 2500}, {"SHARPEDONITE", 2500}, {"SKARMORITE", 2500}, {"SLOWBRONITE", 2500},
            {"STARAPTITE", 2500}, {"STARMINITE", 2500}, {"STEELIXITE", 2500}, {"SWAMPERTITE", 2500}, {"TATSUGIRINITE", 2500},
            {"TYRANITARITE", 2500}, {"VENUSAURITE", 2500}, {"VICTREEBELITE", 2500}, {"ZERAORITE", 2500}, {"ZYGARDITE", 2500}
        };
    }
    addItems(megasList, megas);
}

void ShopDialog::onBuyClicked() {
    QListWidget* activeList = qobject_cast<QListWidget*>(tabs->currentWidget());
    if (!activeList || !activeList->currentItem()) return;

    QString itemName = activeList->currentItem()->data(Qt::UserRole).toString();
    int price = activeList->currentItem()->data(Qt::UserRole + 1).toInt();

    if (controller.spendMoney(price)) {
        controller.addItem(itemName.toStdString(), 1);
        updateMoneyDisplay();
        QMessageBox::information(this, "Transaction Successful", QString("Successfully purchased 1x %1.").arg(itemName));
    }
    else {
        QMessageBox::critical(this, "Insufficient Funds", "You do not have enough Pokédollars to complete this transaction.");
    }
}