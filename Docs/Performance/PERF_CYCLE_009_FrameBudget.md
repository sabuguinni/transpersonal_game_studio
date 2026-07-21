# Performance Frame Budget — PROD_CYCLE_AUTO_20260618_009
**Agent #04 — Performance Optimizer**

## CVars Applied This Cycle

### Shadow Budget
| CVar | Value | Rationale |
|------|-------|-----------|
| `r.Shadow.MaxResolution` | `1024` | Halves shadow map VRAM vs default 2048 |
| `r.Shadow.CSM.MaxCascades` | `2` | 2 cascades sufficient for outdoor prehistoric world |
| `r.Shadow.RadiusThreshold` | `0.03` | Culls tiny shadow casters |
| `r.Shadow.DistanceScale` | `0.6` | Reduces shadow draw distance 40% |

### Sky & Atmosphere
| CVar | Value | Rationale |
|------|-------|-----------|
| `r.SkyAtmosphere.FastSkyLUT` | `1` | Fast LUT path — negligible quality loss |
| `r.SkyAtmosphere.SampleCountMax` | `14` | Reduced from 32 default |

### Lumen (Global Illumination)
| CVar | Value | Rationale |
|------|-------|-----------|
| `r.Lumen.DiffuseIndirect.Allow` | `1` | Keep Lumen GI active |
| `r.Lumen.Reflections.Allow` | `1` | Keep Lumen reflections |
| `r.Lumen.TraceMeshSDFs` | `1` | SDF tracing for accuracy |
| `r.DynamicGlobalIlluminationMethod` | `1` | Lumen method |
| `r.ReflectionMethod` | `1` | Lumen reflections |

### LOD & Culling
| CVar | Value | Rationale |
|------|-------|-----------|
| `foliage.LODDistanceScale` | `0.7` | LOD transitions 30% closer — less poly at distance |
| `r.StaticMeshLODDistanceScale` | `0.8` | Rocks/props LOD earlier |
| `r.SkeletalMeshLODBias` | `0` | Dinos keep full LOD chain |
| `r.MaxAnisotropy` | `8` | Texture quality balanced |

## LOD Enforcement Rules

- **Foliage/rocks beyond 5000 units** → `MaxDrawDistance = 8000` applied
- **All skeletal mesh actors (dinos)** → `ForcedLODModel = 0` (auto-LOD)
- **Static mesh actors** → `ForcedLODModel = 0` (auto-LOD)

## Frame Budget Targets

| Platform | Target FPS | Frame Time Budget |
|----------|-----------|-------------------|
| PC High-End | 60 fps | 16.6ms |
| Console | 30 fps | 33.3ms |

### Budget Breakdown (16.6ms @ 60fps)
| System | Budget | Notes |
|--------|--------|-------|
| CPU Game Thread | 4ms | Character, AI, physics |
| CPU Render Thread | 5ms | Draw calls, state changes |
| GPU | 7ms | Shading, shadows, Lumen |
| OS/Driver overhead | 0.6ms | Unavoidable |

## Profiling Commands Active
```
stat fps
stat unit
stat game
stat gpu
stat scenerendering
stat memory
stat streaming
```

## Laws Enforced
1. **Shadow caps** — MaxResolution 1024, CSM 2 cascades
2. **LOD bias** — foliage 0.7x, staticmesh 0.8x distance scale
3. **Culling** — MaxDrawDistance 8000 on distant foliage/rocks
4. **Lumen** — Active with SDF tracing, fast sky LUT
5. **NavMesh** — P.RebuildNavigation triggered
6. **Map saved** — `/Game/Maps/MinPlayableMap`

## Next Cycle Priorities
- Verify `stat unit` output — confirm GPU < 7ms, CPU < 5ms
- Audit any actors with `bCastDynamicShadow=true` beyond 3000 units → disable
- Check foliage instance count — HISM actors with >10k instances need LOD chain
- Confirm `BP_TranspersonalCharacter` is set as DefaultPawnClass in WorldSettings
- Test WASD movement in PIE (Play In Editor)
