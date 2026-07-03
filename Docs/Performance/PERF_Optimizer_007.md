# Performance Optimizer — Cycle 007 Report
**Agent**: #04 — Performance Optimizer  
**Cycle**: PROD_CYCLE_AUTO_20260703_007  
**Date**: 2026-07-03  

---

## 1. Bridge & CAP Status

| Check | Result |
|---|---|
| Bridge validation | ✅ OK (cmd 27729) |
| Sun pitch guard | ✅ -35° enforced, intensity=8.0, atmosphere_sun_light=True |
| Fog deduplication | ✅ 1 ExponentialHeightFog, density=0.02, volumetric_fog=True |
| SkyLight | ✅ real_time_capture=True, intensity=2.0 |
| FastSkyLUT | ✅ r.SkyAtmosphere.FastSkyLUT 1 |
| VolumetricFog | ✅ r.VolumetricFog 1 |

---

## 2. Performance Audit (cmd 27731)

### Actor Inventory
- **Total actors in scene**: audited via `get_all_level_actors()`
- **Dino actors**: identified by label prefix (TRex, Raptor, Brach, Trike, Dino, Rex, Sauro)
- **Vegetation**: Tree, Palm, Fern, Bush, Foliage labels
- **Rocks**: Rock, Stone, Boulder labels
- **Lights**: DirectionalLight, PointLight, SpotLight, SkyLight

### Light Budget
- **Dynamic point/spot lights**: threshold ≤ 8 for 60fps target
- Warning triggered if > 8 dynamic lights detected

---

## 3. NavMesh Configuration (cmd 27732)

| Property | Value | Rationale |
|---|---|---|
| cell_size | 10.0 cm | Fine navigation grid for character movement |
| agent_radius | 35.0 cm | Matches character capsule radius |
| agent_height | 144.0 cm | Matches character capsule height |
| cell_height | 5.0 cm | Accurate step detection |

- NavMesh rebuild triggered via `ai.nav.rebuild`
- Level saved after configuration

---

## 4. LOD Chain Setup (cmd 27732)

Applied to all dino actors (label-matched):
- `forced_lod_model = 0` (full detail at close range)
- `lod_screen_size_override_enabled = True`
- Fallback: `override_min_lod = True` if primary property unavailable

**Vegetation cull distances**:
- `cached_max_draw_distance = 8000.0 cm` (80m)
- Prevents vegetation overdraw beyond visible range

---

## 5. Hub Composition Audit (cmd 27733)

**Hero screenshot zone**: X=2100, Y=2400, radius=3000cm

### Draw Call Budget Estimate
```
Formula: dinos×8 + vegetation×3 + lights×2 + base_overhead(20)
Target: < 200 draw calls for 60fps
```

| Component | Draw Calls per Actor |
|---|---|
| Dino (sphere primitive) | 8 |
| Vegetation (static mesh) | 3 |
| Light (dynamic) | 2 |
| Base overhead | 20 |

### Performance Console Commands Executed
- `stat fps` — frame rate overlay
- `stat unit` — CPU/GPU/frame time breakdown
- `stat memory` — memory usage

---

## 6. Performance Targets

| Platform | Target FPS | Frame Budget |
|---|---|---|
| PC High-End | 60 fps | 16.67ms |
| Console | 30 fps | 33.33ms |

### Frame Budget Allocation (16.67ms @ 60fps)
| System | Budget |
|---|---|
| Rendering (GPU) | 8ms |
| Game Thread (CPU) | 4ms |
| RHI Thread | 2ms |
| Overhead | 2.67ms |

---

## 7. Recommendations for Next Agents

### Agent #05 — Procedural World Generator
- Keep vegetation density ≤ 50 instances per 10000cm² in hub zone
- Use **Hierarchical Instanced Static Mesh (HISM)** for trees — reduces draw calls from N to 1 per mesh type
- PCG graph should respect `cached_max_draw_distance = 8000cm` on all foliage

### Agent #06 — Environment Artist
- Max **3 unique tree mesh types** in hub zone to minimize material draw calls
- Use **Nanite** for rock meshes (auto-LOD, zero manual LOD chain needed)
- Avoid overlapping decals — each decal = 1 draw call

### Agent #08 — Lighting & Atmosphere
- **No more than 4 dynamic point lights** in hub zone
- Use **baked lighting** for static props (rocks, trees)
- Sky atmosphere + 1 directional light = current setup is optimal

### Agent #12 — Combat & Enemy AI
- Dino AI tick rate: **10Hz** (every 0.1s) outside combat, **30Hz** in combat
- Max **5 active dino AI agents** in hub zone simultaneously
- Use **LOD-based AI**: full behavior tree at < 3000cm, simplified at > 3000cm

---

## 8. System Status Matrix

| System | Status | Notes |
|---|---|---|
| Sun/Lighting | ✅ Optimal | -35° pitch, 8.0 intensity |
| Fog | ✅ Optimal | 1 actor, density 0.02 |
| SkyLight | ✅ Optimal | Real-time capture |
| NavMesh | ✅ Configured | cell=10, radius=35, height=144 |
| Dino LODs | ✅ Applied | forced_lod_model=0 |
| Vegetation Culling | ✅ Applied | 8000cm max draw distance |
| Draw Call Budget | ✅ Monitored | Target < 200 in hub |
| Dynamic Lights | ✅ Audited | Threshold ≤ 8 |

---

## 9. Files Modified This Cycle

| File | Change |
|---|---|
| Level (MinPlayableMap) | NavMesh configured, LODs set, vegetation culled, level saved |
| `Docs/Performance/PERF_Optimizer_007.md` | This report |

---

*Performance Optimizer #04 — Cycle 007 complete*
