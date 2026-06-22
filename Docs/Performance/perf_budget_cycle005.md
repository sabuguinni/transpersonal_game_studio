# Performance Budget Report — PROD_CYCLE_AUTO_20260622_005
**Agent #04 — Performance Optimizer**

## Frame Budget Targets
| Platform | Target FPS | Frame Budget |
|----------|-----------|--------------|
| PC (High) | 60 fps | 16.6 ms |
| Console | 30 fps | 33.3 ms |

## Actor Budget Limits (MinPlayableMap)
| Category | Budget | Rationale |
|----------|--------|-----------|
| StaticMesh actors | ≤ 150 | Batched draw calls via ISM where possible |
| SkeletalMesh actors | ≤ 20 | Each has full animation + physics cost |
| Dynamic lights (Point/Spot) | ≤ 8 | Each adds full shadow map pass |
| Niagara particle systems | ≤ 10 | GPU particle budget |
| NavMesh volumes | ≤ 3 | Recast build cost |

## CVars Applied This Cycle
```ini
r.Shadow.MaxResolution=1024
r.Shadow.RadiusThreshold=0.03
r.Streaming.PoolSize=1024
r.LOD.ForcedLODModel=-1
r.HZBOcclusion=1
r.OcclusionQueryLocation=1
r.AllowOcclusionQueries=1
r.Streaming.UseFixedPoolSize=1
r.Streaming.MaxTempMemoryAllowed=50
r.SkyAtmosphere.FastSkyLUT=1
r.SkyAtmosphere.AerialPerspectiveLUT.FastApply=1
```

## LOD Strategy
- All StaticMesh actors: `forced_lod_model=0` (auto LOD selection)
- Max draw distance: 50,000 UU (~500m) — beyond this actors cull
- Fill lights (PointLight/SpotLight): shadows DISABLED, shadow_resolution_scale=0.5
- Key light (DirectionalLight): pitch=-45°, intensity=10.0, shadows ON

## Survival Tick Cost Estimate (TranspersonalCharacter)
- Stamina update: O(1) per frame — ~0.001ms
- Survival tick (hunger/thirst): every 5s interval — ~0.002ms amortised
- Fear speed modifier: O(1) per frame — negligible
- **Total survival overhead: < 0.01ms/frame** ✅ well within budget

## Shadow Optimisation Rules
1. DirectionalLight: 1 only, cascaded shadow maps (CSM), max 4 cascades
2. PointLight/SpotLight: NO dynamic shadows (use baked or stationary)
3. Dinosaur pawns: shadow casting ON (key gameplay visibility)
4. Foliage/rocks: shadow casting OFF (use contact shadows instead)

## Next Performance Priorities
1. Profile dinosaur AI tick cost when BehaviorTree is active (target: <2ms for 5 dinos)
2. Implement LOD chain for dinosaur skeletal meshes (LOD0=full, LOD1=50%, LOD2=25%)
3. Enable Nanite for landscape/terrain when mesh complexity > 500k tris
4. Profile PCGWorldGenerator spawn cost (target: <100ms one-time, not per-frame)
5. Implement object pooling for projectiles/debris when combat system is added
