# Jogo 007 - Duelo de Reflexo e Estratégia

## Overview
Jogo 007 is an exciting quick-draw duel game implemented in C using the raylib graphics library. Players face off against the CPU in a turn-based combat where resource management (ammunition) and quick decision-making are crucial. This is a Portuguese-language game featuring a 007-themed dueling mechanic.

## Project Information
- **Language**: C (C99 standard)
- **Graphics Library**: raylib 5.5
- **Build System**: Make
- **Platform**: Linux (Replit/NixOS)

## Recent Changes
- **2024-11-16**: Initial import to Replit environment
  - Installed C toolchain (c-clang14) and raylib dependencies
  - Fixed compilation issues (removed typo "tecla" on line 943)
  - Updated makefile to use pkg-config for proper library paths
  - Configured VNC workflow for GUI display
  - Game now runs successfully in Replit environment

## Project Architecture

### File Structure
```
.
├── jogo_007.c          # Main game source code (1049 lines)
├── makefile            # Build configuration
├── ranking_007.dat     # Player rankings data file
└── README.md           # Portuguese game documentation
```

### Key Components

#### Data Structures
- **Pilha (Stack)**: Manages ammunition using a stack structure (max 6 bullets)
- **NodeRanking (Doubly-Linked List)**: Stores player rankings with scores, victories, defeats, and rounds
- **Algoritmo (Insertion Sort)**: Sorts player statistics by points and victories

#### Game States
1. `MENU_PRINCIPAL` - Main menu
2. `ENTRADA_NOME` - Name input screen
3. `REGRAS` - Rules display
4. `ESPERANDO_007` - "007" countdown animation
5. `ESCOLHENDO_ACAO` - Action selection (2-second timer)
6. `MOSTRANDO_ACOES` - Display chosen actions
7. `PROCESSANDO_TURNO` - Process turn results
8. `PAUSADO` - Paused state
9. `SCORE` - Final score screen

#### Game Mechanics
- **Actions**:
  - Q: CARREGAR (Load) - Add 1 bullet (max 6)
  - W: ATIRAR (Shoot) - Use 1 bullet to attack
  - E: DEFENDER (Defend) - Block normal shots with blue shield
  - R: SUPER TIRO (Super Shot) - Use 3 bullets for unblockable attack
  - P: Pause game
  - O: Return to main menu
  
- **Combat System**: Each player has 3 lives, 2-second action timer per round
- **Scoring**: Based on remaining lives, ammunition, and rounds survived

### Build System
The makefile uses pkg-config to automatically find raylib headers and libraries in the Nix environment:
- Compiler: gcc with C99 standard
- Flags: -Wall -Wextra -O2 -g
- Libraries: raylib, math, pthread, dl, rt
- Display: Uses GALLIUM_DRIVER=llvmpipe for software rendering

## Running the Game
The game automatically starts via the "Jogo 007" workflow, which runs `make run`. The game displays in a VNC window where you can:
1. Press 1 to start playing
2. Press 2 to view rules
3. Press ESC to exit
4. Enter your name and engage in duels against the CPU

## Development Notes
- The game uses software OpenGL rendering (llvmpipe) for compatibility in the Replit environment
- Rankings are persisted to `ranking_007.dat` binary file
- The game features animated characters, bullets, and visual effects
- All game text is in Portuguese

## Team
- Rodrigo Tenório
- Gabriel Ferraz
- Paulo Portella
- Ícaro Barros
