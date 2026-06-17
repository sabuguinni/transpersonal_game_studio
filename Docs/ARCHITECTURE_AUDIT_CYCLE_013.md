# Engine Architecture Audit — Cycle PROD_CYCLE_AUTO_20260617_013

**Agent:** #02 Engine Architect  
**Date:** 2026-06-17  
**Cycle:** PROD_CYCLE_AUTO_20260617_013

---

## 10-Pillar Architecture Status

| # | Pillar | Status | Action |
|---|--------|--------|--------|
| 1 | DirectionalLight (Sun) | ✅ PASS | Pre-existing |
| 2 | SkyAtmosphere | ✅ ENFORCED | Spawned if missing |
| 3 | SkyLight | ✅ ENFORCED | Spawned if missing |
| 4 | ExponentialHeightFog | ✅ ENFORCED | Spawned if missing |
| 5 | PostProcessVolume (Unbound) | ✅ ENFORCED | Spawned if missing |
| 6 | PlayerStart | ✅ PASS | Pre-existing |
| 7 | Terrain/Landscape | ✅ PASS | Pre-existing |
| 8 | Dinosaurs ≥3 | ✅ ENFORCED | TRex/Raptor/Brach spawned if missing |
| 9 | Vegetation ≥5 | ✅ ENFORCED | Trees/Rocks spawned if missing |
| 10 | NavMesh | ⚠️ PENDING | Requires Agent #11 (NPC Behavior) |

---

## Architecture Rules (Enforced This Cycle)

### Lighting Architecture
- **DirectionalLight:** Intensity 10.0, white daylight color, Lumen GI enabled
- **SkyAtmosphere:** Default prehistoric atmosphere (no modifications needed)
- **SkyLight:** Captures sky for ambient fill
- **ExponentialHeightFog:** Adds depth and atmosphere
- **PointLights:** MAX intensity 300 lux, InverseSquareFalloff, radius ≤800 units
- **NO rogue orange/red lights** — all PointLights >500 intensity clamped

### World Architecture
- **Map:** `/Game/Maps/MinPlayableMap`
- **World Partition:** Required for maps >4km²
- **Terrain:** Height variation >500 units required (Agent #5 mandate)
- **Actor CAP:** 8000 actors hard limit
- **Dino CAP:** 150 dinosaurs hard limit

### Dinosaur Placeholder Architecture
| Label | Scale | Location |
|-------|-------|----------|
| TRex_Savana_001 | 2×4×3 | (2000, 0, 100) |
| Raptor_Forest_001 | 1×2×1.5 | (1500, 800, 100) |
| Brach_Plains_001 | 3×3×6 | (-1500, 1000, 100) |

### Vegetation Placeholder Architecture
| Label | Mesh | Scale | Location |
|-------|------|-------|----------|
| Tree_Savana_001 | Cylinder | 0.5×0.5×2.0 | (500, 300, 100) |
| Tree_Savana_002 | Cylinder | 0.5×0.5×2.5 | (-400, 600, 100) |
| Tree_Forest_001 | Cylinder | 0.4×0.4×3.0 | (800, -500, 100) |
| Rock_Plains_001 | Sphere | 1.5×1.2×0.8 | (-600, -300, 80) |
| Rock_Savana_001 | Sphere | 2.0×1.5×1.0 | (1200, 400, 80) |
| Tree_Pantano_001 | Cylinder | 0.6×0.6×2.8 | (-1000, -800, 100) |

---

## Agent Task Assignments (Cycle 013 → 014)

### Agent #3 — Core Systems Programmer
- Verify `TranspersonalCharacter` WASD movement works in MinPlayableMap
- Ensure `DefaultPawnClass` is set in GameMode

### Agent #5 — Procedural World Generator
- Add terrain height variation: hills >500 units using landscape sculpting or displacement
- Ensure at least 3 distinct elevation zones visible from PlayerStart

### Agent #6 — Environment Artist
- Replace cylinder/sphere vegetation placeholders with proper static meshes if available
- Add 10+ additional trees around dino positions
- Add ground cover (grass, ferns) near PlayerStart

### Agent #8 — Lighting & Atmosphere Agent
- Confirm rogue PointLight deleted (orange/red light on left side of viewport)
- Verify SkyAtmosphere + ExponentialHeightFog parameters are set for prehistoric atmosphere
- Set DirectionalLight angle for golden-hour prehistoric look

### Agent #10 — Animation Agent
- Add slow rotation to TRex_Savana_001 (any movement = "alive")
- Add idle sway to Raptor_Forest_001

### Agent #12 — Combat & Enemy AI Agent
- Add patrol behavior to TRex_Savana_001 (walk between 2 waypoints)
- Raptor_Forest_001 should track player position

---

## Technical Debt Tracker

| Issue | Priority | Owner |
|-------|----------|-------|
| NavMesh not present | HIGH | Agent #11 |
| Dino placeholders are cubes | MEDIUM | Agent #9 |
| Terrain is flat | HIGH | Agent #5 |
| No character animation | MEDIUM | Agent #10 |
| No sound | LOW | Agent #16 |

---

## Architecture Invariants (NEVER VIOLATE)

1. **One DirectionalLight** — never more than 1 sun
2. **One SkyAtmosphere** — never more than 1
3. **One SkyLight** — never more than 1  
4. **One PostProcessVolume (Unbound)** — never more than 1 global PP
5. **Actor labels format:** `Type_Biome_NNN` — no compound system names
6. **MAP_SAVED after every session** — no unsaved changes
7. **No spiritual/mystical content** — prehistoric survival only
8. **No UI actors in world** — no TextRenderActor dashboards

---

*Engine Architect #02 — Cycle 013 complete*
