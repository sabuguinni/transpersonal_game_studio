# Cretaceous Ruins — Placement Guide
**Agent #7 — Architecture & Interior | Cycle PROD_CYCLE_AUTO_20260629_011**

## Overview
This document describes the Cretaceous stone ruin system placed at biome center (X=50000, Y=50000, Z=100).
The ruins serve as navigation landmarks, shelter anchors, and environmental storytelling elements.

## Generated Assets

### Standing Limestone Pillar
- **Meshy Task ID:** `019f142d-2e1a-7891-b12d-4162d66ef159`
- **GLB URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/meshes/1782749965557_Cretaceous_era_prehistoric_stone_ruin_an.glb`
- **Polycount:** 8,000 triangles
- **Style:** Realistic PBR, moss-covered limestone, 3m tall

### Import into UE5
```python
import unreal

# Import GLB into Content Browser
task = unreal.AssetImportTask()
task.filename = "/path/to/downloaded/CretaceousPillar.glb"
task.destination_path = "/Game/Architecture/Ruins"
task.destination_name = "SM_CretaceousPillar_Standing"
task.replace_existing = True
task.automated = True
task.save = True

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks([task])
```

## Placed Actors in MinPlayableMap

| Label | Type | Location | Purpose |
|-------|------|----------|---------|
| Arch_CretaceousPillar_01 | StaticMeshActor | (50000, 50000, 100) | Primary landmark pillar |
| Arch_CretaceousPillar_02 | StaticMeshActor | (50200, 49800, 100) | Secondary pillar, 30° rotated |
| Arch_CretaceousPillar_03 | StaticMeshActor | (49800, 50200, 100) | Tertiary pillar, 60° rotated |
| Arch_RuinTorch_01 | PointLight | (50000, 50000, 300) | Warm amber ruin atmosphere |
| Arch_RuinTorch_02 | PointLight | (50200, 49900, 280) | Secondary torch glow |
| Arch_RuinAmbient_01 | PointLight | (49850, 50150, 320) | Ambient golden light |
| Arch_CaveEntrance_Placeholder | StaticMeshActor | (50000, 49600, 100) | Cave/shelter entrance arch |

## Design Intent
These ruins tell a story without words:
- **Who built them?** Unknown — could be natural limestone formations or evidence of a lost pre-human civilization
- **What happened here?** The pillars stand in a rough circle, suggesting a gathering place or territorial marker
- **What does the player find?** Shelter from predators, crafting materials (stone blocks), and navigation reference

## Narrative Rules (Anti-Hallucination Compliance)
- NO spiritual/sacred meaning — these are geological or primitive construction features
- YES to practical survival use: shelter, stone resource nodes, predator sight-line blockers
- YES to environmental storytelling: claw marks on stone, bones near base, old fire pit remnants

## Next Steps for Agent #8 (Lighting)
1. Add volumetric fog inside the cave entrance area (Arch_CaveEntrance_Placeholder)
2. Create a subtle god-ray light shaft between the 3 pillars
3. Ensure the ruin area has slightly darker ambient than surrounding jungle (shadow from pillars)
