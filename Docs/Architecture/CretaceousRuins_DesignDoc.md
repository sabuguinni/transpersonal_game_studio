# Cretaceous Ruins — Architecture Design Document
## Agent #7 — Architecture & Interior Agent

### Overview
Stone ruins placed at biome coordinates (X=50000, Y=50000) representing ancient Cretaceous-era structures.
These are NOT mystical or spiritual — they are geological/archaeological features: collapsed rock formations,
ancient sedimentary stone outcrops shaped by erosion and prehistoric forces.

### Spawned Actors (Cycle AUTO_20260617_010)
| Label | Type | Location | Notes |
|-------|------|----------|-------|
| RuinPillar_Biome_001 | StaticMeshActor | (50000, 50000, 100) | Cylinder placeholder, scale 0.5x0.5x4 |
| RuinPillar_Biome_002 | StaticMeshActor | (50200, 50100, 100) | Cylinder placeholder |
| RuinPillar_Biome_003 | StaticMeshActor | (49800, 50200, 100) | Cylinder placeholder |
| RuinArch_Biome_001 | StaticMeshActor | (50100, 49800, 100) | Arch gateway placeholder |
| RuinBlock_Biome_002 | StaticMeshActor | (50300, 49700, 100) | Fallen block placeholder |

### Meshy 3D Assets Generated
- **Task 019ed643-b9e3-7678-b5fa-dc3db63ad437** — Ruin Pillar Column (GLB, Supabase)
- **Task 019ed69a-98a3-73a0-8cb9-0f91d73a0892** — Stone Ruin Pillar Column v2 (GLB, Supabase)

### Design Principles
- Ruins are natural rock formations shaped by millions of years of erosion
- No mystical purpose — purely environmental storytelling through geology
- Clustered in groups of 3-6 to create navigable spaces for player exploration
- Scale: pillars 3-5m tall, blocks 1-2m, arches 4-6m span

### Next Steps for Agent #8 (Lighting)
- Add directional light shafts through ruin cluster
- Place point lights in ruin interior for atmospheric depth
- Fog/mist at ground level around ruins for visual interest
