# Crowd & Traffic Simulation System — Agent #13 — Cycle PROD_CYCLE_AUTO_20260617_007

## Overview
This cycle implements the full Mass AI crowd simulation layer that responds to Combat AI zones
established by Agent #12. The system defines herbivore behavioral states, exclusion/avoidance zones,
stampede corridors, and migration paths — all wired to the combat trigger system.

---

## Zone Map (wired to Agent #12 Combat AI)

### Exclusion Zones (no crowd agents path through)
| Zone | Center | Radius | Trigger |
|------|--------|--------|---------|
| `MassAI_Exclusion_TRex_001` | (1800, 500) | 2000u | Always active — T-Rex territory |
| `MassAI_DangerZone_Raptor_001` | (-800, 1000) | 600u | Active when raptor pack hunts |
| `MassAI_Avoidance_Ankylo_001` | (-200, -1500) | 500u | Passive — Ankylosaurus grazing |

### Stampede System
- **Trigger**: `Stampede_TRex_Trigger_001` at (1400, 600) — activates when `Ambush_TRex_Origin_001` fires
- **Flee vector**: NW direction away from T-Rex origin (1800, 500)
- **Flee corridor**: 4 waypoints from (600,-200) → (-1600,-1600)
- **Flee speed multiplier**: 2.5× normal herbivore walk speed
- **Panic radius**: 2000u from T-Rex trigger — all herbivores within radius enter FLEE state

### Raptor Pack Role Anchors (Mass AI Entity Configs)
| Role | Position | Behavior |
|------|----------|----------|
| `MassAI_Raptor_Alpha_001` | (-800, 1200) | Frontal charge — highest aggression |
| `MassAI_Raptor_Beta_001` | (-1100, 1500) | Left flank — intercept fleeing prey |
| `MassAI_Raptor_Gamma_001` | (-500, 1500) | Right flank — cut off escape routes |

---

## Migration Corridor

Wide arc from south plains to northern water source — avoids all predator territories.

```
Migration path (6 waypoints):
  001 (-2000, -2500) → 002 (-2400, -1500) → 003 (-2600, -500)
  → 004 (-2400, 600) → 005 (-2000, 1500) → 006 (-1500, 2200)
```

- **Corridor width**: 800u each side of waypoint line
- **Migration speed**: 0.6× walk speed (grazing migration)
- **Time of day**: Dawn (06:00-09:00) and Dusk (17:00-20:00)
- **Herd size**: 8-15 Parasaurolophus, 3-6 Brachiosaurus

### Safe Grazing Zones
| Zone | Position | Species |
|------|----------|---------|
| `GrazingZone_Savanna_001` | (-2200, -1800) | Parasaurolophus, Iguanodon |
| `GrazingZone_Plains_001` | (-2500, 0) | Brachiosaurus, Diplodocus |
| `GrazingZone_North_001` | (-1800, 2000) | Mixed herbivores near water |

---

## Herbivore Behavioral State Machine

```
IDLE/GRAZE ──────────────────────────────────────────────────────┐
    │                                                              │
    │ Predator detected (radius 1500u)                            │
    ▼                                                              │
ALERT ──────────────────────────────────────────────────────────  │
    │                                                              │
    │ Predator within 800u OR stampede trigger fires              │
    ▼                                                              │
FLEE ──── Follow flee corridor waypoints ──── Speed 2.5×         │
    │                                                              │
    │ Safe zone reached AND no predator within 3000u              │
    ▼                                                              │
RECOVER ──── 30s calm period ──── Return to IDLE ───────────────┘
```

### State Transitions
- **IDLE → ALERT**: Any predator (T-Rex, Raptor, Carnotaurus) within 1500u
- **ALERT → FLEE**: Predator within 800u OR `Stampede_TRex_Trigger_001` activates
- **FLEE → RECOVER**: Reached `GrazingZone_*` AND no predator within 3000u for 5s
- **RECOVER → IDLE**: 30s elapsed in safe zone

---

## Mass AI Entity Configuration (Design Spec)

### Herbivore Entity (Parasaurolophus)
```
Mass Entity Traits:
  - MassMovement: MaxSpeed=600, TurnRate=90deg/s
  - MassAvoidance: PersonalSpace=200u, GroupSpace=400u
  - MassStateTree: BT_Herbivore_Crowd
  - LOD: Full AI 0-3000u, Reduced 3000-8000u, Dormant 8000u+
  - Flock size: 8-15 per herd
  - Herd anchor: HerdAnchor_Brachio_* (from cycle 006)
```

### Herbivore Entity (Brachiosaurus — large)
```
Mass Entity Traits:
  - MassMovement: MaxSpeed=350, TurnRate=45deg/s
  - MassAvoidance: PersonalSpace=600u, GroupSpace=1000u
  - MassStateTree: BT_Brachio_Crowd
  - LOD: Full AI 0-5000u, Reduced 5000-12000u, Dormant 12000u+
  - Flock size: 3-6 per herd
```

---

## LOD Chain (Performance)

| Distance | Behavior | Update Rate |
|----------|----------|-------------|
| 0-500u | Full simulation (pathfinding, state machine, animation) | 60Hz |
| 500-2000u | Simplified movement (direct to waypoint, no avoidance) | 10Hz |
| 2000-5000u | Position interpolation only | 2Hz |
| 5000-10000u | Static pose, no movement | 0.5Hz |
| 10000u+ | Dormant (invisible, no tick) | Off |

---

## Integration with Combat AI (Agent #12)

### Event Bus (Design)
```
Agent #12 fires → Agent #13 responds:

CombatEvent_TRexAmbush_Activated:
  → All herbivores within 2000u of (1800,500) enter FLEE state
  → Flee corridor activates (Stampede_FleePoint_001-004)
  → Herd anchors temporarily disabled

CombatEvent_RaptorPack_Hunt:
  → Herbivores within 1000u of (-800,1000) scatter (random flee vectors)
  → No organized corridor — panic scatter

CombatEvent_Combat_Ended:
  → 60s cooldown before herbivores return to migration/grazing
  → Gradual re-entry from safe zones
```

---

## Actors Spawned This Cycle

| Label | Position | Purpose |
|-------|----------|---------|
| `Stampede_TRex_Trigger_001` | (1400, 600, 120) | Stampede activation zone |
| `Stampede_FleePoint_001` | (600, -200, 100) | Flee corridor WP1 |
| `Stampede_FleePoint_002` | (-200, -600, 100) | Flee corridor WP2 |
| `Stampede_FleePoint_003` | (-900, -1100, 100) | Flee corridor WP3 |
| `Stampede_FleePoint_004` | (-1600, -1600, 100) | Flee corridor WP4 |
| `MassAI_Exclusion_TRex_001` | (1800, 500, 150) | T-Rex exclusion zone |
| `MassAI_DangerZone_Raptor_001` | (-800, 1000, 150) | Raptor danger zone |
| `MassAI_Avoidance_Ankylo_001` | (-200, -1500, 150) | Ankylo avoidance zone |
| `MassAI_Raptor_Alpha_001` | (-800, 1200, 100) | Raptor alpha role anchor |
| `MassAI_Raptor_Beta_001` | (-1100, 1500, 100) | Raptor beta role anchor |
| `MassAI_Raptor_Gamma_001` | (-500, 1500, 100) | Raptor gamma role anchor |
| `Migration_Waypoint_001-006` | Various | Migration corridor |
| `GrazingZone_Savanna_001` | (-2200, -1800, 100) | Safe grazing |
| `GrazingZone_Plains_001` | (-2500, 0, 100) | Safe grazing |
| `GrazingZone_North_001` | (-1800, 2000, 100) | Safe grazing near water |

**Total new actors: 20**
**MAP_SAVED: True**

---

## Handoff to Agent #14 — Quest & Mission Designer

### Quest Hooks Available
1. **"The Migration"** — Player must escort/observe herbivore migration from south to north water source
   - Trigger: Player within 500u of `Migration_Waypoint_001`
   - Objective: Follow migration without triggering stampede
   - Fail condition: Player enters T-Rex exclusion zone during migration

2. **"Stampede Warning"** — Player must warn survivor camp before T-Rex stampede reaches it
   - Trigger: `Stampede_TRex_Trigger_001` activates
   - Objective: Reach camp at (0,0) before stampede arrives (flee corridor passes nearby)
   - Reward: Camp survivors provide crafting materials

3. **"Safe Passage"** — Player must guide a wounded herbivore through raptor territory
   - Trigger: Player interacts with wounded Parasaurolophus near `MassAI_DangerZone_Raptor_001`
   - Objective: Escort to `GrazingZone_North_001` avoiding raptor pack
   - Mechanic: Herbivore follows player, raptors attracted to herbivore scent

4. **"Raptor Pack Observation"** — Player observes raptor hunt from safe distance
   - Trigger: Player within 2000u of `MassAI_Raptor_Alpha_001` but outside 600u danger zone
   - Objective: Observe full hunt sequence (Alpha charge → Beta/Gamma flank → takedown)
   - Reward: Player learns raptor behavioral patterns (gameplay knowledge unlock)

### Zone Data for Quest Triggers
```
Safe observation posts (outside all danger zones):
  - Post_A: (-1400, 800, 100) — overlooks raptor territory from west ridge
  - Post_B: (0, -800, 100) — overlooks Ankylo grazing from south
  - Post_C: (-2800, 0, 100) — overlooks migration corridor from far west
```
