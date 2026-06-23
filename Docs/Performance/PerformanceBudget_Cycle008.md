# Performance Budget — Cycle 008
**Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260623_008**

## Frame Budget (60fps PC Target = 16.67ms total)

| System | Budget (ms) | Notes |
|--------|-------------|-------|
| CPU GameThread | 4.0 | Character tick, AI, physics |
| CPU RenderThread | 5.0 | Draw calls, state changes |
| GPU Geometry | 3.0 | Nanite, mesh rendering |
| GPU Lighting | 2.5 | Lumen GI + shadows |
| GPU PostProcess | 1.5 | TSR, bloom, DOF |
| GPU VFX | 0.67 | Niagara particles |
| **TOTAL** | **16.67** | = 60fps |

## Console Target (30fps = 33.33ms)
All budgets doubled. Scalability preset: Medium.

## Settings Applied This Cycle

### Scalability (Epic)
- `sg.ShadowQuality 3`, `sg.GlobalIlluminationQuality 3`, `sg.ReflectionQuality 3`
- `sg.FoliageQuality 3`, `sg.ShadingQuality 3`

### TSR (Temporal Super Resolution)
- `r.TemporalAA.Algorithm 1` — TSR enabled
- `r.TSR.History.ScreenPercentage 200` — high quality history

### Lumen
- `r.Lumen.Reflections.Allow 1`
- `r.Lumen.DiffuseIndirect.Allow 1`
- `r.LumenScene.SurfaceCacheResolution 1.0`
- `r.Lumen.ScreenProbeGather.RadianceCache.NumProbeTracesBudget 200`

### Shadows
- `r.Shadow.MaxResolution 2048`
- `r.Shadow.CSM.MaxCascades 3`
- `r.Shadow.CacheWholeSceneShadows 1`

### Streaming & Memory
- `r.Streaming.PoolSize 2048` MB
- `r.Streaming.MaxTempMemoryAllowed 256` MB

### GC
- `gc.TimeBetweenPurgingPendingKillObjects 60` — reduce GC hitches

### Nanite
- `r.Nanite 1`, `r.Nanite.MaxPixelsPerEdge 1.0`

### Occlusion
- `r.HZBOcclusion 1`, `r.AllowOcclusionQueries 1`

### Tick Optimisation
- Static meshes: tick **disabled**
- Lights/fog/sky: tick interval **0.1s** (10fps)
- Dino pawns: tick interval **0.05s** (20fps)
- Character: tick every frame (required for input/survival)

### Draw Call Reduction
- `r.MeshDrawCommands.DynamicInstancing 1`
- `r.MeshDrawCommands.UseCachedCommands 1`
- `r.GPUScene.UploadEveryFrame 0`

### Volumetric Fog
- `r.VolumetricFog 1`, `r.VolumetricFog.GridPixelSize 8`
- `r.VolumetricFog.GridSizeZ 128`

### Physics
- `p.MaxPhysicsDeltaTime 0.033`, `p.Substepping 1`
- `p.SubsteppingMaxSubsteps 6`, `p.SubsteppingMaxTickRate 120`

## Recommendations for Next Agents

- **Agent #5 (World Generator)**: Keep PCG foliage density ≤ 10,000 instances per tile. Use Nanite for all rocks/trees.
- **Agent #6 (Environment Artist)**: Use ISM/HISM for repeated props. Max 3 unique materials per mesh.
- **Agent #8 (Lighting)**: Max 4 dynamic point lights per zone. Use static/stationary for distant lights.
- **Agent #12 (Combat AI)**: Dino AI tick at 20fps (0.05s interval) is enforced. Do not override to per-frame.
- **Agent #17 (VFX)**: Niagara budget = 0.67ms GPU. Max 500 active particles per emitter in open world.
