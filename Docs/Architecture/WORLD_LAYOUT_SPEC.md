# WORLD LAYOUT SPECIFICATION — MinPlayableMap
## Engine Architect #02 — Transpersonal Game Studio

---

## COORDINATE SYSTEM

- **Origin (0,0,0)**: Player spawn point
- **Scale**: 1 Unreal Unit = 1 cm (standard UE5)
- **Playable radius**: 10,000 UU (100 meters real-world equivalent at 1:1 scale, or 10km at 1:100 scale)
- **Up axis**: Z+

---

## BIOME MAP

```
Y+5000  [SWAMP]         Dense vegetation, shallow water, fog
Y+2500  [TRANSITION]    Forest edge → Swamp
Y 0     [RIVER/CENTER]  River valley, PlayerStart, base camp
Y-2500  [TRANSITION]    Savanna edge → Mountain foothills  
Y-5000  [MOUNTAIN]      Rocky terrain, high elevation, predator nests

X-5000  [FOREST]        Dense canopy, herbivores, low visibility
X-2500  [TRANSITION]    Forest → River valley
X 0     [CENTER]        River, PlayerStart, open ground
X+2500  [TRANSITION]    River → Savanna
X+5000  [SAVANNA]       Open plains, T-Rex territory, high visibility
```

---

## BIOME SPECIFICATIONS

### SAVANNA (5000, 0) — Primary Danger Zone
- **Terrain**: Flat to gently rolling, Z: 100-400 UU
- **Vegetation**: Sparse tall grass, isolated trees
- **Fauna**: T-Rex (apex predator), Triceratops herds
- **Visibility**: High (500m clear sightlines)
- **Player risk**: EXTREME — open ground, no cover
- **NavMesh priority**: HIGH (T-Rex patrols)

### FOREST (-5000, 0) — Medium Danger Zone
- **Terrain**: Uneven, roots, Z: 150-350 UU
- **Vegetation**: Dense canopy, ferns, fallen logs
- **Fauna**: Raptors (pack hunters), Stegosaurus
- **Visibility**: Low (50m max through trees)
- **Player risk**: HIGH — ambush predators
- **NavMesh priority**: HIGH (Raptor pack AI)

### SWAMP (0, 5000) — Hazard Zone
- **Terrain**: Waterlogged, Z: 50-200 UU (below water table)
- **Vegetation**: Reeds, mangroves, floating plants
- **Fauna**: Ankylosaur, Spinosaurus (water predator)
- **Visibility**: Medium (fog reduces to 100m)
- **Player risk**: HIGH — movement penalty, ambush
- **NavMesh priority**: MEDIUM (water navigation)

### MOUNTAIN (0, -5000) — Resource Zone
- **Terrain**: Rocky, steep, Z: 500-2000 UU
- **Vegetation**: Sparse alpine, cliff faces
- **Fauna**: Pteranodon (aerial), small prey animals
- **Visibility**: Very high (clear mountain air)
- **Player risk**: MEDIUM — fall damage, aerial predators
- **NavMesh priority**: LOW (limited ground AI)

### RIVER VALLEY (0, 0) — Safe Zone (Relative)
- **Terrain**: Flat river bed, Z: 50-150 UU
- **Vegetation**: Riverside reeds, berry bushes
- **Fauna**: Small herbivores, fish
- **Visibility**: High along river, medium in valley
- **Player risk**: LOW — open escape routes
- **NavMesh priority**: HIGH (player base area)

---

## ACTOR PLACEMENT RULES

### Label Convention
```
Format: Type_Biome_NNN
Examples:
  TRex_Savanna_001
  Raptor_Forest_003
  Rock_Mountain_042
  Tree_Forest_017
  Water_Swamp_001
  Cave_Mountain_001
```

### Forbidden Labels
- Any label with concatenated system names: `FootstepEmitter_CombatZone_QuestTarget` ❌
- Any label over 40 characters ❌
- Any label with spaces ❌

### Placement Density Limits (per biome)
| Biome | Max Trees | Max Rocks | Max Dinos | Max Props |
|-------|-----------|-----------|-----------|-----------|
| Savanna | 20 | 15 | 5 | 10 |
| Forest | 80 | 30 | 8 | 20 |
| Swamp | 40 | 10 | 4 | 15 |
| Mountain | 10 | 60 | 3 | 8 |
| River | 25 | 20 | 3 | 12 |
| **TOTAL** | **175** | **135** | **23** | **65** |

### CAP LIMITS (HARD)
- **Total actors**: 8000 max
- **Dinosaurs**: 150 max (all types combined)
- **Lights**: 500 max (performance)

---

## NAVIGATION ARCHITECTURE

### NavMesh Coverage
- **Volume**: NavMesh_World_001 at origin, scale 200×200×20
- **Coverage**: 20km × 20km × 2km
- **Agent height**: 200 UU (2m — human scale)
- **Agent radius**: 50 UU (0.5m)
- **Step height**: 45 UU (can step up 45cm)

### AI Navigation Zones
| Zone | Priority | Agent Types |
|------|----------|-------------|
| Savanna open | HIGH | T-Rex, Triceratops |
| Forest paths | HIGH | Raptors, Stegosaurus |
| River bank | MEDIUM | All herbivores |
| Swamp shallow | LOW | Ankylosaur, Spinosaurus |
| Mountain base | LOW | Pteranodon (air) |

---

## LIGHTING ARCHITECTURE

### Primary Light: Sun_Directional_001
- **Type**: DirectionalLight
- **Rotation**: Pitch=-45, Yaw=45 (golden afternoon angle)
- **Intensity**: 10 lux (bright prehistoric sun)
- **Color**: Warm white (5500K)
- **Mobility**: Movable (for day/night cycle)

### Atmosphere: SkyAtmosphere_001
- **Type**: SkyAtmosphere
- **Rayleigh scattering**: Default (blue sky)
- **Mie scattering**: Slightly increased (hazy prehistoric air)

### Fog: HeightFog_001
- **Type**: ExponentialHeightFog
- **Density**: 0.02 (light prehistoric haze)
- **Height falloff**: 0.2
- **Start distance**: 1000 UU
- **Color**: Slight warm tint

### Biome Accent Lights (PointLights)
| Light | Purpose | Color |
|-------|---------|-------|
| Biome_Savanna_Center | Warm savanna glow | Gold |
| Biome_Forest_Center | Cool forest ambient | Green |
| Biome_Swamp_Center | Murky swamp light | Teal |
| Biome_Mountain_Center | Cold mountain light | Ice blue |
| Biome_River_Center | Water reflection | Blue |

---

## SURVIVAL SYSTEM ZONES

### Danger Levels (for survival stat drain rates)
| Zone | Hunger Drain | Thirst Drain | Temperature | Predator Threat |
|------|-------------|-------------|-------------|-----------------|
| Savanna | 1.0x | 1.5x | HOT | EXTREME |
| Forest | 0.8x | 0.8x | MILD | HIGH |
| Swamp | 1.0x | 0.5x | HUMID | HIGH |
| Mountain | 1.2x | 1.0x | COLD | MEDIUM |
| River | 0.7x | 0.3x | MILD | LOW |

### Resource Nodes (planned placement)
- **Water**: River valley (0,0) — always available
- **Food**: Forest berries (-3000, 1000), Swamp roots (500, 3500)
- **Stone**: Mountain base (500, -3000), River rocks (-500, 500)
- **Wood**: Forest (-4000, -500), Swamp edge (-500, 3000)
- **Shelter**: Cave_Mountain_001 (500, -800)

---

## REVISION HISTORY
| Cycle | Change |
|-------|--------|
| 004 | Initial biome zone system deployed |
| 005 | Biome landmark PointLights added |
| 006 | Full world layout spec documented, NavMesh + lighting deployed |
