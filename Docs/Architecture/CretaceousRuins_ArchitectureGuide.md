# Cretaceous Ruins — Architecture & Interior Design Guide
## Agent #7 — Architecture & Interior Agent

### Overview
This document defines the architectural language for all prehistoric structures in the game world.
All structures follow the principle: **every interior is a document of the civilization that built it.**

---

## Structural Types

### Type 1: Stone Shelter Ruins (Priority: HIGH)
- **Description**: Collapsed limestone walls, partial roofs, primitive construction
- **Scale**: 4m x 6m footprint, 2-3m standing wall height
- **Materials**: Weathered limestone, moss, ferns, mud mortar
- **Interior Props**: Fire pit, animal hide bedding, stone tool cache, bone refuse pile
- **Gameplay Role**: Safe rest point, crafting station, loot location
- **Meshy Asset**: `Arch_CretaceousRuinsPillar_01` (GLB generated, task 019eeeb8)

### Type 2: Standing Stone Columns
- **Description**: Monolithic limestone pillars, 3-5m tall, partially fallen
- **Scale**: 1m diameter base, 3-5m height
- **Materials**: Rough-cut limestone, lichen, moss
- **Placement**: Biome coords X=50000, Y=50000, Z=100 (primary cluster)
- **Gameplay Role**: Navigation landmarks, shelter from rain

### Type 3: Cave Entrance Archways
- **Description**: Natural rock formations shaped into crude archways
- **Scale**: 3m wide, 2.5m tall opening
- **Materials**: Basalt, sandstone, cave moss
- **Interior**: Leads to cave system with stalactites

---

## Interior Design Principles

### The "Who Lived Here" Rule
Every interior must answer: **who lived here, and what happened to them?**

1. **Evidence of Daily Life**: Food preparation areas, sleeping areas, tool storage
2. **Evidence of Danger**: Scratch marks on walls, hastily abandoned tools, blood stains
3. **Evidence of Time**: Layers of ash in fire pits, collapsed sections, overgrown floors

### Prop Placement Guidelines
- Fire pit: Always center-left of main entrance
- Sleeping area: Furthest from entrance, against solid wall
- Tool cache: Near entrance for quick access
- Food storage: Elevated (on rocks/ledges) away from ground moisture

---

## UE5 Implementation Notes

### Spawned Actors (Cycle 007)
- `Arch_CretaceousRuinsPillar_01` — StaticMeshActor at (50000, 50000, 100)
  - Scale: (1.5, 1.5, 6.0) — tall pillar proportions
  - Mesh: Cube placeholder (replace with Meshy GLB when imported)
  - GLB URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/meshes/1782121587010_Cretaceous_era_ancient_stone_ruins_pilla.glb`

### Meshy Asset Pipeline
- Task ID: `019eeeb8-9343-74df-8803-3e5ba1fecec8`
- Status: SUCCEEDED
- Format: GLB
- Import to: `/Game/Architecture/Ruins/SM_CretaceousRuinsPillar`

### Material Setup
- Base: M_Limestone_Weathered (create from UE5 material instance)
- Overlay: M_Moss_Wet (vertex paint blend)
- Detail: Normal map from Meshy PBR textures

---

## Next Steps for Agent #8 (Lighting)
- Apply volumetric fog inside ruin interiors (density 0.08)
- Shaft lighting through collapsed roof openings
- Firepit point light: warm orange, radius 300cm, intensity 2000 lux
- Ambient occlusion boost inside enclosed spaces

## Dependencies
- Agent #6 (Environment): Jungle vegetation surrounding ruins
- Agent #8 (Lighting): Interior atmospheric lighting
- Agent #12 (Combat AI): Dinosaur patrol routes around ruins
