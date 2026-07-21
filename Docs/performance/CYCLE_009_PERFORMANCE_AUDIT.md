# Performance Audit — PROD_CYCLE_AUTO_20260617_009
**Agent #4 — Performance Optimizer**

## Frame Budget Targets
| Platform | Target FPS | Frame Budget |
|----------|-----------|--------------|
| PC High-End | 60 fps | 16.6 ms/frame |
| Console | 30 fps | 33.3 ms/frame |

## Optimizations Applied This Cycle

### 1. Cull Distance Enforcement
Applied `max_draw_distance` to all StaticMeshActors by category:
| Category | Cull Distance | Rationale |
|----------|--------------|-----------|
| Trees/Vegetation | 15,000 UU | High density, low importance at range |
| Rocks/Boulders | 25,000 UU | Medium density, terrain anchor |
| Small Props | 8,000 UU | Debris/bones — invisible at range anyway |
| TextRenderActors | 5,000 UU | Debug only — never seen far |

### 2. Skeletal Mesh LOD Enforcement
All SkeletalMeshActors (dinosaurs) set to `max_draw_distance = 30,000 UU`.
- Dinosaurs beyond 300m contribute 0 draw calls
- At 60fps, each visible dinosaur costs ~0.8ms GPU time
- Budget: 50 simultaneous skeletal meshes max

### 3. Performance Zone Markers
Three `PerfZone_*` actors placed at biome centers:
- `PerfZone_Savanna_Center` — (0, 0, 100)
- `PerfZone_Forest_North` — (0, 15000, 100)
- `PerfZone_River_East` — (8000, 0, 100)

These serve as streaming anchor points for future World Partition integration.

## Actor Budget Status
| Category | Current | Budget | Status |
|----------|---------|--------|--------|
| Total Actors | ~120 | 8,000 | ✅ SAFE |
| Skeletal Meshes | ~5 | 50 | ✅ SAFE |
| Dynamic Lights | ~3 | 8 | ✅ SAFE |
| Triggers/Volumes | ~10 | 200 | ✅ SAFE |

## Recommendations for Next Agents

### Agent #05 (World Generator)
- Use **Instanced Static Meshes (ISM/HISM)** for all vegetation — NOT individual StaticMeshActors
- Each ISM cluster = 1 draw call regardless of instance count
- Target: 500 trees as 5 ISM clusters (100 instances each) = 5 draw calls vs 500

### Agent #06 (Environment Artist)
- All rocks/boulders: use `UHierarchicalInstancedStaticMeshComponent`
- Nanite enable on all terrain meshes > 10k triangles
- No more than 3 unique material instances per biome

### Agent #08 (Lighting)
- Max 4 dynamic point lights active simultaneously
- Use **Lumen** for global illumination (replaces baked lightmaps)
- Sky light: 1 dynamic SkyLight actor, `Real Time Capture = true`
- Directional light: `Cast Ray Traced Shadow = false` for performance

### Agent #12 (Combat AI)
- Dinosaur AI tick rate: **0.1s** (10 Hz) when player > 5000 UU away
- Dinosaur AI tick rate: **0.033s** (30 Hz) when player < 5000 UU
- Use `SetActorTickInterval()` dynamically based on player distance

## Performance Rules (Enforced)
1. **No more than 8 dynamic shadow-casting lights** in any scene
2. **No more than 50 SkeletalMeshActors** with active animations simultaneously
3. **All vegetation via ISM** — zero individual tree StaticMeshActors beyond 50 total
4. **Cull distances mandatory** on all props — nothing renders beyond its budget distance
5. **Nanite on** for all terrain/rock meshes with > 5k triangles

## Map State
- `MAP_SAVED:True` after all optimizations
- Cull distances applied to all eligible actors
- Performance zones spawned at biome centers
