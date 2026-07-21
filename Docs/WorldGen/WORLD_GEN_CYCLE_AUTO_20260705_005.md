# World Generator — Cycle PROD_CYCLE_AUTO_20260705_005
**Agent:** #05 — Procedural World Generator  
**Date:** 2026-07-05  
**Status:** DEGRADED MODE — Bridge DOWN + OpenAI API 401

---

## Infrastructure Status

| System | Status | Detail |
|--------|--------|--------|
| UE5 Bridge | ❌ TIMEOUT | 5 consecutive cycles (002–005) all timeout |
| OpenAI Image API | ❌ 401 UNAUTHORIZED | API key invalid/expired |
| GitHub Read/Write | ✅ OPERATIONAL | Only available tools |

**Pattern:** 5+ consecutive cycles with 100% bridge failure confirms the UE5 editor process is either terminated or the Remote Control plugin is not running. This is an infrastructure issue requiring Hugo's intervention.

---

## DEGRADED MODE PROTOCOL (Active)

Per Brain Memory (imp=10): When ue5_execute bridge validation returns FAIL on retry AND generate_image returns FAIL (401), enter DEGRADED MODE:
- ✅ Execute ONLY GitHub operations (file_write, file_read, list_directory)
- ❌ STOP all ue5_execute attempts (bridge is dead, not unstable)
- ❌ STOP all generate_image attempts (API key invalid)
- ❌ STOP all meshy_generate/text_to_speech/search_sounds

---

## World Generation Design — Consolidated Spec for Next Live Cycle

This document consolidates all prior WorldGen cycles into a single actionable spec ready for execution when the bridge is restored.

### Priority 1: Content Hub Composition (X=2100, Y=2400)
Per Brain Memory (imp=20, hugo_hub_quality_v2_fix):
- **Target:** Hero screenshot clearing at world coords X=2100, Y=2400
- **Required elements:**
  - Recognizable dinosaur poses (not placeholder cubes)
  - Dense vegetation surrounding the clearing
  - Bright daylight — sun angle -45° pitch, no fog override
  - Living Cretaceous forest aesthetic

### Biome Layout (from accumulated cycles)

```
WORLD COORDINATE MAP (top-down, 10,000 x 10,000 units)
┌─────────────────────────────────────────────────────┐
│  TUNDRA PLATEAU        │  VOLCANIC HIGHLANDS        │
│  (0,0)→(2500,5000)     │  (7500,0)→(10000,5000)    │
├────────────────────────┼────────────────────────────┤
│  CRETACEOUS FOREST     │  OPEN SAVANNA              │
│  (0,5000)→(5000,10000) │  (5000,5000)→(10000,10000)│
│  ← CONTENT HUB HERE   │                            │
│  X=2100, Y=2400        │                            │
└─────────────────────────────────────────────────────┘
```

### Vegetation Density Targets (per biome)
| Biome | Trees | Ferns | Rocks | Dinosaur Types |
|-------|-------|-------|-------|----------------|
| Cretaceous Forest | 120 | 200 | 40 | TRex, Raptor, Brachiosaurus |
| Open Savanna | 30 | 80 | 20 | Triceratops, Parasaurolophus |
| Volcanic Highlands | 10 | 15 | 80 | Pterodactyl, Ankylosaurus |
| Tundra Plateau | 20 | 30 | 60 | Mammoth-era fauna |

### Actor Naming Convention (Brain Memory imp=20)
Pattern: `Type_Bioma_NNN`
- Trees: `Tree_Floresta_001` through `Tree_Floresta_120`
- Ferns: `Fern_Floresta_001` through `Fern_Floresta_200`
- Rocks: `Rock_Floresta_001` through `Rock_Floresta_040`
- Dinosaurs: `TRex_Floresta_001`, `Raptor_Floresta_001`, etc.

### UE5 Python Script — Ready for Execution When Bridge Restored

```python
import unreal
import math
import random

# ============================================================
# CONTENT HUB VEGETATION DENSITY SCRIPT
# Target: X=2100, Y=2400 clearing with dense forest surround
# ============================================================

random.seed(42)  # Deterministic for reproducibility

# Load mesh references
tree_mesh_path = '/Engine/BasicShapes/Cylinder'
fern_mesh_path = '/Engine/BasicShapes/Cone'
rock_mesh_path = '/Engine/BasicShapes/Sphere'

tree_mesh = unreal.load_object(None, tree_mesh_path)
fern_mesh = unreal.load_object(None, fern_mesh_path)
rock_mesh = unreal.load_object(None, rock_mesh_path)

actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# Hub center
HUB_X = 2100.0
HUB_Y = 2400.0
HUB_Z = 0.0

CLEARING_RADIUS = 400.0   # Open space around PlayerStart
FOREST_RADIUS = 2000.0    # Dense forest ring

spawned = []

# Spawn trees in ring around clearing
for i in range(60):
    angle = (i / 60.0) * 2 * math.pi
    # Vary radius between clearing edge and forest boundary
    radius = CLEARING_RADIUS + random.uniform(100, FOREST_RADIUS - CLEARING_RADIUS)
    x = HUB_X + math.cos(angle) * radius
    y = HUB_Y + math.sin(angle) * radius
    z = HUB_Z
    
    loc = unreal.Vector(x, y, z)
    rot = unreal.Rotator(0, random.uniform(0, 360), 0)
    
    actor_class = unreal.load_class(None, '/Script/Engine.StaticMeshActor')
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, loc, rot)
    
    if actor:
        label = f"Tree_Floresta_{i+1:03d}"
        actor.set_actor_label(label)
        
        # Scale variation for natural look
        scale_h = random.uniform(3.0, 8.0)
        scale_w = random.uniform(0.5, 1.5)
        actor.set_actor_scale3d(unreal.Vector(scale_w, scale_w, scale_h))
        
        # Set mesh
        mesh_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        if mesh_comp and tree_mesh:
            mesh_comp.set_static_mesh(tree_mesh)
        
        spawned.append(label)

unreal.log(f"Spawned {len(spawned)} forest trees around content hub")
unreal.EditorLevelLibrary.save_current_level()
print(f"FOREST_SPAWN_OK: {len(spawned)} trees")
```

### Lighting Spec (Sun Guard — Brain Memory imp=20)
```python
# MANDATORY: Sun pitch must be between -30 and -60 degrees
# Max 1 DirectionalLight in scene
# atmosphere_sun_light = True
# Fog DISABLED (not sky fog, volumetric fog disabled)

sun_pitch = -45.0  # Safe value within [-30, -60] guard range
sun_yaw = 45.0     # Morning angle for hero shot
```

---

## PCG Biome Scatter — Technical Spec

### River System
- Main river: flows from Volcanic Highlands (NE) to Cretaceous Forest (SW)
- River path: X=7500,Y=0 → X=5000,Y=3000 → X=2500,Y=6000 → X=0,Y=8000
- Width: 200-500 units (narrows in highlands, widens in forest)
- Implementation: Spline mesh with water material (blue-tinted, reflective)

### Terrain Height Map
- Volcanic Highlands: +800 to +1500 units elevation
- Tundra Plateau: +200 to +600 units
- Cretaceous Forest: -50 to +200 units (valley floor)
- Open Savanna: 0 to +150 units (flat grassland)

### Erosion Simulation (PCG)
- River valleys carved by simulated water flow
- Cliff faces at biome transitions (Highlands → Forest)
- Alluvial fans at river mouth (Forest → coast if applicable)

---

## Deliverables This Cycle

| Type | Item | Status |
|------|------|--------|
| DEGRADED MODE | Bridge DOWN confirmed (5 consecutive failures) | ✅ Documented |
| DEGRADED MODE | OpenAI API 401 confirmed | ✅ Documented |
| SPEC | Content Hub composition spec (X=2100, Y=2400) | ✅ Written |
| SPEC | Biome layout map (4 biomes, coordinate ranges) | ✅ Written |
| SPEC | Vegetation density targets per biome | ✅ Written |
| SPEC | Actor naming convention (Type_Bioma_NNN) | ✅ Written |
| SCRIPT | UE5 Python forest spawn script (ready to execute) | ✅ Written |
| SPEC | Lighting sun guard spec (-30 to -60 pitch) | ✅ Written |
| SPEC | River system path and dimensions | ✅ Written |
| SPEC | Terrain height map per biome | ✅ Written |

---

## Escalation Required

**HUGO INTERVENTION NEEDED:**
1. **UE5 Bridge:** Editor process appears terminated or Remote Control plugin not running. 5+ consecutive cycles (PROD_002 through PROD_005) all timeout. Restart UE5 editor + bridge Python process required.
2. **OpenAI API Key:** 401 Unauthorized on generate_image. Key needs renewal at platform.openai.com/account/api-keys.

---

## Next Agent (#06 — Environment Artist)

When bridge is restored, execute the forest spawn script above targeting X=2100, Y=2400. Priority:
1. Dense vegetation ring around the content hub clearing
2. Replace placeholder cylinder/cone trees with proper Quixel Megascans foliage if available
3. Add ground cover (ferns, moss, fallen logs) within the clearing
4. Ensure dinosaur actors are visible and posed (not stacked on same coordinates)

**Handoff:** This spec document is the complete world generation blueprint. All prior cycle docs are superseded by this consolidated version.
