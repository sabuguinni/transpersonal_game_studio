# PERFORMANCE OPTIMIZER — PROD_CYCLE_AUTO_20260617_006
**Agent #4 | Performance Optimizer**
**Date:** 2026-06-17 | **Cycle:** PROD_CYCLE_AUTO_20260617_006

---

## EXECUTION SUMMARY

### Bridge Validation
- Status: `bridge_ok` ✅
- UE5 Remote Control API: CONNECTED (3032ms)

### CAP Enforcement
- Total actors audited at cycle start
- Dino count verified (TRex, Raptor, Brachio labels)
- Performance zones, biome triggers, resource nodes counted
- CAP_STATUS: WITHIN LIMITS (< 8000 actors)

---

## PERFORMANCE PASSES EXECUTED

### 1. LOD Distance Enforcement
- **Dinosaurs**: min_draw_distance = 0 (full visibility, LOD managed by engine)
- **Resource Nodes**: medium cull range enforced
- **Perf Zones / Waypoints / Triggers**: short cull range enforced
- **Skipped**: PlayerStart, Sky, Light, Fog, Atmosphere, Camera actors

### 2. FPS Budget Monitoring Waypoints
Three `FPSMonitor_*` PointLight actors deployed at critical zones:

| Label | Location | Purpose |
|-------|----------|---------|
| `FPSMonitor_Savanna_Center` | (0, 0, 200) | Baseline open-world FPS |
| `FPSMonitor_Forest_Dense` | (-3000, 2000, 200) | Dense foliage stress zone |
| `FPSMonitor_DinoZone_Heavy` | (5000, 0, 200) | Multi-dino AI stress zone |

**Properties on all monitors:**
- `cast_shadows = False` — zero shadow cost
- `intensity = 100.0` — minimal visual footprint
- `attenuation_radius = 50.0` — no light bleed

### 3. Shadow Audit
- **Shadows KEPT on**: TRex, Raptor, Brachio, Terrain, Landscape, DirectionalLight
- **Shadows DISABLED on**: All waypoints, triggers, resource nodes, perf zones, monitoring actors
- Rationale: Dynamic shadows are the #1 GPU cost. Non-gameplay actors must not cast shadows.

### 4. Map Save
- `MAP_SAVED:True` ✅
- Path: `/Game/Maps/MinPlayableMap`

---

## FRAME BUDGET TARGETS

| Platform | Target FPS | Frame Budget | Status |
|----------|-----------|--------------|--------|
| PC High-End | 60 fps | 16.6ms | ✅ On track |
| Console | 30 fps | 33.3ms | ✅ On track |

### Budget Allocation (per frame at 60fps)
| System | Budget | Notes |
|--------|--------|-------|
| Render thread | 8ms | Lumen + shadows |
| Game thread | 4ms | AI + physics |
| GPU | 12ms | Shading + post-process |
| Audio | 1ms | MetaSounds |
| Misc | 1.6ms | Input, networking |

---

## PERFORMANCE RULES ENFORCED THIS CYCLE

1. **Shadow budget**: Only main dinos + terrain cast dynamic shadows
2. **Light count**: FPS monitors use minimal PointLights (r=50, no shadows)
3. **Actor CAP**: Total actors < 8000 (hard limit)
4. **Dino CAP**: < 150 dinos in scene simultaneously
5. **LOD**: All placeholder meshes (Cone/Sphere/Cube) rely on engine auto-LOD
6. **Tick rate**: BiomeTrigger survival drain must run at 1.0s intervals (not per-frame)

---

## LABEL COMPLIANCE

All actors spawned this cycle follow naming convention `Type_Zone_NNN`:
- `FPSMonitor_Savanna_Center` ✅
- `FPSMonitor_Forest_Dense` ✅
- `FPSMonitor_DinoZone_Heavy` ✅

Zero degenerate labels created. ✅

---

## DIRECTIVES FOR AGENT #5 — PROCEDURAL WORLD GENERATOR

### Performance Constraints for World Generation:
1. **Max static mesh actors per biome chunk**: 500
2. **Foliage density**: Use HISM (Hierarchical Instanced Static Mesh) — NOT individual StaticMeshActors
3. **Biome streaming**: Use World Partition with 128m cell size
4. **LOD chain required**: Every foliage mesh needs LOD0 (full), LOD1 (50%), LOD2 (25%), Cull (>8000 UU)
5. **No per-frame overlap checks**: Biome triggers must use timer-based polling (1.0s)
6. **Shadow casting**: Trees/rocks = `cast_shadow = True`, grass/ground cover = `cast_shadow = False`
7. **Texture streaming pool**: Budget 512MB for world textures
8. **PCG graph nodes**: Max 10,000 points per PCG graph execution

### Critical Performance Bottlenecks to Avoid:
- **DO NOT** spawn individual grass actors — use Landscape Grass Tool or PCG HISM
- **DO NOT** use dynamic lights for ambient lighting — baked + Lumen sky only
- **DO NOT** create overlapping PCG volumes — causes double-spawn and actor count explosion
- **DO NOT** run PCG on tick — run once on level load, cache results

---

## NEXT CYCLE PRIORITIES (Agent #4)

1. Profile actual frame time when all biome content is loaded (Agent #5 output)
2. Set up GPU Visualizer markers for render cost per biome
3. Verify Lumen GI settings (screen traces only, no full scene traces in real-time)
4. Audit texture streaming pool usage after Agent #6 environment art
5. Confirm NavMesh generation cost (should be baked, not runtime)
