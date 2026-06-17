# NPC Behavior System — Agent #11 — Cycle PROD_CYCLE_AUTO_20260617_007

## Overview
This cycle implements the raptor pack patrol system and survivor NPC camp layout
for the MinPlayableMap. All content created via UE5 Python (no C++ — editor runs
pre-compiled binary).

---

## Voice Lines Generated

### SurvivorNPC_Warning
> "Stay still. The raptor hunts by movement. If you breathe too loud, it will find you.
> Wait for it to pass — then run north toward the river crossing."
- Audio: `tts/1781702136752_SurvivorNPC_Warning.mp3`
- Trigger: Player enters raptor patrol zone within 800 units of NPC_Scout_001
- Tone: Tense, urgent whisper

### ElderNPC_HuntingAdvice
> "The old hunter says: watch the tree line at dawn. That is when the pack moves.
> Three raptors, maybe four. They circle before they strike.
> You will hear the leaves before you see them."
- Audio: `tts/1781702159145_ElderNPC_HuntingAdvice.mp3`
- Trigger: Player interacts with NPC_Elder_001 (proximity dialogue)
- Tone: Calm, experienced, authoritative

---

## Actors Placed in MinPlayableMap

### Raptor Pack Patrol Waypoints (Forest Biome)
Triangle patrol pattern — raptors cycle between these points:

| Label | Location | Role |
|-------|----------|------|
| Waypoint_Floresta_011 | (-800, 1200, 100) | Patrol Point A |
| Waypoint_Floresta_012 | (-1400, 800, 100) | Patrol Point B |
| Waypoint_Floresta_013 | (-600, 400, 100)  | Patrol Point C |

**Behavior Design:**
- Raptors patrol A→B→C→A in a 45-second loop
- At each waypoint, raptor pauses 3-5 seconds (idle/sniff animation)
- If player detected within 600 units: break patrol, enter chase state
- If player escapes beyond 1500 units: return to nearest waypoint, resume patrol

### Survivor NPC Camp (Savana Biome)
| Label | Location | Behavioral Role |
|-------|----------|----------------|
| NPC_Scout_001   | (500, -800, 100)  | Patrols 200-unit radius around camp perimeter |
| NPC_Elder_001   | (600, -900, 100)  | Stationary near fire — dialogue trigger |
| NPC_Crafter_001 | (550, -1000, 100) | Works at crafting station — busy animation |
| CampFire_001    | (575, -900, 100)  | Camp center anchor — all NPCs orient toward this |

---

## NPC Behavior State Machine Design

### Scout (NPC_Scout_001)
```
IDLE → PATROL → ALERT → WARN_PLAYER → FLEE
         ↑                    ↓
         └──────── RESUME ────┘
```
- PATROL: 200-unit radius loop around camp, 30s cycle
- ALERT: triggered when raptor detected within 1000 units
- WARN_PLAYER: plays SurvivorNPC_Warning voice line, gestures
- FLEE: runs to cave shelter marker if raptor within 400 units

### Elder (NPC_Elder_001)
```
IDLE_FIRE → DIALOGUE → IDLE_FIRE
```
- IDLE_FIRE: sits near campfire, stokes fire animation loop
- DIALOGUE: triggered by player proximity (< 200 units)
- Plays ElderNPC_HuntingAdvice voice line
- Shares hunting knowledge: raptor patrol times, safe paths

### Crafter (NPC_Crafter_001)
```
CRAFT_LOOP → INSPECT_ITEM → CRAFT_LOOP
```
- Continuous crafting animation (stone tool creation)
- Reacts to loud sounds (T-Rex roar): looks up, freezes 2s, resumes
- Does NOT flee — trusts camp defenses

---

## Raptor Pack AI Design

### Pack Behavior (3 Raptors)
- **Alpha Raptor**: leads patrol, first to detect player
- **Beta Raptor**: flanks left when chasing
- **Gamma Raptor**: flanks right, cuts off escape routes

### Detection System
| State | Trigger | Response |
|-------|---------|---------|
| Patrol | Default | Follow waypoint triangle |
| Alert | Sound/movement within 600u | Stop, orient toward source |
| Hunt | Visual confirmation | Alpha charges, Beta/Gamma flank |
| Kill | Player within 150u | Attack animation |
| Lose | Player > 1500u for 10s | Return to nearest waypoint |

### Memory System
- Raptors remember last known player position for 30 seconds
- If player hides (no LOS for 15s), raptors search last position then resume patrol
- Pack shares information: if Alpha spots player, Beta/Gamma immediately know

---

## Integration Notes for Agent #12 (Combat & Enemy AI)

### Handoff Data
1. **Raptor waypoints** placed at Waypoint_Floresta_011/012/013 — use these as
   NavMesh patrol targets for actual raptor pawn AI controllers
2. **NPC camp** at (575, -900, 100) — NPCs need proper pawn classes with
   behavior trees referencing these TargetPoint actors
3. **Voice lines** are ready at Supabase URLs — wire to dialogue trigger components
4. **Detection radii**: Scout=1000u, Raptor=600u, T-Rex=3000u (from previous cycles)

### Behavior Tree Assets Needed (Agent #12 to create)
- `BT_Raptor_Pack` — uses Waypoint_Floresta_011/012/013 as patrol keys
- `BT_Survivor_Scout` — patrol + alert + flee states
- `BT_Survivor_Elder` — idle + dialogue trigger
- `BB_Raptor` — blackboard with: TargetActor, PatrolIndex, bIsHunting, LastKnownPos
- `BB_Survivor` — blackboard with: bIsAlert, DialoguePlayed, NearestShelter

---

## Map Status
- MAP_SAVED: True
- Total new actors this cycle: 7 (3 raptor waypoints + 3 NPC markers + 1 campfire)
- All labels follow format: Type_Bioma_NNN (compliant with hugo_no_degenerate_labels)
