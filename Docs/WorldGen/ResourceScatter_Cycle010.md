# Resource Node Scatter Report — Cycle 010
**Agent #05 — Procedural World Generator**
**Date:** PROD_CYCLE_AUTO_20260618_010

---

## PCG Resource Distribution

### Strategy
Resource nodes were scattered using a PCG-style algorithm with:
- **Minimum spacing:** 500 units between nodes of any type
- **Biome-aware placement:** Each resource type weighted per biome ecology
- **Seeded randomness:** `random.seed(42)` for reproducibility
- **Collision avoidance:** Checks against all existing resource positions before placing

---

## Biome Resource Tables

| Biome | Centre (X,Y) | Radius | Berry | Flint | WaterHole | Fern_Edible |
|-------|-------------|--------|-------|-------|-----------|-------------|
| Forest   | (-2000,-2000) | 1800 | 10 | 4  | 0 | 6 |
| Savanna  | ( 2500, 1500) | 2000 | 6  | 8  | 3 | 0 |
| Highland | (    0, 3500) | 1500 | 4  | 10 | 2 | 0 |
| Swamp    | (-3000, 2500) | 1200 | 8  | 0  | 4 | 6 |
| Rocky    | ( 3000,-2000) | 1400 | 3  | 12 | 2 | 0 |

**Total target nodes:** ~88 resource nodes across 5 biomes

---

## Rock Formation Distribution

| Biome | Boulders | Cliffs | Total |
|-------|----------|--------|-------|
| Rocky    | 9 | 5 | 14 |
| Highland | 7 | 3 | 10 |

**Total rock formations:** ~24 new formations

---

## Label Convention
All spawned actors follow the mandatory label format:

```
Resource_{Type}_{Biome}_{NNN}   — resource nodes
Rock_{Type}_{Biome}_{NNN}       — rock formations
```

Examples:
- `Resource_Berry_Forest_001`
- `Resource_Flint_Rocky_012`
- `Resource_WaterHole_Swamp_003`
- `Rock_Boulder_Rocky_007`
- `Rock_Cliff_Highland_002`

---

## Scale Conventions

| Type | Scale |
|------|-------|
| WaterHole | (3.0, 3.0, 0.3) — wide, flat |
| Flint | (0.4, 0.4, 0.4) — small stone |
| Berry / Fern | (0.7, 0.7, 0.7) — bush-sized |
| Boulder | (0.8–2.5, 0.8–2.5, proportional) |
| Cliff | (1.5–3.0, 1.5–3.0, 3.0–6.0) — tall |

---

## Dependencies for Agent #06 (Environment Artist)

Agent #06 should:
1. **Replace BasicShape placeholders** on all `Resource_*` actors with proper static meshes:
   - Berry → `/Game/Assets/Vegetation/SM_BerryBush`
   - Flint → `/Game/Assets/Props/SM_FlintRock`
   - WaterHole → `/Game/Assets/Terrain/SM_WaterHole`
   - Fern_Edible → `/Game/Assets/Vegetation/SM_Fern`
2. **Replace Rock_Boulder/Cliff** actors with 3-LOD rock meshes
3. **Add particle effects** to WaterHole nodes (ripple Niagara system)
4. **Verify no overlap** with existing foliage from Cycles 007–009

---

## Performance Notes (from Agent #04 budget)
- Resource nodes are static — zero tick cost
- LOD scale 1.5 applied globally — nodes will LOD at ~3000 units
- NavMesh rebake covers all new node positions
- Total actor count remains well within 8000 CAP

---

## Map State After Cycle 010
- `MAP_SAVED:True` — `/Game/Maps/MinPlayableMap`
- All resource nodes persistent across editor sessions
- CAP_SAFE confirmed before and after scatter operations
