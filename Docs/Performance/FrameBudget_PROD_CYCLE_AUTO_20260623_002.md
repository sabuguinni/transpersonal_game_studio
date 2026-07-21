# Frame Budget — PROD_CYCLE_AUTO_20260623_002
**Agent #04 — Performance Optimizer**

## Target Platforms
| Platform | Target FPS | Frame Budget |
|----------|-----------|--------------|
| PC (High-end) | 60 fps | 16.67 ms |
| Console | 30 fps | 33.33 ms |

## Frame Budget Split (PC 60fps)
| System | Budget | Notes |
|--------|--------|-------|
| GPU (Lumen GI + Reflections) | 8.0 ms | Epic scalability, r.Shadow.MaxCSMResolution 2048 |
| CPU (AI + Physics) | 5.0 ms | Dino AI ticks at 10Hz, physics 2 substeps max |
| RHI | 2.0 ms | Async compute enabled (r.AsyncComputeBarriers 1) |
| Game (Survival/Crowd ticks) | 1.67 ms | SurvivalComp 10Hz, crowd 5Hz |
| **Total** | **16.67 ms** | **= 60 fps** |

## CVars Applied This Cycle
```ini
; Occlusion
r.HZBOcclusion=1
r.AllowOcclusionQueries=1
r.OcclusionQueryLocation=1

; Distance / LOD
r.ForceLOD=-1
r.ViewDistanceScale=1.0
foliage.MinimumScreenSize=0.0002
r.StaticMeshLODDistanceScale=1.0
r.SkeletalMeshLODBias=0
r.MaxAnisotropy=8

; Streaming
r.Streaming.PoolSize=2048
r.Streaming.MaxTempMemoryAllowed=50

; GC (no mid-combat hitches)
gc.TimeBetweenPurgingPendingKillObjects=60

; Physics (dino ragdolls)
p.MaxSubsteps=2
p.MaxPhysicsDeltaTime=0.033

; GPU Parallelism
r.AsyncComputeBarriers=1

; World Partition
wp.Runtime.UpdateStreamingPerFrame=4

; Shadows
r.Shadow.MaxCSMResolution=2048
r.Shadow.CSMDepthBias=10
r.Shadow.DistanceScale=1.0

; Scalability (Epic baseline for profiling)
sg.ResolutionQuality=100
sg.ViewDistanceQuality=3
sg.AntiAliasingQuality=3
sg.ShadowQuality=3
sg.GlobalIlluminationQuality=3
sg.ReflectionQuality=3
sg.PostProcessQuality=3
sg.TextureQuality=3
sg.EffectsQuality=3
sg.FoliageQuality=3
sg.ShadingQuality=3

; Sky
r.SkyAtmosphere.FastSkyLUT=1
r.SkyAtmosphere.AerialPerspectiveLUT.FastApply=1

; Lumen + Nanite
r.Lumen.Reflections.Allow=1
r.Lumen.GlobalIllumination.Allow=1
r.Nanite=1
```

## Actor CAP Limits (MinPlayableMap)
| Category | CAP | Reason |
|----------|-----|--------|
| Total actors | 150 | 60fps with full Lumen |
| Dynamic lights (Point/Spot/Rect) | 8 | Each costs ~0.5ms GPU |
| Dino AI actors | 10 | Each BT costs ~0.3ms CPU |
| Crowd agents (Mass AI) | 50,000 | Mass AI uses instancing |

## Tick Interval Policy
| Component | Tick Rate | Interval |
|-----------|-----------|----------|
| SurvivalComponent (hunger/thirst/stamina) | 10 Hz | 0.1s |
| DinoAI BehaviorTree | 10 Hz | 0.1s |
| CrowdSimulation (Mass AI) | 5 Hz | 0.2s |
| CharacterMovement | 60 Hz | 0.0167s (engine default) |
| WeatherSystem | 1 Hz | 1.0s |
| BiomeManager | 0.2 Hz | 5.0s |

## SurvivalComponent Stamina Budget
- Sprint drain: **10 stamina/sec** (depletes full bar in 10s of sprinting)
- Stamina regen: **5/sec** (takes 20s to fully recover)
- Sprint guard: only allowed if stamina > 5.0f (prevents micro-sprint spam)
- MaxWalkSpeed: 600 cm/s (walk), 900 cm/s (sprint) — within CPU budget

## Next Cycle Recommendations
1. **Agent #5 (World Generator)**: Use `r.Streaming.PoolSize 2048` — biome streaming cells should be ≤512MB each
2. **Agent #12 (Combat AI)**: Cap simultaneous combat-state dinos at 5 (each adds ~0.6ms BT evaluation)
3. **Agent #13 (Crowd)**: Mass AI agents are free — use instancing, not individual actors
4. **Agent #17 (VFX)**: Niagara emitters must use LOD chain: Full (≤10m), Medium (≤50m), Disabled (>100m)
