# Combat & Enemy AI System — Agent #12 Cycle 010
## Transpersonal Game Studio — Prehistoric Survival Game

---

## Overview

This document describes the Combat AI system implemented in PROD_CYCLE_AUTO_20260617_010.
All combat zones, waypoints, and AI behavior definitions are placed in `/Game/Maps/MinPlayableMap`.

---

## Combat Zones Spawned This Cycle

### T-Rex Combat AI Zone
- **AggroZone**: `CombatAI_TRex_AggroZone_001` at (2000, 1500, 100) — scale 20×20×5
  - Trigger: Player entering this box activates T-Rex combat state
  - Behavior: T-Rex charges player, uses stomp attack if player is within 300u
  - Retreat: Player must exit zone radius (2000u) to disengage

### Raptor Pack Flanking System
- **FlankTrigger**: `CombatAI_Raptor_FlankTrigger_001` at (-1200, 800, 100)
  - Trigger: Player entering activates 3-raptor split attack
  - Alpha charges from front (CombatWP_Raptor_Alpha_Charge)
  - Beta flanks left (CombatWP_Raptor_Beta_FlankL)
  - Gamma flanks right (CombatWP_Raptor_Gamma_FlankR)
  - Delta cuts off retreat (CombatWP_Raptor_Delta_Rear)
- **PincerLeft**: `CombatAI_Raptor_PincerLeft_001` at (-1500, 600, 100)
- **PincerRight**: `CombatAI_Raptor_PincerRight_001` at (-900, 1000, 100)

### Brachiosaur Stampede Corridor
- **StampedeCorridor**: `CombatAI_Brachio_StampedeCorridor_001` at (500, 3000, 100)
  - Passive threat: Brachiosaurs stampede if player fires projectile nearby
  - Corridor is 80u wide × 300u long — player must dodge or be trampled
  - Damage: 80 HP per stomp, knockback 500u

### Safe Retreat Zone
- **SafeRetreat**: `CombatAI_SafeRetreat_001` at (0, 0, 100) — scale 12×12×3
  - Predators will NOT enter this zone (camp fire radius)
  - NPC_Hunter_001 can be called for help from within this zone
  - Morale system: NPC deaths reduce camp morale (Elder reacts)

---

## Combat Waypoints

### T-Rex Patrol Circuit
5 waypoints forming a patrol loop around territory center (2000, 1500):
- `CombatWP_TRex_Patrol_N` → (2000, 2200, 100)
- `CombatWP_TRex_Patrol_E` → (2700, 1500, 100)
- `CombatWP_TRex_Patrol_S` → (2000, 800, 100)
- `CombatWP_TRex_Patrol_W` → (1300, 1500, 100)
- `CombatWP_TRex_Patrol_Center` → (2000, 1500, 100)

**Patrol behavior**: T-Rex moves between waypoints at 400 cm/s (walk speed).
On player detection (sight range 2500u, hearing range 1500u): switches to charge (800 cm/s).

### Raptor Pack Positions
- `CombatWP_Raptor_Alpha_Charge` → (-1200, 800, 100) — frontal charge
- `CombatWP_Raptor_Beta_FlankL` → (-1500, 400, 100) — left flank
- `CombatWP_Raptor_Gamma_FlankR` → (-900, 1200, 100) — right flank
- `CombatWP_Raptor_Delta_Rear` → (-800, 600, 100) — rear cutoff

---

## Combat Visual Indicators (Point Lights)

| Label | Position | Color | Meaning |
|-------|----------|-------|---------|
| `CombatLight_TRex_Danger_001` | (2000, 1500, 300) | Red-orange | T-Rex territory danger |
| `CombatLight_Raptor_Ambush_001` | (-1200, 800, 200) | Orange | Raptor ambush zone |
| `CombatLight_SafeCamp_001` | (0, 0, 250) | Blue-white | Safe camp zone |

---

## Combat AI Behavior Design

### T-Rex — Apex Predator
```
States: PATROL → ALERT → CHARGE → ATTACK → RECOVER
- PATROL: Follows waypoint circuit, ignores player beyond 2500u
- ALERT: Player detected — T-Rex stops, turns, vocalizes (roar SFX)
- CHARGE: Sprints at player (800 cm/s), cannot turn sharply
- ATTACK: Bite (150 dmg) + Stomp (80 dmg AoE 300u radius)
- RECOVER: 3s cooldown after attack, returns to PATROL if player escapes
```

### Raptor Pack — Coordinated Ambush
```
States: STALK → SIGNAL → SPLIT → PINCER → LEAP
- STALK: Pack follows player at distance (800u), staying in cover
- SIGNAL: Alpha vocalizes — pack splits to flanking positions
- SPLIT: Beta/Gamma move to pincer positions (2s travel time)
- PINCER: All raptors charge simultaneously from 3 directions
- LEAP: Raptor leaps at player (50 dmg + knockdown 1.5s)
```

### Brachiosaur — Passive Threat
```
States: GRAZE → STARTLED → STAMPEDE → CALM
- GRAZE: Moves slowly along corridor, ignores player
- STARTLED: Loud noise (projectile, roar) within 500u triggers alert
- STAMPEDE: Runs at 600 cm/s along corridor for 5s
- CALM: Returns to GRAZE after 10s
```

---

## NPC Combat Integration (from Agent #11)

### NPC_Hunter_001 as Combat Ally
- Player can call Hunter from SafeRetreat zone
- Hunter joins combat with bow (30 dmg/arrow, 3 arrows)
- Hunter flees if health < 30%
- Hunter death → Elder reacts, camp morale -20%

### NPC Death → Morale System
- Each NPC death reduces tribe morale by 15-20%
- Elder NPC_Elder_001 delivers voice line on NPC death
- At morale < 30%: NPCs refuse to leave camp
- At morale 0%: Camp abandoned, player must survive alone

---

## Voice Lines Generated This Cycle

| File | Character | Line |
|------|-----------|------|
| `tts/1781717565339_CombatNarrator_TRex.mp3` | Combat Narrator | "It sees you. Do not run. If you run, you are already dead. Back away slowly — keep the fire between you and it." |
| `tts/1781717577879_CombatNarrator_Raptor.mp3` | Combat Narrator | "Raptor on your left! Use the spear — aim for the neck! Do not let it pin you to the ground!" |

---

## Handoff to Agent #13 — Crowd & Traffic Simulation

### What Exists in MinPlayableMap
- **6 Combat Zones**: AggroZone, FlankTrigger, PincerL/R, StampedeCorridor, SafeRetreat
- **9 Combat Waypoints**: 5 T-Rex patrol + 4 Raptor pack positions
- **3 Combat Lights**: Red (T-Rex), Orange (Raptor), Blue (Safe)
- **4 NPC Behavior Zones** from Agent #11
- **4 NPC Characters** from Agent #11

### Crowd Simulation Directives
1. **Brachio Herd**: Simulate 8-12 Brachiosaurus moving along StampedeCorridor (500, 3000)
2. **Raptor Pack**: 4-6 raptors using pincer waypoints as crowd agents
3. **Prey Animals**: Small herbivores (Parasaurolophus, Triceratops) grazing in open areas
4. **Flee Behavior**: All prey animals flee when T-Rex enters their zone
5. **Keep predator count**: Max 1 T-Rex + 4 Raptors active simultaneously (performance)
6. **Safe zone**: No crowd agents within 800u of origin (camp)

### Performance Constraints
- Max 150 dinosaur actors total in map
- Crowd agents beyond 3000u from player: LOD disabled, position-only simulation
- T-Rex: single high-detail agent, never crowd-simulated
- Raptors: up to 6 as individual agents, beyond that use crowd system

---

*Generated by Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260617_010*
