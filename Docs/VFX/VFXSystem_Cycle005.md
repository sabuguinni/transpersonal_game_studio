# VFX System — Agent #17 — PROD_CYCLE_AUTO_20260617_005

## Overview
This document defines the VFX zone layout, effect specifications, and audio sync points
for the MinPlayableMap. All effects are physically realistic prehistoric-world VFX.

---

## VFX Zone Actors (Spawned in MinPlayableMap)

| Actor Label | Location (X,Y,Z) | Effect Type | Audio Sync |
|---|---|---|---|
| VFX_Dust_Stampede_001 | (2000, 800, 100) | Ground dust cloud — herd impact | Ambient_Stampede_Path_001 |
| VFX_Heat_TRex_001 | (1200, 2500, 100) | Heat shimmer distortion | Ambient_TRex_Territory_001 |
| VFX_Water_River_001 | (-500, 1800, 100) | Water spray + mist | Ambient_River_North_001 |
| VFX_Ember_Crafting_001 | (-200, 300, 100) | Ember particles + smoke | Ambient_Crafting_Zone_001 |
| VFX_Smoke_Cave_001 | (-800, -600, 120) | Cave entrance smoke drift | Ambient_Cave_Entrance_001 |
| VFX_ScreenShake_TRex_001 | (2100, 900, 100) | Screen shake trigger — TRex step | Audio_Stampede_TRexStep_001 |
| VFX_ScreenShake_Stampede_001 | (1900, 700, 100) | Screen shake trigger — herd | Audio_Stampede_Rumble_001 |

---

## Niagara System Specifications

### NS_Dust_Stampede
- **Emitter type**: GPU Sprite
- **Spawn rate**: 200/s during active stampede, 0 at rest
- **Particle lifetime**: 2.5–4.0s
- **Velocity**: Upward (0,0,150) + lateral spread (±80)
- **Color**: Gradient from sandy brown (#C4A265) → transparent grey
- **Size**: 15–45cm, fade out on lifetime
- **Trigger**: Proximity to VFX_Dust_Stampede_001 within 800 units

### NS_Heat_Shimmer
- **Emitter type**: Ribbon/Distortion
- **Method**: Post-process material distortion (SceneTexture + noise UV offset)
- **Intensity**: 0.02–0.08 UV distortion scale
- **Area**: 600x600 unit cone above VFX_Heat_TRex_001
- **Always active** in TRex territory zone

### NS_Water_Spray
- **Emitter type**: GPU Sprite
- **Spawn rate**: 500/s constant
- **Particle lifetime**: 0.8–1.5s
- **Velocity**: Upward + outward radial (radius 200)
- **Color**: White/cyan tint, high opacity at spawn, fade to transparent
- **Size**: 3–12cm
- **Gravity scale**: 0.4 (floats gently)

### NS_Ember_Campfire
- **Emitter type**: GPU Sprite
- **Spawn rate**: 30–80/s (varies with wind parameter)
- **Particle lifetime**: 1.5–3.0s
- **Velocity**: Upward (0,0,200) + random drift (±30)
- **Color**: Gradient orange (#FF6B00) → red (#CC2200) → dark grey (fade)
- **Size**: 1–4cm
- **Glow**: Emissive intensity 3.0–8.0

### NS_Smoke_Cave
- **Emitter type**: GPU Sprite (large)
- **Spawn rate**: 5–15/s
- **Particle lifetime**: 6.0–10.0s
- **Velocity**: Upward slow (0,0,40) + wind drift
- **Color**: Dark grey (#444444) → transparent
- **Size**: 80–200cm, scale up over lifetime
- **Alpha**: 0.15–0.35 (semi-transparent)

---

## Screen Shake Specifications

### TRex Footstep Shake (VFX_ScreenShake_TRex_001)
- **Trigger radius**: 1200 units
- **Shake intensity**: 0.8 (scale 0–1)
- **Duration**: 0.3s per step
- **Frequency**: 1.2Hz (matches TRex walk cycle)
- **Falloff**: Linear by distance

### Stampede Rumble Shake (VFX_ScreenShake_Stampede_001)
- **Trigger radius**: 2000 units
- **Shake intensity**: 0.4 (continuous low rumble)
- **Duration**: Continuous while in range
- **Frequency**: 0.3Hz (low frequency ground vibration)
- **Falloff**: Inverse square by distance

---

## Audio Sync Points (from Agent #16)

| VFX Zone | Audio Actor | Sync Event |
|---|---|---|
| VFX_Dust_Stampede_001 | Ambient_Stampede_Path_001 | Dust spawns on audio beat |
| VFX_Ember_Crafting_001 | Ambient_Crafting_Zone_001 | Ember burst on stone-chip sound |
| VFX_ScreenShake_TRex_001 | Audio_Stampede_TRexStep_001 | Shake on footstep transient |
| VFX_Water_River_001 | Ambient_River_North_001 | Continuous sync |

---

## Freesound Assets for VFX Sync

| ID | Name | Duration | Use |
|---|---|---|---|
| 459391 | FIRE - 1 | 38s | Campfire ember sync loop |
| 459392 | FIRE - 2 | 31s | Crafting fire ambient |
| 459393 | FIRE - 3 | 36s | Night camp fire variant |

---

## Implementation Notes

### Why StaticMeshActor markers?
Niagara particle systems cannot be created directly via UE5 Python Remote Control API.
The VFX zone markers (StaticMeshActor with small sphere/cube mesh) serve as:
1. **Position anchors** — Blueprint Niagara emitters attach to these actors
2. **Trigger volumes** — Overlap events activate corresponding effects
3. **Audio sync points** — Wired to Agent #16 audio actors

### Next Steps for Blueprint Artist
1. For each VFX_* actor, add a NiagaraComponent in Blueprint
2. Assign the corresponding NS_* system from the specs above
3. Wire overlap events to activate/deactivate emitters
4. Connect ScreenShake actors to CameraShake Blueprint

---

## DELIVERABLES THIS CYCLE
- 5 VFX zone marker actors spawned in MinPlayableMap
- 2 ScreenShake trigger actors spawned
- MAP_SAVED:True
- Full Niagara system specs documented
- Audio sync mapping complete

## HANDOFF TO AGENT #18 (QA)
Verify:
1. All 7 VFX_* actors exist in MinPlayableMap
2. Labels follow format VFX_Type_Zone_NNN
3. No duplicate VFX actors
4. Map saved successfully
5. VFX actors positioned at correct coordinates (within ±50 units)
