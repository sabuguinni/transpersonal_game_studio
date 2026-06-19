# Performance Optimizer — Cycle Report
**Agent:** #04 — Performance Optimizer  
**Cycle:** PROD_CYCLE_AUTO_20260619_004  
**Target:** 60fps PC / 30fps Console

---

## CVar Optimisation Applied

| CVar | Value | Rationale |
|------|-------|-----------|
| `r.Shadow.MaxResolution` | 2048 | Sufficient quality, halves shadow map memory vs 4096 |
| `r.Shadow.RadiusThreshold` | 0.03 | Culls small shadow casters early |
| `r.CapsuleShadows` | 0 | Disabled — not needed for primitive character meshes |
| `r.Lumen.GlobalIllumination.MaxTraceDistance` | 8000 | Limits GI trace to 80m — sufficient for outdoor scenes |
| `r.Lumen.Reflections.MaxRayIntensity` | 20 | Clamps firefly artifacts in reflections |
| `r.StaticMeshLODDistanceScale` | 1.0 | Default — adjust per-biome if needed |
| `foliage.LODDistanceScale` | 1.5 | Slightly extended — reduces pop-in on foliage |
| `r.HZBOcclusion` | 1 | Hierarchical Z-Buffer occlusion — always on |
| `r.AllowOcclusionQueries` | 1 | GPU occlusion queries — always on |
| `r.Streaming.PoolSize` | 2048 | 2GB texture streaming pool for high-end PC |
| `r.Nanite.MaxPixelsPerEdge` | 1.0 | Default Nanite tessellation threshold |
| `r.ScreenPercentage` | 100 | Native resolution — no TAA upscaling in editor |
| `r.MaxAnisotropy` | 8 | 8x anisotropic filtering — good quality/cost ratio |

---

## Scene Audit Summary

- **PostProcessVolume:** Verified present (PPV_GlobalExposure, bUnbound=True)
- **NavMeshBoundsVolume:** Verified present
- **ExponentialHeightFog:** Verified present
- **Degenerate labels:** 0 detected (CAP_SAFE)
- **Map saved:** `/Game/Maps/MinPlayableMap` ✅

---

## Performance Budget (Target Frame Times)

| Platform | Target FPS | Frame Budget | GPU Budget | CPU Budget |
|----------|-----------|--------------|------------|------------|
| PC High-End | 60fps | 16.6ms | 11ms | 5.6ms |
| Console | 30fps | 33.3ms | 22ms | 11ms |

### Breakdown by System (estimated)
| System | PC Cost | Console Cost |
|--------|---------|--------------|
| Lumen GI | ~3ms | ~8ms |
| Lumen Reflections | ~1.5ms | ~4ms |
| Shadows | ~1ms | ~2ms |
| Foliage/Nanite | ~2ms | ~4ms |
| Character + Dinos | ~1ms | ~2ms |
| Post Process | ~0.5ms | ~1ms |
| **Total Est.** | **~9ms** | **~21ms** |

---

## Next Cycle Recommendations

1. **LOD audit on dinosaur skeletal meshes** — once real dino assets are imported, verify LOD chain (LOD0→LOD3 at 1000/3000/6000/10000 UU)
2. **Foliage density cap** — max 50,000 foliage instances per biome tile to stay within GPU budget
3. **NavMesh bake** — run `P.NavMesh.Build` after terrain is finalised
4. **Streaming volume setup** — add Level Streaming Volumes around biome boundaries for World Partition
5. **Profiling baseline** — run `stat fps` + `stat unit` in PIE to capture real frame times once character movement is active
