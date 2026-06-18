# Crowd & Traffic Simulation System — Agent #13
## Transpersonal Game Studio — Prehistoric Survival Game

---

## Overview

The crowd simulation system governs all multi-agent behavior in the prehistoric world:
- Herbivore herds (Brachiosaurus, Triceratops, Parasaurolophus)
- Raptor pack coordination and patrol
- Panic propagation when predators are detected
- Migration routes along river corridors
- LOD-based density management for performance

---

## Actors Placed in MinPlayableMap (This Cycle)

### Panic Propagation Zones (Orange-Red Lights)
| Label | Location | Radius | Trigger |
|-------|----------|--------|---------|
| PanicZone_Herbivore_TRex_001 | (1200, 800, 200) | 1500 units | CombatZone_TRex_001 active |
| PanicZone_Herbivore_Raptor_001 | (-800, 1500, 180) | 1500 units | CombatZone_Raptor_001 active |
| PanicZone_Herbivore_Raptor_002 | (2200, -600, 180) | 1500 units | CombatZone_Raptor_002 active |

**Behavior:** When a combat zone becomes active (predator engages), all herbivore agents within the panic zone radius switch from IDLE/GRAZE to FLEE state. Flee direction = away from predator origin.

### Brachiosaurus Herd Formation (Green Lights)
| Label | Location | Role |
|-------|----------|------|
| Herd_Brachio_Lead_001 | (3000, 2000, 180) | Lead adult — sets direction |
| Herd_Brachio_Flank_001 | (3200, 1800, 160) | Right flank adult — defensive |
| Herd_Brachio_Flank_002 | (2800, 1800, 160) | Left flank adult — defensive |
| Herd_Brachio_Juvenile_001 | (3000, 1600, 150) | Juvenile — protected center |

**Formation Logic:**
- Juvenile always moves to center of adult triangle
- Adults maintain 200-unit spacing
- On predator detection: adults rotate outward (defensive circle), juvenile stays inside
- Formation breaks only if predator enters 400-unit radius

### Migration Waypoints (Blue Lights)
| Label | Location | Corridor |
|-------|----------|----------|
| MigrationWP_Brachio_001 | (3000, 2000, 160) | Savanna start |
| MigrationWP_Brachio_002 | (2500, 3500, 160) | River bend |
| MigrationWP_Brachio_003 | (1800, 5000, 160) | Northern forest |

**Migration Schedule:** Herd moves north at dusk (18:00 game time), returns south at dawn (06:00). Migration pauses if predator detected within 2000 units.

### Raptor Pack Patrol Waypoints (Purple Lights)
| Label | Location | Order |
|-------|----------|-------|
| RaptorPack_PatrolWP_001 | (-800, 1500, 160) | 1st — territory center |
| RaptorPack_PatrolWP_002 | (-1600, 2200, 160) | 2nd — northwest |
| RaptorPack_PatrolWP_003 | (-400, 2800, 160) | 3rd — northeast |
| RaptorPack_PatrolWP_004 | (200, 1800, 160) | 4th — east boundary |

**Pack Patrol Logic:**
- Alpha follows waypoints in order, loops
- Beta trails Alpha by 2 waypoints (staggered)
- Gamma patrols inverse order (counter-patrol)
- All 3 converge on player detection — flanking formation from Agent #12

### Crowd LOD Zones (Yellow Lights)
| Label | Location | Radius | LOD Level |
|-------|----------|--------|-----------|
| CrowdLOD_HighDensity_001 | (0, 0, 200) | 2000 units | Full simulation |
| CrowdLOD_MedDensity_001 | (2000, 2000, 200) | 2000 units | Simplified behavior |
| CrowdLOD_LowDensity_001 | (4000, 4000, 200) | 2000 units | Static imposters |

---

## Crowd Simulation Architecture

### Agent State Machine
```
IDLE ──────────────────► GRAZE
  │                        │
  │ predator_nearby        │ predator_nearby
  ▼                        ▼
ALERT ──────────────────► FLEE
  │                        │
  │ predator_gone          │ safe_distance
  ▼                        ▼
RETURN ◄────────────────── CALM
```

### Panic Propagation Algorithm
```
1. CombatZone becomes ACTIVE (predator engages prey)
2. PanicZone radius check: all herbivores within 1500 units
3. Panic signal propagates outward at 800 units/second
4. Each herbivore receives panic signal → switches to FLEE
5. Flee vector = normalize(herbivore_pos - predator_pos) * flee_speed
6. Panic decays after 30 seconds if no new predator events
7. Herd cohesion: fleeing agents pull nearby agents (radius 300 units)
```

### LOD Performance Budget
| Zone | Max Agents | Update Rate | Behavior Complexity |
|------|-----------|-------------|---------------------|
| High (0-2000u) | 20 | 60Hz | Full state machine |
| Medium (2000-4000u) | 50 | 10Hz | Simplified (3 states) |
| Low (4000u+) | 200 | 2Hz | Static + occasional move |

**Target:** 50,000 total agents at 30fps on target hardware
- High zone: 20 agents × full sim = 20 full agents
- Medium zone: 50 agents × simplified = equivalent of 10 full agents
- Low zone: 200 agents × static = equivalent of 5 full agents
- Total budget: ~35 full-agent equivalents per frame

---

## Integration with Agent #12 Combat System

### Combat Zone → Crowd Trigger Mapping
```
CombatZone_TRex_001 (1200, 800)
  └─► PanicZone_Herbivore_TRex_001 (1500u radius)
      └─► Herd_Brachio_* agents flee north toward MigrationWP_Brachio_002

CombatZone_Raptor_001 (-800, 1500)
  └─► PanicZone_Herbivore_Raptor_001 (1500u radius)
      └─► Any herbivores in range flee east

CombatZone_Raptor_002 (2200, -600)
  └─► PanicZone_Herbivore_Raptor_002 (1500u radius)
      └─► Any herbivores in range flee northwest
```

### Raptor Pack ↔ Crowd Coordination
- When RaptorPack detects player, pack abandons patrol route
- Pack converges using flanking formation (from Agent #12)
- Patrol waypoints become inactive during active hunt
- After hunt ends (player escapes or 120s timeout), pack resumes patrol from nearest waypoint

---

## Next Steps for Agent #14 (Quest & Mission Designer)

### Crowd-Based Quest Triggers
1. **"The Migration"** — Player must escort Brachio herd from WP_001 to WP_003 without predator attack
2. **"Pack Territory"** — Player must cross raptor patrol zone without triggering pack convergence
3. **"Panic in the Valley"** — Player triggers TRex combat → observe herbivore panic propagation → follow fleeing herd to hidden cave
4. **"Herd Defender"** — Juvenile Brachio separated from herd → player must reunite before raptor pack finds it

### Observable Crowd Events (Quest Hooks)
- Herd formation breaks → quest trigger: "Something spooked the herd"
- Raptor patrol deviates from route → quest trigger: "The raptors are tracking something"
- Mass panic propagation → quest trigger: "The whole valley is running"
- Migration starts at dusk → quest trigger: "Follow the giants"

---

## Technical Notes

### Mass AI Integration (Future)
When UE5 Mass AI is available in the project:
- Replace point light markers with `UMassEntitySubsystem` agents
- Each herd agent = `FMassEntityHandle` with `FMassTransformFragment`
- Panic propagation = `UMassSignalSubsystem` broadcast
- LOD = `UMassLODSubsystem` with distance-based processor switching

### Current Implementation
Point lights serve as visual markers for:
- Designer reference during level building
- Runtime query targets (overlap detection)
- Debug visualization of crowd zones

Color coding:
- 🟠 Orange-red = Panic/danger zones
- 🟢 Green = Herd agents
- 🔵 Blue = Migration waypoints
- 🟣 Purple = Raptor patrol
- 🟡 Yellow = LOD boundaries
