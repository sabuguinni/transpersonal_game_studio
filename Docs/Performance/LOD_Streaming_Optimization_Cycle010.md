# LOD & Streaming Optimization — Cycle 010
**Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260624_010**

## Frame Budget Targets
| Platform | Target FPS | Frame Budget |
|----------|-----------|--------------|
| PC High-End | 60fps | 16.67ms |
| Console | 30fps | 33.33ms |

## Frame Budget Breakdown (PC 60fps)
| Pass | Budget |
|------|--------|
| Shadows (CSM 4 cascades, 2048 res) | 2.5ms |
| Lumen GI (RadianceCache 200 probes) | 3.0ms |
| Base Pass (Nanite enabled) | 4.0ms |
| Translucency (no front-layer reflections) | 1.5ms |
| Post Process | 1.5ms |
| Misc / CPU | 1.17ms |

## CVars Applied This Cycle
```ini
r.Shadow.MaxCSMResolution=2048
r.Shadow.CSM.MaxCascades=4
r.Shadow.RadiusThreshold=0.03
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget=200
r.Lumen.TranslucencyReflections.FrontLayer.Allow=0
r.Nanite=1
r.AllowOcclusionQueries=1
r.HZBOcclusion=1
r.Streaming.PoolSize=2048
r.TextureStreaming=1
r.SkeletalMeshLODBias=0
r.StaticMeshLODBias=0
```

## Dinosaur Performance Rules
- **URO (Update Rate Optimization)**: Enabled on all SkeletalMeshActors
- **Correct scales**: TRex=3.0, Raptor=1.5, Brachio=3.0, Trike=2.5
- **Cull distances**: Set per actor based on scale (small=50, medium=100 desired size)
- **Max dino actors**: 50 simultaneous with URO; 20 without

## Memory Budget
| Resource | Budget |
|----------|--------|
| Texture streaming pool | 2048 MB |
| Temp memory | 128 MB |
| Skeletal mesh actors | 50 max |
| Static mesh actors | 500 max (Nanite) |
| Dynamic point lights | 8 max |

## SurvivalComponent Integration (from Cycle 009)
The SurvivalComponent (hunger/thirst/stamina/fear) ticks at 0.5s intervals
to minimize CPU overhead. Stats degrade at realistic rates:
- Hunger: -0.5/min | Thirst: -1.0/min | Stamina: recovers +20/s at rest
- Fear: spikes near predators, decays -5/s when safe

## Next Agent (#05 — World Generator) Recommendations
1. Use World Partition for streaming — chunk size 12800 units
2. PCG foliage density: max 1000 instances per 10000m² cell
3. Landscape resolution: 505x505 per component (Nanite terrain)
4. Biome transitions: blend zone 2000 units minimum
5. River/water: use Water Plugin (not custom mesh) for performance
