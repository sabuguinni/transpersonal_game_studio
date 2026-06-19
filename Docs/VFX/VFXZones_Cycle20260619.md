# VFX Zones — Cycle PROD_CYCLE_AUTO_20260619_001
**Agent:** #17 VFX Agent  
**Date:** 2026-06-19  
**Status:** COMPLETE — MAP_SAVED:True

---

## Overview

This cycle implements 8 VFX zone actors in MinPlayableMap, co-located with Audio Agent #16's audio zones. Each VFX zone uses a colored point/spot/rect light as an in-editor anchor and marker for the Niagara particle system to be attached when Niagara assets are available.

---

## VFX Actors Placed This Cycle

| Label | Location | Light Color | Effect Type | Audio Zone Pair |
|-------|----------|-------------|-------------|-----------------|
| `VFX_Dust_TRexDanger_001` | (2200, 1800, 50) | Orange-brown | Ground dust/vibration | AudioZone_TRexDanger_001 |
| `VFX_Ember_ElderFire_001` | (0, 0, 50) | Orange-red | Ember/heat shimmer | AudioZone_Elder_Dialogue_001 |
| `VFX_DustMotes_LoreStone_001` | (800, -400, 50) | Cool blue | Floating dust motes | AudioZone_LoreStone_LostHunter_001 |
| `VFX_Debris_RaptorPatrol_001` | (1500, -800, 50) | Green-yellow | Leaf/debris scatter | AudioZone_Raptor_Patrol_001 |
| `VFX_Campfire_ElderZone_001` | (0, 0, 80) | Warm orange | Campfire glow | AudioZone_Elder_Dialogue_001 |
| `VFX_GroundShake_TRex_001` | (2200, 1800, 100) | Deep red | Ground shake spotlight | AudioZone_TRexDanger_001 |
| `VFX_DustMotes_LoreStone_Glow_001` | (800, -400, 120) | Blue-white | Rect light shaft | AudioZone_LoreStone_LostHunter_001 |
| `VFX_Debris_RaptorCorridor_001` | (1500, -800, 80) | Green | Debris corridor | AudioZone_Raptor_Patrol_001 |

---

## Niagara System Specifications

### NS_Dino_FootstepDust
- **Zone:** VFX_Dust_TRexDanger_001 (2200, 1800)
- **Trigger:** Player enters radius 700 units of TRex position
- **Emitter type:** GPU Sprite, burst on footstep event
- **Particle count:** 80–150 per burst (LOD0), 40–60 (LOD1), 10–15 (LOD2)
- **Lifetime:** 1.5–3.0s
- **Color:** Brown/tan (0.6, 0.4, 0.2) → transparent
- **Size:** 15–45 units, random
- **Velocity:** Radial outward + upward, gravity drag
- **Material:** M_VFX_Dust (additive, soft particle)
- **LOD distances:** LOD0 <500, LOD1 500–1500, LOD2 1500–3000, cull >3000

### NS_Fire_Campfire
- **Zone:** VFX_Campfire_ElderZone_001 (0, 0)
- **Emitter type:** GPU Sprite + Mesh (log embers)
- **Particle count:** 30–60 continuous (LOD0), 15–25 (LOD1), 5–8 (LOD2)
- **Lifetime:** 0.3–0.8s (flame), 2.0–5.0s (embers)
- **Color:** Yellow (1.0, 0.9, 0.1) → orange (1.0, 0.4, 0.0) → dark red → transparent
- **Sub-emitters:** Smoke (gray, slow rise), Embers (orange sparks, random scatter)
- **Material:** M_VFX_Fire (translucent, emissive), M_VFX_Smoke (additive)
- **LOD distances:** LOD0 <300, LOD1 300–800, LOD2 800–1500, cull >1500

### NS_Ambient_DustMotes
- **Zone:** VFX_DustMotes_LoreStone_001 (800, -400)
- **Emitter type:** GPU Sprite, continuous low-rate
- **Particle count:** 200 continuous (LOD0), 80 (LOD1), 20 (LOD2)
- **Lifetime:** 8.0–15.0s
- **Color:** White/gold (0.9, 0.85, 0.7) at 0.3 opacity
- **Size:** 2–8 units
- **Velocity:** Slow drift upward + random lateral, very low gravity
- **Material:** M_VFX_DustMote (additive, soft)
- **LOD distances:** LOD0 <200, LOD1 200–600, LOD2 600–1000, cull >1000

### NS_Ambient_LeafDebris
- **Zone:** VFX_Debris_RaptorPatrol_001 (1500, -800)
- **Emitter type:** Mesh particle (leaf mesh), continuous
- **Particle count:** 15–25 continuous (LOD0), 8–12 (LOD1), 3–5 (LOD2)
- **Lifetime:** 4.0–8.0s
- **Color:** Green (0.2, 0.5, 0.1) → brown (0.4, 0.25, 0.05)
- **Size:** 8–20 units
- **Velocity:** Wind-driven horizontal + tumble rotation
- **Material:** M_VFX_Leaf (masked, two-sided)
- **LOD distances:** LOD0 <400, LOD1 400–1000, LOD2 1000–2000, cull >2000

---

## Screen Shake System Specification

### CameraShake_TRexFootstep
- **Trigger zone:** Radius 700 units around VFX_GroundShake_TRex_001 (2200, 1800)
- **Shake type:** Perlin noise oscillation
- **Amplitude:** 2.0 (near, <200 units), 0.8 (mid, 200–500), 0.2 (far, 500–700)
- **Frequency:** 8 Hz (matches T-Rex footstep cadence ~0.8s interval)
- **Duration:** 0.4s per footstep event
- **Falloff:** Linear with distance
- **Implementation:** Blueprint TriggerVolume → PlayerController.ClientStartCameraShake()

---

## Material Requirements

| Material | Type | Usage |
|----------|------|-------|
| M_VFX_Dust | Additive, soft particle | Footstep dust, ground impact |
| M_VFX_Fire | Translucent, emissive | Campfire flames |
| M_VFX_Smoke | Additive, soft | Campfire smoke |
| M_VFX_DustMote | Additive, soft | Ambient floating particles |
| M_VFX_Leaf | Masked, two-sided | Leaf debris |
| M_VFX_Blood | Translucent | Combat impacts (future) |
| M_VFX_WaterSplash | Additive | Rain/river impacts (future) |

---

## Integration Status

| System | Status | Notes |
|--------|--------|-------|
| Audio Zone co-location | ✅ DONE | All 4 Audio Agent #16 zones matched |
| VFX anchor actors placed | ✅ DONE | 8 actors in MinPlayableMap |
| Niagara specs written | ✅ DONE | 4 systems fully specified |
| Screen shake spec | ✅ DONE | TRex zone defined |
| Niagara assets created | ⏳ PENDING | Requires Niagara plugin access |
| Materials created | ⏳ PENDING | Requires UE5 Material Editor |
| Blueprint triggers | ⏳ PENDING | QA Agent to verify trigger volumes |

---

## Handoff to QA Agent #18

### Test Checklist
1. **VFX Actor Count:** Verify 8 VFX_ labeled actors exist in MinPlayableMap
2. **Co-location:** VFX actors within 50 units of matching AudioZone actors
3. **No degenerate labels:** All VFX labels match format `VFX_Type_Zone_NNN`
4. **Light colors:** Each zone has distinct color matching its effect type
5. **Map saved:** MinPlayableMap last save timestamp after this cycle
6. **No actor cap violation:** Total actor count < 8000
7. **No dino count violation:** Dino actors < 150

### Known Gaps (for QA to flag)
- Niagara particle assets not yet created (requires Niagara plugin)
- Materials are placeholders (light actors used as proxies)
- Screen shake Blueprint not yet implemented

---

## Next Cycle Priorities

1. **QA Agent #18:** Verify all 8 VFX actors exist, validate co-location with audio zones
2. **Integration Agent #19:** Merge VFX zone data with audio zone data into unified zone manifest
3. **Future VFX cycle:** Create actual Niagara systems using NS_ naming convention
4. **Future VFX cycle:** Implement M_VFX_* materials in UE5 Material Editor
