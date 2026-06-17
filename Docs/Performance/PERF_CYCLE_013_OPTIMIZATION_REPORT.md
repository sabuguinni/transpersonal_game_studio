# Performance Optimization Report — Cycle PROD_CYCLE_AUTO_20260617_012

**Agent:** #4 Performance Optimizer  
**Date:** 2026-06-17  
**Map:** `/Game/Maps/MinPlayableMap`

---

## Frame Budget Analysis

| Actor Type | Count | Cost/Actor (est.) | Total Cost |
|------------|-------|-------------------|------------|
| StaticMeshActor | ~40 | 0.05ms | ~2.0ms |
| SkeletalMeshActor | ~5 | 0.30ms | ~1.5ms |
| Lights/Sky/Fog | ~4 | 0.50ms | ~2.0ms |
| Emitters/Niagara | ~3 | 0.80ms | ~2.4ms |
| Triggers/Volumes | ~4 | 0.01ms | ~0.04ms |
| **TOTAL** | | | **~7.9ms** |

**Target:** <16.6ms (60fps) ✅ **Status: WITHIN BUDGET**

---

## Optimizations Applied This Cycle

### 1. Cull Distance Optimization
Applied `ld_max_draw_distance` to all static mesh actors:

| Category | Cull Distance | Actors Affected |
|----------|--------------|-----------------|
| Small props (Flint, Berry, Waypoints) | 3,000 units | Flint_Rock_*, BerryBush_*, PatrolWaypoint_* |
| Medium props (Trees, Rocks) | 8,000 units | Tree_*, Rock_* |
| Water sources | 12,000–15,000 units | WaterSource_* |

**Rationale:** Small survival resources are only interactable within ~500 units. Culling at 3000 saves significant GPU time when player is far away.

### 2. Shadow Optimization
Disabled `cast_shadow` on:
- All `Flint_Rock_*` actors (tiny, no meaningful shadow)
- All `BerryBush_*` actors (small, shadow cost > visual benefit)
- All `PatrolWaypoint_*` actors (invisible gameplay markers)
- All `WaterSource_*` actors (flat planes, self-shadowing artifacts)

**Expected shadow draw call reduction:** ~30–40% on small prop pass

### 3. Water Source Actors Spawned
| Actor | Location | Scale | Cull | Shadow |
|-------|----------|-------|------|--------|
| `WaterSource_River_001` | (8000, 0, 0) | 8×8×0.3 | 15000 | Off |
| `WaterSource_Pond_002` | (-5000, 6000, 0) | 5×5×0.2 | 12000 | Off |

---

## Performance Budget Rules (Enforced)

```
FRAME BUDGET ALLOCATION (16.6ms @ 60fps):
  Rendering (GPU):     8.0ms  — meshes, materials, lighting
  Game Thread (CPU):   4.0ms  — AI, physics, gameplay
  RHI Thread:          2.0ms  — draw call submission
  Headroom:            2.6ms  — spikes, GC, streaming
  
ACTOR CAPS (enforced):
  Total actors:        MAX 8,000
  Dinosaur pawns:      MAX 150
  Dynamic lights:      MAX 8 (Lumen handles rest via GI)
  Niagara emitters:    MAX 20 active simultaneously
  Skeletal meshes:     MAX 50 visible at once
```

---

## LOD Recommendations for Next Agents

### Agent #6 (Environment Artist)
When replacing sphere placeholders with real meshes:
- **Trees:** Must have LOD0 (full), LOD1 (50% tris), LOD2 (25% tris), LOD3 (billboard)
- **Rocks:** LOD0 + LOD1 sufficient (static, low poly already)
- **Use Nanite** for rocks >500 tris if available in project settings

### Agent #5 (World Generator)
- Landscape terrain: Use **Landscape LOD** system (built-in), not custom
- Set `LandscapeLODDistanceFactor = 1.5` for performance/quality balance
- Max 4 landscape components visible at once in MinPlayableMap scale

### Agent #8 (Lighting)
- **Directional light:** 1 only, dynamic, cascaded shadow maps (3 cascades max)
- **Point lights:** Use static baked where possible, dynamic only for fire/torch
- **Lumen:** Enable for GI, disable screen-space reflections on water (use planar instead)

---

## Next Cycle Priorities

1. **LOD verification** — confirm real meshes (when added by Agent #6) have proper LOD chains
2. **Niagara emitter audit** — campfire/footstep emitters need LOD distance culling
3. **NavMesh bounds** — verify NavMesh volume doesn't cover entire map (expensive)
4. **Streaming volumes** — add World Partition streaming for areas >10,000 units from origin

---

## Map State After This Cycle

| Resource | Count | Notes |
|----------|-------|-------|
| Flint_Rock_* | 3 | Cull 3000, no shadow |
| BerryBush_* | 3 | Cull 3000, no shadow |
| PatrolWaypoint_* | 4 | Cull 3000, no shadow |
| WaterSource_* | 2 | NEW this cycle, cull 12-15k |
| Total actors | ~55 | Well within 8000 CAP |

**MAP_SAVED:** ✅ `/Game/Maps/MinPlayableMap`
