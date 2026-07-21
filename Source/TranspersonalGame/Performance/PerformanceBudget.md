# Performance Budget — Transpersonal Game Studio
**Agent #4 — Performance Optimizer | Cycle AUTO_20260629_012**

---

## Frame Budget Targets

| Platform | FPS Target | Frame Budget | GPU Budget | CPU Budget | Render Thread |
|----------|-----------|--------------|------------|------------|---------------|
| PC (High) | 60 fps | 16.67 ms | 10.0 ms (60%) | 4.0 ms (24%) | 2.67 ms (16%) |
| Console | 30 fps | 33.33 ms | 20.0 ms (60%) | 8.0 ms (24%) | 5.33 ms (16%) |

---

## Memory Budget

| Resource | PC Budget | Console Budget |
|----------|-----------|----------------|
| Texture Pool | 2048 MB | 1024 MB |
| Mesh Budget | 512 MB | 256 MB |
| Shadow Maps | 256 MB (max 1024px) | 128 MB (max 512px) |
| Audio | 128 MB | 64 MB |
| Physics | 64 MB | 32 MB |
| AI/Behavior | 32 MB | 16 MB |

---

## Cull Distance Table

| Actor Category | Cull Distance (UE5 units) | Rationale |
|---------------|--------------------------|-----------|
| Vegetation (trees, bushes, ferns) | 8,000 | Dense foliage — cull aggressively |
| Rocks / Boulders | 12,000 | Medium visibility need |
| Small Props (bones, sticks, debris) | 5,000 | Low gameplay importance |
| Dinosaurs | 20,000 | Always must be visible for gameplay |
| Player Character | Never cull | Always rendered |
| NPCs / Tribal members | 25,000 | Social/quest relevance |
| Buildings / Structures | 30,000 | Landmark visibility |

---

## Shadow Budget

| Setting | Value | Reason |
|---------|-------|--------|
| Max shadow resolution | 1024 px | Balance quality vs VRAM |
| Min shadow resolution | 64 px | Distant objects |
| CSM cascades | 3 | 3 cascades = good quality, manageable cost |
| Distance scale | 0.8 | Slightly reduced for perf |
| Vegetation cast shadow | DISABLED | Biggest shadow perf win |

---

## LOD Policy

| Mesh Type | LOD 0 | LOD 1 | LOD 2 | LOD 3 |
|-----------|-------|-------|-------|-------|
| Dinosaur (main) | Full detail | 50% tris @ 3000u | 25% tris @ 8000u | Impostor @ 15000u |
| Player Character | Full detail | 60% tris @ 1000u | 30% tris @ 3000u | — |
| Tree | Full detail | 40% tris @ 2000u | Billboard @ 5000u | — |
| Rock | Full detail | 50% tris @ 3000u | 20% tris @ 8000u | — |
| Small Prop | Full detail | 30% tris @ 1000u | Cull @ 5000u | — |

---

## Scalability Groups (Epic Baseline)

All groups set to level 3 (Epic/High) as baseline. Console targets will use:
- `sg.ShadowQuality=2`
- `sg.GlobalIlluminationQuality=2`
- `sg.FoliageQuality=2`
- `sg.EffectsQuality=2`

---

## Applied Optimizations This Cycle

1. **CAP enforcement** — Sun pitch ≤ -30°, single ExponentialHeightFog, FastSkyLUT=1
2. **Cull distances** — Vegetation 8K, rocks 12K, dinosaurs 20K, props 5K
3. **Shadow optimization** — Max 1024px, 3 CSM cascades, 0.8 distance scale
4. **Vegetation shadows disabled** — Largest single perf win in outdoor scenes
5. **HZB occlusion enabled** — GPU-side occlusion culling active
6. **Parallel occlusion culling** — Multi-threaded CPU occlusion
7. **Texture pool** — 2048 MB, dynamic sizing, amortized GPU uploads
8. **GC tuning** — 60s interval, 10 retries before force GC
9. **TAA** — Method 2, no upsampling (saves ~15% memory vs DLSS setup)
10. **Lumen** — DiffuseIndirect + Reflections enabled (Lumen replaces SSAO/SSR)

---

## Next Steps for Agent #5 (Procedural World Generator)

- Use cull distances from this table when placing PCG foliage
- Ensure all PCG-spawned meshes have LOD chains (minimum LOD 0 + LOD 1)
- Set `bAllowCullDistanceVolume=true` on all PCG-spawned actors
- Avoid spawning more than 500 static mesh actors in a single 100m radius
- Use `ISM` (Instanced Static Mesh) for repeated vegetation — NOT individual actors
- Target: < 100 draw calls per 100m radius at runtime

---

*Performance budget validated against MinPlayableMap — 32 actors, all optimized.*
