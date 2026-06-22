# Performance Budget — Cycle 012
**Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260622_012**

## Frame Budget Targets
| Platform | Target FPS | Frame Budget | GPU | CPU Game | CPU Render |
|----------|-----------|-------------|-----|----------|------------|
| PC High-End | 60 fps | 16.7 ms | 10 ms | 4 ms | 2.7 ms |
| Console | 30 fps | 33.3 ms | 20 ms | 8 ms | 5 ms |

## Scalability Tiers Applied
| Setting | Low (0) | Medium (1) | High (2) | Epic (3) |
|---------|---------|-----------|---------|---------|
| Shadow MaxRes | 256 | 512 | 1024 | **2048** |
| CSM Cascades | 1 | 2 | 3 | **4** |
| Lumen GI | OFF | OFF | ON (0.8) | **ON (1.0)** |
| Lumen Reflections | OFF | OFF | ON (Q0) | **ON (Q1)** |
| Texture Pool | 512 MB | 1024 MB | 1536 MB | **2048 MB** |
| Foliage Density | 0% | 40% | 70% | **100%** |
| Grass Density | 0% | 30% | 60% | **100%** |
| Particles LOD Bias | 3 | 2 | 1 | **0** |

## Optimisations Applied This Cycle
- **LOD**: StaticMesh=1.0, SkeletalMesh=0, Foliage=1.0
- **Occlusion**: HZB=1, OcclusionQueries=1
- **Shadows**: MaxRes=2048, CSM=4, DistScale=1.0
- **Texture Streaming**: Pool=2048 MB, MaxAnisotropy=8
- **Async Loading**: 5ms limit + 15ms priority extra time
- **Sky**: FastSkyLUT=1, AerialPerspectiveLUT FastApply=1
- **Screen %**: 100 (Epic quality)
- **Motion Blur**: Quality=2
- **Depth of Field**: Quality=2
- **AO**: MaxQuality=1

## Prehistoric Survival Game Notes
- **Foliage density** kept at 1.0 for Epic — dense prehistoric jungle/forest biomes require full density
- **Lumen GI** enabled at Epic for realistic cave/forest lighting with dinosaurs
- **CSM 4 cascades** required for large open-world prehistoric maps (view distances >10km)
- **Async loading** tuned for large dinosaur skeletal meshes streaming in/out of World Partition cells
- **Texture pool 2048 MB** supports 4K textures on dinosaur characters + environment

## Cycle History
| Cycle | Key Change |
|-------|-----------|
| 009 | Initial PerformanceOptimizer.cpp + DefaultScalability.ini |
| 010 | stat unit/fps baseline, scalability sg.*=3 Epic confirmed |
| 011 | Epic quality baseline, Lumen ON, Nanite ON validated |
| **012** | Full 4-tier scalability config, memory audit, LOD/shadow/occlusion/texture optimisations |

## Next Cycle Recommendations (Agent #05 — Procedural World Generator)
- Apply `foliage.DensityScale` and `grass.DensityScale` from DefaultScalability.ini in PCG biome configs
- Use World Partition with streaming cell size ≥ 128m for prehistoric open-world
- Ensure PCG-generated foliage uses LOD chains (3 levels minimum)
- Terrain heightmap resolution: 4033×4033 for 8km² maps
