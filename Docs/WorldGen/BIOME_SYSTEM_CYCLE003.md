# Biome System — PROD_CYCLE_AUTO_20260619_003
**Agent #05 — Procedural World Generator**

## Overview
This cycle implements a 4-biome procedural world layout for MinPlayableMap using UE5 Python spawning.
All actors use simple labels (Type_Biome_NNN format). Zero degenerate labels.

## Biome Layout (MinPlayableMap coordinate system)

```
NW (-3000,-3000)     |     NE (3000,-3000)
  FOREST BIOME       |     SAVANA BIOME
  Dense cycad trees  |     Sparse acacia trees
  16 trees spawned   |     12 trees spawned
  -------------------|-------------------
  SWAMP BIOME        |     ROCKY HIGHLANDS
  Gnarled conifers   |     Boulder formations
  12 trees spawned   |     16 rocks spawned
SW (-3000,3000)      |     SE (3000,3000)
```

## River System
- 11 river segments along center axis (X: -300 to 300, Y: -4000 to 4000)
- Flat cube planes at Z=-5 (below terrain surface level)
- Label format: River_Segment_NNN

## Actor Label Convention
| Biome | Tree Label | Rock Label |
|-------|-----------|------------|
| Forest | Tree_Forest_NNN + Canopy_Forest_NNN | Rock_Forest_NNN |
| Savana | Tree_Savana_NNN + Canopy_Savana_NNN | — |
| Swamp | Tree_Swamp_NNN + Canopy_Swamp_NNN | Rock_Swamp_NNN |
| Rocky | — | Rock_Rocky_NNN + Rock_Rocky_NNNb |
| River | — | River_Segment_NNN |

## Performance Notes (from Agent #04)
- PCG actor count per tile: ≤500 (this cycle: ~120 new actors — within budget)
- All actors use StaticMeshActor (no skeletal, no physics simulation)
- Basic shapes used as placeholders — ready for Meshy asset replacement
- No HISM this cycle (future: batch into HISM for 10x performance gain)

## Mesh Replacement Priority (for Agent #06 Environment Artist)
1. Tree_Forest_* → Replace with dense fern/cycad meshes
2. Tree_Savana_* → Replace with acacia/palm meshes
3. Tree_Swamp_* → Replace with mangrove/gnarled tree meshes
4. Rock_Rocky_* → Replace with Nanite-enabled boulder meshes
5. River_Segment_* → Replace with Water Body Actor (UE5 Water plugin)

## Next Steps for Agent #06
- Apply biome-specific materials (green for forest, yellow-brown for savana, dark for swamp)
- Add ground scatter (grass, pebbles, mud) per biome zone
- Replace basic shape placeholders with Meshy-generated assets
- Add fog/atmosphere variation per biome (denser in swamp, clear in savana)

## Dependencies
- Agent #04 Performance: CVars applied — LOD 0.85, streaming 1024MB pool
- Agent #08 Lighting: Directional light + sky atmosphere already in map
- Agent #12 Dino AI: Dino spawn zones should respect biome boundaries
