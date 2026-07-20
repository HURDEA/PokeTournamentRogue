#include <QApplication>
#include "UI/MainHall.h"
#include "Controller/Controller.h"
#include "Repository/CSVRepository.h"
#if defined(BUILD_TESTS) && BUILD_TESTS
#include "Tests/Tests.h"
#endif
#include <memory>
#include <iostream>
#include <QDir>

int main(int argc, char* argv[]) {
    // 1. Run the automated test suite (only when enabled)
#if defined(BUILD_TESTS) && BUILD_TESTS
    Tests::runAllTests();
#endif

    // 2. Initialize the Qt Application
    QApplication a(argc, argv);

    // If ASSETS_DIR_PATH was configured at compile time, prefer it as the
    // application's working directory so all relative asset paths resolve
    // correctly (images, tilesets, Items/ etc.). This fixes cases where the
    // IDE runs the binary from the build folder but assets live in the
    // project source tree.
#ifdef ASSETS_DIR_PATH
    {
        const QString assetsRoot = QStringLiteral(ASSETS_DIR_PATH);
        if (QDir(assetsRoot).exists()) {
            QDir::setCurrent(assetsRoot);
        }
    }
#endif

    // 3. Inject dependencies
    auto repo = std::make_unique<CSVRepository>("archive/pokemon_complete.csv");
    Controller ctrl(std::move(repo));

    // 4. Launch the Central Hub
    MainHall hub(ctrl);
    hub.show();

    return a.exec();
}