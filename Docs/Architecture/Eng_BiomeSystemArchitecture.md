# Biome System Architecture - Engine Architect Specification

**Author**: Engine Architect Agent #02  
**Cycle**: PROD_CYCLE_AUTO_20260617_001  
**Date**: 17 June 2026  
**Status**: ACTIVE SPECIFICATION

---

## OVERVIEW

The Biome System is the foundational environmental architecture that defines how the prehistoric world is structured, rendered, and experienced. This system controls terrain generation, vegetation distribution, weather patterns, lighting conditions, and dinosaur spawn logic across distinct ecological zones.

---

## BIOME ZONE DEFINITIONS

### 1. SAVANNA BIOME
- **Location**: Positive X axis (5000+ units)
- **Terrain**: Rolling grasslands, scattered acacia-like trees, rocky outcrops
- **Vegetation Density**: LOW (15% coverage)
- **Primary Dinosaurs**: T-Rex, Triceratops, Parasaurolophus
- **Weather**: Clear skies, occasional dust storms
- **Lighting**: High sun intensity, long shadows
- **Technical Marker**: `Savanna_TechZone_001` (Point Light - Orange)

### 2. FOREST BIOME
- **Location**: Negative X, Positive Y quadrant (-5000, +5000)
- **Terrain**: Dense tree coverage, uneven ground, fallen logs
- **Vegetation Density**: HIGH (70% coverage)
- **Primary Dinosaurs**: Velociraptors, Protoceratops, small herbivores
- **Weather**: Frequent fog, occasional rain
- **Lighting**: Dappled light through canopy, lower ambient
- **Technical Marker**: `Forest_TechZone_001` (Point Light - Green)

### 3. SWAMP BIOME
- **Location**: Negative X, Negative Y quadrant (-5000, -5000)
- **Terrain**: Shallow water pools, mud, thick vegetation
- **Vegetation Density**: MEDIUM (45% coverage, aquatic plants)
- **Primary Dinosaurs**: Brachiosaurus, Ankylosaurus, amphibious species
- **Weather**: High humidity, persistent mist
- **Lighting**: Low intensity, greenish tint
- **Technical Marker**: `Swamp_TechZone_001` (Point Light - Dark Green)

### 4. MOUNTAIN BIOME
- **Location**: Positive X, Positive Y quadrant (+5000, +5000)
- **Terrain**: Steep slopes, rocky terrain, sparse vegetation
- **Vegetation Density**: VERY LOW (5% coverage)
- **Primary Dinosaurs**: Pteranodons, cliff-dwelling species
- **Weather**: High winds, occasional snow at peaks
- **Lighting**: High contrast, dramatic shadows
- **Technical Marker**: `Mountain_TechZone_001` (Point Light - Gray)

---

## TECHNICAL ARCHITECTURE

### Biome Detection System
```python
def get_biome_at_location(location: unreal.Vector) -> str:
    """
    Determines biome type based on world coordinates.
    Used by vegetation, weather, and spawn systems.
    """
    x, y, z = location.x, location.y, location.z
    
    if x > 2500 and y > 2500:
        return "Mountain"
    elif x > 2500 and y < -2500:
        return "Savanna"
    elif x < -2500 and y > 2500:
        return "Forest"
    elif x < -2500 and y < -2500:
        return "Swamp"
    else:
        return "Transition"  # Blended zone
```

### Vegetation Distribution Rules
- **Savanna**: Grass meshes (80%), sparse trees (15%), rocks (5%)
- **Forest**: Dense trees (60%), undergrowth (30%), fallen logs (10%)
- **Swamp**: Aquatic plants (40%), mud patches (35%), dead trees (25%)
- **Mountain**: Rocks (70%), sparse shrubs (20%), snow patches (10%)

### Weather System Integration
Each biome has a weather profile that controls:
- Fog density and color
- Rain probability and intensity
- Wind speed and direction
- Sky atmosphere tint

### Lighting Profiles
- **Savanna**: DirectionalLight intensity 10.0, warm color (1.0, 0.9, 0.7)
- **Forest**: DirectionalLight intensity 6.0, cool color (0.7, 0.8, 0.7)
- **Swamp**: DirectionalLight intensity 4.0, greenish tint (0.6, 0.7, 0.6)
- **Mountain**: DirectionalLight intensity 12.0, neutral color (1.0, 1.0, 1.0)

---

## PERFORMANCE CONSTRAINTS

### Actor Limits Per Biome
- **Total Actors**: 8000 max across all biomes
- **Dinosaurs Per Biome**: 30-40 max
- **Vegetation Instances**: Managed via Foliage System (instanced static meshes)
- **Props Per Biome**: 500 max (rocks, logs, structures)

### LOD Chain Requirements
- **LOD0**: 0-1000 units (full detail)
- **LOD1**: 1000-3000 units (medium detail)
- **LOD2**: 3000-6000 units (low detail)
- **LOD3**: 6000+ units (impostor/culled)

### Streaming Zones
Each biome is a separate World Partition cell:
- **Cell Size**: 10km x 10km
- **Streaming Distance**: 2km buffer
- **Unload Distance**: 3km from player

---

## INTEGRATION POINTS

### Agent Dependencies
- **#03 Core Systems**: Physics materials per biome (mud friction, rock hardness)
- **#05 Procedural World Generator**: Terrain heightmap generation
- **#06 Environment Artist**: Vegetation placement and material assignment
- **#08 Lighting & Atmosphere**: Dynamic lighting per biome
- **#12 Combat & Enemy AI**: Dinosaur spawn logic based on biome type

### Blueprint Interfaces
- `IBiomeAware` - Actors that adapt behavior based on current biome
- `IWeatherResponsive` - Systems that react to weather changes
- `IBiomeTransition` - Handles smooth blending between biomes

---

## VALIDATION CRITERIA

### Functional Tests
1. Player can walk from Savanna to Forest and observe biome transition
2. Vegetation density changes correctly when crossing biome boundaries
3. Weather effects activate/deactivate based on biome entry/exit
4. Dinosaurs spawn only in their designated biomes
5. Lighting profiles blend smoothly during transitions

### Performance Tests
1. FPS remains above 30 when standing at biome intersection (4 biomes visible)
2. Memory usage stays below 8GB with all biomes loaded
3. Streaming completes within 2 seconds when entering new biome
4. No hitches during biome transition (frame time <33ms)

---

## CURRENT STATUS

**Deployed**: 4 biome zone technical markers in MinPlayableMap  
**Next Steps**:
1. Agent #05 (Procedural World Generator) - Generate terrain heightmaps per biome
2. Agent #06 (Environment Artist) - Populate biomes with vegetation
3. Agent #08 (Lighting & Atmosphere) - Apply lighting profiles
4. Agent #12 (Combat & Enemy AI) - Configure dinosaur spawn zones

**Technical Markers Active**:
- Savanna_TechZone_001 (5000, 0, 100) - Orange light
- Forest_TechZone_001 (-5000, 5000, 100) - Green light
- Swamp_TechZone_001 (-5000, -5000, 50) - Dark green light
- Mountain_TechZone_001 (5000, 5000, 300) - Gray light

---

**END SPECIFICATION**
