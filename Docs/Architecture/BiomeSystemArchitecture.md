# Biome System Architecture — Engine Architect #02
## Transpersonal Game Studio — Prehistoric Survival Game
### Version: PROD_CYCLE_AUTO_20260702_011

---

## 1. OVERVIEW

The Biome System defines the ecological zones of the prehistoric world. Each biome has distinct:
- **Terrain characteristics** (height, slope, moisture)
- **Flora composition** (Cretaceous-accurate vegetation)
- **Fauna behavior** (which dinosaur species inhabit each zone)
- **Environmental hazards** (temperature, visibility, danger level)
- **Survival resource distribution** (water, food, shelter materials)

This document is the **authoritative reference** for all agents implementing world content.

---

## 2. BIOME ZONES (MinPlayableMap)

### Zone 1: SAVANA (Savanna)
- **Location**: East quadrant (X: 1000–3000, Y: -1500–1500)
- **Marker actor**: `Zone_Savana_001`
- **Terrain**: Flat to gently rolling, dry grassland, scattered rock outcrops
- **Flora**: Sparse cycads, drought-resistant ferns, dead tree stumps
- **Fauna**: 
  - TRex (apex predator, territorial, 1 per zone)
  - Ankylosaur (grazer, passive unless threatened)
  - Small mammals (prey items, ambient life)
- **Hazards**: Heat exposure (stamina drain in midday), TRex patrol routes
- **Resources**: Flint rocks (tools), dry wood (fire), large bones (weapons)
- **Danger Level**: HIGH (TRex territory)

### Zone 2: FLORESTA (Forest)
- **Location**: North quadrant (X: -1500–1500, Y: 1000–3000)
- **Marker actor**: `Zone_Floresta_001`
- **Terrain**: Dense canopy, uneven ground, fallen logs, stream beds
- **Flora**: Tall conifers, tree ferns, cycads, dense undergrowth, mosses
- **Fauna**:
  - Raptor pack (3–5 individuals, coordinated hunters)
  - Parasaurolophus (herd grazer, alarm caller)
  - Pterosaurs (aerial, nest in tall trees)
- **Hazards**: Low visibility (ambush risk), Raptor pack coordination
- **Resources**: Berries, medicinal plants, wood, vines (rope crafting)
- **Danger Level**: EXTREME (pack predators, ambush terrain)

### Zone 3: RIO (River Delta)
- **Location**: West quadrant (X: -3000 to -1000, Y: -1500–1500)
- **Marker actor**: `Zone_Rio_001`
- **Terrain**: Flat floodplain, muddy banks, shallow water crossings, reed beds
- **Flora**: Aquatic ferns, horsetails, water plants, willow-like trees
- **Fauna**:
  - Brachiosaurus (mega-herbivore, feeding at water's edge)
  - Spinosaurus (semi-aquatic, fish hunter, extremely dangerous)
  - Crocodilians (ambush predators in water)
- **Hazards**: Drowning, Spinosaurus ambush, quicksand patches
- **Resources**: Fresh water (hydration), fish (food), clay (building), reeds (crafting)
- **Danger Level**: HIGH (Spinosaurus, drowning risk)

### Zone 4: CAMP (Player Base)
- **Location**: Center (X: -300–300, Y: -300–300)
- **Marker actors**: `Shelter_Wall_N/S/E/W_001`
- **Terrain**: Slightly elevated rocky plateau, natural defensive position
- **Flora**: Minimal (cleared by player)
- **Fauna**: None (player-controlled safe zone)
- **Features**: Campfire, stone shelter walls, storage area
- **Danger Level**: LOW (player-maintained perimeter)

---

## 3. NAVIGATION WAYPOINT SYSTEM

Navigation waypoints (`NavWP_*`) define the pathfinding graph for:
- **Player movement** between zones
- **Dinosaur patrol routes** (AI behavior trees reference these)
- **Quest objective markers** (Agent #14 uses these as anchor points)

| Waypoint | Location | Purpose |
|----------|----------|---------|
| `NavWP_Camp_001` | (0, 0, 50) | Player base anchor |
| `NavWP_Savana_001` | (1000, 0, 50) | Savanna entry point |
| `NavWP_Floresta_001` | (0, 1000, 50) | Forest entry point |
| `NavWP_Rio_001` | (-1000, 0, 50) | River delta entry point |
| `NavWP_Cliff_001` | (500, -800, 200) | High ground vantage point |

---

## 4. TECHNICAL ARCHITECTURE RULES

### 4.1 World Partition Strategy
- **Streaming cells**: 512m × 512m per cell
- **Load radius**: 2 cells around player (1024m)
- **Unload radius**: 3 cells (1536m)
- **Required for**: Any map > 4km² (current map: ~6km²)

### 4.2 PCG (Procedural Content Generation) Integration
Agent #5 (World Generator) MUST use these PCG rules:
```
PCG Graph: BP_PCG_BiomeScatter
  Input: Landscape heightmap + biome mask texture
  Output: Foliage instances (ISM/HISM)
  
  Rules:
  - Slope > 45°: NO vegetation placement
  - Slope 30-45°: Rocks only
  - Slope < 30°: Full vegetation scatter
  - Within 200m of NavWP: Reduced density (pathfinding clearance)
  - Within 500m of Zone_Savana: Dry/sparse vegetation set
  - Within 500m of Zone_Floresta: Dense/lush vegetation set
  - Within 500m of Zone_Rio: Aquatic/wetland vegetation set
```

### 4.3 Dinosaur Spawn Rules
Agent #12 (Combat AI) MUST follow these spawn constraints:
```
TRex_Savana_001:
  - Spawn location: (1200, 0, 100) [existing marker]
  - Patrol radius: 800m
  - Aggro range: 600m (sight), 300m (sound)
  - Sleep cycle: 22:00–06:00 (reduced aggro)

Raptor_Savana_001/002:
  - Spawn location: (800, ±300, 100) [existing markers]
  - Pack behavior: Always within 200m of each other
  - Patrol radius: 500m
  - Aggro range: 400m (sight), 200m (sound)
  - Flanking AI: One distracts, others flank

Brach_Savana_001:
  - Spawn location: (-1500, 0, 100) [existing marker]
  - Wander radius: 300m (slow mover)
  - Passive unless cornered
  - Aggro range: 0m (never attacks first)
```

### 4.4 Survival Resource Distribution
Agent #9 (Character) and Agent #14 (Quest) reference these resource nodes:
```
Water sources: Zone_Rio_001 (primary), 2 small ponds in Zone_Floresta
Food sources: Zone_Floresta (berries, small game), Zone_Rio (fish)
Flint/stone: Zone_Savana (rock outcrops), NavWP_Cliff_001 vicinity
Wood: Zone_Floresta (abundant), Zone_Savana (sparse)
Fire fuel: Zone_Savana (dry grass, dead wood)
```

---

## 5. PERFORMANCE BUDGET

| System | Budget | Notes |
|--------|--------|-------|
| Foliage instances | 50,000 max | HISM, LOD3 at 500m |
| Dinosaur AI actors | 20 max | Full BT only within 800m |
| Dynamic shadows | 3 sources max | Sun + 2 campfires |
| Landscape components | 64 max | 8×8 grid, 512m cells |
| NavMesh tiles | 256 max | Auto-rebuilt on landscape change |

---

## 6. AGENT DEPENDENCY MAP

```
Engine Architect #02 (THIS DOC)
    ↓
World Generator #05 — Implement PCG scatter, landscape heightmap
    ↓
Environment Artist #06 — Place rocks, props, vegetation meshes
    ↓
Architecture #07 — Build cave shelter, stone structures
    ↓
Lighting #08 — Day/night cycle, biome-specific atmosphere
    ↓
Character Artist #09 — Replace sphere markers with dinosaur meshes
    ↓
Animation #10 — Idle/walk/attack animations per species
    ↓
NPC Behavior #11 — Patrol routes using NavWP_* waypoints
    ↓
Combat AI #12 — Aggro ranges, pack tactics, spawn rules above
```

---

## 7. CURRENT MAP STATE (Cycle AUTO_011)

**Total actors in MinPlayableMap**: ~50+
**Biome zone markers**: 3 (Savana, Floresta, Rio)
**Shelter walls**: 4 (primitive stone enclosure at camp)
**Navigation waypoints**: 5
**Dinosaur placeholders**: 4 (TRex×1, Raptor×2, Brach×1) — sphere meshes
**Camp lights**: 4 (warm orange point lights)
**Lighting**: DirectionalLight (sun -45°), ExponentialHeightFog (density 0.02), SkyLight (real_time_capture)

**NEXT PRIORITY**: Agent #5 must replace flat terrain with heightmap landscape featuring:
- Hills in Savanna zone (5–15m elevation)
- Dense forest floor in Floresta zone (uneven, 2–8m variation)
- Flat floodplain in Rio zone (0–2m, near water level)
- Rocky plateau at Camp zone (10–20m elevation, natural defense)

---

*Document maintained by Engine Architect #02 — update each production cycle*
