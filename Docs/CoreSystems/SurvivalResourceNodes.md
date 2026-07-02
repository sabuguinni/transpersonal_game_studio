# Survival Resource Nodes — Core Systems #03
## Cycle: PROD_CYCLE_AUTO_20260702_011

### Overview
This document defines all survival resource nodes placed in MinPlayableMap by Agent #03 (Core Systems Programmer).
These nodes drive the four core survival stats: **Health**, **Hunger**, **Thirst**, **Stamina**.

---

## Survival Stats (TranspersonalCharacter)

| Stat | Default | Min | Max | Decay Rate | Critical Threshold |
|------|---------|-----|-----|------------|-------------------|
| Health | 100 | 0 | 100 | 0 (only from damage) | 20 |
| Hunger | 100 | 0 | 100 | 1.0/min | 15 |
| Thirst | 100 | 0 | 100 | 2.0/min | 10 |
| Stamina | 100 | 0 | 100 | 10/sprint | 20 |
| Fear | 0 | 0 | 100 | -5/min (decay) | 80 (panic) |

---

## Resource Nodes Placed in MinPlayableMap

### Flint Rock Clusters (Stone Tool Crafting)
| Actor Label | Location | Yield | Respawn |
|-------------|----------|-------|---------|
| Resource_Flint_Savana_001 | (400, 200, 0) | 3 flint shards | 300s |
| Resource_Flint_Savana_002 | (600, -300, 0) | 2 flint shards | 300s |
| Resource_Flint_Savana_003 | (-500, 400, 0) | 4 flint shards | 300s |

**Use:** Craft stone knife, hand axe, spear tip. Required for all Tier 1 weapons.

### Water Sources (Thirst)
| Actor Label | Location | Yield | Quality |
|-------------|----------|-------|---------|
| Resource_Water_Rio_001 | (1200, 800, -10) | Unlimited | Drinkable (no disease) |
| Resource_Water_Rio_002 | (1400, 600, -10) | Unlimited | Drinkable (no disease) |

**Use:** Drink to restore Thirst +40. Boiling at campfire removes disease risk.
**Note:** Water plane meshes placed at Z=-10 (below terrain level in Rio zone).

### Berry Bushes (Hunger)
| Actor Label | Location | Yield | Nutrition |
|-------------|----------|-------|-----------|
| Resource_Berries_Floresta_001 | (-300, 700, 0) | 5 berries | +8 Hunger each |
| Resource_Berries_Floresta_002 | (-500, 900, 0) | 4 berries | +8 Hunger each |
| Resource_Berries_Floresta_003 | (-200, 1100, 0) | 6 berries | +8 Hunger each |

**Use:** Eat raw for +8 Hunger. Respawn 600s. Cannot cook (no caloric benefit from cooking berries).

---

## Interaction Triggers

### Campfire Trigger
| Actor Label | Location | Radius | Effect |
|-------------|----------|--------|--------|
| Trigger_CampFire_Camp_001 | (0, 0, 50) | 200cm | Warmth +5/s, Stamina regen +20%, Fear -10/s |

**Overlap logic (Blueprint):**
- Player enters → enable warmth buff, enable rest menu
- Player exits → remove buffs
- Night time: Fear reduction doubled (-20/s)

### TRex Danger Zone
| Actor Label | Location | Radius | Effect |
|-------------|----------|--------|--------|
| DangerZone_TRex_Savana_001 | (2000, 0, 0) | 800m sphere | Fear +5/s when TRex active |

**Logic:** When TRex_Savana_001 is alive and player is inside this sphere, Fear increases.
At Fear=80 (panic), player movement speed -30%, aim accuracy -50%.

---

## Crafting System (Tier 1 — Stone Age)

### Required Resources
```
Stone Knife:    2x Flint + 1x Vine (gathered from Floresta)
Hand Axe:       3x Flint + 1x Wood Branch
Spear:          4x Flint + 2x Wood Branch + 1x Vine
Torch:          1x Wood Branch + 1x Dry Grass
Shelter Frame:  8x Wood Branch + 4x Vine
```

### Crafting Stations
- **Ground crafting**: Stone Knife, Hand Axe (no station needed)
- **Campfire**: Torch, Cooked Meat
- **Shelter Frame**: Requires Hand Axe to cut branches

---

## Navigation Waypoints (for AI pathfinding)

| Label | Location | Purpose |
|-------|----------|---------|
| NavWP_Camp_001 | (0, 0, 0) | Player spawn / home base |
| NavWP_Savana_001 | (800, 0, 0) | Savana biome entry |
| NavWP_Floresta_001 | (-400, 800, 0) | Forest biome entry |
| NavWP_Rio_001 | (1200, 600, 0) | River zone entry |
| NavWP_Cliff_001 | (0, -1200, 200) | Cliff overlook |

---

## Integration Dependencies

| System | Depends On | Status |
|--------|-----------|--------|
| SurvivalComponent | TranspersonalCharacter | ✅ Integrated |
| Resource nodes | StaticMeshActor (Engine) | ✅ Placed |
| Campfire trigger | TriggerBox (Engine) | ✅ Placed |
| Danger zone | TriggerSphere (Engine) | ✅ Placed |
| Crafting UI | Quest/Inventory system (Agent #14) | ⏳ Pending |
| Berry respawn | GameMode timer (Agent #14) | ⏳ Pending |
| Fear → AI reaction | Combat AI (Agent #12) | ⏳ Pending |

---

## Performance Budget

- Resource nodes: 8 StaticMeshActors (negligible)
- Trigger volumes: 2 (zero render cost)
- Total new actors this cycle: 10
- Draw calls added: ~8 (basic shape meshes)
- Memory: <1MB

---

*Agent #03 — Core Systems Programmer | PROD_CYCLE_AUTO_20260702_011*
