# Survival System Core Specification v1.0
**Agent:** #03 — Core Systems Programmer  
**Cycle:** PROD_CYCLE_AUTO_20260702_010  
**Status:** Scene markers placed, tick logic spec complete  

---

## Overview

The Survival System governs the player's biological needs in the prehistoric world. Every second the player exists, they consume resources. Failure to replenish them leads to debuffs, then death.

This system is **P3 priority** per the GDD rewrite priorities. It builds on `TranspersonalCharacter` which already exposes: `Health`, `Hunger`, `Thirst`, `Stamina`, `Fear`.

---

## Survival Stats

| Stat     | Range  | Drain Rate (per second) | Critical Threshold | Effect at Critical |
|----------|--------|------------------------|--------------------|--------------------|
| Health   | 0–100  | 0 (only damaged)       | ≤ 20               | Death at 0         |
| Hunger   | 0–100  | 0.5 / sec              | ≤ 15               | Health drain 1/sec |
| Thirst   | 0–100  | 0.8 / sec              | ≤ 10               | Health drain 2/sec |
| Stamina  | 0–100  | 5.0 / sec (sprinting)  | ≤ 5                | Cannot sprint      |
| Fear     | 0–100  | +10 near predator      | ≥ 80               | Stamina drain 2x   |

---

## Tick Logic (Target Implementation)

```
Every 1.0 seconds (server tick):
  Hunger -= 0.5
  Thirst -= 0.8
  
  If Hunger <= 15:
    Health -= 1.0
    
  If Thirst <= 10:
    Health -= 2.0
    
  If sprinting:
    Stamina -= 5.0
  Else:
    Stamina += 2.0 (regen)
    
  Fear = clamp(Fear, 0, 100)
  If Fear >= 80:
    Stamina drain *= 2
    
  If Health <= 0:
    TriggerPlayerDeath()
```

---

## Scene Markers (Placed in MinPlayableMap)

The following actors were placed in `MinPlayableMap` as visual anchors for the survival system:

| Actor Label                        | Position        | Purpose                          |
|------------------------------------|-----------------|----------------------------------|
| `SurvivalManager_Core_001`         | (0, 0, 120)     | System manager marker (sphere)   |
| `SurvivalStat_Health_Indicator`    | (80, 0, 50)     | Health stat visual anchor        |
| `SurvivalStat_Hunger_Indicator`    | (0, 80, 50)     | Hunger stat visual anchor        |
| `SurvivalStat_Thirst_Indicator`    | (-80, 0, 50)    | Thirst stat visual anchor        |
| `SurvivalStat_Stamina_Indicator`   | (0, -80, 50)    | Stamina stat visual anchor       |
| `SurvivalStat_Fear_Indicator`      | (0, 0, 150)     | Fear stat visual anchor          |
| `Raptor_Savana_004`                | (600, 200, 50)  | Predator threat (fear source)    |
| `Raptor_Savana_005`                | (700, -300, 50) | Predator threat (fear source)    |
| `Raptor_Savana_006`                | (-500, 400, 50) | Predator threat (fear source)    |

---

## Integration Points

### With TranspersonalCharacter
- Properties already exist: `Health`, `Hunger`, `Thirst`, `Stamina`, `Fear`
- Tick logic should be added to `TranspersonalCharacter::Tick()` or a dedicated `USurvivalComponent`
- Blueprint override recommended for UI feedback

### With DinosaurAI (#12)
- Dinosaur proximity triggers `Fear += 10` per second within 500 units
- T-Rex proximity (< 300 units): `Fear += 25` per second
- Raptor pack (3+ within 800 units): `Fear += 15` per second

### With World Generation (#05)
- Water sources (rivers, lakes) allow `Thirst` replenishment
- Berry bushes, prey carcasses allow `Hunger` replenishment
- Shelter reduces `Fear` decay rate by 50%

### With Combat (#04 / #12)
- Taking damage: `Health -= damage_value`
- Killing prey: `Hunger += 30` (after crafting/cooking)
- Successful escape from predator: `Fear -= 20`

---

## Resource Nodes (To Be Placed by Agent #06)

| Resource Type     | Replenishes | Amount | Respawn Time |
|-------------------|-------------|--------|--------------|
| Water source      | Thirst      | +40    | Permanent    |
| Berry bush        | Hunger      | +15    | 5 minutes    |
| Prey carcass      | Hunger      | +50    | One-time     |
| Fire/shelter      | Fear        | -30    | Permanent    |
| Medicinal plant   | Health      | +20    | 10 minutes   |

---

## Next Steps for Agent #04 (Performance Optimizer)

1. Survival tick should run at **1Hz** (not per-frame) — use `FTimerManager::SetTimer` with 1.0s interval
2. Fear calculation uses distance checks — cache dinosaur positions, update every 0.5s not every frame
3. UI update for survival stats: throttle to 10Hz (every 0.1s) — no need for per-frame HUD refresh
4. LOD for survival: players > 5000 units from each other use simplified tick (multiplayer consideration)

---

*Binding specification for agents #04, #06, #09, #12, #14*
