# Crowd & Traffic Simulation Agent #13 — Cycle 009 Report

## Overview
This cycle integrates crowd simulation with Combat AI outputs from Agent #12.
All crowd actors are placed in response to combat zone data, creating emergent
crowd behaviour that reacts to predator presence.

---

## Combat AI Integration

### Inputs from Agent #12
| Actor | Location | Crowd Response |
|---|---|---|
| `CombatZone_TRex_Aggro_001` | (800, 1800, 100) | Crowd exclusion zone — no agents within 2000u |
| `CombatZone_Raptor_Pack_001` | (-200, 1200, 100) | Panic scatter zone — 1500u panic radius |
| `Waypoint_Combat_Flank_001–004` | NE/NW vectors | Crowd-avoided paths — predator approach vectors |
| `CombatMarker_ScoutAlert_001` | (600, 900, 150) | Panic chain origin — Scout alert propagates outward |
| `TRex_Alpha_Combat_001` | (900, 2000, 100) | Dynamic exclusion anchor — crowd flow shifts with T-Rex |

---

## Actors Spawned This Cycle

### Crowd Exclusion Zones
| Label | Location | Radius | Purpose |
|---|---|---|---|
| `CrowdExclusion_TRex_Zone_001` | (800, 1800, 120) | 2000u | No crowd agents enter T-Rex territory |
| `CrowdPanic_Raptor_Zone_001` | (-200, 1200, 120) | 1500u | Crowd enters panic state near raptors |

### Safe Flow Corridors (6 waypoints)
Crowd agents route through these waypoints when predators are active.
Corridors run SW and S — perpendicular to predator NE/NW flanking vectors.

| Label | Location | Corridor |
|---|---|---|
| `CrowdFlow_SafePath_001` | (-800, 400, 100) | SW corridor start |
| `CrowdFlow_SafePath_002` | (-1200, 200, 100) | SW corridor mid |
| `CrowdFlow_SafePath_003` | (-1600, -100, 100) | SW corridor end |
| `CrowdFlow_SafePath_004` | (200, -600, 100) | S corridor start |
| `CrowdFlow_SafePath_005` | (400, -1200, 100) | S corridor mid |
| `CrowdFlow_SafePath_006` | (600, -1800, 100) | S corridor end |

### Panic Propagation Relay Chain (5 nodes)
Chain propagates from Scout alert outward through camp → gathering → river → forest.

| Label | Location | Relay Stage |
|---|---|---|
| `PanicRelay_Scout_001` | (600, 900, 150) | Origin — Scout detection |
| `PanicRelay_Camp_001` | (300, 600, 150) | Stage 1 — Camp area |
| `PanicRelay_Gather_001` | (0, 300, 150) | Stage 2 — Gathering point |
| `PanicRelay_River_001` | (-300, 0, 150) | Stage 3 — River crossing |
| `PanicRelay_Forest_001` | (-600, -300, 150) | Stage 4 — Forest refuge |

### Herd Scatter Anchors (3 anchors)
Herbivore herds scatter from these anchor points when predator alert triggers.

| Label | Location | Species |
|---|---|---|
| `HerdScatter_Brachio_001` | (2500, 1500, 100) | Brachiosaurus herd |
| `HerdScatter_Trike_001` | (-1800, 2200, 100) | Triceratops herd |
| `HerdScatter_Pachy_001` | (3200, -800, 100) | Pachycephalosaurus herd |

---

## Crowd Behaviour Design

### Panic Propagation System
```
NPC_Scout_001 detects predator
    → PanicRelay_Scout_001 activates (t=0s)
    → PanicRelay_Camp_001 activates (t=2s, 300u radius)
    → PanicRelay_Gather_001 activates (t=4s, 300u radius)
    → PanicRelay_River_001 activates (t=6s, 300u radius)
    → PanicRelay_Forest_001 activates (t=8s, 300u radius)
    → All crowd agents within 1500u of relay chain enter PANIC state
    → Crowd flows to nearest CrowdFlow_SafePath waypoint
```

### Crowd State Machine
```
IDLE → ALERT (predator within 3000u) → PANIC (predator within 1500u) → FLEE (on safe path)
FLEE → SHELTER (reached forest/cave) → IDLE (predator gone >5min)
```

### Exclusion Zone Logic
- T-Rex exclusion: Hard boundary — crowd agents reroute around 2000u radius
- Raptor panic zone: Soft boundary — crowd agents accelerate and scatter at 1500u
- Safe corridors: Preferred paths — crowd agents use these routes during predator events

### Herd Scatter Behaviour
- Herbivore herds maintain anchor points during IDLE
- On predator alert: herds scatter radially from anchor (random 500-1500u displacement)
- Herd reforms at anchor after 10 minutes if predator gone
- Scatter direction: away from predator, toward forest/water

---

## Crowd Density Map (Cycle 009 State)

```
NORTH (+Y)
    |
    |  [TRex_Alpha] [CombatZone_TRex] [CrowdExclusion_TRex]
    |       ↑ EXCLUSION ZONE (2000u radius)
    |
    |  [CombatZone_Raptor] [CrowdPanic_Raptor]
    |       ↑ PANIC ZONE (1500u radius)
    |
    |  [PanicRelay_Scout] ← Scout alert origin
    |  [PanicRelay_Camp]
    |  [PanicRelay_Gather]  ← PLAYER START (0,0,0)
    |  [PanicRelay_River]
    |  [PanicRelay_Forest]
    |
    |  [CrowdFlow_SafePath_001-003] ← SW corridor
    |  [CrowdFlow_SafePath_004-006] ← S corridor
    |
SOUTH (-Y)
```

---

## Cumulative Crowd Infrastructure (All Cycles)

### Previous Cycles
- **Cycle 006**: Herbivore herd anchors (Brachio, Stego, Iguanodon, Parasaur)
- **Cycle 007**: Migration corridor waypoints, tribal camp patrol routes
- **Cycle 008**: Crowd scatter zones, evacuation routes, LOD flow markers

### This Cycle (009)
- Combat AI integration: exclusion zones, panic zones, safe corridors
- Panic propagation relay chain (5 nodes)
- Herd scatter anchors (3 species)

### Total Crowd Actors Placed (estimate across cycles)
- ~45 crowd-related actors in MinPlayableMap
- 6 safe flow corridor waypoints
- 5 panic relay nodes
- 3 herd scatter anchors
- 2 combat integration zones

---

## Handoff to Agent #14 — Quest & Mission Designer

### Quest Integration Points
1. **Predator Encounter Quest**: T-Rex exclusion zone is the quest trigger boundary
   - Quest activates when player enters 2500u of `CrowdExclusion_TRex_Zone_001`
   - Objective: "Survive the T-Rex encounter" — reach `CrowdFlow_SafePath_001`

2. **Herd Migration Quest**: Migration corridor waypoints define quest path
   - Objective: "Follow the Brachiosaurus migration" — track herd from anchor to scatter point
   - Reward: discover new territory, find resources along migration path

3. **Rescue Quest**: Panic relay chain defines NPC rescue locations
   - NPCs scatter along relay chain during predator event
   - Quest: find and rescue 3 NPCs at relay nodes before predator reaches them

4. **Scout Alert Quest**: `PanicRelay_Scout_001` is quest trigger
   - NPC_Scout_001 alerts player → quest activates
   - Objective: "Warn the camp" — reach `PanicRelay_Camp_001` before panic propagates

### Crowd-Driven Narrative Moments
- Player witnesses herd scatter → environmental storytelling (predator is near)
- Panic chain propagation → visual cue that something dangerous is approaching
- Crowd flows to safe corridors → player can follow crowd to safety
- Herd reforms after predator gone → world returns to equilibrium

---

## Technical Notes

### Mass AI Integration (Future)
When UE5 Mass AI is enabled:
- `CrowdExclusion_TRex_Zone_001` → Mass Avoidance Obstacle (radius 2000u)
- `CrowdPanic_Raptor_Zone_001` → Mass Signal Emitter (panic signal, radius 1500u)
- `CrowdFlow_SafePath_*` → Mass Navigation Waypoints (preferred path weight +500)
- `PanicRelay_*` → Mass Signal Relay (propagation delay 2s per node)

### Performance Budget
- Panic relay chain: 5 actors, minimal overhead
- Safe corridors: 6 actors, used as navmesh hints
- Scatter anchors: 3 actors, referenced by herd AI
- Total new actors this cycle: 16

### MAP_SAVED: True
All actors saved to `/Game/Maps/MinPlayableMap`

---

*Generated by Crowd & Traffic Simulation Agent #13 — PROD_CYCLE_AUTO_20260617_009*
