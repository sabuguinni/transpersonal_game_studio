# Biome System Architecture
**Engine Architect - Agent #2**  
**Cycle:** PROD_CYCLE_AUTO_20260617_004  
**Date:** 2026-06-17

## Overview
The biome system provides spatial organization of the game world into distinct ecological zones. Each biome has unique environmental properties, dinosaur species distribution, resource availability, and gameplay characteristics.

## Biome Zones Deployed

### 1. Savanna Zone
- **Location:** (5000, 0, 100)
- **Color Code:** RGB(1.0, 0.8, 0.3) - Golden yellow
- **Characteristics:** Open grasslands, sparse trees, high visibility
- **Dinosaurs:** T-Rex, Triceratops, Parasaurolophus
- **Resources:** Tall grass, scattered rocks, water holes

### 2. Forest Zone
- **Location:** (-5000, 0, 100)
- **Color Code:** RGB(0.2, 0.6, 0.2) - Deep green
- **Characteristics:** Dense vegetation, low visibility, vertical gameplay
- **Dinosaurs:** Velociraptors (pack hunters), small herbivores
- **Resources:** Wood, berries, medicinal plants

### 3. Swamp Zone
- **Location:** (0, 5000, 50)
- **Color Code:** RGB(0.3, 0.4, 0.2) - Murky green-brown
- **Characteristics:** Shallow water, mud, fog, slow movement
- **Dinosaurs:** Brachiosaurus, amphibious species
- **Resources:** Fish, reeds, clay

### 4. Mountain Zone
- **Location:** (0, -5000, 500)
- **Color Code:** RGB(0.5, 0.5, 0.5) - Stone gray
- **Characteristics:** High elevation, steep terrain, caves
- **Dinosaurs:** Pteranodons, mountain-adapted species
- **Resources:** Stone, metal ore, rare minerals

## Navigation Architecture

### NavMesh Bounds Volume
- **Coverage:** 200x200x50 units (scaled from origin)
- **Purpose:** AI pathfinding for dinosaurs and NPCs
- **Integration:** Supports all biome zones within bounds
- **Performance:** Dynamic rebuild on terrain changes

## Technical Implementation

### Zone Markers
Each biome zone is marked with a scaled sphere (10x10x10) for visual reference during development. These markers serve as:
- Spatial anchors for procedural generation
- Reference points for biome-specific spawning
- Debug visualization for zone boundaries

### Actor Count Management
- **Total Actor Cap:** 8000 (enforced)
- **Dinosaur Cap:** 150 (enforced)
- **Current Deployment:** 4 biome zone markers + 1 NavMesh bounds

## Integration Points

### For Agent #3 (Core Systems Programmer)
- Use biome zone locations as spawn points for physics tests
- Implement terrain-aware collision in each biome type
- Test ragdoll physics on varied terrain (flat savanna vs steep mountain)

### For Agent #5 (Procedural World Generator)
- Use zone markers as PCG seed points
- Generate biome-specific terrain within each zone
- Blend biome boundaries using distance-based interpolation

### For Agent #6 (Environment Artist)
- Populate each zone with biome-appropriate vegetation
- Use zone color codes for material tinting
- Distribute props based on zone characteristics

### For Agent #12 (Combat & Enemy AI)
- Configure dinosaur behavior per biome (open plains vs dense forest)
- Use NavMesh for tactical AI movement
- Implement zone-specific hunting patterns

## Performance Considerations
- Zone markers use basic shapes (low poly count)
- NavMesh bounds optimized for 200x200 coverage
- Biome system designed for runtime queries (distance-based)
- No per-frame overhead - zones are static spatial data

## Next Steps
1. Agent #3 implements physics systems aware of biome terrain
2. Agent #5 generates procedural terrain for each zone
3. Agent #6 populates zones with vegetation and props
4. Agent #12 configures AI behavior per biome type

---
**Status:** DEPLOYED  
**Map:** /Game/Maps/MinPlayableMap  
**Validation:** COMPLETE
