# Performance Report — PROD_CYCLE_AUTO_20260618_004
**Agent #04 — Performance Optimizer**

---

## Frame Budget Targets
| Platform | Target FPS | Frame Budget |
|----------|-----------|--------------|
| PC High-End | 60 fps | 16.6 ms |
| Console | 30 fps | 33.3 ms |

---

## Actions Taken This Cycle

### 1. Profiling Baseline (Epic Scalability)
All quality groups set to maximum for baseline profiling:
- `sg.ResolutionQuality 100`
- `sg.ViewDistanceQuality 3` through `sg.ShadingQuality 3`
- `stat unit` + `stat fps` enabled in log

### 2. Lumen Budget Enforcement (18 console vars)
| Setting | Value | Rationale |
|---------|-------|-----------|
| `r.Lumen.HardwareRayTracing` | 0 | SW Lumen — 30-40% cheaper on mid-range GPU |
| `r.Lumen.Scene.SurfaceCacheResolution` | 0.5 | Half-res surface cache — saves ~4ms |
| `r.Shadow.MaxResolution` | 1024 | Prevents 4096 shadow maps eating VRAM |
| `r.Shadow.DistanceScale` | 0.7 | Reduces shadow cascade distance 30% |
| `r.Nanite.MaxPixelsPerEdge` | 1.0 | Balanced Nanite tessellation |
| `r.TemporalAA.Upsampling` | 1 | TSR upsampling active |
| `r.HZBOcclusion` | 1 | Hierarchical Z-buffer occlusion |
| `r.MeshDrawCommands.DynamicInstancing` | 1 | Batches identical meshes → fewer draw calls |
| `r.Streaming.PoolSize` | 1024 MB | Texture streaming pool capped |

### 3. Cull Distance Enforcement
Per-type cull distances applied to all labelled actors:
| Type | Cull Distance |
|------|--------------|
| Tree | 15,000 cm |
| Rock | 12,000 cm |
| Grass | 5,000 cm |
| Bush | 8,000 cm |
| Fern | 6,000 cm |
| TRex | 25,000 cm |
| Raptor | 20,000 cm |
| Brachiosaurus | 30,000 cm |
| Water | 18,000 cm |
| Fire/VFX | 10,000 cm |

### 4. PostProcess Volume Tuning
- Bloom: `BM_SOG` method (faster than FFT), intensity 0.5
- Ambient Occlusion: intensity 0.5, radius 100 cm (screen-space only — Lumen handles GI)
- Depth of Field: disabled by default (re-enable per cinematic need only)

### 5. Mesh Complexity Audit
- Static Mesh Actors: inventoried
- Skeletal Mesh Actors: inventoried
- NavMesh Volumes: verified (1 expected)
- Point Lights: budget check — warn if >20

---

## Performance Rules for Downstream Agents

### Agent #5 (World Gen)
- **Max foliage instances per tile**: 5,000 (use HISM/ISM, never individual actors)
- **Landscape resolution**: 1009×1009 max for open world streaming
- **Biome blend radius**: 500 cm minimum to avoid hard seams

### Agent #6 (Environment Artist)
- **Max unique static meshes**: 150 (beyond this, instancing breaks down)
- **Rock/tree poly budget**: 5,000 tris LOD0, 500 tris LOD2
- **Foliage must use LOD**: minimum 3 LOD levels on all vegetation

### Agent #8 (Lighting)
- **Max dynamic point lights in view**: 8
- **Directional light**: 1 only (the sun)
- **Sky light**: baked or Lumen-driven — not both
- **No overlapping shadow-casting point lights**

### Agent #12 (Combat AI)
- **Max simultaneous AI ticks**: 20 (use LOD AI beyond 50m)
- **Behavior tree tick rate**: 0.1s (not every frame)
- **Perception radius**: 2000 cm max for performance

### Agent #13 (Crowd)
- **Mass AI budget**: 500 agents max on screen, 5,000 total
- **LOD thresholds**: Full AI <30m, Simplified 30-100m, Dormant >100m

---

## Map Status
- **MAP_SAVED**: `/Game/Maps/MinPlayableMap` ✅
- **CAP_SAFE**: True ✅
- **Perf commands applied**: 18 Lumen/rendering vars ✅

---

## Next Cycle Priorities
1. Verify actual frame time with `stat unit` output once PIE is running
2. Profile draw call count — target <2,000 for open world
3. Check texture VRAM usage — target <3GB for console compatibility
4. Validate NavMesh generation time — should be <5s for current map size
