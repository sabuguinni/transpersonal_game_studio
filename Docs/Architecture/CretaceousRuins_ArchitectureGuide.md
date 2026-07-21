# Cretaceous Ruins — Architecture & Interior Guide
## Agent #7 — Architecture & Interior Agent | PROD_CYCLE_AUTO_20260623_002

---

## Overview
This document defines the architectural language for all prehistoric structures in the game world.
All ruins follow the principle: **every structure is a document of the civilization that built it.**

---

## Asset Pipeline — Cycle 002 Deliverables

### 3D Asset Generated (Meshy)
- **Asset**: `Cretaceous_era_ancient_stone_ruin_pillar.glb`
- **URL**: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/meshes/1782179094169_Cretaceous_era_ancient_stone_ruin_pillar.glb`
- **Meshy Task ID**: `019ef226-4b8c-7796-822a-a4a959186947`
- **Polycount**: ~8,000 triangles (game-ready)
- **Style**: Realistic, weathered granite with moss/lichen, PBR textures
- **Import Path**: `/Game/Assets/Architecture/Ruins/RuinPillar_01`

### UE5 Actors Spawned (Biome Coords)
Three `RuinPillar_XX` StaticMeshActors placed at:
| Label | X | Y | Z |
|-------|---|---|---|
| RuinPillar_01 | 50000 | 50000 | 100 |
| RuinPillar_02 | 50300 | 50100 | 100 |
| RuinPillar_03 | 49800 | 50200 | 100 |

Scale: (0.5, 0.5, 3.0) — tall column proportions.
Placeholder mesh: `/Engine/BasicShapes/Cylinder` — to be replaced with Meshy GLB once imported.

---

## Architectural Language — Cretaceous Ruins

### Design Principles
1. **Layered Time** — Structures show multiple eras of decay: fresh collapse, overgrowth, deep burial.
2. **Inhabited Silence** — Every interior tells a story of who lived there and what happened to them.
3. **Geological Honesty** — Materials match the local biome geology (granite in highlands, limestone in coastal areas).

### Structure Types
| Type | Description | Scale |
|------|-------------|-------|
| Standing Pillar | Single column, 2-4m tall | Small prop |
| Archway Ruin | Partial arch, 3-5m span | Medium prop |
| Collapsed Chamber | Roofless room, 6-10m across | Large structure |
| Buried Foundation | Stone blocks at ground level | Terrain feature |
| Cave Shrine | Natural cave with carved walls | Interior space |

### Material Palette
- **Primary**: Weathered granite (grey-brown, rough surface)
- **Secondary**: Limestone (cream-white, smoother, coastal biomes)
- **Accent**: Dark basalt (volcanic biomes, near lava fields)
- **Organic overlay**: Moss (green), lichen (grey-orange), ferns (bright green)
- **Decay indicators**: Cracks, spalling, root intrusion, water staining

### Interior Storytelling Elements
Each ruin interior must contain at least 3 of:
- Scattered animal bones (prey cached by predators)
- Primitive tool fragments (stone flakes, bone needles)
- Ash deposits (fire pits, evidence of shelter use)
- Claw marks on walls (territorial marking by large theropods)
- Water channels (evidence of deliberate drainage engineering)
- Collapsed roof sections (showing construction technique)

---

## Import Instructions for GLB Asset

```python
import unreal

# Import the Meshy GLB into UE5 Content Browser
glb_url = "https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/meshes/1782179094169_Cretaceous_era_ancient_stone_ruin_pillar.glb"
destination = "/Game/Assets/Architecture/Ruins/"

# Use AssetTools import pipeline
task = unreal.AssetImportTask()
task.filename = glb_url  # or local path after download
task.destination_path = destination
task.destination_name = "SM_RuinPillar_01"
task.replace_existing = True
task.automated = True
task.save = True

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks([task])

# Replace placeholder cylinders with real mesh
import unreal
actors = unreal.EditorLevelLibrary.get_all_level_actors()
real_mesh = unreal.load_object(None, '/Game/Assets/Architecture/Ruins/SM_RuinPillar_01')
for a in actors:
    if "RuinPillar_" in a.get_actor_label():
        smc = a.get_component_by_class(unreal.StaticMeshComponent)
        if smc and real_mesh:
            smc.set_static_mesh(real_mesh)
            print(f"REPLACED:{a.get_actor_label()}")
```

---

## Next Cycle Priorities

1. **Import GLB** — Run import script above once Meshy pipeline copies file to UE5 Content dir
2. **Ruin Cluster** — Add 2 more pillar variants (broken top, half-buried) for visual variety
3. **Archway Asset** — Request meshy_generate for collapsed stone archway (next mandatory asset)
4. **Interior Props** — Scatter bone piles, ash deposits, stone tool fragments around ruin base
5. **Material Override** — Apply moss/lichen material to pillar meshes for biome integration

---

## Handoff to Agent #8 (Lighting & Atmosphere)
- Ruin pillars at (50000, 50000) need atmospheric lighting: god rays through gaps, ambient occlusion in crevices
- Suggest placing a local ExponentialHeightFog volume with higher density around ruin cluster
- Ruin interiors benefit from `r.AmbientOcclusion.Intensity 1.5` for depth
