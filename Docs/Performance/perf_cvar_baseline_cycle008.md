# Performance CVar Baseline — PROD_CYCLE_AUTO_20260619_008

**Agent:** #04 Performance Optimizer  
**Cycle:** PROD_CYCLE_AUTO_20260619_008  
**Target:** 60fps PC / 30fps Console  

## Applied CVars (24 total)

| CVar | Value | Rationale |
|------|-------|-----------|
| r.ShadowQuality | 3 | High (not ultra) — balanced for open world |
| r.Shadow.CSM.MaxCascades | 3 | 3 cascades sufficient for terrain scale |
| r.Shadow.MaxResolution | 2048 | 2K shadow maps — quality/perf balance |
| r.Lumen.Reflections.Allow | 1 | Enabled — water/wet rock reflections needed |
| r.Lumen.GlobalIllumination.Allow | 1 | Enabled — cave/forest GI required |
| r.Lumen.GlobalIllumination.MaxTraceDistance | 8000 | 80m trace — sufficient for survival game scale |
| r.VolumetricFog | 1 | Enabled — prehistoric atmosphere |
| r.VolumetricFog.GridPixelSize | 8 | 8px grid — performance-safe |
| r.MotionBlurQuality | 1 | Low — survival game needs clarity |
| r.AmbientOcclusionLevels | 2 | Screen space AO — 2 levels |
| r.SSR.Quality | 2 | Medium SSR — water surfaces |
| r.PostProcessAAQuality | 4 | TAA — stable image |
| r.ScreenPercentage | 100 | No upscaling penalty |
| r.StaticMeshLODDistanceScale | 1.0 | Default — open world balance |
| r.SkeletalMeshLODBias | 0 | No LOD bias — dinosaur detail preserved |
| foliage.LODDistanceScale | 1.5 | Extended foliage LOD — denser visible forest |
| r.HZBOcclusion | 1 | HZB occlusion culling — on |
| r.MaxAnisotropy | 8 | 8x anisotropy — good quality/perf ratio |
| r.Streaming.PoolSize | 2048 | 2GB texture pool |
| r.Streaming.MaxTempMemoryAllowed | 64 | 64MB temp streaming buffer |
| r.Nanite.MaxPixelsPerEdge | 1.0 | Default Nanite tessellation |
| r.SkyAtmosphere.FastSkyLUT | 1 | Fast sky LUT — no render warning |
| r.SkyAtmosphere.AerialPerspectiveLUT.FastApply | 1 | Fast aerial perspective |

## SurvivalComponent Tick Recommendation

Previous cycle integrated `SurvivalComponent` into `TranspersonalCharacter`.  
**Performance concern:** Survival stat drain (hunger/thirst/stamina) must NOT run every frame.

**Required implementation in SurvivalComponent.cpp:**
```cpp
// In BeginPlay — set timer instead of Tick
GetWorld()->GetTimerManager().SetTimer(
    SurvivalTickHandle,
    this,
    &USurvivalComponent::UpdateSurvivalStats,
    0.5f,   // Every 500ms — not every frame
    true
);

// Threat detection — sphere sweep max every 0.5s
GetWorld()->GetTimerManager().SetTimer(
    ThreatDetectionHandle,
    this,
    &USurvivalComponent::CheckNearbyThreats,
    0.5f,
    true
);
```

**Tick group:** If Tick is needed, use `TG_DuringPhysics` with `bCanEverTick = false` by default.  
**Override in constructor:** `PrimaryComponentTick.bCanEverTick = false;`

## Frame Budget (60fps = 16.67ms)

| System | Budget | Notes |
|--------|--------|-------|
| Render thread | 8ms | Lumen GI + shadows |
| Game thread | 4ms | Character + AI + survival |
| GPU | 12ms | Nanite + Lumen + VFX |
| SurvivalComponent tick | <0.1ms | Timer-based, not per-frame |
| DinosaurAI BehaviorTree | <2ms | 5 dinos × 0.4ms each |

## Next Performance Tasks
- [ ] Verify `SurvivalComponent::PrimaryComponentTick.bCanEverTick = false`
- [ ] Add LOD chain to dinosaur skeletal meshes (3 levels)
- [ ] Profile `PCGWorldGenerator` — ensure terrain gen is async
- [ ] Add `r.Nanite.Streaming.AsyncRasterization 1` when Nanite meshes added
