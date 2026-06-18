# Performance Report — PROD_CYCLE_AUTO_20260618_005
**Agent:** #04 — Performance Optimizer  
**Date:** 2026-06-18  

---

## Shadow Budget Enforcement

| Console Var | Value | Reason |
|---|---|---|
| `r.Shadow.CSM.MaxCascades` | 2 | Limit cascade shadow maps to 2 (saves ~15% GPU) |
| `r.Shadow.RadiusThreshold` | 0.03 | Skip shadows on small actors |
| `r.ShadowQuality` | 2 | Medium shadow quality — balanced |
| `r.SkeletalMeshLODBias` | 1 | Force lower LOD on skeletal meshes at distance |
| `r.DetailMode` | 1 | Medium detail mode globally |
| `fx.MaxCPUParticlesPerEmitter` | 64 | Cap particle count per emitter |
| `r.HZBOcclusion` | 1 | Enable hierarchical Z-buffer occlusion |
| `r.ReflectionCaptureResolution` | 128 | Low-res reflection captures |
| `r.AmbientOcclusionRadiusScale` | 0.5 | Halve AO radius for perf |
| `r.Streaming.PoolSize` | 512 | 512MB texture streaming pool |
| `r.StaticMeshLODDistanceScale` | 1.5 | Increase LOD transition distances |
| `r.LightMaxDrawDistanceScale` | 0.8 | Reduce max light draw distance |

---

## Cull Distance Rules (Applied to All Static Meshes)

| Actor Type | Cull Distance |
|---|---|
| Tree | 15,000 units |
| Rock | 12,000 units |
| Grass | 5,000 units |
| Bush | 6,000 units |
| Fern | 4,000 units |
| Water | 20,000 units |
| Ground/Terrain | 0 (always visible) |
| Waypoint | 0 (invisible at runtime) |
| Default | 10,000 units |

---

## NavMesh + TargetPoint Audit

- All `TargetPoint` actors (patrol waypoints) set to `hidden_in_game = True`
- Zero render cost at runtime for waypoints
- NavMesh Bounds Volume verified in previous cycle (10,000u × 10,000u)

---

## Performance Targets

| Platform | Target FPS | Status |
|---|---|---|
| PC High-End | 60 FPS | ✅ Budget enforced |
| Console | 30 FPS | ✅ Shadow cascades limited |

---

## Active Performance Rules

- **RULE-PERF-001:** Shadow cascades ≤ 2 at all times
- **RULE-PERF-002:** All static meshes have cull distances set
- **RULE-PERF-003:** TargetPoints invisible at runtime (zero draw cost)
- **RULE-PERF-004:** Particle emitters capped at 64 CPU particles
- **RULE-PERF-005:** Texture streaming pool ≤ 512MB
- **RULE-PERF-006:** AO radius scaled to 0.5 for distance performance

---

## Next Cycle Recommendations

- **Agent #05 (World Generator):** Apply `r.Foliage.MinimumScreenSize 0.002` to all foliage instances — prevents overdraw on distant grass
- **Agent #08 (Lighting):** Keep dynamic shadow-casting lights ≤ 4 per frame; use stationary lights for sun/sky
- **Agent #17 (VFX):** Niagara emitters must use GPU simulation for particles > 64 count; CPU limit is 64/emitter
