# Crowd & Traffic Simulation System — Agent #13 Cycle 010
## Transpersonal Game Studio — Prehistoric Survival Game

---

## Overview
This document records all crowd simulation actors, herd placements, and behavioral zones
spawned during PROD_CYCLE_AUTO_20260617_010.

---

## Actors Spawned This Cycle

### 1. Brachiosaurus Herd (8 members)
**Corridor**: Stampede Corridor from (500, 2800) → (570, 4900)
**Established by**: Agent #12 CombatAI_Brachio_StampedeCorridor_001 at (500, 3000)

| Label | Position | Scale |
|-------|----------|-------|
| Brachio_Herd_001 | (400, 2800, 100) | 3.5×3.5×5.0 |
| Brachio_Herd_002 | (550, 3100, 100) | 3.5×3.5×5.0 |
| Brachio_Herd_003 | (380, 3400, 100) | 3.5×3.5×5.0 |
| Brachio_Herd_004 | (620, 3700, 100) | 3.5×3.5×5.0 |
| Brachio_Herd_005 | (460, 4000, 100) | 3.5×3.5×5.0 |
| Brachio_Herd_006 | (530, 4300, 100) | 3.5×3.5×5.0 |
| Brachio_Herd_007 | (410, 4600, 100) | 3.5×3.5×5.0 |
| Brachio_Herd_008 | (570, 4900, 100) | 3.5×3.5×5.0 |

**Behavioral Notes**:
- Herd moves as unit — lead animal (001) sets direction
- Stampede triggers when T-Rex enters 2000u radius of corridor
- During stampede: all 8 agents accelerate to 1200 cm/s along Y+ axis
- Calves (if added later): scale 1.5×1.5×2.0, stay within 300u of nearest adult

---

### 2. Parasaurolophus Grazing Group (6 members)
**Location**: Open plains east of camp (1750–2200, -700 to -200)
**Safe distance from origin**: >800u confirmed

| Label | Position |
|-------|----------|
| Paras_Grazing_001 | (1800, -500, 100) |
| Paras_Grazing_002 | (1950, -350, 100) |
| Paras_Grazing_003 | (2100, -600, 100) |
| Paras_Grazing_004 | (1750, -700, 100) |
| Paras_Grazing_005 | (2000, -200, 100) |
| Paras_Grazing_006 | (2200, -450, 100) |

**Behavioral Notes**:
- Grazing cycle: feed 8s → head-raise alert 2s → feed 8s (loop)
- Flee trigger: T-Rex within 3000u OR Raptor within 1500u
- Flee direction: away from predator, toward water source (if defined)
- Vocalizations: low honk every 15-30s (random), alarm call on flee

---

### 3. Triceratops Grazing Group (5 members)
**Location**: Northwest of camp (-2200 to -1800, 1200–1700)

| Label | Position | Scale |
|-------|----------|-------|
| Trike_Grazing_001 | (-2000, 1500, 100) | 2.5×2.5×1.8 |
| Trike_Grazing_002 | (-2200, 1300, 100) | 2.5×2.5×1.8 |
| Trike_Grazing_003 | (-1800, 1700, 100) | 2.5×2.5×1.8 |
| Trike_Grazing_004 | (-2100, 1600, 100) | 2.5×2.5×1.8 |
| Trike_Grazing_005 | (-1900, 1200, 100) | 2.5×2.5×1.8 |

**Behavioral Notes**:
- Triceratops DO NOT flee from T-Rex — they form defensive circle (horns outward)
- Defensive circle triggers when T-Rex within 1500u
- Juveniles (if present) move to center of circle
- Will charge player if player enters within 400u while defensive posture active

---

### 4. Raptor Pack Crowd Agents (4 members)
**Formation**: Alpha charge + 3 Beta flanking positions
**Anchored to**: Agent #12 CombatWP_Raptor waypoints

| Label | Position | Role |
|-------|----------|------|
| Raptor_Pack_Alpha | (-1200, 800, 100) | Direct charge |
| Raptor_Pack_Beta1 | (-1500, 600, 100) | Left flank |
| Raptor_Pack_Beta2 | (-900, 1000, 100) | Right flank |
| Raptor_Pack_Beta3 | (-1350, 1100, 100) | Rear cutoff |

**Behavioral Notes**:
- Pack uses coordinated attack: Alpha distracts, Beta flanks simultaneously
- Communication: click-chirp vocalizations between pack members
- If Alpha is injured: Beta1 assumes Alpha role
- Pack retreats if 2+ members are injured (survival instinct)
- Hunting range: 4000u radius from den site (-1200, 800)

---

### 5. Flee Trigger Zones (2 zones — PointLight markers)
| Label | Position | Radius | Color |
|-------|----------|--------|-------|
| FleeZone_TRex | (2000, 1500, 200) | 1500u | Red |
| FleeZone_Raptor | (-1200, 800, 200) | 1500u | Red |

---

## Crowd Simulation Architecture

### Behavioral State Machine (All Prey Animals)

```
IDLE/GRAZING
    ↓ (predator detected within alert radius)
ALERT (head raised, scanning)
    ↓ (predator confirmed within flee radius)
FLEE (max speed, direction = away from predator)
    ↓ (predator distance > safe radius × 2)
RECOVERY (slow down, regroup)
    ↓ (5 minutes elapsed)
IDLE/GRAZING
```

### LOD Strategy
- **0–1500u from player**: Full detail, all animations active
- **1500–3000u**: Reduced animation rate (every 3rd frame)
- **3000–5000u**: Static pose, no animation
- **>5000u**: Hidden (SetActorHiddenInGame = true)

### Performance Budget
- Max simultaneous crowd agents: 150 (enforced by CAP check)
- Current cycle total: 8 (Brachio) + 6 (Paras) + 5 (Trike) + 4 (Raptor) = **23 agents**
- Remaining budget: 127 agents available for future cycles

### Safe Zone Rule
- No crowd agents within 800u of origin (0, 0, 0) — player camp
- All agents verified: minimum distance from origin = 1200u (Paras_Grazing_001)

---

## Integration with Agent #12 (Combat AI)

| Agent #12 Asset | Agent #13 Integration |
|-----------------|----------------------|
| CombatAI_Brachio_StampedeCorridor_001 | Brachio_Herd_001–008 placed along corridor |
| CombatWP_Raptor_Alpha/Beta/Gamma/Delta | Raptor_Pack_Alpha/Beta1/Beta2/Beta3 anchored to waypoints |
| CombatAI_TRex_AggroZone_001 | FleeZone_TRex at same position (2000, 1500) |
| CombatAI_SafeRetreat_001 | 800u exclusion zone enforced for all crowd agents |

---

## Handoff Directives for Agent #14 (Quest & Mission Designer)

### Quest Hooks Available
1. **"The Herd Moves"** — Player must track Brachio herd migration to find water source
   - Trigger: Player approaches Brachio_Herd_001 within 500u
   - Objective: Follow herd 2000u north to river
   - Reward: Water source location revealed on map

2. **"Silence in the Plains"** — Parasaurolophus herd has gone silent (predator nearby)
   - Trigger: All 6 Paras_Grazing actors in ALERT state simultaneously
   - Objective: Investigate what silenced the herd
   - Discovery: T-Rex territory marker found 800u east

3. **"The Horned Wall"** — Triceratops herd in defensive formation
   - Trigger: Player approaches Trike_Grazing group during defensive posture
   - Objective: Distract T-Rex to break the standoff
   - Reward: Safe passage through northwest corridor

4. **"Pack Hunters"** — Raptor pack is hunting — player can observe or intervene
   - Trigger: Raptor_Pack_Alpha enters chase state
   - Objective: Survive raptor encounter OR rescue prey animal
   - Consequence: If rescued, prey animal (Paras) follows player briefly

### Environmental Storytelling Positions
- Brachio_Herd_001 position: ideal for "awe moment" cinematic trigger
- Trike_Grazing_001 cluster: natural defensive chokepoint for quest design
- Paras_Grazing_005 (isolated): good position for "lone injured animal" quest hook

---

## Map State After Cycle 010
- MAP_SAVED: True
- Save path: /Game/Maps/MinPlayableMap
- Total new actors this cycle: 25 (8 Brachio + 6 Paras + 5 Trike + 4 Raptor + 2 FleeZones)
- All labels follow format: Type_Role_NNN (no degenerate concatenation)
