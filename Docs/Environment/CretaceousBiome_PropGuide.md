# Cretaceous Biome — Environment Prop Guide
## Agent #6 Environment Artist — Production Reference

### Overview
This document defines the prop placement strategy, naming conventions, and asset pipeline
for the Cretaceous biome environment in the Transpersonal Game Studio dinosaur survival game.

---

## Naming Convention
All environment props follow the format: `Type_Biome_NNN`

| Type | Example Label | Description |
|------|--------------|-------------|
| Rock | `Rock_Cretaceous_001` | Boulder/rock formation |
| FallenLog | `FallenLog_Cretaceous_001` | Decayed ancient log |
| Tree | `Tree_Cretaceous_001` | Cycad or conifer |
| Fern | `Fern_Cretaceous_001` | Giant ground fern |
| Bush | `Bush_Cretaceous_001` | Undergrowth shrub |

**RULE:** Labels MUST be ≤ 40 characters. Never append system names to labels.

---

## Biome Prop Zones (MinPlayableMap coords)

### Zone A — Jungle Core (X=48000–52000, Y=48000–52000)
- Dense cycad tree clusters (spacing: 800–1200 UU)
- Giant fern undergrowth (spacing: 300–600 UU)
- Fallen log obstacles (1 per 5000 sq UU)
- Moss-covered boulder formations (clusters of 3–5)

### Zone B — River Edge (X=45000–48000, Y=50000–54000)
- Horsetail plant clusters near water
- Exposed sandstone rocks (flat, water-worn)
- Driftwood logs along bank
- Mudflat patches (no vegetation)

### Zone C — Open Savanna (X=52000–58000, Y=48000–54000)
- Sparse cycad trees (spacing: 2000–3000 UU)
- Dry grass patches (Niagara grass system)
- Isolated boulder outcrops
- Dinosaur trail paths (worn ground texture)

---

## Meshy Asset Pipeline

### Fallen Log (COMPLETED — Cycle 012)
- **Meshy Task ID:** `019edc63-90ab-7fa5-bb05-4956028e3c92`
- **GLB URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/meshes/1781814021746_Large_prehistoric_Cretaceous_fallen_log_.glb`
- **Status:** SUCCEEDED
- **Import Path:** `/Game/Environment/Props/FallenLog_Cretaceous`
- **Poly Count:** ~8000 triangles (preview quality)
- **Notes:** Replace cylinder placeholders (FallenLog_Cretaceous_001–003) with this mesh

### Pending Assets (to request next cycles)
1. `cretaceous_boulder_rock` — Large sandstone boulder with moss
2. `cycad_tree_prehistoric` — 6m tall cycad palm, game-ready
3. `giant_fern_cretaceous` — 2m tall prehistoric ground fern
4. `horsetail_plant_cluster` — Water-edge vegetation cluster
5. `mudflat_terrain_patch` — Flat mud ground decal/mesh

---

## Placement Rules

### DO:
- Cluster rocks in groups of 3–7 (natural geological formations)
- Place fallen logs at angles (15–45 degrees to terrain)
- Use scale variation: rocks 1.2–2.8x, trees 0.8–1.4x
- Orient fallen logs along prevailing wind direction (Y axis)
- Add moss/fern props near logs and north-facing rocks

### DON'T:
- Place props in perfect grids or rows
- Use identical scale for repeated props
- Place vegetation in water areas
- Overlap collision volumes of large props
- Spawn more than 500 props per zone (performance limit)

---

## Lighting Integration Notes (for Agent #8)
- Golden hour sun angle: 15–25 degrees elevation, 270 degrees azimuth
- Volumetric fog density: 0.02–0.05 in jungle zones
- Ambient occlusion boost near rock clusters: +0.15
- God ray intensity through canopy: 0.3–0.6 (Lumen)

---

## Current Map Status (Cycle 012)
- Rocks spawned: 5 (Rock_Cretaceous_001–005)
- Fallen logs spawned: 3 (FallenLog_Cretaceous_001–003)
- All props at biome coords X=47500–51800, Y=48800–52000
- Map saved: `/Game/Maps/MinPlayableMap`

---

*Last updated: Cycle PROD_CYCLE_AUTO_20260618_012 — Agent #6 Environment Artist*
