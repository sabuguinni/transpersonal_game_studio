# Biome System & PCG World Generation Specification
## Agent #05 — Procedural World Generator
## Status: DEGRADED MODE — Bridge DOWN, APIs DOWN (Cycle AUTO_20260705_003)

---

## DEGRADED MODE STATUS

**UE5 Bridge**: TIMEOUT (3rd consecutive cycle — commands 28835, 28836, 28940)
**generate_image API**: 401 Unauthorized (OpenAI key invalid/expired)
**Mode**: GitHub documentation-only per Brain Memory DEGRADED MODE TOTAL protocol

---

## World Generation Architecture

### Overview
The procedural world uses UE5's PCG (Procedural Content Generation) framework combined with
World Partition for streaming. The world is structured as a 16km × 16km map divided into
biome zones, each with distinct terrain, vegetation, and fauna profiles.

### Biome Zones (Cretaceous Period)

#### 1. Coastal Lowlands (0–50m elevation)
- **Area**: Western edge, ~20% of map
- **Terrain**: Flat alluvial plains, river deltas, shallow lagoons
- **Vegetation**: Cycads, horsetails, ferns, mangrove-analogues
- **Fauna**: Mosasaurus (water), Pteranodon (sky), Edmontosaurus (herds)
- **PCG Density**: High vegetation (8–12 plants/100m²), low rock density
- **Ambient**: Humid, coastal winds, water sounds

#### 2. Lowland Forest (50–200m elevation)
- **Area**: Central-west, ~25% of map
- **Terrain**: Rolling hills, river valleys, dense canopy
- **Vegetation**: Araucaria conifers (30–40m), tree ferns, angiosperms (early flowering)
- **Fauna**: Triceratops (herds), Velociraptor (packs), Parasaurolophus
- **PCG Density**: Very high (15–20 plants/100m²), medium rock density
- **Ambient**: Bird calls, insect chorus, wind through canopy

#### 3. Savanna Plains (100–300m elevation)
- **Area**: Central, ~30% of map — PRIMARY PLAYER HUB AREA
- **Terrain**: Open grassland with scattered tree clusters, dry riverbeds
- **Vegetation**: Proto-grasses, cycad clusters, isolated conifers
- **Fauna**: T-Rex (territorial), Ankylosaurus, Gallimimus (flocks)
- **PCG Density**: Low-medium (3–5 plants/100m²), scattered boulders
- **Ambient**: Wind, distant roars, open sky

#### 4. Highland Forest (300–600m elevation)
- **Area**: Eastern ridgeline, ~15% of map
- **Terrain**: Steep slopes, cliff faces, mountain streams
- **Vegetation**: Dense conifer forest, moss-covered rocks, ferns
- **Fauna**: Pachycephalosaurus, Oviraptor nests, Pterosaurs
- **PCG Density**: High (10–15 plants/100m²), high rock density
- **Ambient**: Wind howl, stream sounds, echo effects

#### 5. Volcanic Badlands (400–800m elevation)
- **Area**: Northeast corner, ~10% of map
- **Terrain**: Lava fields (cooled), obsidian outcrops, geothermal vents
- **Vegetation**: Sparse heat-tolerant ferns, dead trees
- **Fauna**: Carnotaurus (apex predator here), Iguanodon
- **PCG Density**: Very low (0–2 plants/100m²), high rock/obsidian density
- **Ambient**: Steam vents, distant rumbles, crackling rock

---

## PCG Implementation Plan

### Phase 1: Terrain Foundation (Priority P1)
```
PCGWorldGenerator.cpp — ACTIVE (12,099 bytes)
BiomeManager.cpp — ACTIVE (6,471 bytes)
```

**Terrain Generation Steps:**
1. Base heightmap via Perlin noise (3 octaves, persistence 0.5)
2. Biome mask overlay (Voronoi-based zone assignment)
3. Erosion pass (hydraulic + thermal, 200 iterations)
4. River carving (flow simulation from highlands to coast)
5. Beach/shore smoothing at elevation 0–10m

### Phase 2: Vegetation Placement (Priority P1)
```
FoliageManager.cpp — ACTIVE (referenced in codebase)
```

**PCG Foliage Rules:**
- Slope mask: No trees on slopes > 45°
- Altitude mask: Species-specific elevation ranges
- Proximity rules: Min distance between same-species instances
- Cluster bias: Trees prefer to cluster (Poisson disk with attraction)
- River proximity: Increased density within 50m of water

### Phase 3: Dinosaur Spawning (Priority P2)
```
DinosaurBase.cpp — ACTIVE (8,164 bytes)
DinosaurAIController.cpp — ACTIVE (13,775 bytes)
```

**Spawn Rules:**
- Territory radius per species (T-Rex: 500m, Raptor: 200m, Triceratops: 150m)
- Herd size: Herbivores 3–12, Carnivores 1–3
- Day/night spawn variation: Nocturnal species more active at night
- Player proximity: Spawn/despawn at 800m radius

---

## Content Hub Composition (X=2100, Y=2400)

Per Brain Memory `hugo_hub_quality_v2_fix` — this is the PRIORITY composition:

**Required Elements at Hub Coordinates:**
1. **T-Rex** — Label: `TRex_Savana_001` — Posed at clearing center, alert stance
2. **Triceratops** — Label: `Trike_Savana_001` — 80m from T-Rex, grazing pose
3. **Raptor Pack** — Labels: `Raptor_Savana_001/002/003` — Flanking formation
4. **Dense Vegetation Ring** — 15m radius of ferns + cycads around clearing
5. **Araucaria Trees** — 4× large conifers at clearing edges (30m height)
6. **Directional Light** — Sun pitch: -45° (per CAP enforcement rule)
7. **Volumetric Fog** — Light morning mist, density 0.02

**When Bridge Recovers:**
Execute Python script to verify/create all hub actors with correct labels.
Script template ready in: `Docs/WorldGen/hub_spawn_script.py`

---

## PCG Graph Configuration

### BiomeBlendGraph (to be created in UE5 editor)
```
[Heightmap Input] → [Biome Classifier] → [Per-Biome PCG Graphs]
                                        ├── CoastalPCG
                                        ├── ForestPCG  
                                        ├── SavannaPCG ← Primary
                                        ├── HighlandPCG
                                        └── VolcanicPCG
```

### SavannaPCG Node Graph
```
[Surface Sampler] → [Slope Filter (< 30°)] → [Altitude Filter (100-300m)]
    → [Density Noise] → [Mesh Spawner: CycadMesh, FernMesh, GrassMesh]
    → [Collision Avoidance] → [Final Placement]
```

---

## River System Design

### Primary Rivers (3 total)
1. **Great River** — Highland (NE) → Coastal (W), 12km length, 30–80m wide
2. **Raptor Creek** — Central highlands → Savanna plains, 6km, 10–25m wide  
3. **Volcanic Stream** — Badlands → Forest, 4km, 5–15m wide (hot springs)

### River Implementation (UE5)
- Spline-based river actors with Water plugin
- Dynamic water material (depth-based opacity, foam at rapids)
- Riverside vegetation density boost (PCG proximity rule)
- Dinosaur pathfinding avoidance at deep sections

---

## Performance Budget (per Agent #04 constraints)

| System | Target | Budget |
|--------|--------|--------|
| Foliage instances (visible) | < 50,000 | 2ms GPU |
| PCG evaluation per frame | < 1ms | CPU |
| Biome streaming chunks | 4×4 active | 256MB |
| Dinosaur AI actors | < 30 active | 1.5ms |
| Water simulation | Disabled at > 500m | 0.5ms |

---

## Files Status

| File | Status | Size | Notes |
|------|--------|------|-------|
| PCGWorldGenerator.cpp | ACTIVE | 12,099 | Core PCG logic |
| PCGWorldGenerator.h | ACTIVE | 4,597 | Header |
| BiomeManager.cpp | ACTIVE | 6,471 | Biome classification |
| BiomeManager.h | ACTIVE | 4,675 | Header |
| BiomeSystem.h | ACTIVE | 7,757 | Shared biome types |
| PCGBiomeSystem.h | ACTIVE | 7,886 | PCG-specific types |
| SharedTypes.h | ACTIVE | 11,382 | Cross-agent types |

---

## Next Cycle Priority (When Bridge Recovers)

1. **IMMEDIATE**: Bridge validation → spawn hub actors at X=2100, Y=2400
2. **HIGH**: Apply CAP enforcement (sun pitch -45°, fog density 0.02)
3. **HIGH**: Verify TRex_Savana_001, Trike_Savana_001, Raptor_Savana_001/002/003 exist
4. **MEDIUM**: PCG foliage density pass around hub clearing
5. **LOW**: River spline placement (Great River path)

---

*Generated by Agent #05 — Procedural World Generator*
*Cycle: PROD_CYCLE_AUTO_20260705_003*
*Status: DEGRADED MODE (Bridge DOWN + APIs DOWN)*
