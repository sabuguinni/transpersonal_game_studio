# Survival Trigger System — Core Systems Programmer #03
## Cycle: PROD_CYCLE_AUTO_20260617_007

## Overview
Five survival trigger volumes deployed in MinPlayableMap. Each TriggerBox represents a gameplay interaction zone that modifies the player's survival stats when entered.

## Trigger Volumes Deployed

| Actor Label | Location (UU) | Scale | Stat Effect |
|---|---|---|---|
| `SurvivalTrigger_Water_001` | (-3000, 1500, 100) | 5×5×2 | Thirst +40 |
| `SurvivalTrigger_Food_001` | (-1500, 3000, 100) | 4×4×2 | Hunger +30 |
| `SurvivalTrigger_Fear_TRex_001` | (5000, 0, 100) | 15×15×5 | Fear +60 |
| `SurvivalTrigger_Shelter_001` | (0, -2000, 100) | 6×6×2 | Stamina +25 |
| `SurvivalTrigger_Swamp_Cold_001` | (-4000, -3000, 80) | 20×20×3 | Temperature -20 |

## Debug Markers (TextRenderActor)

| Actor Label | Location | Text |
|---|---|---|
| `SurvivalMarker_Water` | (-3000, 1500, 300) | [WATER] Thirst +40 |
| `SurvivalMarker_Food` | (-1500, 3000, 300) | [FOOD] Hunger +30 |
| `SurvivalMarker_Fear` | (5000, 0, 500) | [DANGER] Fear +60 |
| `SurvivalMarker_Shelter` | (0, -2000, 300) | [SHELTER] Stamina +25 |
| `SurvivalMarker_Swamp` | (-4000, -3000, 300) | [SWAMP] Temp -20 |

## Survival Stats (from TranspersonalCharacter)
- **Health** — depletes from combat/temperature extremes
- **Hunger** — decays over time; replenished at Food zones
- **Thirst** — decays faster than hunger; replenished at Water zones
- **Stamina** — depletes from sprinting/combat; recovers at Shelter
- **Fear** — increases near TRex/DangerZone; reduces decision-making

## Gameplay Loop
```
PlayerStart
    ↓ (hunger/thirst decay begins)
Water Zone (-3000, 1500) — thirst replenish
Food Zone (-1500, 3000) — hunger replenish
    ↓ (explore outward)
TRex DangerZone (5000, 0) — fear spike, avoid or flee
Swamp Zone (-4000, -3000) — temperature hazard, avoid or equip
    ↓ (return to safety)
Shelter Zone (0, -2000) — stamina recovery, campfire warmth
    ↑ (loop repeats with increasing difficulty)
```

## Next Steps for Agent #04 (Performance Optimizer)
- Verify trigger volumes do not overlap NavMesh bounds
- Confirm TriggerBox extents do not cause physics overhead
- Audit TextRenderActor draw calls (5 actors, minimal cost)
- Recommend LOD distance for debug markers (disable at >5000 UU)

## Implementation Notes
- All triggers use `unreal.TriggerBox` (built-in UE5 class, no C++ required)
- Labels follow `SurvivalTrigger_Type_NNN` convention (no degenerate concatenation)
- Map saved to `/Game/Maps/MinPlayableMap` after all spawns
- Zero degenerate labels confirmed in final audit
