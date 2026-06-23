# Performance Budget — Cycle 009
**Agent #04 — Performance Optimizer**
**Date:** PROD_CYCLE_AUTO_20260623_009

---

## Target Framerate
| Platform | Target FPS | Budget (ms/frame) |
|----------|-----------|-------------------|
| PC High  | 60 fps    | 16.67 ms          |
| Console  | 30 fps    | 33.33 ms          |

---

## Console Variables Applied (MinPlayableMap)

### Shadow Quality
```
r.Shadow.MaxResolution 2048
r.Shadow.CSM.MaxCascades 3
r.Shadow.DistanceScale 0.8
r.Shadow.RadiusThreshold 0.03
```

### TSR (Temporal Super Resolution)
```
r.TemporalAA.Upsampling 1
r.TSR.History.ScreenPercentage 200
r.ScreenPercentage 85
```
> 85% internal resolution + TSR upscale = near-native quality at ~20% GPU cost reduction.

### Lumen GI
```
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.Reflections.Allow 1
r.Lumen.TraceMeshSDFs.Allow 1
r.Lumen.MaxTraceDistance 20000
r.Lumen.SceneDetail 1.0
```

### GC & Streaming
```
gc.MaxObjectsNotConsideredByGC 1
s.AsyncLoadingThreadEnabled 1
s.LevelStreamingActorsUpdateTimeLimit 5.0
```

### LOD
```
foliage.LODDistanceScale 1.5
r.StaticMeshLODDistanceScale 1.0
r.SkeletalMeshLODBias 0
r.ForceLOD -1
```

### Occlusion
```
r.HZBOcclusion 1
r.OcclusionQueryLocation 1
```

### Nanite
```
r.Nanite 1
r.Nanite.MaxPixelsPerEdge 1.0
```

### Scalability (PC High)
```
sg.ShadowQuality 3
sg.TextureQuality 3
sg.EffectsQuality 3
sg.PostProcessQuality 3
sg.FoliageQuality 3
```

### Scalability (Console 30fps fallback)
```
sg.ShadowQuality 2
sg.TextureQuality 2
sg.EffectsQuality 2
sg.PostProcessQuality 2
sg.FoliageQuality 2
```

---

## Draw Call Budget

| Category              | Budget (draw calls) | Notes                          |
|-----------------------|--------------------|---------------------------------|
| Static Meshes (world) | ≤ 800              | Use ISM for 3+ identical meshes |
| Skeletal Meshes (dinos)| ≤ 20              | 5 dinos × 4 LODs max           |
| Foliage (HISM)        | ≤ 200              | Hierarchical ISM via PCG        |
| Lights (dynamic)      | ≤ 4                | 1 sun + 3 point lights max      |
| Particles (Niagara)   | ≤ 50               | GPU particles preferred         |
| UI                    | ≤ 30               | HUD elements                    |
| **TOTAL**             | **≤ 1100**         |                                 |

---

## ISM Conversion Candidates
Any static mesh type with **3+ instances** in the level MUST use Instanced Static Mesh (ISM) or Hierarchical ISM (HISM).

- Trees (placeholder cylinders): convert to HISM via PCG
- Rocks (placeholder boxes): convert to HISM
- Biome boundary markers: convert to ISM

**Expected draw call reduction:** ~60-70% for foliage/rocks.

---

## Memory Budget

| Asset Type       | Budget  |
|-----------------|---------|
| Textures        | ≤ 2 GB  |
| Static Meshes   | ≤ 512 MB|
| Skeletal Meshes | ≤ 256 MB|
| Audio           | ≤ 128 MB|
| **Total GPU**   | **≤ 4 GB** |

---

## Character LOD Chain (TranspersonalCharacter)
| LOD | Distance (cm) | Triangles | Notes                    |
|-----|--------------|-----------|--------------------------|
| 0   | 0–500        | ~15,000   | Full detail              |
| 1   | 500–1500     | ~8,000    | Reduced fingers/face     |
| 2   | 1500–4000    | ~3,000    | Silhouette only          |
| 3   | 4000+        | ~800      | Billboard or cull        |

---

## Dinosaur LOD Chain
| LOD | Distance (cm) | Triangles | Notes                    |
|-----|--------------|-----------|--------------------------|
| 0   | 0–1000       | ~25,000   | Full detail + animations |
| 1   | 1000–3000    | ~12,000   | Reduced detail           |
| 2   | 3000–8000    | ~4,000    | Simplified mesh          |
| 3   | 8000+        | ~1,000    | Imposter/billboard       |

---

## Next Steps for Agent #05 (Procedural World Generator)
1. Use **HISM** (Hierarchical Instanced Static Mesh) for ALL foliage placement — never raw StaticMeshActors for trees/rocks
2. Set **`r.Nanite 1`** on all high-poly rock/terrain meshes
3. World Partition cell size: **12800 cm** (128m) — matches streaming budget
4. Max actors per streaming cell: **200** — enforced by PCG density settings
5. Avoid overlapping Lumen scene proxy bounds — keep large meshes in separate cells
