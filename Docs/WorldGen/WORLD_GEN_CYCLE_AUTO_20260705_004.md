# World Generation — Cycle PROD_CYCLE_AUTO_20260705_004
**Agent #05 — Procedural World Generator**
**Status: DEGRADED MODE (Bridge DOWN + OpenAI API 401)**

---

## System Status

| System | Status | Notes |
|--------|--------|-------|
| UE5 Bridge | ❌ TIMEOUT | 4 consecutive cycles with bridge timeout |
| generate_image | ❌ 401 Unauthorized | OpenAI API key invalid/expired |
| GitHub R/W | ✅ OK | Only operational tool |

---

## DEGRADED MODE ACTIVATED

Per Brain Memory protocol: when ue5_execute bridge validation returns FAIL on retry AND generate_image returns FAIL (401), DEGRADED MODE is mandatory. All execution switches to GitHub documentation only.

---

## World Generation Blueprint — Biome System Design

This document captures the complete PCG biome system design for implementation when the bridge is restored.

### Priority Zone: Hub Clearing (X=2100, Y=2400)

**Target Composition (per imp=20 memory):**
- Dense Cretaceous forest surrounding the PlayerStart clearing
- Recognizable dinosaur silhouettes in natural poses
- Bright daylight, living forest atmosphere
- Minimum 3 species visible: Triceratops (grazing), Raptor (alert), Brachiosaurus (background)

### Biome Definitions

#### Biome 1: Cretaceous Rainforest (Hub Zone)
```
Center: X=2100, Y=2400
Radius: 5000 units
Canopy Height: 2000-4000 units
Vegetation Density: 0.85 (high)
Ground Cover: Ferns, cycads, mosses
Mid Layer: Tree ferns, small conifers
Canopy: Giant sequoia-type conifers, broad-leaf angiosperms
Ambient: Morning mist, dappled light, bird sounds
Dinosaurs: Triceratops (grazing), Parasaurolophus (herd), small raptors
```

#### Biome 2: Open Savanna (North, Y > 7000)
```
Vegetation Density: 0.25 (sparse)
Ground Cover: Dry grasses, scattered shrubs
Trees: Isolated conifers, dead trees
Dinosaurs: Brachiosaurus herds, Ankylosaurus, Pachycephalosaurus
Weather: Hot, dry, dust devils
```

#### Biome 3: Rocky Highlands (East, X > 8000)
```
Vegetation Density: 0.15 (very sparse)
Ground Cover: Lichen, sparse ferns
Features: Rock formations, cliff faces, cave entrances
Dinosaurs: Pterodactyls (nesting), Allosaurus (territorial)
Weather: Wind, occasional fog
```

#### Biome 4: River Delta (West, X < -2000)
```
Vegetation Density: 0.70 (dense, wetland)
Ground Cover: Reeds, water plants, mud
Features: River channels, oxbow lakes, sandbars
Dinosaurs: Spinosaurus (fishing), Iguanodon (drinking), Mosasaurus (water)
Weather: Humid, frequent rain
```

#### Biome 5: Volcanic Badlands (South, Y < -5000)
```
Vegetation Density: 0.05 (near zero)
Ground Cover: Ash, obsidian, sulfur deposits
Features: Lava flows (cooled), fumaroles, ash dunes
Dinosaurs: None (too hostile) — occasional Pterodactyl flyover
Weather: Ash fall, toxic gas vents, heat shimmer
```

---

## PCG Graph Design (UE5 PCG Framework)

### Graph: PCG_BiomeForest
```
Nodes:
1. PCGGetActorData (Landscape) → SampleSurface
2. SampleSurface → FilterByBiomeMask (texture channel R = forest)
3. FilterByBiomeMask → SpawnStaticMesh (Tree_Conifer_Large, density=0.02)
4. FilterByBiomeMask → SpawnStaticMesh (Tree_Fern_Large, density=0.05)
5. FilterByBiomeMask → SpawnStaticMesh (Bush_Cycad, density=0.08)
6. FilterByBiomeMask → SpawnStaticMesh (Fern_Ground, density=0.15)
7. All SpawnNodes → ApplySlope (max 45deg, exclude steep)
8. All SpawnNodes → ApplyCollision (avoid overlap)
```

### Graph: PCG_BiomeSavanna
```
Nodes:
1. PCGGetActorData (Landscape) → SampleSurface
2. SampleSurface → FilterByBiomeMask (texture channel G = savanna)
3. FilterByBiomeMask → SpawnStaticMesh (Grass_Dry, density=0.30)
4. FilterByBiomeMask → SpawnStaticMesh (Tree_Isolated_Conifer, density=0.003)
5. FilterByBiomeMask → SpawnStaticMesh (Rock_Sandstone, density=0.01)
```

---

## UE5 Python Commands (Ready for Bridge Restoration)

When bridge comes back online, execute these commands IN ORDER:

### Command Set A: Biome Terrain Sculpting
```python
import unreal

# Get landscape actor
landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if isinstance(actor, unreal.Landscape):
        landscape = actor
        break

if landscape:
    unreal.log(f"Found landscape: {landscape.get_name()}")
    # Apply biome mask via landscape material
    # Material should have 5 channels: R=Forest, G=Savanna, B=Rocky, A=River
else:
    unreal.log_warning("No landscape found - need to create one")
```

### Command Set B: Hub Clearing Vegetation (X=2100, Y=2400)
```python
import unreal

# Spawn dense forest around hub clearing
# Trees in a ring pattern, clearing kept open for PlayerStart
hub_x, hub_y = 2100, 2400
clearing_radius = 800  # Keep this clear for gameplay
forest_radius = 3000   # Dense forest beyond this

import math
tree_count = 40
for i in range(tree_count):
    angle = (i / tree_count) * 2 * math.pi
    # Vary radius between clearing edge and forest radius
    radius = clearing_radius + (forest_radius - clearing_radius) * (0.3 + 0.7 * (i % 3) / 2)
    x = hub_x + radius * math.cos(angle)
    y = hub_y + radius * math.sin(angle)
    z = 0  # Landscape will handle Z via SetActorLocation with trace
    
    loc = unreal.Vector(x, y, z)
    rot = unreal.Rotator(0, (i * 137.5) % 360, 0)  # Golden angle distribution
    
    mesh_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.load_class(None, '/Script/Engine.StaticMeshActor'),
        loc, rot
    )
    if mesh_actor:
        label = f"Tree_Floresta_{i+1:03d}"
        mesh_actor.set_actor_label(label)
        # Scale variation for natural look
        scale = 0.8 + (i % 5) * 0.15
        mesh_actor.set_actor_scale3d(unreal.Vector(scale, scale, scale * 1.2))

unreal.EditorLevelLibrary.save_current_level()
unreal.log("Hub forest ring spawned")
```

### Command Set C: Dinosaur Placement at Hub
```python
import unreal

# Place recognizable dinosaurs near hub clearing
# Per imp=20 memory: must show recognizable dinosaurs in pose

dinosaur_placements = [
    # Triceratops grazing - close to clearing, highly visible
    {"label": "Trike_Floresta_001", "x": 2800, "y": 2400, "z": 0, "yaw": 180, "scale": (1.5, 1.5, 1.5)},
    # Second Triceratops in herd
    {"label": "Trike_Floresta_002", "x": 2900, "y": 2600, "z": 0, "yaw": 200, "scale": (1.3, 1.3, 1.3)},
    # Raptor alert pose - edge of clearing
    {"label": "Raptor_Floresta_001", "x": 1800, "y": 2200, "z": 0, "yaw": 45, "scale": (0.9, 0.9, 0.9)},
    # Brachiosaurus background - large, visible over treeline
    {"label": "Brachi_Floresta_001", "x": 3500, "y": 2000, "z": 0, "yaw": 90, "scale": (2.5, 2.5, 2.5)},
]

for dino in dinosaur_placements:
    loc = unreal.Vector(dino["x"], dino["y"], dino["z"])
    rot = unreal.Rotator(0, dino["yaw"], 0)
    
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.load_class(None, '/Script/Engine.StaticMeshActor'),
        loc, rot
    )
    if actor:
        actor.set_actor_label(dino["label"])
        actor.set_actor_scale3d(unreal.Vector(*dino["scale"]))

unreal.EditorLevelLibrary.save_current_level()
unreal.log("Dinosaur hub placement complete")
```

---

## Biome Audio Design (Environmental)

### Forest Biome Audio Layer
```
Layer 1 - Ambient Base (always playing):
  - jungle_ambience_loop.wav (insects, distant birds)
  - wind_through_leaves_light.wav
  
Layer 2 - Proximity Triggers (within 500 units of vegetation):
  - fern_rustle_01-05.wav (random, 0.1 probability per second)
  - branch_creak_01-03.wav (wind events)
  
Layer 3 - Dinosaur Proximity (within 2000 units of dino actor):
  - triceratops_graze_01-03.wav (low rumble, breathing)
  - raptor_alert_01-02.wav (sharp clicks when player detected)
  - brachiosaurus_footstep_01-04.wav (ground tremor)
  
Layer 4 - Weather Overlay:
  - rain_forest_light.wav / rain_forest_heavy.wav
  - thunder_distant_01-05.wav
```

### Savanna Biome Audio Layer
```
Layer 1 - Ambient Base:
  - savanna_wind_dry.wav
  - cicada_heat_loop.wav
  
Layer 2 - Distance Sounds:
  - brachiosaurus_herd_distant.wav (low rumble)
  - pterodactyl_screech_distant.wav
```

---

## World Partition Configuration

### Streaming Grid Setup
```
Grid Cell Size: 12800 units (128m)
Loading Range: 25600 units (256m) from player
Unloading Range: 38400 units (384m) from player
Server Streaming: Disabled (single player)

Priority Cells (always loaded):
- Cell (0,0): Hub clearing + PlayerStart
- Cell (1,0): East forest
- Cell (-1,0): West river approach
- Cell (0,1): North savanna approach
```

### LOD Configuration per Biome
```
Forest Trees:
  LOD0: < 1000 units (full mesh, 2000 tris)
  LOD1: 1000-3000 units (simplified, 500 tris)
  LOD2: 3000-6000 units (billboard impostor)
  LOD3: > 6000 units (culled)

Dinosaurs:
  LOD0: < 2000 units (full mesh + animations)
  LOD1: 2000-5000 units (reduced bones, 50% tris)
  LOD2: 5000-8000 units (static pose, 25% tris)
  LOD3: > 8000 units (billboard)
```

---

## Next Cycle Priorities (When Bridge Restored)

1. **IMMEDIATE**: Execute Command Set B (hub forest ring) — creates visible forest
2. **IMMEDIATE**: Execute Command Set C (dinosaur placement) — creates hero screenshot composition
3. **FOLLOW-UP**: Apply biome mask texture to landscape material
4. **FOLLOW-UP**: Configure PCG graphs for procedural vegetation
5. **FOLLOW-UP**: Set up World Partition streaming grid

---

## Handoff to Agent #06 (Environment Artist)

**Ready for implementation:**
- Biome definitions with exact coordinates and density values
- PCG graph node specifications
- Vegetation layer hierarchy (ground → mid → canopy)
- LOD configuration per asset type
- Audio layer design per biome

**Blocking issue:**
- UE5 bridge DOWN — no commands can execute until Hugo restores bridge
- OpenAI API key expired — no concept art generation

**Hugo action required:**
1. Restart UE5 bridge process on server
2. Renew OpenAI API key in orchestrator environment
