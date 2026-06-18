# Crowd & Traffic Simulation — Agent #13 Cycle 005 Documentation

## Systems Implemented This Cycle

### 1. Migration Corridor System
- **6 waypoint markers** defining a seasonal migration path across the map
- **8 Stegosaurus herd members** moving along the corridor (SW to NE diagonal)
- Amber point lights (200 intensity, 800 radius) mark the corridor visually
- Migration path: (-3000,-4000) → (-1500,-2500) → (0,-1000) → (1500,500) → (3000,2000) → (4500,3500)

### 2. Territorial Boundary System
- **T-Rex territory**: 8 red markers in a 2500-unit radius circle centered at (2000,1500)
- **Raptor Pack Alpha**: 6 purple markers, 1200-unit radius at (-1000,2000)
- **Raptor Pack Beta**: 6 purple markers, 1000-unit radius at (3500,-500)
- **Raptor Pack Gamma**: 6 purple markers, 900-unit radius at (-2500,1000)
- **3 Herbivore Safe Zones**: Green lights at riverside, open plain, dense forest

### 3. Watering Hole Congregation System
- **Primary water source** at (-500, 1500)
- **8 Triceratops** clustered at water center (dominant herbivores hold ground)
- **6 Parasaurolophus** at water edge (skittish, flee first)
- **4 Predator ambush positions** at cardinal points around water (N/S/E/W)

## Crowd Ecology Design Principles

### Species Hierarchy at Water Sources
1. **T-Rex** — approaches alone, all others flee
2. **Triceratops herd** — holds ground, may charge smaller predators
3. **Brachiosaurus** — wades in, ignores smaller animals
4. **Parasaurolophus** — flees at first sign of predator
5. **Raptors** — hunt at water edges, target isolated prey

### Migration Behavior Logic
- Herds follow waypoint corridors seasonally (dry season → wet season movement)
- Migration triggers when food/water in home zone drops below threshold
- Predators learn migration routes and set ambushes at chokepoints
- Player can exploit migration patterns for hunting

### Territorial Dynamics
- T-Rex holds ~6km² exclusive territory
- Raptor packs hold ~3km² overlapping territories
- Territory boundaries shift after dominant animal death
- Player entering territory triggers escalating threat response

## Actor Label Convention
All crowd actors follow: `Type_Zone_NNN` format
- `Stego_Migration_001` through `Stego_Migration_008`
- `TRex_Territory_Marker_01` through `TRex_Territory_Marker_08`
- `Raptor_Alpha_Territory_01` through `Raptor_Alpha_Territory_06`
- `Trike_WaterHole_001` through `Trike_WaterHole_008`
- `Para_WaterHole_001` through `Para_WaterHole_006`
- `Ambush_West/East/North/South`
- `SafeZone_Riverside/OpenPlain/DenseForest`

## Cumulative Crowd Infrastructure (Cycles 001-005)

| System | Actors | Type |
|--------|--------|------|
| Brachiosaurus Herd | 12 | Formation |
| Raptor Pack Formation | 5 | Hunting |
| Crowd Scatter (mixed species) | 12 | Ambient |
| Combat Zone Crowd Response | 8 | Behavioral |
| Migration Corridor | 14 | Waypoints + Herd |
| Territory Markers | 26 | Boundary |
| Watering Hole | 18 | Congregation |
| **TOTAL** | **95** | |

## Dependencies
- **From Agent #12**: Combat zones, predator AI positions (used for territory placement)
- **To Agent #14**: Quest triggers can use migration waypoints, territory boundaries, and watering hole as mission locations

## Next Cycle Recommendations
1. **Panic cascade system** — when one animal flees, nearby animals of same species also flee
2. **Day/night crowd shifts** — nocturnal predators active at night, herbivores shelter
3. **Seasonal migration trigger** — tie migration to weather/time system from Agent #08
4. **Scent trail system** — predators follow scent markers left by prey herds
