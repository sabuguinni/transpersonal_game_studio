# Performance Architecture - Engine Architect Specification

**Author**: Engine Architect Agent #02  
**Cycle**: PROD_CYCLE_AUTO_20260617_001  
**Date**: 17 June 2026  
**Status**: ACTIVE SPECIFICATION

---

## PERFORMANCE TARGETS

### Platform Requirements
- **PC (High)**: 60 FPS @ 1080p, 16GB RAM, GTX 1070 equivalent
- **PC (Medium)**: 45 FPS @ 1080p, 12GB RAM, GTX 1060 equivalent
- **PC (Low)**: 30 FPS @ 720p, 8GB RAM, GTX 960 equivalent
- **Console (PS5/Xbox Series X)**: 30 FPS @ 4K, dynamic resolution scaling

### Hard Limits (Non-Negotiable)
- **Total Actors**: 8000 max in active world partition cells
- **Dinosaurs**: 150 max across all loaded cells
- **Draw Calls**: 3000 max per frame
- **Memory Budget**: 8GB max for game content (excluding OS/engine)
- **Frame Time**: 33ms max (30 FPS minimum)

---

## LOD CHAIN ARCHITECTURE

### Distance-Based LOD Rules
All visual assets MUST implement 4-level LOD chain:

#### LOD0 (0-1000 units)
- Full polygon count
- High-resolution textures (2K-4K)
- All material features enabled (normal maps, roughness, AO)
- Skeletal mesh: Full bone count
- Animation: 60 FPS sample rate

#### LOD1 (1000-3000 units)
- 50% polygon reduction
- Medium-resolution textures (1K-2K)
- Simplified materials (normal maps only)
- Skeletal mesh: 70% bone count
- Animation: 30 FPS sample rate

#### LOD2 (3000-6000 units)
- 75% polygon reduction
- Low-resolution textures (512-1K)
- Base color + simple lighting only
- Skeletal mesh: 40% bone count
- Animation: 15 FPS sample rate

#### LOD3 (6000+ units)
- 90% polygon reduction OR impostor billboard
- Ultra-low textures (256-512)
- Unlit material
- Static mesh proxy for skeletal meshes
- No animation (frozen pose or culled)

### Automatic LOD Switching
```python
# UE5 Python - Configure LOD distances for all static meshes
import unreal

def configure_lod_distances(mesh_path: str):
    mesh = unreal.load_asset(mesh_path)
    if isinstance(mesh, unreal.StaticMesh):
        lod_settings = mesh.get_editor_property('lod_group')
        mesh.set_editor_property('lod_distances', [1000, 3000, 6000])
        mesh.set_editor_property('auto_compute_lod_distances', False)
        unreal.log(f"LOD configured: {mesh_path}")
```

---

## CULLING STRATEGIES

### Frustum Culling
- Enabled by default (UE5 automatic)
- Aggressive culling for small objects (<100cm)
- Conservative culling for large objects (>1000cm)

### Occlusion Culling
- **Precomputed Visibility**: Enabled for static geometry
- **Dynamic Occlusion Queries**: Enabled for moving actors
- **Hierarchical Z-Buffer**: Enabled (UE5 Nanite fallback)

### Distance Culling
- **Vegetation**: Cull beyond 5000 units
- **Small Props**: Cull beyond 3000 units
- **Dinosaurs**: Never cull (fade to impostor at LOD3)
- **Terrain**: Never cull (use World Partition streaming)

---

## WORLD PARTITION CONFIGURATION

### Cell Structure
- **Cell Size**: 10km x 10km
- **Runtime Grid**: 4x4 cells (40km x 40km playable area)
- **Streaming Distance**: 2km from player
- **Unload Distance**: 3km from player

### Streaming Priority
1. **Critical**: Player cell + adjacent 8 cells (always loaded)
2. **High**: Cells within 2km (load within 1 second)
3. **Medium**: Cells within 3km (load within 3 seconds)
4. **Low**: Cells beyond 3km (background streaming)

### Data Layers
- **Base Terrain**: Always loaded
- **Vegetation**: Load on demand per biome
- **Structures**: Load on demand per quest state
- **Dinosaurs**: Load based on spawn zones

---

## MEMORY MANAGEMENT

### Asset Streaming
- **Texture Streaming Pool**: 2GB reserved
- **Mesh Streaming**: Enabled for all static meshes >10MB
- **Audio Streaming**: Enabled for all sounds >5MB
- **Animation Streaming**: Enabled for all sequences >2MB

### Garbage Collection
- **Incremental GC**: Enabled (max 5ms per frame)
- **Full GC Trigger**: Every 60 seconds if memory >6GB
- **Forced GC**: On biome transition or quest completion

### Memory Budgets Per System
- **Terrain**: 1.5GB
- **Vegetation**: 1.0GB
- **Dinosaurs**: 1.5GB
- **Props/Structures**: 1.0GB
- **Audio**: 0.5GB
- **UI/HUD**: 0.3GB
- **Reserved**: 2.2GB (engine overhead, buffers)

---

## RENDERING OPTIMIZATIONS

### Lumen Settings (Global Illumination)
- **Lumen Scene Detail**: Medium (balance quality/performance)
- **Lumen Reflections**: Enabled, max trace distance 5000 units
- **Software Ray Tracing**: Enabled (no hardware RT required)
- **Update Frequency**: 30 FPS (half-rate for GI)

### Nanite (Virtualized Geometry)
- **Enabled For**: Terrain, large rocks, structures
- **Disabled For**: Vegetation (use instanced foliage), dinosaurs (skeletal meshes)
- **Fallback LOD**: Always provide LOD3 for non-Nanite platforms

### Virtual Shadow Maps
- **Enabled**: Yes (replaces traditional shadow maps)
- **Max Resolution**: 8K per light
- **Clipmap Levels**: 4 (near to far)
- **Cache Invalidation**: On object movement >10cm

---

## PROFILING CHECKPOINTS

### Mandatory Performance Tests
All agents must validate their systems against these benchmarks:

#### Test 1: Biome Intersection
- **Location**: Standing at (0, 0, 100) - center of map
- **Expected**: 4 biomes visible, 2000+ actors in view
- **Target**: 30 FPS minimum, <8GB memory

#### Test 2: Dinosaur Herd
- **Setup**: 20 Triceratops in Savanna biome
- **Action**: Player approaches herd, triggers flee behavior
- **Target**: 30 FPS minimum, no animation hitches

#### Test 3: Dense Forest
- **Location**: Forest biome center (-5000, 5000, 100)
- **Expected**: 500+ tree instances, heavy foliage
- **Target**: 30 FPS minimum, no streaming pop-in

#### Test 4: Combat Scenario
- **Setup**: Player vs 3 Velociraptors in Forest
- **Action**: Melee combat, particle effects, audio
- **Target**: 30 FPS minimum, <33ms frame time

### UE5 Console Commands for Profiling
```
stat fps          // Show FPS counter
stat unit         // Show frame time breakdown
stat memory       // Show memory usage
stat scenerendering  // Show draw calls, triangles
r.Lumen.Visualize  // Debug Lumen GI
r.Nanite.Visualize  // Debug Nanite geometry
```

---

## PERFORMANCE ENFORCEMENT RULES

### Pre-Commit Validation
Before any agent commits assets to the repository:
1. Run `stat fps` in worst-case scenario (biome intersection)
2. Verify FPS >30 and memory <8GB
3. Check draw calls <3000 with `stat scenerendering`
4. Confirm no red warnings in Output Log

### Automated Performance Tests
- **Frequency**: Every 5 production cycles
- **Executor**: Agent #04 (Performance Optimizer)
- **Blocker**: If any test fails, Agent #18 (QA) blocks the build

### Performance Regression Policy
If a new asset/system causes FPS drop >5% or memory increase >500MB:
1. Agent responsible must optimize within 1 cycle
2. If unfixable, asset is reverted from build
3. Alternative approach required before re-submission

---

## INTEGRATION WITH OTHER AGENTS

### Agent #03 (Core Systems Programmer)
- Physics simulation budget: 5ms per frame
- Ragdoll limit: 10 active ragdolls max
- Collision queries: <1000 per frame

### Agent #05 (Procedural World Generator)
- Terrain generation: Async, never block main thread
- Heightmap resolution: Max 2K per cell
- Material layers: Max 4 per terrain section

### Agent #06 (Environment Artist)
- Vegetation instances: Use Hierarchical Instanced Static Meshes (HISM)
- Foliage density: Max 10 instances per m² in dense areas
- Material complexity: Max 50 shader instructions for foliage

### Agent #12 (Combat & Enemy AI)
- AI tick rate: 10 Hz for distant dinosaurs, 30 Hz for combat
- Behavior tree complexity: Max 20 nodes per tree
- Pathfinding: Async queries, max 100 active paths

---

## CURRENT STATUS

**Performance Baseline** (as of 17 June 2026):
- **Active Actors**: ~150 (well below 8000 limit)
- **Dinosaurs**: ~12 (well below 150 limit)
- **FPS**: 60+ (exceeds 30 FPS target)
- **Memory**: ~3GB (well below 8GB limit)

**Next Optimization Targets**:
1. Implement LOD chain for all dinosaur skeletal meshes
2. Configure World Partition streaming for biome cells
3. Enable Lumen with optimized settings
4. Set up Virtual Shadow Maps for directional light

**Performance Headroom**:
- **Actors**: 7850 remaining capacity
- **Dinosaurs**: 138 remaining capacity
- **Memory**: 5GB remaining capacity

---

**END SPECIFICATION**
