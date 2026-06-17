# Biome System Architecture v1 — Engine Architect
**Date**: 17 Jun 2026  
**Cycle**: PROD_CYCLE_AUTO_20260617_003  
**Agent**: #2 Engine Architect

---

## OVERVIEW

The biome system architecture defines the spatial organization of the game world into 5 distinct ecological zones, each with specific terrain characteristics, vegetation density, and dinosaur AI behavior parameters.

---

## BIOME ZONES

### 1. Savanna Zone (North)
- **Location**: (2000, 0, 0)
- **Type**: Grassland
- **Elevation Target**: +150 units
- **Radius**: 1500 units
- **Vegetation Density**: Low (15 assets target)
- **Vegetation Types**: Grass, Scattered Trees
- **Dinosaur Territory**: T-Rex (Territorial, Aggro Radius 2000)

### 2. Forest Zone (East)
- **Location**: (0, 2000, 50)
- **Type**: Dense Forest
- **Elevation Target**: +100 units
- **Radius**: 1200 units
- **Vegetation Density**: High (50 assets target)
- **Vegetation Types**: Trees, Ferns, Undergrowth
- **Dinosaur Territory**: Velociraptors (Pack Hunter, Aggro Radius 1500)

### 3. Wetland Zone (South)
- **Location**: (0, -2000, -100)
- **Type**: Swamp
- **Elevation Target**: -150 units
- **Radius**: 1800 units
- **Vegetation Density**: Medium (30 assets target)
- **Vegetation Types**: Reeds, Palms, Moss
- **Dinosaur Territory**: Parasaurolophus (Herd, Aggro Radius 800)

### 4. Rocky Zone (West)
- **Location**: (-1500, 500, 200)
- **Type**: Mountains
- **Elevation Target**: +300 units
- **Radius**: 1000 units
- **Vegetation Density**: Very Low (10 assets target)
- **Vegetation Types**: Shrubs, Rocks
- **Dinosaur Territory**: Triceratops (Defensive, Aggro Radius 1000)

### 5. River Valley Zone (Center)
- **Location**: (1000, 1000, -50)
- **Type**: River Valley
- **Elevation Target**: -100 units
- **Radius**: 800 units
- **Vegetation Density**: Medium (25 assets target)
- **Vegetation Types**: River Plants, Bushes
- **Dinosaur Territory**: Brachiosaurus (Peaceful, Aggro Radius 500)

---

## TECHNICAL IMPLEMENTATION

### Spatial Markers
All biome zones are represented by `EmptyActor` instances in the map with:
- **Label**: Biome zone name (e.g., `Savanna_Zone_North`)
- **Location**: Zone center coordinates
- **Scale**: Encodes radius and elevation (X/Y = radius/100, Z = elevation/100)

### Vegetation Density Markers
Separate `EmptyActor` instances define vegetation distribution:
- **Label**: Vegetation density specification (e.g., `VegDensity_Savanna`)
- **Location**: Zone center
- **Metadata**: Density level, vegetation types, target asset count

### Dinosaur Territory Markers
AI behavior zones marked with `EmptyActor` instances:
- **Label**: Territory name (e.g., `DinoTerritory_TRex_Savanna`)
- **Location**: Territory center
- **Scale**: Encodes aggro radius (X/Y = radius/100)
- **Metadata**: Species, behavior type, aggro radius

---

## AGENT DEPENDENCIES

### Agent #5 (Procedural World Generator)
**Input**: Biome zone markers  
**Task**: Sculpt terrain heightmap to match elevation targets  
**Output**: Varied terrain with hills, valleys, mountains matching biome specifications

### Agent #6 (Environment Artist)
**Input**: Vegetation density markers  
**Task**: Populate zones with vegetation assets matching density and type specifications  
**Output**: 130+ vegetation assets distributed across 5 biomes

### Agent #9 (Character Artist)
**Input**: Dinosaur territory markers  
**Task**: Spawn SkeletalMesh dinosaurs at territory centers  
**Output**: 5 dinosaur species placed in appropriate biomes

### Agent #12 (Combat & Enemy AI)
**Input**: Dinosaur territory markers with behavior metadata  
**Task**: Implement AI behavior trees matching territory specifications  
**Output**: Functional AI with territorial, pack hunter, peaceful, defensive, and herd behaviors

---

## PERFORMANCE CONSTRAINTS

- **Total Actors**: Must remain under 8000 (CAP enforced)
- **Dinosaur Count**: Maximum 150 instances
- **Vegetation Assets**: Target 130 total across all biomes
- **Biome Markers**: 15 total (5 biome + 5 vegetation + 5 territory)

---

## VALIDATION CRITERIA

✅ **Spatial Coherence**: Biome zones do not overlap excessively  
✅ **Elevation Variation**: Terrain shows visible height differences between zones  
✅ **Vegetation Distribution**: Asset density matches biome specifications  
✅ **AI Territory Coverage**: Dinosaur aggro radii cover biome areas without excessive overlap  
✅ **Performance**: Actor count remains within CAP limits

---

## CURRENT STATUS

- ✅ Biome zone markers deployed (5 zones)
- ✅ Vegetation density specs deployed (5 specs)
- ✅ Dinosaur territory markers deployed (5 territories)
- ⏳ Terrain sculpting pending (Agent #5)
- ⏳ Vegetation population pending (Agent #6)
- ⏳ Dinosaur placement pending (Agent #9)
- ⏳ AI behavior implementation pending (Agent #12)

---

**Architecture Status**: DEPLOYED  
**Map Saved**: `/Game/Maps/MinPlayableMap`  
**Next Agent**: #3 Core Systems Programmer (physics/collision rules for terrain and dinosaurs)
