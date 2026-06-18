# NPC Behavior System — Cycle 019 (PROD_CYCLE_AUTO_20260618_001)

## Agent #11 — NPC Behavior Agent

### Overview
This document describes the NPC behavior architecture deployed in MinPlayableMap during cycle 019.
All content was created via UE5 Python (Remote Control API) — no C++ compilation required.

---

## Behavior Zones (5 zones deployed)

| Label | Location | Behavior Type | Description |
|-------|----------|---------------|-------------|
| BehaviorZone_TRex_Territory | (3000, 2000, 100) | AGGRESSIVE | T-Rex roams this zone; attacks on sight within 3000 units |
| BehaviorZone_Raptor_Hunt | (-2000, 3500, 100) | PACK_HUNT | Raptor pack coordinates flanking attacks |
| BehaviorZone_Brachio_Feed | (1500, -3000, 100) | PASSIVE_FEED | Brachiosaurus grazes; flees if threatened |
| BehaviorZone_WaterSource | (0, 4500, 50) | WATER_GATHER | All animals converge at dawn/dusk |
| BehaviorZone_Raptor_Nest | (-3500, -1500, 100) | NEST_DEFEND | Extreme aggression — raptors defend nest at all costs |

Zones are marked with colored point lights (visible in editor):
- 🔴 Red = AGGRESSIVE (T-Rex territory)
- 🟠 Orange = PACK_HUNT (Raptor hunting ground)
- 🟢 Green = PASSIVE_FEED (Herbivore feeding)
- 🔵 Blue = WATER_GATHER (Water source)
- 🩷 Pink = NEST_DEFEND (Raptor nest)

---

## Patrol Waypoints (11 waypoints deployed)

### T-Rex Patrol Circuit (6 waypoints)
Wide perimeter patrol — 5000 unit radius around territory center.

```
PatrolWP_TRex_001 → (3000, 2000, 100)
PatrolWP_TRex_002 → (4500, 1000, 100)
PatrolWP_TRex_003 → (5000, -500, 100)
PatrolWP_TRex_004 → (4000, -2000, 100)
PatrolWP_TRex_005 → (2500, -1500, 100)
PatrolWP_TRex_006 → (1800, 500, 100)
```

### Raptor Pack Patrol Circuit (5 waypoints)
Tighter circuit — raptors move faster and cover smaller but denser territory.

```
PatrolWP_Raptor_001 → (-2000, 3500, 100)
PatrolWP_Raptor_002 → (-1200, 4200, 100)
PatrolWP_Raptor_003 → (-500, 3800, 100)
PatrolWP_Raptor_004 → (-800, 2800, 100)
PatrolWP_Raptor_005 → (-1800, 2500, 100)
```

---

## NPC Behavior State Machine (Design)

### T-Rex States
```
PATROL → (player within 3000 units) → CHASE
CHASE  → (player within 300 units)  → ATTACK
ATTACK → (player escapes >4000 units) → PATROL
PATROL → (hunger trigger)           → HUNT_PREY
HUNT_PREY → (prey killed)           → FEED
FEED   → (satiated)                 → REST
REST   → (rest_timer expired)       → PATROL
```

### Raptor Pack States
```
PATROL → (player detected by any raptor) → ALERT_PACK
ALERT_PACK → (pack assembled)            → FLANK
FLANK  → (surround complete)             → COORDINATED_ATTACK
COORDINATED_ATTACK → (player escapes)    → PURSUE
PURSUE → (lost track >2000 units)        → SEARCH
SEARCH → (search_timer expired)          → PATROL
```

### Brachiosaurus States
```
GRAZE → (threat within 1500 units) → ALERT
ALERT → (threat within 800 units)  → FLEE
ALERT → (threat retreats)          → GRAZE
FLEE  → (safe distance >3000 units) → CALM_DOWN
CALM_DOWN → (calm_timer expired)   → GRAZE
```

---

## Voice Lines (TTS Generated)

### Line 1 — Herd Observation
> "The herd moves at dawn. Stay downwind, stay low. If the Brachiosaurus raises its head — freeze. It has already seen you."
- Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781746754492_SurvivalNarrator.mp3
- Duration: ~8s
- Use: Tutorial hint when player first encounters herbivore herd

### Line 2 — Raptor Chase
> "Run. Run now. The pack flanks from the left — do not look back. Your only chance is the river. Go!"
- Audio: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781746770260_SurvivalNarrator.mp3
- Duration: ~7s
- Use: Triggered when raptor pack enters FLANK state targeting player

---

## Integration Notes for Agent #12 (Combat & Enemy AI)

The following data is ready for Combat AI to consume:

1. **Patrol waypoints** — 11 waypoints in MinPlayableMap, labeled `PatrolWP_TRex_NNN` and `PatrolWP_Raptor_NNN`
2. **Behavior zones** — 5 zones labeled `BehaviorZone_*` with behavior type encoded in label
3. **State machine design** — T-Rex, Raptor, Brachio state transitions defined above
4. **Detection ranges**:
   - T-Rex sight: 3000 units
   - T-Rex attack: 300 units
   - Raptor sight: 2000 units (pack alert triggers on ANY raptor detection)
   - Raptor attack: 200 units
   - Brachio alert: 1500 units, flee: 800 units

### What Combat AI (#12) Should Build Next
- Implement actual AI Controller Blueprint that reads PatrolWP actors
- Wire behavior zone overlap events to state transitions
- Implement pack coordination: when one raptor detects player, broadcast to all raptors within 2000 units
- T-Rex stomp attack: AoE damage 300 unit radius, knockback
- Raptor pounce: leap attack from 400 units, pin player briefly

---

## Files Modified
- `/Game/Maps/MinPlayableMap` — 11 patrol waypoints + 5 behavior zones added, MAP_SAVED:True
- `Docs/AI/NPC_BehaviorSystem_Cycle019.md` — this document

## Map State After Cycle
- Total actors: verified via CAP_CHECK
- Patrol waypoints: 11 (TRex×6, Raptor×5)
- Behavior zones: 5 (Territory, Hunt, Feed, Water, Nest)
- Map saved: True
