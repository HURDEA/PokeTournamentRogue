cmake_minimum_required(VERSION 3.16)
project(PokeTournamentRogue VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path that contains runtime assets (images, Tilesets, Items, etc).
# By default this is the project source dir but users can override it at
# configure time: -DASSETS_DIR="C:/path/to/assets_root"
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Runtime asset root directory")

# Enable Qt automatic tools at target level below. Keep the globals off to avoid
# interfering with other projects or system-wide settings. Users should set
# Qt6_DIR or CMAKE_PREFIX_PATH externally to point to their Qt installation.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

if(NOT DEFINED Qt6_DIR AND NOT DEFINED CMAKE_PREFIX_PATH)
    if(DEFINED ENV{Qt6_DIR})
        set(Qt6_DIR $ENV{Qt6_DIR})
    elseif(DEFINED ENV{QTDIR})
        # QTDIR is sometimes set by Qt installers
        set(CMAKE_PREFIX_PATH $ENV{QTDIR})
    endif()
endif()

# If user provided a common local Qt installation path, prefer it when nothing else is set
if(NOT DEFINED Qt6_DIR AND NOT DEFINED CMAKE_PREFIX_PATH)
    if(EXISTS "C:/Qt/6.11.1/msvc2022_64")
        set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64" CACHE PATH "Default Qt location")
    endif()
endif()

find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)

# Option to enable/disable building tests. Default OFF so tests don't interfere.
option(BUILD_TESTS "Build test sources" OFF)

# Gather sources and conditionally include tests
set(APP_SOURCES
    main.cpp
    Domain/Pokemon.h
    Repository/IRepository.h
    Repository/CSVRepository.h
    Repository/CSVRepository.cpp
    Command/Command.h
    Command/CatchCommand.h
    Command/ReleaseCommand.h
    Command/ItemCommand.h
    Command/MoveCommand.h
    Command/ChangeMoveCommand.h
    Command/EvolveCommand.h
    Command/UpdateCommand.h
    Filter/IPokemonFilter.h
    Controller/Controller.h
    Controller/Controller.cpp
    UI/MainWindow.h
    UI/MainWindow.cpp
    UI/MainHall.h
    UI/MainHall.cpp
    Dialog/SummaryDialog.h
    Dialog/SummaryDialog.cpp
    Dialog/SafariOverworldDialog.h
    Dialog/SafariOverworldDialog.cpp
    Dialog/SafariZoneDialog.h
    Dialog/SafariZoneDialog.cpp
    Dialog/ShopDialog.h
    Dialog/ShopDialog.cpp
    Dialog/TrainerDialog.h
    Dialog/TrainerDialog.cpp
    Dialog/BattleDialog.h
    Dialog/BattleDialog.cpp
    Engine/BattleEngine.h
    Engine/BattleEngine.cpp
    Engine/AI/AiDetector.h 
   Engine/AI/AiDetector.cpp
   Engine/AI/Simulator.h 
    Engine/AI/Simulator.cpp
)

if(BUILD_TESTS)
    list(APPEND APP_SOURCES
        Tests/Tests.h
        Tests/Tests.cpp
    )
else()
    message(STATUS "BUILD_TESTS is OFF - Tests/Tests.cpp will not be compiled into the executable.")
endif()

add_executable(PokeTournamentRogue ${APP_SOURCES} "Progression/Trainer.h" "Progression/ProgressionManager.cpp" "Dialog/GauntletDialog.h" "Dialog/GauntletDialog.cpp" "Dialog/TournamentDialog.h" "Dialog/TournamentDialog.cpp" "Progression/TrainerRoster.h" "Progression/TrainerRoster.cpp")

# Expose BUILD_TESTS value to the compiler as a macro (0 or 1)
target_compile_definitions(PokeTournamentRogue PRIVATE BUILD_TESTS=$<BOOL:${BUILD_TESTS}>)
# Expose the configured ASSETS_DIR to the application so it can resolve relative
# asset paths at runtime. This embeds the path into a C++ macro ASSETS_DIR_PATH.
if(DEFINED ASSETS_DIR)
    # Escape backslashes for MSVC command-line macros
    string(REPLACE "\\" "\\\\" ASSETS_DIR_ESCAPED "${ASSETS_DIR}")
    target_compile_definitions(PokeTournamentRogue PRIVATE ASSETS_DIR_PATH="${ASSETS_DIR_ESCAPED}")
endif()

target_link_libraries(PokeTournamentRogue PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)

# Ensure runtime Qt DLLs are available next to the executable when running from the IDE.
# This copies the proper Debug/Release DLLs from the Qt bin folder into the target dir.
set(QT_BIN_DIR "")
if(DEFINED CMAKE_PREFIX_PATH AND EXISTS "${CMAKE_PREFIX_PATH}/bin")
    set(QT_BIN_DIR "${CMAKE_PREFIX_PATH}/bin")
elseif(DEFINED Qt6_DIR AND EXISTS "${Qt6_DIR}/bin")
    set(QT_BIN_DIR "${Qt6_DIR}/bin")
elseif(EXISTS "C:/Qt/6.11.1/msvc2022_64/bin")
    set(QT_BIN_DIR "C:/Qt/6.11.1/msvc2022_64/bin")
endif()

if(QT_BIN_DIR)
    message(STATUS "Qt bin dir: ${QT_BIN_DIR}")
    # Core, Gui, Widgets (use debug suffix for Debug config)
    add_custom_command(TARGET PokeTournamentRogue POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN_DIR}/$<IF:$<CONFIG:Debug>,Qt6Cored.dll,Qt6Core.dll>" $<TARGET_FILE_DIR:PokeTournamentRogue>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN_DIR}/$<IF:$<CONFIG:Debug>,Qt6Guid.dll,Qt6Gui.dll>" $<TARGET_FILE_DIR:PokeTournamentRogue>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_BIN_DIR}/$<IF:$<CONFIG:Debug>,Qt6Widgetsd.dll,Qt6Widgets.dll>" $<TARGET_FILE_DIR:PokeTournamentRogue>
        # Ensure platforms/ subfolder exists next to exe so plugins can be copied there
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:PokeTournamentRogue>/platforms
    )

    # Locate qwindows.dll in common Qt plugin locations first (preferred), then fallback to bin
    set(QWINDOWS_SRC "")
    if(DEFINED CMAKE_PREFIX_PATH AND EXISTS "${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll")
        set(QWINDOWS_SRC "${CMAKE_PREFIX_PATH}/plugins/platforms/qwindows.dll")
    elseif(DEFINED Qt6_DIR AND EXISTS "${Qt6_DIR}/plugins/platforms/qwindows.dll")
        set(QWINDOWS_SRC "${Qt6_DIR}/plugins/platforms/qwindows.dll")
    elseif(EXISTS "C:/Qt/6.11.1/msvc2022_64/plugins/platforms/qwindows.dll")
        set(QWINDOWS_SRC "C:/Qt/6.11.1/msvc2022_64/plugins/platforms/qwindows.dll")
    elseif(EXISTS "${QT_BIN_DIR}/qwindows.dll")
        set(QWINDOWS_SRC "${QT_BIN_DIR}/qwindows.dll")
    endif()

    if(QWINDOWS_SRC)
        add_custom_command(TARGET PokeTournamentRogue POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QWINDOWS_SRC}" $<TARGET_FILE_DIR:PokeTournamentRogue>/platforms/qwindows.dll
        )
    else()
        message(WARNING "qwindows.dll not found in expected Qt locations; runtime may fail. Consider running windeployqt or setting Qt6_DIR / CMAKE_PREFIX_PATH correctly.")
    endif()

    # If a plugins directory exists in the Qt installation, copy common plugin subfolders
    set(QT_PLUGINS_DIR "")
    if(DEFINED CMAKE_PREFIX_PATH AND EXISTS "${CMAKE_PREFIX_PATH}/plugins")
        set(QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/plugins")
    elseif(DEFINED Qt6_DIR AND EXISTS "${Qt6_DIR}/plugins")
        set(QT_PLUGINS_DIR "${Qt6_DIR}/plugins")
    elseif(EXISTS "C:/Qt/6.11.1/msvc2022_64/plugins")
        set(QT_PLUGINS_DIR "C:/Qt/6.11.1/msvc2022_64/plugins")
    endif()

    if(QT_PLUGINS_DIR)
        add_custom_command(TARGET PokeTournamentRogue POST_BUILD
            # Copy platforms (required for qwindows and any other platform plugins)
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${QT_PLUGINS_DIR}/platforms" $<TARGET_FILE_DIR:PokeTournamentRogue>/platforms
            # Copy imageformats and styles which are commonly needed
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${QT_PLUGINS_DIR}/imageformats" $<TARGET_FILE_DIR:PokeTournamentRogue>/imageformats
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${QT_PLUGINS_DIR}/styles" $<TARGET_FILE_DIR:PokeTournamentRogue>/styles
        )
    endif()

    # If windeployqt is available, invoke it to ensure all required Qt DLLs and plugins are deployed
    if(EXISTS "${QT_BIN_DIR}/windeployqt.exe")
        add_custom_command(TARGET PokeTournamentRogue POST_BUILD
            COMMAND "${QT_BIN_DIR}/windeployqt.exe" "$<TARGET_FILE:PokeTournamentRogue>"
        )
    endif()

    # Copy runtime data folders and files from the source tree into the target directory
    # so the application can open relative paths like "archive/pokemon_complete.csv" at runtime.
    if(EXISTS "${ASSETS_DIR}/archive")
        add_custom_command(TARGET PokeTournamentRogue POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}/archive" $<TARGET_FILE_DIR:PokeTournamentRogue>/archive
        )
    endif()

    if(EXISTS "${ASSETS_DIR}/ShowdownData")
        add_custom_command(TARGET PokeTournamentRogue POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}/ShowdownData" $<TARGET_FILE_DIR:PokeTournamentRogue>/ShowdownData
        )
    endif()

    if(EXISTS "${ASSETS_DIR}/trainer_save.csv")
        add_custom_command(TARGET PokeTournamentRogue POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSETS_DIR}/trainer_save.csv" $<TARGET_FILE_DIR:PokeTournamentRogue>/trainer_save.csv
        )
    endif()

    # Copy other asset folders commonly referenced by the code so relative paths resolve at runtime
    foreach(dir IN LISTS "Items" "assets" "CatchPoke" "Tilesets" "Trainers" "Audio")
        if(EXISTS "${ASSETS_DIR}/${dir}")
            add_custom_command(TARGET PokeTournamentRogue POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}/${dir}" $<TARGET_FILE_DIR:PokeTournamentRogue>/${dir}
            )
        endif()
    endforeach()

    # Copy party save if present
    if(EXISTS "${ASSETS_DIR}/party_save.csv")
        add_custom_command(TARGET PokeTournamentRogue POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSETS_DIR}/party_save.csv" $<TARGET_FILE_DIR:PokeTournamentRogue>/party_save.csv
        )
    endif()
else()
    message(WARNING "Qt bin directory not found automatically. If you get runtime DLL errors, set Qt6_DIR or CMAKE_PREFIX_PATH to your Qt installation root (e.g. C:/Qt/6.11.1/msvc2022_64).")
endif()

# Make project root and common subfolders visible to all sources so headers in subfolders can be referenced
target_include_directories(PokeTournamentRogue PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Domain
    ${CMAKE_CURRENT_SOURCE_DIR}/Repository
    ${CMAKE_CURRENT_SOURCE_DIR}/Controller
    ${CMAKE_CURRENT_SOURCE_DIR}/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/Dialog
    ${CMAKE_CURRENT_SOURCE_DIR}/Command
    ${CMAKE_CURRENT_SOURCE_DIR}/Filter
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/Tests
)

# Ensure AUTOMOC/AUTOUIC/AUTORCC are enabled on the target (helpful when using Qt headers in headers)
set_target_properties(PokeTournamentRogue PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# When running under the Visual Studio debugger, use the project source directory
# as the working directory so relative paths used by the application (for
# example "Items/..." or "assets/...") resolve against the project tree where
# your image and asset folders live.
set_target_properties(PokeTournamentRogue PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
