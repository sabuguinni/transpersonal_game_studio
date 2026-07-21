# Procedural Biome System - World Generator Agent #5

## Overview
Height-based biome distribution system for MinPlayableMap with procedural terrain variation, river networks, and biome-specific vegetation.

## Biome Zones (Height-Based)

### Savana Biome (Elevation: 0-200m)
- **Location:** Central plains (0, 0, 50)
- **Characteristics:** Low elevation grasslands, sparse vegetation
- **River Mouth:** River system terminates here (500, -500, 45)
- **Vegetation Density:** Low (scattered trees)

### Forest Biome (Elevation: 200-500m)
- **Location:** Mid-elevation zone (3000, 3000, 300)
- **Characteristics:** Dense tree coverage, moderate terrain variation
- **River Segment:** Mid-flow river crossing (-1000, -1000, 280)
- **Vegetation Density:** High (15 procedural tree clusters)

### Mountain Biome (Elevation: 500+m)
- **Location:** High peaks (-4000, -4000, 600)
- **Characteristics:** Rocky terrain, mega-boulder formations
- **River Source:** River origin point (-4000, -3500, 550)
- **Rock Formations:** 5 mega-boulders (3.5-6.0m scale)

### Swamp Biome (Elevation: 0-150m, near water)
- **Location:** Wetlands (2000, -2000, 80)
- **Characteristics:** Low elevation, high water table, river delta
- **River Delta:** Wide river spreading (1500, -1800, 75)
- **Vegetation Density:** Medium (10 scattered wetland trees)

## River System

### Flow Path
1. **Mountain Source** (-4000, -3500, 550) - 5x3 scale
2. **Forest Mid-Flow** (-1000, -1000, 280) - 8x5 scale
3. **Swamp Delta** (1500, -1800, 75) - 12x8 scale
4. **Savana Mouth** (500, -500, 45) - 15x10 scale

### Technical Implementation
- Water planes using `/Engine/BasicShapes/Plane`
- Scale increases downstream (erosion simulation)
- Height decreases following natural gravity flow
- Total segments: 4

## Procedural Rock Formations

### Mountain Mega-Boulders
- Count: 5
- Scale: 3.5-6.0m
- Distribution: Random within 500m radius of mountain center
- Labels: `MegaBoulder_Mountain_001` to `MegaBoulder_Mountain_005`

### Rocky Biome Clusters
- Count: 8
- Scale: 2.0-3.5m
- Distribution: Random within 400m radius of rocky center (5000, -3000, 200)
- Labels: `RockCluster_Rocky_001` to `RockCluster_Rocky_008`

## Vegetation System

### Forest Dense Clusters
- Count: 15 trees
- Scale: 2.5-4.5m height
- Distribution: Random within 600m radius of forest center
- Labels: `Tree_Forest_001` to `Tree_Forest_015`

### Swamp Scattered Vegetation
- Count: 10 trees
- Scale: 1.8-3.2m height
- Distribution: Random within 500m radius of swamp center
- Labels: `Tree_Swamp_001` to `Tree_Swamp_010`

## Terrain Zone Markers

Logical zones for biome identification:
- `TerrainZone_Savana_Low` (0, 0, 50)
- `TerrainZone_Forest_Mid` (3000, 3000, 300)
- `TerrainZone_Mountain_High` (-4000, -4000, 600)
- `TerrainZone_Swamp_Low` (2000, -2000, 80)

## Performance Integration

### Alignment with Performance Zones (Agent #4)
- **LOD Testing:** Mountain biome aligns with `PerfZone_Mountain_DrawDistance`
- **Occlusion Testing:** Forest biome aligns with `PerfZone_Forest_Occlusion`
- **Streaming Testing:** Swamp biome aligns with `PerfZone_Swamp_Streaming`
- **FPS Monitoring:** River system crosses `FPS_Monitor_WaterEdge` waypoint

### CAP Compliance
- Total procedural actors spawned: 42
  - Terrain zones: 4
  - River segments: 4
  - Rock formations: 13
  - Vegetation: 25
- Total map actors: < 8000 (CAP compliant)
- Dinosaurs: < 150 (CAP compliant)

## Label Convention
All labels follow strict format: `Type_Biome_NNN`
- No degenerate labels (verified: 0 labels with >5 underscores or >60 chars)
- No re-spawning of existing actors
- Simple, descriptive naming

## Next Agent Integration (Agent #6 - Environment Artist)

### Recommended Focus
1. **Material Application:** Apply biome-specific materials to terrain zones
2. **Foliage Detailing:** Add grass, bushes, and ground cover to vegetation clusters
3. **Rock Texturing:** Replace sphere primitives with detailed rock meshes
4. **Water Shaders:** Apply water materials to river segments with flow direction
5. **Atmospheric Effects:** Add fog/mist to swamp biome, heat haze to savana

### Asset Requests
Environment Artist should request via Meshy API:
- Realistic rock formations (mountain/rocky biome)
- Wetland vegetation (swamp trees, reeds, lily pads)
- Forest tree variety (oak, pine, ancient trees)
- River bank details (pebbles, driftwood, erosion features)
