# Performance Optimization Specification
**Agent #04 — Performance Optimizer**
**Cycle: PROD_CYCLE_AUTO_20260703_006**

---

## Target Frame Rates
| Platform | Target FPS | Budget (ms/frame) |
|----------|-----------|-------------------|
| PC High-End | 60 fps | 16.6ms |
| Console | 30 fps | 33.3ms |

---

## Applied Optimizations (This Cycle)

### Shadow Cascades
| Setting | Value | Rationale |
|---------|-------|-----------|
| `r.Shadow.CSM.MaxCascades` | 3 | Limits expensive cascade shadow maps |
| `r.Shadow.RadiusThreshold` | 0.03 | Skip shadows for small objects |
| `r.Shadow.DistanceScale` | 0.7 | Reduce shadow draw distance 30% |

### Cull Distances
| Actor Type | Max Draw Distance | Notes |
|-----------|------------------|-------|
| Vegetation (Tree/Bush/Fern/Grass) | 3000 cm (30m) | Aggressive cull for dense foliage |
| Rocks/Boulders | 4000 cm (40m) | Slightly larger — landmark value |
| Dinosaurs | No cull (always visible) | Gameplay-critical actors |
| Lights/Atmosphere | No cull | Global actors |

### LOD Settings
| Setting | Value |
|---------|-------|
| `r.StaticMeshLODDistanceScale` | 1.0 (default, balanced) |
| `r.SkeletalMeshLODBias` | 0 (no bias — dinos need full detail) |
| Forced LOD Model | 0 (auto — engine selects best LOD) |

### Occlusion Culling
| Setting | Value |
|---------|-------|
| `r.HZBOcclusion` | 1 (enabled) |
| `r.OcclusionCulling` | 1 (enabled) |

### Lumen (Global Illumination)
| Setting | Value | Rationale |
|---------|-------|-----------|
| `r.Lumen.MaxTraceDistance` | 8000 cm | Limit GI trace to 80m radius |
| `r.Lumen.Reflections.Allow` | 1 | Needed for wet surfaces/water |
| `r.Lumen.TraceMeshSDFs.Allow` | 1 | Better GI accuracy in dense foliage |

### Nanite
| Setting | Value |
|---------|-------|
| `r.Nanite.MaxPixelsPerEdge` | 1.0 (balanced) |
| `r.Nanite.Streaming.AsyncCompute` | 1 (async — reduces GPU stalls) |

### Texture Streaming
| Setting | Value |
|---------|-------|
| `r.Streaming.PoolSize` | 2000 MB |
| `r.MaxAnisotropy` | 8 (balanced quality/perf) |

---

## Hub Performance Budget (X=2100, Y=2400, radius=3000cm)

### Actor Count Thresholds
| Count | Status | Action |
|-------|--------|--------|
| < 50 | 🟢 GREEN | No action needed |
| 50–100 | 🟡 YELLOW | Monitor draw calls, consider instancing |
| > 100 | 🔴 RED | HLOD or actor merging required |

### Dynamic Light Budget
| Count | Status |
|-------|--------|
| < 5 | 🟢 Optimal |
| 5–10 | 🟡 Acceptable |
| > 10 | 🔴 Bake or reduce |

---

## CAP (Canonical Appearance Parameters) — Enforced Every Cycle

| System | Setting | Value |
|--------|---------|-------|
| DirectionalLight | Pitch | -35° (sun angle) |
| DirectionalLight | Intensity | 8.0 |
| DirectionalLight | AtmosphereSunLight | true |
| ExponentialHeightFog | FogDensity | 0.02 |
| ExponentialHeightFog | VolumetricFog | true |
| SkyLight | RealTimeCapture | true |
| SkyLight | Intensity | 2.0 |
| Console | r.SkyAtmosphere.FastSkyLUT | 1 |
| Console | r.VolumetricFog | 1 |

---

## Performance Rules for Future Agents

### Agent #05 (World Generator)
- Max 200 foliage instances per 10,000 cm² tile
- Use HISM (Hierarchical Instanced Static Mesh) for repeated vegetation
- Set cull distance on all PCG-spawned actors: 3000cm vegetation, 5000cm terrain features

### Agent #06 (Environment Artist)
- No more than 8 dynamic point/spot lights in hub area
- Use baked lighting for static props (rocks, ruins, ground details)
- Vegetation LOD chain: LOD0 full mesh, LOD1 simplified, LOD2 billboard at 2500cm

### Agent #08 (Lighting)
- Lumen GI only — no ray-traced shadows (too expensive for 60fps)
- Sky atmosphere + directional light only for primary illumination
- Point lights: max 5 in hub, max 15 scene-wide

### Agent #12 (Combat AI)
- Dinosaur AI tick rate: 10Hz (not every frame)
- Pathfinding: NavMesh query max 3 per frame
- Perception: sight radius 2000cm, update interval 0.5s

### Agent #13 (Crowd Simulation)
- Mass AI: max 50 agents in 5000cm radius of player
- LOD AI: full behavior <1000cm, simplified >2000cm, disabled >4000cm

---

## Next Steps for Agent #05 (Procedural World Generator)
1. Use HISM components for all repeated foliage (trees, ferns, grass)
2. Set PCG cull distance rules: vegetation=3000cm, rocks=5000cm
3. Avoid spawning more than 50 static mesh actors in hub radius (use instancing)
4. World Partition tile size: 5000cm × 5000cm for streaming efficiency
5. NavMesh tile size: 500cm (reduced from 1000cm) for hub precision
