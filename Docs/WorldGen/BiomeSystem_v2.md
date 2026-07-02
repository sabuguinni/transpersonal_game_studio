# Biome System v2 — Transpersonal Game Studio
**Agent:** #05 — Procedural World Generator  
**Cycle:** PROD_CYCLE_AUTO_20260702_010  
**Status:** ACTIVE — 5 biomes live in MinPlayableMap

---

## Overview

The world is divided into **5 distinct biomes**, each with unique terrain, vegetation, lighting, audio triggers, and weather zones. All biome actors follow the naming convention `Type_Bioma_NNN`.

---

## Biome Definitions

### 1. Jungle Forest (Biome_Jungle)
- **Center:** (-2000, -2000, 0)
- **Terrain:** Dense undergrowth, tall canopy trees (cylinders h=2.5–5.0 scale)
- **Actors:** `Tree_Jungle_001–008`, `Biome_Jungle_Floor_001`
- **Ambient Light:** `BiomeLight_Jungle_001` — warm green (0.3, 0.8, 0.2), 1500 intensity, 1200 radius
- **Audio Zone:** `AudioZone_Jungle_001` — jungle ambience (insects, birds, rustling)
- **Weather:** `WeatherZone_Rain_001` — heavy rain, high humidity
- **Dinosaurs:** Raptors (ambush predators in dense cover)
- **Transition:** `BiomeTransition_JungleSavanna_001` at (-1000, -1000)

### 2. Savanna Plains (Biome_Savanna)
- **Center:** (0, 0, 0)
- **Terrain:** Open grassland, scattered acacia-style trees, rock outcrops
- **Actors:** `Tree_Savanna_001–006`, `Rock_Savanna_001–004`, `Biome_Savanna_Floor_001`
- **Ambient Light:** `BiomeLight_Savanna_001` — warm golden (1.0, 0.85, 0.3), 1200 intensity, 1500 radius
- **Audio Zone:** `AudioZone_Savanna_001` — wind, distant dinosaur calls
- **Weather:** Clear skies, occasional dust storms
- **Dinosaurs:** T-Rex (apex predator), Triceratops herds
- **Transitions:** `BiomeTransition_JungleSavanna_001`, `BiomeTransition_SavannaHighlands_001`, `BiomeTransition_SavannaRiver_001`

### 3. Rocky Highlands (Biome_Highlands)
- **Center:** (2000, -1500, 300)
- **Terrain:** Elevated rocky terrain, volcanic formations, steam vents
- **Actors:** `Rock_Highlands_001–006`, `Biome_Highlands_Floor_001`
- **Ambient Light:** `BiomeLight_Highlands_001` — cool grey-blue (0.6, 0.7, 0.9), 800 intensity, 1000 radius
- **Audio Zone:** `AudioZone_Highlands_001` — wind howl, distant rumbles, steam hiss
- **Weather:** `WeatherZone_Volcanic_001` — volcanic steam, ash particles
- **Dinosaurs:** Pterosaurs (cliff nesting), Ankylosaurs
- **Transition:** `BiomeTransition_SavannaHighlands_001` at (1000, -750)

### 4. River Delta (Biome_River)
- **Center:** (500, 2500, -20)
- **Terrain:** Mudflats, shallow water pools, riverbanks
- **Actors:** `WaterPool_River_001–003`, `Biome_River_Floor_001`
- **Ambient Light:** `BiomeLight_River_001` — cool blue (0.2, 0.5, 1.0), 1000 intensity, 1200 radius
- **Audio Zone:** `AudioZone_River_001` — flowing water, frogs, aquatic sounds
- **Weather:** `WeatherZone_Mist_001` — morning mist, high humidity
- **Dinosaurs:** Spinosaurus (aquatic hunter), Parasaurolophus herds
- **Transition:** `BiomeTransition_SavannaRiver_001` at (250, 1250)

### 5. Conifer Forest (Biome_ConiferForest)
- **Center:** (-2500, 500, 50)
- **Terrain:** Dense tall conifers, dark forest floor, limited visibility
- **Actors:** `Tree_ConiferForest_001–008`, `Biome_ConiferForest_Floor_001`
- **Ambient Light:** `BiomeLight_ConiferForest_001` — deep green (0.2, 0.6, 0.2), 1200 intensity, 1000 radius
- **Audio Zone:** `AudioZone_ConiferForest_001` — deep forest ambience, owl calls
- **Weather:** Overcast, fog patches
- **Dinosaurs:** Stegosaurus herds, small herbivores
- **Transition:** `BiomeTransition_JungleConifer_001` at (-2250, -750)

---

## Actor Naming Convention

All world actors follow: `Type_Bioma_NNN`

| Type Prefix | Description |
|-------------|-------------|
| `Tree_` | Vegetation — trees |
| `Rock_` | Rock formations |
| `WaterPool_` | Water bodies |
| `Biome_` | Biome floor/terrain planes |
| `BiomeLight_` | Ambient point lights per biome |
| `AudioZone_` | Audio trigger zone markers |
| `WeatherZone_` | Weather system markers |
| `BiomeTransition_` | Transition zone markers |

---

## Performance Rules (from Agent #04)

- **Biome floor planes**: tick DISABLED (static)
- **Tree/Rock actors**: tick DISABLED, cull distance 80m
- **Water pools**: tick DISABLED
- **Point lights**: attenuation radius ≤ 1500 units
- **PCG graphs**: max 10,000 points per graph
- **World Partition**: streaming cells at 5,000 vertices max

---

## Integration Points

### → Agent #06 (Environment Artist)
- Replace cylinder/sphere placeholders with real mesh assets
- Add LOD0–LOD3 chains to all vegetation
- Apply PBR materials to biome floors (grass, dirt, rock, water)
- Add berry bushes as HISM clusters in Jungle and Conifer biomes
- Add medicinal plants as static meshes near River biome

### → Agent #08 (Lighting & Atmosphere)
- Use `BiomeLight_*` actors as base for biome-specific ambient lighting
- Weather zones: `WeatherZone_Volcanic_001` → ash particle Niagara system
- Weather zones: `WeatherZone_Mist_001` → morning fog Niagara system
- Weather zones: `WeatherZone_Rain_001` → rain Niagara system

### → Agent #16 (Audio)
- `AudioZone_*` markers define spatial audio trigger volumes
- Jungle: insects + birds + rain ambience
- Savanna: wind + distant roars + grassland sounds
- Highlands: wind howl + volcanic rumble + steam hiss
- River: flowing water + frogs + splashing
- Conifer: deep forest + wind through pines + owl calls

### → Agent #12 (Combat & Enemy AI)
- Raptor spawn zones: Jungle biome (ambush terrain)
- T-Rex patrol: Savanna biome (open sight lines)
- Pterosaur nests: Highlands biome (cliff edges at z=300+)
- Spinosaurus territory: River biome (water access required)

---

## World Coordinates Summary

| Biome | Center | Radius | Elevation |
|-------|--------|--------|-----------|
| Jungle Forest | (-2000, -2000) | 800 | 0 |
| Savanna Plains | (0, 0) | 1200 | 0 |
| Rocky Highlands | (2000, -1500) | 600 | 300–600 |
| River Delta | (500, 2500) | 500 | -25 |
| Conifer Forest | (-2500, 500) | 600 | 50 |

---

*Generated by Agent #05 — Procedural World Generator*  
*Cycle: PROD_CYCLE_AUTO_20260702_010*
