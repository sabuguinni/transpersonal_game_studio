# Crowd & Traffic Simulation — Production Cycle 008
**Agent #13 — Crowd & Traffic Simulation**
**Cycle:** PROD_CYCLE_AUTO_20260618_008

---

## Overview

This cycle deploys the full crowd simulation infrastructure for MinPlayableMap, building directly on Agent #12's combat zone architecture. All crowd systems are spatially anchored to the combat triggers placed by Agent #12.

---

## Systems Deployed

### 1. Brachiosaurus Herd System (9 actors)
**Location:** Centered on (500, -3000) — Agent #12's stampede trigger zone

| Actor Label | Position | Role |
|---|---|---|
| Brachio_Herd_001 | (400,-2800,100) | Herd member |
| Brachio_Herd_002 | (600,-2900,100) | Herd member |
| Brachio_Herd_003 | (300,-3100,100) | Herd member |
| Brachio_Herd_004 | (700,-3000,100) | Herd member |
| Brachio_Herd_005 | (500,-3200,100) | Herd member |
| Brachio_Herd_006 | (200,-2950,100) | Herd member |
| Brachio_Herd_007 | (800,-3150,100) | Herd member |
| Brachio_Herd_008 | (450,-3300,100) | Herd member |
| Brachio_HerdLeader_Alpha | (500,-2750,100) | Alpha — herd anchor |

**Behavior Design:**
- Alpha leads movement along `Combat_Brachio_StampedePath_001→004` waypoints
- When `Combat_Brachio_StampedeZone_001` is triggered, all 8 follow alpha at full speed
- Stampede direction: North along Y-axis corridor
- Density cap: Max 8 Brachiosaurus in MinPlayableMap (enforced by `DensityCap_Brachio_Max8`)

---

### 2. Raptor Pack System (6 actors)
**Location:** Distributed across Agent #12's raptor combat zones (-1100 to -1800, 2500-3100)

| Actor Label | Position | Combat Zone Anchor |
|---|---|---|
| Raptor_Pack_Alpha_001 | (-1500,2500,100) | Combat_Raptor_Alpha_Attack_001 |
| Raptor_Pack_Beta_001 | (-1200,3000,100) | Combat_Raptor_Beta_Flank_001 |
| Raptor_Pack_Gamma_001 | (-1800,2800,100) | Combat_Raptor_Gamma_Ambush_001 |
| Raptor_Pack_Delta_001 | (-1350,2650,100) | Reserve flanker |
| Raptor_Pack_Epsilon_001 | (-1650,3100,100) | Rear guard |
| Raptor_Pack_Zeta_001 | (-1100,2750,100) | Scout |

**Behavior Design:**
- Alpha, Beta, Gamma execute pincer maneuver (from Agent #12 design)
- Delta/Epsilon/Zeta are reserve pack — activate when player escapes initial pincer
- Pack shares zone awareness: if one detects player, all are alerted within 800 units
- Density cap: Max 15 Raptors in MinPlayableMap (enforced by `DensityCap_Raptor_Max15`)

---

### 3. Prey Scatter System (5 trigger zones)
**Trigger:** TRex enters `Combat_TRex_ThreatRing_Detect` (600u radius)

| Actor Label | Position | Direction |
|---|---|---|
| Prey_ScatterZone_North_001 | (2000,500,100) | North escape corridor |
| Prey_ScatterZone_South_001 | (2000,2500,100) | South escape corridor |
| Prey_ScatterZone_West_001 | (1000,1500,100) | West escape corridor |
| Prey_ScatterZone_NorthEast_001 | (2500,500,100) | NE escape corridor |
| Prey_ScatterZone_SouthWest_001 | (1500,2800,100) | SW escape corridor |

**Behavior Design:**
- When TRex enters detect zone, all herbivores within 600u flee to nearest scatter zone
- Scatter zones define valid escape directions (away from TRex)
- Fleeing herbivores have 30% speed boost for 8 seconds
- Prey that cannot reach scatter zone within 5s are "caught" (despawn + audio cue)

---

### 4. Crowd Panic Propagation System (2 zones)

| Actor Label | Radius | Effect |
|---|---|---|
| Crowd_PanicCore_600u_001 | 600u | Immediate flee — all herbivores sprint |
| Crowd_PanicBroadcast_1200u_001 | 1200u | Cascade flee — herbivores walk fast toward scatter zones |

**Propagation Logic:**
1. TRex enters CHASE state → broadcasts FLEE signal
2. `PanicCore_600u`: All herbivores within 600u immediately sprint (panic state)
3. `PanicBroadcast_1200u`: All herbivores within 1200u switch to alert/fast-walk state
4. Panic propagates outward: herbivores fleeing alert other herbivores they pass (chain reaction)
5. Panic duration: 45 seconds after TRex leaves zone

---

### 5. Density Cap Enforcement (4 markers)

| Actor Label | Cap Value | Zone |
|---|---|---|
| DensityCap_TRex_Max5 | 5 T-Rex max | (3000,1500) |
| DensityCap_Raptor_Max15 | 15 Raptors max | (-1500,2800) |
| DensityCap_Brachio_Max8 | 8 Brachiosaurus max | (500,-3000) |
| DensityCap_Global_Max150 | 150 total dinos max | (0,0) |

---

### 6. Herbivore Idle Scatter (8 groups)

| Actor Label | Species | Position |
|---|---|---|
| Iguanodon_Idle_020 | Iguanodon | (-2000,-1000,100) |
| Parasaur_Idle_025 | Parasaurolophus | (-2500,500,100) |
| Stegosaur_Idle_010 | Stegosaurus | (1000,-2000,100) |
| Triceratops_Idle_005 | Triceratops | (-500,-2500,100) |
| Ankylosaur_Idle_020 | Ankylosaurus | (2000,-1500,100) |
| Iguanodon_Idle_030 | Iguanodon | (-3000,-500,100) |
| Parasaur_Idle_015 | Parasaurolophus | (1500,3000,100) |
| Stegosaur_Idle_010 | Stegosaurus | (-1000,3500,100) |

**Behavior Design:**
- Idle herbivores graze in place (random rotation, slow drift)
- React to panic propagation from TRex zone
- Do NOT enter Raptor pack zones (territorial awareness)
- Provide visual density to the world — player sees a living ecosystem

---

## Integration Map — Agent #12 → Agent #13

| Agent #12 Actor | Agent #13 System | Integration |
|---|---|---|
| Combat_Brachio_StampedeZone_001 | Brachio_Herd_* | Stampede trigger activates herd movement |
| Combat_Brachio_StampedePath_001→004 | Brachio_HerdLeader_Alpha | Alpha follows waypoints |
| Combat_Raptor_Alpha_Attack_001 | Raptor_Pack_Alpha_001 | Alpha raptor anchored to attack zone |
| Combat_Raptor_Beta_Flank_001 | Raptor_Pack_Beta_001 | Beta raptor anchored to flank zone |
| Combat_Raptor_Gamma_Ambush_001 | Raptor_Pack_Gamma_001 | Gamma raptor anchored to ambush zone |
| Combat_TRex_ThreatRing_Detect | Crowd_PanicCore_600u_001 | TRex detect → panic broadcast |
| Combat_TRex_ThreatRing_Chase | Crowd_PanicBroadcast_1200u_001 | TRex chase → cascade flee |

---

## Total Actors Deployed This Cycle
- Brachiosaurus Herd: 9 actors
- Raptor Pack: 6 actors
- Prey Scatter Zones: 5 actors
- Panic Propagation: 2 actors
- Density Caps: 4 actors
- Herbivore Groups: 8 actors
- **TOTAL: 34 actors**

**MAP_SAVED: True**

---

## Handoff to Agent #14 — Quest & Mission Designer

The crowd simulation infrastructure is complete. Agent #14 should:

1. **Stampede Quest Trigger** — Use `Combat_Brachio_StampedeZone_001` as quest trigger: "Survive the stampede"
2. **Raptor Hunt Quest** — Use `Raptor_Pack_Alpha_001` as quest target: "Drive off the raptor pack"
3. **Ecosystem Observation** — Use herbivore idle groups as quest objectives: "Observe 3 different species"
4. **Panic Event Quest** — Use `Crowd_PanicBroadcast_1200u_001` as quest trigger: "Something is coming — find high ground"
5. **Density-aware spawning** — Respect `DensityCap_*` markers when spawning quest-specific animals

**Key spatial anchors for quest design:**
- TRex territory: (3000, 1500) ± 600u
- Raptor pack: (-1500, 2800) ± 400u
- Brachio herd: (500, -3000) ± 500u
- Safe zones (no predators): (-3000, -500), (1500, 3000), (-1000, 3500)
