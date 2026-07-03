# Performance Budget Report — Cycle PROD_CYCLE_AUTO_20260703_001
**Agent**: #04 Performance Optimizer  
**Date**: 2026-07-03  

---

## Frame Budget Targets

| Platform | Target FPS | Frame Time Budget |
|----------|-----------|-------------------|
| PC High-End | 60 fps | 16.67 ms |
| PC Mid-Range | 60 fps | 16.67 ms |
| Console | 30 fps | 33.33 ms |

---

## Scene Actor Budget

| Zone | Radius | Actor Limit | Status |
|------|--------|-------------|--------|
| Hero Clearing | < 600u | 20 actors max | ✅ Monitored |
| Mid Range | 600–1500u | 40 actors max | ✅ Monitored |
| Outer Range | 1500–4000u | 60 actors max | ✅ Monitored |
| Far | > 4000u | Culled | ✅ Enforced |

**Total scene budget**: < 150 actors for 60fps on mid-range PC.

---

## LOD & Cull Distance Rules

| Actor Type | LOD Bias | Cull Distance | Cast Shadow |
|------------|----------|---------------|-------------|
| Hero Dinos (TRex, Raptor, Trike, Brachi) | 0 | 15,000u | YES |
| Trees | +1 | 8,000u | YES |
| Rocks | +1 | 5,000u | NO |
| Small Props / Resource Nodes | +2 | 3,000u | NO |
| Terrain / Landscape | 0 | Unlimited | YES |

---

## Shadow Budget

| Setting | Value | Rationale |
|---------|-------|-----------|
| `r.Shadow.MaxResolution` | 1024 | Balanced quality/cost |
| `r.Shadow.DistanceScale` | 0.8 | 20% shadow distance reduction |
| `r.Shadow.RadiusThreshold` | 0.03 | Skip tiny shadow casters |

---

## Render Console Vars Applied

```
r.SkyAtmosphere.FastSkyLUT 1
r.Shadow.MaxResolution 1024
r.Shadow.DistanceScale 0.8
r.Shadow.RadiusThreshold 0.03
r.Foliage.LODDistanceScale 1.0
r.StaticMesh.LODDistanceScale 1.0
r.SkeletalMesh.LODDistanceScale 1.0
r.Lumen.Reflections.Allow 1
r.Lumen.DiffuseIndirect.Allow 1
```

---

## CAP Enforcement (Every Cycle)

| System | Setting | Value |
|--------|---------|-------|
| Sun (DirectionalLight) | Pitch | -45° (guard: never > -30°) |
| Sun | Intensity | 8.0 |
| Sun | Color | RGB(255,220,150) warm amber |
| Sun | atmosphere_sun_light | True |
| Fog | Count | 1 (dedup enforced) |
| Fog | fog_density | 0.02 |
| SkyLight | real_time_capture | True |
| SkyLight | Intensity | 2.0 |

---

## Hero Clearing Dino Roster

| Actor Label | Position | Scale | Role | Shadow |
|-------------|----------|-------|------|--------|
| TRex_Savana_001 | (2100, 2400, 0) | 4×4×4 | Apex predator center | YES |
| Raptor_Floresta_001 | (2220, 2480, 0) | 2×2×2 | Predator flank right | YES |
| Raptor_Floresta_002 | (1980, 2480, 0) | 2×2×2 | Predator flank left | YES |
| Trike_Savana_001 | (2300, 2300, 0) | 3.5×3.5×3 | Herbivore foreground | YES |
| Brachi_Savana_001 | (1900, 2200, 0) | 6×6×8 | Herbivore background | YES |

---

## Anti-Pattern Rules (Enforced)

1. **No duplicate actors at same position** — checked every cycle
2. **No duplicate ExponentialHeightFog** — dedup enforced
3. **No subsystem-prefixed duplicates** — use existing actors by label lookup
4. **No camera modifications** — headless editor rule
5. **No C++ file writes** — pre-built binary, Python/Blueprint only

---

## Next Cycle Priorities for Performance

| Priority | Task | Agent |
|----------|------|-------|
| P1 | Assign real skeletal meshes to dino actors (eliminates placeholder draw calls) | #9 Character |
| P2 | Replace 12 tree placeholders with instanced static mesh foliage (ISM) | #6 Environment |
| P3 | Add NavMesh bounds volume for AI pathfinding budget | #12 Combat AI |
| P4 | Enable World Partition streaming for terrain chunks | #5 World Gen |
| P5 | Profile actual GPU frame time with `stat gpu` after skeletal mesh assignment | #4 Performance |
