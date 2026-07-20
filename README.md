# Pokémon Champions: Rogue-Lite Engine

> **Disclaimer:** This is a non-commercial, educational fan project. It is not affiliated with, endorsed, or sponsored by Nintendo, Game Freak, or The Pokémon Company. All Pokémon images, names, and related media are intellectual property of their respective owners.

##  About The Project

This project was built as a personal challenge to scale up from standard university-level assignments into a complex, multi-system software architecture. It serves as a practical, hands-on exploration of **C++17**, **Qt6**, and advanced Object-Oriented Programming principles.

Building a Pokémon battle engine from scratch is notoriously difficult due to the sheer volume of edge cases, stat modifiers, and interlocking mechanics. This project tackles that complexity head-on, featuring a fully functional rogue-lite progression system, a custom AI behavior tree, and a robust PC Box management system.

##  Key Technical Achievements

This isn't just a game; it's a demonstration of software engineering patterns:

* **MVC Architecture:** Strict separation of the underlying game logic (Domain/Repository), the game loop (Controller), and the user interface (Qt Dialogs).
* **Command Pattern Implementation:** The PC Box management system utilizes the Command pattern to support full Undo/Redo functionality across box movements, item management, and releases.
* **Custom AI Simulator (Minimax):** The enemy AI doesn't just pick random attacks. It utilizes a multi-depth simulation engine to project exact damage calculations, predict turn economy, and evaluate the safest switches or Mega Evolutions.
* **Event-Driven UI:** Extensive use of Qt's Signals and Slots mechanism to handle complex UI state changes, dynamic inventory filtering, and custom theme rendering without locking the main thread.

##  Gameplay Features

* **Advanced Battle Engine:** Supports modern generational mechanics, including Mega Evolution, Terrain, Weather, Entry Hazards, and complex ability interactions (e.g., *Neutralizing Gas*, *Protosynthesis*).
* **The Practice Gauntlet:** A rogue-lite survival mode where players draft a team and fight through progressively harder stages of randomized enemy pools to earn currency.
* **The Grand Tournament:** An 8-round bracket against specialized Gym Leaders, Elite Four members, and a final Champion, featuring advanced AI utilizing competitive items and held-item strategies.
* **Dynamic Poké Mart:** An economy system where players can purchase held items, evolution stones, and Poké Balls tailored to their current progression stage.
* **PC Box Manager:** A fully-featured storage system with advanced filtering (by type, logical AND/OR operations), live stat tracking, and party reordering.

##  Tech Stack

* **Language:** C++17
* **Framework:** Qt 6 (Core, Widgets, Gui)
* **Build System:** CMake
* **IDE:** Visual Studio / VS Code

##  Build Instructions

### Prerequisites

* A C++17 compatible compiler (MSVC, GCC, or Clang).
* **CMake** (3.16 or higher).
* **Qt 6** installed and configured on your system.

### Building from Source

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/HURDEA/PokeTournamentRogue.git](https://github.com/HURDEA/PokeTournamentRogue.git)
   cd PokeTournamentRogue
