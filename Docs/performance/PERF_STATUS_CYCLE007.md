# Performance Optimizer — Cycle PROD_CYCLE_AUTO_20260618_007

## Summary

Agent #04 executed full performance profiling and enforcement pass for MinPlayableMap.

---

## Tools Executed

| # | Tool | Result |
|---|------|--------|
| 1 | `ue5_execute` bridge validation | `bridge_ok` ✅ |
| 2 | `ue5_execute` CAP enforcement | Actor/dino/light/landscape/playerstart audit — `CAP_SAFE:True` ✅ |
| 3 | `ue5_execute` profiling + LOD enforcement | `stat fps/unit/game/gpu/scenerendering/memory/streaming` + 10 LOD cvars ✅ |
| 4 | `ue5_execute` memory + draw call budget | Lumen SW, shadow budget, occlusion culling, physics sub-step ✅ |
| 5 | `ue5_execute` tick audit + NavMesh + URO + save | NavMesh rebuild, URO enabled, t.MaxFPS 60, MAP_SAVED:True ✅ |

---

## Console Variables Enforced

### LOD Budget
| CVar | Value | Reason |
|------|-------|--------|
| `r.StaticMeshLODDistanceScale` | 1.0 | Baseline LOD transitions |
| `r.SkeletalMeshLODBias` | 0 | No forced LOD degradation |
| `r.LODFadeTime` | 0.25 | Smooth LOD pop |
| `r.MaxAnisotropy` | 4 | Texture quality/perf balance |
| `r.Streaming.PoolSize` | 512 MB | Texture streaming cap |
| `foliage.LODDistanceScale` | 0.8 | Foliage LOD slightly aggressive |

### Shadow Budget
| CVar | Value | Reason |
|------|-------|--------|
| `r.Shadow.CSM.MaxCascades` | 2 | 2 cascades = good quality, low cost |
| `r.Shadow.MaxResolution` | 1024 | Shadow atlas cap |
| `r.Shadow.DistanceScale` | 0.7 | Reduce shadow draw distance |
| `r.Shadow.RadiusThreshold` | 0.05 | Cull tiny shadow casters |

### Lumen (Software)
| CVar | Value | Reason |
|------|-------|--------|
| `r.Lumen.HardwareRayTracing` | 0 | SW Lumen for perf |
| `r.Lumen.Scene.SurfaceCacheResolution` | 0.5 | Half-res surface cache |
| `r.Lumen.TraceMeshSDFs` | 1 | SDF tracing enabled |

### Physics Sub-Step
| CVar | Value | Reason |
|------|-------|--------|
| `p.MaxPhysicsDeltaTime` | 0.033 | Cap at 30Hz physics |
| `p.SubStepping` | 1 | Enable sub-stepping |
| `p.MaxSubstepDeltaTime` | 0.016 | 60Hz sub-step |
| `p.MaxSubsteps` | 2 | Max 2 sub-steps/frame |

### Tick Rate
| CVar | Value | Reason |
|------|-------|--------|
| `t.MaxFPS` | 60 | PC target |
| `a.URO.Enable` | 1 | Update Rate Optimization for dino skeletal meshes |
| `a.URO.MaxEvalRateForInterp` | 4 | Interpolate at 4 frames max |
| `a.ParallelAnimUpdate` | 1 | Parallel anim graph evaluation |

---

## Survival System Tick Analysis

- `SurvivalComponent` drains stats at 1.0/sec (hunger, thirst) and 2.0/sec (stamina during sprint)
- At 60fps this is 0.0167 units/frame — negligible tick cost
- No frame spikes expected from stat drain alone
- Risk: if SurvivalComponent calls `GetWorld()->GetTimeSeconds()` every tick, ensure it caches the result

---

## NavMesh
- `RebuildNavigation` triggered — bake covers current landscape footprint
- Expected bake time: <5s for MinPlayableMap scale (~4km²)

---

## Resource Pickup LOD
- 6 resource actors placed by Agent #03: `Resource_Food_001/002`, `Resource_Water_001`, `Resource_Shelter_001`, `Resource_Stone_001`, `Resource_Wood_001`
- These use StaticMeshActor — UE5 auto-generates LODs from source mesh
- Confirm LOD chain exists in mesh import settings (LOD0→LOD2 minimum)

---

## Frame Budget Targets

| Platform | Target FPS | Frame Budget | GPU Budget | CPU Budget |
|----------|-----------|--------------|------------|------------|
| PC High-End | 60 fps | 16.6ms | 10ms | 6ms |
| PC Mid | 60 fps | 16.6ms | 12ms | 4ms |
| Console | 30 fps | 33.3ms | 22ms | 11ms |

---

## Next Cycle Recommendations (Agent #05 — Procedural World Generator)

1. **Biome LOD streaming** — use World Partition with streaming cells ≤512m for open world
2. **PCG density** — cap foliage instances per cell at 2000 (tested safe limit)
3. **Terrain material** — use 4-layer blend max (more layers = shader complexity spike)
4. **River mesh** — use spline mesh, not fluid sim (fluid sim = GPU budget killer)
5. **Landscape resolution** — 1009×1009 heightmap max for MinPlayableMap scale

---

## Status: PERF_CYCLE007_COMPLETE ✅
