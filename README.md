# PokeTournamentRogue

An offline, C++17 competitive Pokemon rogue-lite battle simulator. This project features a custom-built battle engine, dynamic AI decision-making, a functioning competitive item shop, and a full tournament gauntlet. 

---

### How to Play (Windows)
**No IDE, installation, or compiling required.**
1. **[Download PokeTournamentRogue v1.0.0 (ZIP)](https://github.com/HURDEA/PokeTournamentRogue/releases/download/v1.0.0/PokeTournamentRogue_Playable.zip)**
2. Extract the `.zip` archive anywhere on your PC.
3. Double-click `PokeTournamentRogue.exe` to enter the arena.
4. **[Linux Version](https://mega.nz/folder/5TFgBChY#HI3dSddf_Xg8ZYG1EgolMg)**
5.  *(Note: Ensure all folders like `assets/` and `CatchPoke/` remain in the same directory as the executable).*

---

### Gameplay Showcase
<img width="700" height="1042" alt="SCRN1" src="https://github.com/user-attachments/assets/829d9799-2b11-43f0-adb4-57bf25873bc5" />
<img width="610" height="860" alt="SCRN2" src="https://github.com/user-attachments/assets/cf66784d-d633-460c-b880-12dbf52d3290" />
<img width="677" height="851" alt="SCRN3" src="https://github.com/user-attachments/assets/5b9beed2-07cc-419a-9a0a-23561dfb18a1" />
<img width="434" height="408" alt="SCRN4" src="https://github.com/user-attachments/assets/36c81cb0-ac0d-48d6-ac0f-473cece635b4" />
<img width="676" height="936" alt="SCRN5" src="https://github.com/user-attachments/assets/8e2513fb-5660-4ebf-a07b-cd40c4172ad6" />
<img width="819" height="918" alt="SCRN6" src="https://github.com/user-attachments/assets/6ae507f1-5a4a-4691-b375-3cb77b84c890" />
<img width="1191" height="791" alt="SCRN7" src="https://github.com/user-attachments/assets/d7145e86-ecff-43d3-9f90-8c2252a10392" />
<img width="537" height="182" alt="SCRN8" src="https://github.com/user-attachments/assets/660eb3e9-d050-4ddb-be71-2c6202d3d507" />
<img width="574" height="800" alt="SCRN9-ezgif com-video-to-gif-converter" src="https://github.com/user-attachments/assets/5d973939-1b67-4479-b8cc-f645a472f66d" />


---

## About The Project

This project was built as a personal challenge to scale up from foundational first-year assignments into a complex, multi-system software architecture. It serves as a practical, hands-on exploration of **C++17**, **Qt 6**, and advanced Object-Oriented Programming principles.

Building a Pokemon battle engine from scratch is notoriously difficult due to the sheer volume of edge cases, stat modifiers, and interlocking mechanics. This project tackles that complexity head-on, featuring a fully functional rogue-lite progression system, a custom AI behavior tree, and a robust PC Box management system without relying on external battle APIs.

## Key Technical Achievements

This isn't just a game; it's a demonstration of software engineering patterns:

* **MVC Architecture:** Strict separation of the underlying game logic (Domain/Repository), the game loop (Controller), and the user interface (Qt Dialogs).
* **Command Pattern Implementation:** The PC Box management system utilizes the Command pattern to support full Undo/Redo functionality across box movements, item management, and releases.
* **Custom AI Simulator (Minimax):** The enemy AI doesn't just pick random attacks. It utilizes a multi-depth simulation engine to project exact damage calculations, predict turn economy, and evaluate the safest switches or Mega Evolutions.
* **Event-Driven UI:** Extensive use of Qt's Signals and Slots mechanism to handle complex UI state changes, dynamic inventory filtering, and custom theme rendering without locking the main thread.

## Gameplay Features

* **Advanced Battle Engine:** Supports modern competitive mechanics, including Mega Evolution, Primal Reversion, Terrain, Weather, Entry Hazards, Choice item locks, and complex ability interactions (e.g., *Neutralizing Gas*, *Protosynthesis*).
* **The Practice Gauntlet:** A rogue-lite survival mode where players draft a team and fight through progressively harder stages of randomized enemy pools to earn currency.
* **The Grand Tournament:** An 8-round bracket against specialized Gym Leaders, Elite Four members, and a final Champion, featuring advanced AI utilizing competitive items and held-item strategies.
* **Dynamic Poke Mart:** An economy system where players can purchase competitively viable held items, evolution stones, and Poke Balls tailored to their current progression stage.
* **PC Box Manager:** A fully-featured storage system with advanced filtering (by type, logical AND/OR operations), live stat tracking, and party reordering.
* **The Final Exam (Secret Boss):** Conquer the tournament to unlock a brutally difficult Level 100 hidden boss encounter against "Immortal Gabi Mircea." Survive mid-battle Object-Oriented Programming pop-quizzes to buff your stats, or suffer a segmentation fault!

## Tech Stack

* **Language:** C++17
* **Framework:** Qt 6 (Core, Widgets, Gui)
* **Build System:** CMake / Ninja
* **IDE:** Visual Studio Community

## Build Instructions (For Developers)

### Prerequisites
* A C++17 compatible compiler (MSVC, GCC, or Clang).
* **CMake** (3.16 or higher).
* **Qt 6** installed and configured on your system.

### Building from Source
1. **Clone the repository:**
   ```bash
   git clone [https://github.com/HURDEA/PokeTournamentRogue.git](https://github.com/HURDEA/PokeTournamentRogue.git)
   cd PokeTournamentRogue


Disclaimer: This is a non-commercial, educational fan project. It is not affiliated with, endorsed, or sponsored by Nintendo, Game Freak, or The Pokemon Company. All Pokemon images, names, and related media are intellectual property of their respective owners.
