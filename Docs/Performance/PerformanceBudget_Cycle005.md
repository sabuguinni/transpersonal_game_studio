# Performance Budget Report — Cycle 005
**Agent:** #04 Performance Optimizer  
**Cycle:** PROD_CYCLE_AUTO_20260617_005  
**Date:** 2026-06-17

---

## Frame Budget (PC Target: 60fps = 16.6ms)

| Category | Estimated Cost | Budget Limit | Status |
|---|---|---|---|
| Point Lights (dynamic) | ~0.08ms each | Max 40 lights = 3.2ms | OK |
| Static Meshes | ~0.02ms each | Max 200 = 4.0ms | OK |
| Skeletal Meshes | ~0.15ms each | Max 20 = 3.0ms | OK |
| Shadows (point lights) | 0ms (disabled on markers) | — | OPTIMIZED |
| **Total Estimated** | **~7ms render** | **16.6ms budget** | **OK** |

---

## LOD Configuration Applied

### Draw Distance Rules
| Actor Type | Max Draw Distance | Rationale |
|---|---|---|
| Trees / Rocks / Bushes | 8000 units | Visible at gameplay range |
| Terrain / Ground | Unlimited (0) | Always visible |
| Generic Props | 5000 units | Mid-range culling |
| Performance Markers | 150 unit radius | Debug only |

### Shadow Optimization
- All `PhysMat_*` marker PointLights: **shadows DISABLED**
- All `PerfZone_*` marker PointLights: **shadows DISABLED**
- All `FPS_Monitor_*` PointLights: **shadows DISABLED**
- Intensity reduced to 100-200 (from default 5000)

---

## Memory Budget (PC Target: 8GB VRAM)

| Component | Estimated Usage | Budget |
|---|---|---|
| Actor transform/tick overhead | ~0.5MB | 512MB limit |
| Static mesh instances | ~10MB | 2048MB limit |
| Texture streaming (estimated) | ~512MB | 4096MB limit |
| **Total Estimated** | **~522MB** | **8192MB** | 
| **Remaining** | **~7670MB** | — |

**Status: GREEN — well within budget for current scene complexity**

---

## Performance Infrastructure Deployed

### FPS Monitoring Waypoints (3 actors)
| Label | Location | Color | Purpose |
|---|---|---|---|
| `FPS_Monitor_Spawn` | (0, 0, 300) | Green | Player spawn area profiling |
| `FPS_Monitor_Savanna` | (5000, 0, 300) | Yellow | Open savanna profiling |
| `FPS_Monitor_Forest` | (-3000, 4000, 300) | Dark Green | Dense foliage profiling |

### DinoZone Trigger Volumes (from Cycle 004)
- 5x `DinoZone_*` TriggerBox actors around dinosaur placeholders
- Scale: 3x3x2 units — danger radius detection for fear system

### Biome Physics Markers (from Cycle 004)
- 5x `PhysMat_*_Marker` PointLights at biome centers
- Colors: Savanna=orange, Forest=green, Swamp=teal, Mountain=grey, River=blue

### PerfZone LOD Markers (from Cycle 002-004)
- Multiple `PerfZone_*` actors for LOD boundary visualization

---

## Recommendations for Next Cycles

### P1 — Critical (implement before 50+ actors added)
1. **Instanced Static Mesh (ISM)** for trees/rocks — replace individual StaticMeshActors with HISM
   - Expected gain: 60-80% draw call reduction for vegetation
   - Threshold: >20 identical meshes → use ISM

2. **DinoZone distance culling** — disable overlap detection >2000 units from player
   - Prevents unnecessary collision queries in background
   - Implementation: Blueprint tick check `GetDistanceTo(PlayerPawn) > 2000 → SetActorEnableCollision(false)`

3. **Survival stat tick batching** — batch all survival stat updates to 1Hz
   - Current risk: if stats tick per-frame at 60fps = 60x unnecessary updates
   - Fix: `PrimaryActorTick.TickInterval = 1.0f` on survival component

### P2 — Important (before Alpha)
4. **Ragdoll cap** — max 3 simultaneous ragdoll simulations
   - Implementation: `RagdollManager` with priority queue (closest to player = active)
   
5. **Spatial hash for biome lookup** — replace linear scan with 2D grid hash
   - Grid size: 1000x1000 units per cell
   - Expected: O(1) biome lookup vs O(n) scan

6. **Texture streaming budget** — set `r.Streaming.PoolSize 2048` for 8GB VRAM systems

### P3 — Future (before Beta)
7. **GPU Occlusion Culling** — enable `r.HZBOcclusion 1` for dense scenes
8. **Nanite** for high-poly rocks/terrain (UE5.5 feature)
9. **Lumen** settings: `r.Lumen.Reflections.Allow 0` in distant biomes

---

## Console Commands for Runtime Profiling

```
stat unit          — Frame time breakdown (Game/Draw/GPU)
stat fps           — FPS counter
stat memory        — Memory usage
stat streaming     — Texture streaming stats
profilegpu         — GPU frame capture
r.VisualizeOccludedPrimitives 1  — Show culled actors
```

---

## Performance Constraints Summary (for all agents)

| System | Hard Limit | Soft Limit |
|---|---|---|
| Total actors in scene | 8000 | 5000 |
| Dinosaur pawns (active AI) | 150 | 80 |
| Dynamic point lights | 40 | 20 |
| Simultaneous ragdolls | 3 | 2 |
| Particle systems (Niagara) | 50 | 30 |
| Survival stat tick rate | 1Hz | 2Hz |
| Biome lookup time | <0.1ms | <0.05ms |

*These limits ensure 60fps on PC (RTX 3070+) and 30fps on console (PS5/XSX)*
