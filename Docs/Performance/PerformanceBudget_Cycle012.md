# Performance Budget — Cycle PROD_CYCLE_AUTO_20260618_012

**Agent:** #04 — Performance Optimizer  
**Date:** 2026-06-18  
**Status:** ✅ CAP_SAFE | ✅ CVars Applied | ✅ LOD Audit | ✅ Nanite/VSM Configured

---

## Frame Budget (60fps PC Target — 16.6ms total)

| System | Budget (ms) | Notes |
|--------|-------------|-------|
| GPU Render (geometry + shading) | 8.0 | Nanite enabled, MaxPixelsPerEdge=1.0 |
| Lumen GI | 3.0 | MaxTraceDistance=8000, SDFs enabled |
| Virtual Shadow Maps | 2.0 | Cache static separate enabled |
| Post Process (bloom, TAA, AO) | 1.5 | Standard quality |
| CPU Game Thread (AI, physics, gameplay) | 4.0 | GC purge every 60s |
| CPU Render Thread (draw call submission) | 3.5 | Dynamic instancing + cached commands |
| GPU Overhead (state changes, barriers) | 1.0 | Async compute enabled |

**Console Target (30fps = 33.3ms):** All budgets ×2

---

## CVars Applied This Cycle

### Shadow Quality
- `r.Shadow.MaxResolution 1024`
- `r.Shadow.RadiusThreshold 0.05`
- `r.Shadow.DistanceScale 0.8`
- `r.Shadow.Virtual.Enable 1`
- `r.Shadow.Virtual.Cache.StaticSeparate 1`

### Lumen (Balanced)
- `r.Lumen.Reflections.Allow 1`
- `r.Lumen.DiffuseIndirect.Allow 1`
- `r.Lumen.TraceMeshSDFs 1`
- `r.Lumen.MaxTraceDistance 8000`

### LOD & Culling
- `r.StaticMeshLODDistanceScale 1.0`
- `r.SkeletalMeshLODBias 0`
- `foliage.MinimumScreenSize 0.0002`
- `foliage.DitheredLOD 1`
- `r.HZBOcclusion 1`
- Cull distance applied to tree/rock/foliage actors: max_draw_distance=15000

### Nanite
- `r.Nanite 1`
- `r.Nanite.MaxPixelsPerEdge 1.0`
- `r.Nanite.ImposterMaxPixels 5.0`

### Draw Call Reduction
- `r.MeshDrawCommands.DynamicInstancing 1`
- `r.MeshDrawCommands.UseCachedCommands 1`
- `r.HLOD 1`

### Streaming & Memory
- `r.Streaming.PoolSize 1024` (MB)
- `r.Streaming.MaxTempMemoryAllowed 50`
- `gc.TimeBetweenPurgingPendingKillObjects 60`

### GPU Scene
- `r.AsyncComputeEnabled 1`
- `r.GPUScene.UploadEveryFrame 0`
- `r.GPUScene.UseGPUSceneForDynamicPrimitives 1`

---

## LOD Audit Results

- StaticMeshActors audited for LOD count
- Actors with ≤1 LOD flagged (trees, rocks, props)
- Cull distances applied: 0–15,000 units for foliage/props
- SkeletalMeshActors (dinosaur placeholders) inventoried

---

## Recommendations for Next Cycles

1. **Agent #6 (Environment):** All new foliage must use at minimum LOD0 + LOD1. Use `foliage.MinimumScreenSize` to control pop-in.
2. **Agent #8 (Lighting):** Keep dynamic light count < 8 per visible area. Use baked lighting for static props.
3. **Agent #12 (Combat AI):** Dinosaur AI tick rate should be throttled to 10Hz when > 50m from player.
4. **Agent #13 (Crowd):** Mass AI agents must use LOD system — full AI only within 30m, simplified beyond.
5. **Agent #17 (VFX):** Niagara emitters must have GPU sim LOD — disable beyond 20m for particle-heavy effects.

---

## Map State
- **Map:** `/Game/Maps/MinPlayableMap`
- **Saved:** ✅ `MAP_SAVED:True`
- **CAP_SAFE:** ✅ `True`
