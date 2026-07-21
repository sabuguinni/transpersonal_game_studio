# Cretaceous Ruins — Architecture & Interior Design Specification
## Agent #7 — Architecture & Interior Agent
## Cycle: PROD_CYCLE_AUTO_20260617_005

---

## Overview

This document defines the architectural language, placement rules, and interior storytelling
for all Cretaceous-period ruins and structural remnants in the prehistoric survival game.

Every structure is a **document of the civilization that built it** — and of the 65+ million
years of geological and biological forces that have acted upon it since.

---

## Structural Typology

### Type 1: Standing Pillars (Ruin_Pillar_NNN)
- **Origin**: Limestone columns from early hominid ceremonial sites
- **Height**: 3–8 meters, heavily eroded
- **Material**: Weathered limestone with lichen patches, moss at base
- **Placement**: Clusters of 3–7, never perfectly aligned (seismic drift)
- **Biome**: Savanna edges, jungle clearings
- **Biome Coords**: Primary cluster at X=50000, Y=50000 (spawned this cycle)

### Type 2: Rock Shelters (Shelter_Rock_NNN)
- **Origin**: Natural overhangs modified by early inhabitants
- **Height**: 2–4 meters clearance
- **Material**: Granite/sandstone, fire-blackened ceiling
- **Interior tells**: Ash deposits, bone fragments, crude tool marks on walls
- **Placement**: Cliff faces, river gorge walls
- **Biome**: Forest/Canyon transition zones

### Type 3: Stone Cairns (Cairn_Stone_NNN)
- **Origin**: Navigation markers or burial mounds
- **Height**: 1–3 meters
- **Material**: Stacked river stones, no mortar
- **Placement**: Ridge lines, river crossings, biome boundaries
- **Spawned**: Previous cycles (Cairn_Stone_01 through Cairn_Stone_05)

### Type 4: Cave Entrances (Cave_Entrance_NNN)
- **Origin**: Natural karst formations
- **Height**: 2–6 meters
- **Material**: Limestone with stalactite formations
- **Interior tells**: Darkness gradient, sound reverb zone trigger
- **Placement**: Hillsides, cliff bases
- **Spawned**: Previous cycles (Cave_Entrance_001)

---

## Meshy 3D Asset Pipeline

### Cycle 005 Asset Generated
- **Asset Name**: Cretaceous Stone Ruin Pillar
- **Meshy Task ID**: 019ed528-8d8a-7480-ad49-19f51e2e491b
- **GLB URL**: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/meshes/1781692703914_Cretaceous_period_ancient_stone_ruin_pil.glb
- **Thumbnail**: https://assets.meshy.ai/7e457a17-614c-4e9e-9aad-9b17c2ddf7a3/tasks/019ed528-8d8a-7480-ad49-19f51e2e491b/output/preview.png
- **Polycount**: ~8000 triangles (game-ready)
- **Status**: SUCCEEDED — ready for UE5 import

### Import Instructions (UE5 Python)
```python
import unreal

# Import GLB via AssetTools
task = unreal.AssetImportTask()
task.filename = "/path/to/downloaded/ruin_pillar.glb"
task.destination_path = "/Game/Architecture/Ruins"
task.destination_name = "SM_RuinPillar_Cretaceous"
task.replace_existing = True
task.automated = True
task.save = True

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks([task])
```

---

## Interior Storytelling Rules

### The Empty Interior Rule
**An empty interior is a lie.** Every enclosed space must answer:
1. Who used this space?
2. What did they need?
3. What happened to them?

### Evidence Layers (by time depth)
| Layer | Age | Evidence Type |
|-------|-----|---------------|
| Surface | Recent | Dinosaur tracks, fresh vegetation growth |
| Shallow | 100-1000 years | Collapsed roof sections, root intrusion |
| Mid | 10k-100k years | Sediment fill, buried artifacts |
| Deep | 1M+ years | Fossilized remains, mineral replacement |

### Interior Props Checklist (per shelter)
- [ ] Fire pit (ash ring, charred stones)
- [ ] Sleeping area (compressed earth, bone fragments)
- [ ] Tool cache (crude stone tools, antler picks)
- [ ] Food storage (pit with animal bones)
- [ ] Defensive position (sight lines to entrance)
- [ ] Escape route (secondary exit or crawl space)

---

## Placement Rules

### Label Format
```
{Type}_{Biome}_{NNN}
Examples:
  Ruin_Pillar_001    (pillar in default biome)
  Shelter_Forest_003 (rock shelter in forest)
  Cairn_Savanna_007  (cairn in savanna)
  Cave_Canyon_002    (cave entrance in canyon)
```

### Density Rules
- Max 1 major ruin cluster per 500m radius
- Max 3 cairns per 200m radius
- Cave entrances: minimum 300m apart
- No two identical structures within visual range

### Biome Affinity
| Structure Type | Primary Biome | Secondary Biome |
|----------------|---------------|-----------------|
| Pillars | Savanna | Jungle clearing |
| Rock Shelters | Forest | Canyon |
| Cairns | Ridge/Highland | River crossing |
| Cave Entrances | Canyon | Hillside |

---

## Spawned Structures This Cycle

### Ruin Pillar Cluster — X=50000, Y=50000
Spawned 5 pillar actors in MinPlayableMap:
- Ruin_Pillar_001 at (50000, 50000, 100) — scale (0.8, 0.8, 3.5)
- Ruin_Pillar_002 at (50300, 50150, 100) — scale (0.8, 0.8, 3.2)
- Ruin_Pillar_003 at (49700, 50200, 100) — scale (0.8, 0.8, 2.9)
- Ruin_Pillar_004 at (50100, 49800, 100) — scale (0.8, 0.8, 2.6)
- Ruin_Pillar_005 at (50400, 49900, 100) — scale (0.8, 0.8, 2.3)

Natural height variation simulates progressive collapse from tallest to shortest.
Rotation varied by 37° increments for organic, non-aligned appearance.

---

## Dependencies

### From Agent #6 (Environment Artist)
- Vegetation placement around ruins (ferns, vines, moss patches)
- Rock scatter around pillar bases
- Terrain height data for cave entrance placement

### To Agent #8 (Lighting & Atmosphere)
- Ruin cluster at X=50000, Y=50000 needs:
  - Ambient occlusion in pillar gaps
  - Dappled light through canopy (if jungle biome)
  - Point light inside cave entrances (bioluminescent moss)
  - Shadow casting from tall pillars (dramatic long shadows at dawn/dusk)

---

## Next Cycle Priorities

1. **Import Meshy GLB** — Replace StaticMeshActor placeholders with actual ruin pillar mesh
2. **Interior Props** — Spawn fire pit, tool cache, bone fragments inside Cave_Entrance_001
3. **River Crossing Cairns** — Place 3 cairns at river crossing points identified by Agent #5
4. **Material Assignment** — Apply weathered limestone material to all ruin actors
5. **Collision Setup** — Ensure pillars have proper collision for player navigation

---
*Architecture & Interior Agent #7 | Transpersonal Game Studio*
*"A structure is a frozen moment of human decision-making." — Stewart Brand*
