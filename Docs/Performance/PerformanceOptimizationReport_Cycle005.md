# Performance Optimization Report — Agent #04 — Cycle AUTO_20260703_005

## Executive Summary
Performance baseline established and optimizations applied to the MinPlayableMap hub clearing (X=2100, Y=2400). All console commands queued via UE5 Remote Control. Target: 60fps PC / 30fps console.

---

## 1. Physics Budget Caps Applied

| Console Command | Value | Rationale |
|---|---|---|
| `p.MaxPhysicsSubsteps` | 2 | Prevents physics spiral on complex collisions |
| `p.MaxSimulatedBodies` | 32 | Hard cap on simultaneous rigid body simulations |
| `p.RigidBodyLODThresholdScaleFactor` | 1.5 | Reduces physics fidelity on distant bodies |

**Expected savings**: ~15% CPU on physics thread in dense dino encounters.

---

## 2. Collision Profile Optimization

All dinosaur StaticMeshComponents within 5000u of hub:
- Collision profile: `BlockAll` (consistent, no per-component overrides)
- `simulate_physics = False` (kinematic display pose — correct for static dinos)
- LOD bias: 0 (full quality at hub center)

**Recommendation for next cycle**: Switch to `BlockAll_Simple` (convex hull) for ~40% additional CPU savings on collision queries. Requires mesh import with convex hull collision generated.

---

## 3. Rendering Performance Settings

### Shadow Budget
| Command | Value |
|---|---|
| `r.Shadow.DistanceScale` | 0.8 |
| `r.Shadow.MaxResolution` | 1024 |
| `r.Shadow.RadiusThreshold` | 0.03 |

### Lumen GI
| Command | Value |
|---|---|
| `r.Lumen.DiffuseIndirect.Allow` | 1 |
| `r.Lumen.Reflections.Allow` | 1 |
| `r.Lumen.TraceMeshSDFs` | 1 |
| `r.Lumen.MaxTraceDistance` | 20000 |
| `r.Lumen.SceneDetail` | 1.0 |

### Nanite
| Command | Value |
|---|---|
| `r.Nanite.MaxPixelsPerEdge` | 1.0 |
| `r.Nanite.ProxyRenderMode` | 0 |

### Occlusion Culling
| Command | Value |
|---|---|
| `r.HZBOcclusion` | 1 |
| `r.OcclusionQueryLocation` | 1 |

### Distance Field AO
| Command | Value |
|---|---|
| `r.DistanceFieldAO` | 1 |
| `r.AOMaxViewDistance` | 20000 |

---

## 4. LOD Chain Recommendations

### Dinosaur LOD Distances (by size tier)

| Species | LOD0 | LOD1 | LOD2 | Cull |
|---|---|---|---|---|
| T-Rex, Brachiosaurus (Large) | 0–3000u | 3000–8000u | 8000–12000u | 15000u |
| Raptor, Triceratops (Medium) | 0–2000u | 2000–5000u | 5000–8000u | 10000u |
| Small fauna | 0–1000u | 1000–3000u | 3000–5000u | 6000u |

### Foliage LOD
- LOD bias set to 1 for all foliage actors (prefer lower-poly mesh)
- Recommend using Hierarchical Instanced Static Mesh (HISM) for trees/ferns at hub

---

## 5. Character Movement Speed Verification

Target values from GDD:
| Stat | Target | Unit |
|---|---|---|
| Max Walk Speed | 600 | cm/s |
| Max Run Speed | 900 | cm/s |
| Jump Z Velocity | 600 | cm/s |
| Ground Friction | 8.0 | — |
| Braking Deceleration | 2048 | cm/s² |

These values should be set on `TranspersonalCharacter` → `CharacterMovementComponent`.

---

## 6. Frame Budget Allocation (60fps target = 16.6ms)

| System | Budget | Notes |
|---|---|---|
| GPU Render | 8.0ms | Lumen GI + shadows |
| CPU Game Thread | 3.0ms | Actor ticks, AI, input |
| CPU Physics | 2.0ms | Capped at 32 bodies, 2 substeps |
| CPU Render Thread | 2.0ms | Draw call submission |
| Audio | 0.5ms | MetaSounds budget |
| Overhead/Margin | 1.1ms | Safety buffer |

---

## 7. Next Cycle Handoff (Agent #05 — Procedural World Generator)

### Priority actions:
1. **HISM conversion**: Replace individual Tree/Fern/Rock StaticMeshActors with HISM clusters — expected 60-70% draw call reduction for foliage
2. **World Partition**: Enable World Partition with 5000u cell size for streaming
3. **PCG Graph**: Use PCG to scatter foliage procedurally with built-in LOD support
4. **Biome density caps**: Max 500 foliage instances per 10000u² cell
5. **Collision LOD**: Use `BlockAll_Simple` for all procedural rocks/terrain features

### Performance contracts for world generation:
- Max draw calls at hub: 800 (current estimate: ~400, headroom for more content)
- Max foliage instances visible: 2000 (HISM batched)
- Terrain must use Nanite-enabled Landscape or Nanite Static Mesh
- No dynamic lights beyond the main DirectionalLight + SkyLight

---

## Validation Checklist

- [x] Bridge validated (command_id 27583)
- [x] CAP enforcement: sun pitch -35°, fog dedup, FastSkyLUT=1, SkyLight real_time_capture
- [x] Hub actor audit completed
- [x] LOD bias applied to dino and foliage components
- [x] Physics budget caps: MaxSubsteps=2, MaxSimBodies=32
- [x] Shadow budget: MaxRes=1024, DistanceScale=0.8
- [x] Lumen tuned: MaxTrace=20000, SceneDetail=1.0
- [x] Nanite: MaxPixelsPerEdge=1.0
- [x] Occlusion: HZB=1
- [x] DFAO: MaxViewDist=20000
- [x] Level saved

**Status: PASS — Performance baseline established. No regressions detected.**
