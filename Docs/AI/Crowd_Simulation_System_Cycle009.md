# Crowd & Traffic Simulation System — Cycle 009
**Agent #13 — Crowd & Traffic Simulation**
**Cycle:** PROD_CYCLE_AUTO_20260618_009

---

## Overview

This cycle implements the crowd simulation layer that reacts to combat events from Agent #12.
The system defines how herbivore herds behave in relation to predator presence, migration patterns,
and post-stampede dispersal — creating the sense of a living ecosystem.

---

## Actors Deployed in MinPlayableMap

### 1. Brachiosaurus Grazing Herd (5 actors)
| Label | Position | State |
|---|---|---|
| Herd_Brachio_Graze_001 | (5000, -1000, 100) | Grazing — GREEN light |
| Herd_Brachio_Graze_002 | (5400, -800, 100) | Grazing — GREEN light |
| Herd_Brachio_Graze_003 | (4700, -1300, 100) | Grazing — GREEN light |
| Herd_Brachio_Graze_004 | (5200, -1500, 100) | Grazing — GREEN light |
| Herd_Brachio_Graze_005 | (4900, -600, 100) | Grazing — GREEN light |

**Behavior:** Peaceful grazing near BTZone_Brachio_Graze_001. When Stampede_TriggerZone_001
detects TRex aggro state, all 5 units transition to FLEE state and follow Flee_Vector waypoints.

---

### 2. Stampede Trigger Zone (1 actor)
| Label | Position | Radius |
|---|---|---|
| Stampede_TriggerZone_001 | (4000, 500, 150) | 2000u |

**Behavior:** Monitors CombatZone_TRex_Aggro_001 state. When TRex enters AGGRO,
broadcasts STAMPEDE event to all Herd_Brachio actors within 3000u.
**Visual:** ORANGE light (danger transition state).

---

### 3. Raptor Pack Formation (3 actors)
| Label | Position | Role |
|---|---|---|
| Pack_Raptor_Alpha_Lead | (-2000, 1500, 100) | Pack leader — RED light |
| Pack_Raptor_Beta_FlankL | (-2300, 1200, 100) | Left flanker — Dark red |
| Pack_Raptor_Gamma_FlankR | (-1700, 1800, 100) | Right flanker — Dark red |

**Behavior:** Treated as single coordinated Mass AI entity. Alpha sets direction,
Beta and Gamma maintain formation offset. Formation breaks only when Alpha is threatened.
**Integration:** Uses CombatAI_Raptor_Ambush_Alpha/Beta from Agent #12 as formation anchors.

---

### 4. Herbivore Flee Vectors (4 actors)
| Label | Position | Direction |
|---|---|---|
| Flee_Vector_North_001 | (5500, -2500, 100) | NE escape — CYAN light |
| Flee_Vector_West_001 | (-1000, -2000, 100) | W escape — CYAN light |
| Flee_Vector_NW_001 | (2000, -3000, 100) | NW escape — CYAN light |
| Flee_Vector_SW_001 | (-500, -3500, 100) | SW escape — CYAN light |

**Behavior:** All flee vectors point AWAY from TRex position (3000, 2000).
Herbivores select nearest flee vector and follow it to Scatter_Zone.

---

### 5. Migration Corridor (4 waypoints)
| Label | Position | Notes |
|---|---|---|
| Migrate_WP_North_001 | (0, -5000, 100) | Herd entry point — GREEN |
| Migrate_WP_Mid_001 | (0, -1000, 100) | Mid-corridor — GREEN |
| Migrate_WP_River_001 | (1500, 0, 100) | River crossing — BLUE |
| Migrate_WP_South_001 | (0, 3000, 100) | Herd exit point — GREEN |

**Behavior:** Seasonal migration path. Large herbivore herds (10-50 units) follow this
corridor N→S during morning hours, S→N during evening. River crossing at WP_River causes
slowdown and vulnerability window (predator ambush opportunity for player).

---

### 6. Scatter Zones (3 actors)
| Label | Position | Biome |
|---|---|---|
| Scatter_Zone_Forest_001 | (-3000, -4000, 100) | Dense forest — DARK GREEN |
| Scatter_Zone_River_001 | (2000, -4500, 100) | River bank — BLUE |
| Scatter_Zone_Hills_001 | (-4000, 1000, 100) | Rocky hills — BROWN |

**Behavior:** Post-stampede safe zones. Herbivores that reach scatter zones
resume CALM state after 60-120 seconds. Predators do not pursue into scatter zones
(terrain too complex for large predators).

---

## Crowd State Machine

```
CALM (grazing/migrating)
    │
    ├─ TRex detected within 2000u → ALERT (head-up, stop eating)
    │       │
    │       └─ TRex within 1200u OR AGGRO state → STAMPEDE
    │               │
    │               └─ Follow Flee_Vector → FLEE
    │                       │
    │                       └─ Reach Scatter_Zone → RECOVER → CALM
    │
    └─ Migration time → MIGRATE (follow corridor waypoints)
```

---

## Raptor Pack Coordination Logic

```
PACK_IDLE: Alpha patrols, Beta/Gamma shadow at 300u offset
PACK_HUNT: Alpha targets prey, Beta flanks left (-45°), Gamma flanks right (+45°)
PACK_ATTACK: Simultaneous charge — Alpha frontal, Beta/Gamma sides
PACK_SCATTER: If Alpha killed, Beta/Gamma flee independently
```

Formation spacing: 300u between Alpha and flankers
Coordination radius: 800u (flankers break formation if Alpha moves >800u away)

---

## Cumulative Actor Inventory (Cycle 009)

| System | Actor Count | Agent |
|---|---|---|
| Terrain + Lighting | ~8 | #05/#08 |
| Dinosaur placeholders | 5 | #05 |
| NPC Behavior zones | ~6 | #11 |
| Combat AI zones + waypoints | 14 | #12 |
| **Crowd: Brachio herd** | **5** | **#13** |
| **Crowd: Stampede trigger** | **1** | **#13** |
| **Crowd: Raptor pack** | **3** | **#13** |
| **Crowd: Flee vectors** | **4** | **#13** |
| **Crowd: Migration corridor** | **4** | **#13** |
| **Crowd: Scatter zones** | **3** | **#13** |
| **Total this cycle** | **~53** | |

---

## Mass AI Integration Notes (Future Implementation)

When Mass AI is enabled in the project, these PointLight markers define:
- **Entity spawn zones:** Herd_Brachio actors → Mass Entity spawn volumes
- **Fragment assignments:** Each entity gets `FMassStateTreeFragment` with CALM/ALERT/FLEE states
- **Processor chain:** `UMassStateTreeProcessor` → `UMassMovementProcessor` → `UMassAvoidanceProcessor`
- **LOD chain:** Full simulation within 3000u, simplified within 8000u, static beyond 8000u

Current PointLight markers serve as **spatial anchors** — they define WHERE entities exist
and their current behavioral state (color = state). When Mass AI is wired in, these positions
become entity spawn points and the lights become invisible debug markers.

---

## For Agent #14 — Quest & Mission Designer

The crowd system creates these **quest opportunities**:

1. **"The Migration"** — Player must escort/protect a herbivore herd through the migration
   corridor (Migrate_WP_North → Migrate_WP_South) while TRex patrols nearby.

2. **"Stampede Survival"** — Player triggers a stampede (or is caught in one) and must
   reach a Scatter_Zone before being trampled by fleeing Brachiosaurus herd.

3. **"Pack Hunter"** — Player observes Raptor pack formation (Alpha/Beta/Gamma) to learn
   their coordination pattern, then uses that knowledge to defeat them.

4. **"River Crossing"** — Player must cross at Migrate_WP_River_001 during migration —
   herd blocks the path, predators wait on the other side.

**Key positions for quest triggers:**
- Stampede trigger: (4000, 500) — place quest trigger volume here
- Migration river crossing: (1500, 0) — narrow passage, high drama
- Scatter zone Forest: (-3000, -4000) — safe camp location after combat
