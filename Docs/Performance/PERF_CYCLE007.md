# Performance Report — PROD_CYCLE_AUTO_20260618_007
**Agent #04 — Performance Optimizer**

---

## Execution Summary

| Step | Tool | Result |
|------|------|--------|
| 1 | Bridge validation | `bridge_ok` ✅ |
| 2 | CAP enforcement | Actor/dino/light/resource audit — `CAP_SAFE:True` ✅ |
| 3 | Profiling commands | `stat fps/unit/game/gpu/scenerendering` sent ✅ |
| 4 | Budget enforcement | 21 performance cvars applied ✅ |
| 5 | Cull distance + volume | Per-type cull distances + `CullDistanceVolume_Global` ✅ |

---

## Performance Budgets Applied

### Shadow Budget
| CVar | Value | Rationale |
|------|-------|-----------|
| `r.Shadow.CSM.MaxCascades` | 2 | Reduces shadow draw calls by ~40% |
| `r.Shadow.MaxResolution` | 1024 | Balanced quality vs VRAM |
| `r.Shadow.MinResolution` | 64 | Prevents micro-shadow waste |
| `r.Shadow.Virtual.Enable` | 1 | VSM enabled for static shadow caching |
| `r.Shadow.Virtual.Cache.StaticSeparate` | 1 | Static geometry cached separately |

### Texture Streaming
| CVar | Value | Rationale |
|------|-------|-----------|
| `r.Streaming.PoolSize` | 1024 MB | Sufficient for open world |
| `r.Streaming.MaxTempMemoryAllowed` | 50 MB | Prevents streaming spikes |

### LOD & Culling
| CVar | Value | Rationale |
|------|-------|-----------|
| `r.StaticMeshLODDistanceScale` | 1.0 | Default — tune down if GPU bound |
| `foliage.LODDistanceScale` | 1.0 | Default — tune down if foliage heavy |
| `r.SkeletalMeshLODBias` | 0 | No bias — dino quality preserved |
| `r.HZBOcclusion` | 1 | Hierarchical Z-Buffer occlusion ON |
| `r.OcclusionQueryLocation` | 1 | Occlusion queries from camera |
| `r.ScreenPercentage` | 100 | Native resolution |

### Particle Budget
| CVar | Value | Rationale |
|------|-------|-----------|
| `fx.Niagara.MaxGPUParticlesSpawnPerFrame` | 500 | Prevents GPU particle spikes |
| `fx.MaxCPUParticlesPerEmitter` | 100 | CPU particle cap per emitter |

### NavMesh
| CVar | Value | Rationale |
|------|-------|-----------|
| `ai.NavMesh.UpdateInterval` | 0.5s | Prevents NavMesh rebuild hitches |

### Lumen (GI + Reflections)
| CVar | Value | Rationale |
|------|-------|-----------|
| `r.Lumen.Reflections.Allow` | 1 | Lumen reflections ON |
| `r.Lumen.DiffuseIndirect.Allow` | 1 | Lumen GI ON |
| `r.Lumen.TraceMeshSDFs` | 1 | SDF tracing for accuracy |
| `r.Lumen.MaxTraceDistance` | 8000 | 8km trace limit — prevents open-world waste |

---

## Cull Distance Rules (Per Actor Type)

| Actor Type | Cull Distance | Notes |
|------------|---------------|-------|
| `Resource_*` | 3,000 units | Pickup items — visible at close range |
| `Trigger_*` | 2,000 units | Trigger volumes — invisible, low priority |
| `Rock_*` | 5,000 units | Environment rocks |
| `Tree_*` | 6,000 units | Foliage trees |
| `TRex_*` | 15,000 units | Boss-tier — always visible |
| `Raptor_*` | 10,000 units | Pack predators |
| `Brachio_*` | 15,000 units | Massive herbivore — always visible |
| `Dino_*` | 10,000 units | Generic dinosaurs |
| `Emitter_*` | 2,000 units | Particle effects — near only |
| `Footstep_*` | 1,500 units | Footstep VFX — very near only |

**`CullDistanceVolume_Global`** spawned at (0,0,500) with scale (200,200,50) to enforce map-wide culling.

---

## Target Frame Budget

| Platform | Target FPS | Frame Budget |
|----------|-----------|--------------|
| PC High-End | 60 fps | 16.6ms |
| PC Mid | 60 fps | 16.6ms |
| Console | 30 fps | 33.3ms |

### Frame Budget Breakdown (60fps target)
| System | Budget | Notes |
|--------|--------|-------|
| GPU Render | 8ms | Lumen + shadows + geometry |
| CPU Game Thread | 4ms | AI, physics, gameplay |
| CPU Render Thread | 3ms | Draw call submission |
| Headroom | 1.6ms | Spikes, GC, streaming |

---

## Profiling Commands Active
- `stat fps` — frame rate counter
- `stat unit` — CPU/GPU/frame timing
- `stat game` — game thread breakdown
- `stat gpu` — GPU timing per pass
- `stat scenerendering` — draw call counts

---

## Next Cycle Recommendations

1. **Agent #5 (World Generator)**: Apply `r.StaticMeshLODDistanceScale 0.8` if landscape foliage density causes GPU spikes
2. **Agent #8 (Lighting)**: Keep dynamic light count < 4 per screen; use `r.Shadow.Virtual.Cache.StaticSeparate 1` for static lights
3. **Agent #12 (Combat AI)**: NavMesh update interval set to 0.5s — AI path queries should be batched, not per-frame
4. **Agent #17 (VFX)**: Niagara GPU cap = 500/frame, CPU cap = 100/emitter — design effects within these limits
5. **Agent #13 (Crowd)**: Mass AI agents should use LOD tiers — full AI within 2000 units, simplified beyond

---

## Files Modified
- `MAP_SAVED: /Game/Maps/MinPlayableMap` — CullDistanceVolume_Global added
