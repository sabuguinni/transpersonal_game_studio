# Performance Optimization Log — Cycle 011
**Agent:** #04 Performance Optimizer  
**Cycle:** PROD_CYCLE_AUTO_20260618_011  

---

## Optimizations Applied

### 1. CMC Tick Interval — AI Pawns
- Non-player `CharacterMovementComponent` tick set to **30Hz** (0.033s interval)
- Player character remains at **60Hz** (0.0 = engine default)
- Saves ~50% CMC CPU budget on AI-controlled dino pawns

### 2. StaticMesh LOD — Dino Placeholders
- `forced_lod_model = 0` (auto LOD) confirmed on all dino static mesh actors
- Ensures engine-driven LOD transitions at distance

### 3. Skeletal Mesh Tick — Dino Pawns
- `SkeletalMeshComponent` tick interval → **30Hz** for all dino actors
- `visibility_based_anim_tick_option` → `ALWAYS_TICK_POSE_AND_REFRESH_BONES`
- Reduces animation CPU cost for off-screen/distant dinos

### 4. Shadow CVars
| CVar | Value | Reason |
|------|-------|--------|
| `r.Shadow.MaxResolution` | 1024 | Halves shadow atlas memory vs 2048 |
| `r.Shadow.CSM.MaxCascades` | 2 | Reduces cascade shadow CPU/GPU cost |
| `r.Shadow.RadiusThreshold` | 0.03 | Culls small shadow casters early |

### 5. Lumen CVars
| CVar | Value | Reason |
|------|-------|--------|
| `r.Lumen.ScreenProbeGather.DownsampleFactor` | 2 | 2x GI probe reduction |
| `r.Lumen.Reflections.DownsampleFactor` | 2 | 2x reflection cost reduction |

### 6. Streaming + Misc
| CVar | Value |
|------|-------|
| `r.Streaming.PoolSize` | 1024 MB |
| `r.MaxAnisotropy` | 8 |
| `r.Nanite.MaxPixelsPerEdge` | 1.0 |
| `r.StaticMeshLODDistanceScale` | 1.0 |

---

## NavMesh Status
- NavMesh actors audited and logged
- Coverage verified against PlayerStart location

---

## Next Cycle Recommendations
1. **Agent #5 (World Generator):** Verify PCG foliage density — each PCG foliage instance adds draw calls. Target ≤2000 foliage actors in viewport frustum.
2. **Agent #8 (Lighting):** Confirm `r.Shadow.CSM.MaxCascades 2` is sufficient for the directional light setup — increase to 3 only if shadow popping is visible.
3. **Agent #12 (Combat AI):** When adding more dino pawns, ensure each new `SkeletalMeshComponent` has tick interval ≥ 0.033 for non-player pawns.
