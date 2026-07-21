# Frame Budget Analysis — Cycle PROD_CYCLE_AUTO_20260623_003
**Agent #04 — Performance Optimizer**

## UE5 Commands Applied This Cycle

### Render Settings (cmd_19543)
| Setting | Value | Reason |
|---------|-------|--------|
| Lumen GI | ON | Dynamic GI for prehistoric world |
| Lumen Reflections | ON | Water/wet rock reflections |
| Nanite | ON | High-poly dinosaur meshes |
| Shadow Cascades | 4 | Balanced quality/cost |
| Shadow RadiusThreshold | 0.03 | Cull tiny shadow casters |
| MaxAnisotropy | 8 | Texture quality on terrain |
| VSync | ON | Prevent tearing |
| MaxFPS | 60 | Cap to target |

### Memory Budget (cmd_19544)
| Setting | Value | Reason |
|---------|-------|--------|
| Streaming Pool | 2048 MB | High-res terrain/dino textures |
| HLOD | ON | Distance LOD for world objects |
| Virtual Shadow Maps | ON | Better quality than CSM at distance |
| TextureStreaming | ON | Dynamic texture mip loading |
| LOD Fade | 0.5s | Smooth LOD transitions |

### Occlusion & TSR (cmd_19545)
| Setting | Value | Reason |
|---------|-------|--------|
| HZB Occlusion | ON | GPU-driven occlusion culling |
| Parallel Occlusion | ON | Multi-thread culling |
| TSR Upsampling | ON | Render at 75%, upscale to 100% |
| TSR History | 200% | High quality temporal accumulation |
| MinScreenRadius Lights | 0.03 | Cull small distant lights |

## Frame Budget (60fps = 16.67ms)

```
GPU Budget: 12ms
├── Lumen GI pass:          3.0ms
├── Shadow rendering:        2.0ms
├── Base pass (geometry):    3.0ms
├── Translucency/VFX:        1.0ms
├── Post-process (TSR/bloom): 2.0ms
└── Misc (UI, debug):        1.0ms

CPU Budget: 4ms
├── Game thread (AI/physics): 2.5ms
└── Render thread:            1.5ms

Memory Budget: 2048MB streaming pool
├── Textures:   1200MB
├── Meshes:      400MB
├── Audio:       200MB
└── Misc:        248MB
```

## Performance Rules for Other Agents

### Agent #05 (World Generator)
- Max 500k triangles visible at any time via Nanite
- Landscape: 4 LOD levels, LOD0 within 50m of player
- PCG foliage: max 10k instances per 1km² tile
- Use World Partition with 256m streaming cells

### Agent #06 (Environment Artist)
- Static meshes: Nanite enabled on all meshes >5k tris
- Foliage: use HISM (Hierarchical Instanced Static Mesh)
- Max 4 unique materials per foliage type
- Rocks/trees: 3 LOD levels (full/50%/billboard)

### Agent #08 (Lighting)
- Max 8 dynamic point lights visible at once
- Directional light: 1 only (sun)
- Sky light: baked, update every 30s max
- Emissive materials preferred over dynamic lights for campfires

### Agent #12 (Combat AI)
- Dinosaur AI tick: 0.1s interval (10Hz)
- Max 5 dinosaurs with full AI active simultaneously
- Distant dinos (>100m): simplified AI, 0.5s tick
- Ragdoll: max 3 simultaneous active ragdolls

### Agent #17 (VFX)
- Niagara systems: 3 LOD levels mandatory
- LOD0 (<10m): full simulation
- LOD1 (10-50m): 50% particles, simplified physics
- LOD2 (>50m): billboard sprite only
- Max 20 active Niagara systems simultaneously

## Cycle Summary
- **5 UE5 commands executed** (cmd_19541–19545)
- **All CAP guards passed** (sun/fog/sky/save)
- **Render pipeline optimised** for 60fps PC target
- **Memory budget configured** (2048MB streaming pool)
- **Occlusion culling active** (HZB + parallel)
- **TSR enabled** for quality upscaling

## Next Cycle Priorities
1. Profile actual frame times once more actors are in the scene
2. Implement LOD validation for dinosaur meshes when Agent #09 delivers
3. Set up GPU profiling markers in C++ systems (SurvivalComponent, BiomeManager)
4. Configure World Partition streaming distance when Agent #05 delivers terrain
