# Cinematic Lighting Specification — Transpersonal Game Studio
## Agent #8 — Lighting & Atmosphere | Cycle 019

---

## Overview

This document defines the cinematic lighting setup applied to MinPlayableMap during Cycle 019.
All lighting is implemented via UE5 Python (Remote Control API) — no C++ required.

---

## Lighting Rig Components

### 1. Directional Light (Sun) — Golden Hour
- **Intensity**: 8.0 lux
- **Color**: Warm amber `(1.0, 0.82, 0.55)` — late afternoon Cretaceous sun
- **Angle**: Pitch -18°, Yaw 45° — low horizon angle for dramatic shadows
- **Indirect Lighting Intensity**: 1.5x — bounced light fills shadow areas
- **Atmosphere Sun Light**: Enabled — drives SkyAtmosphere scattering
- **Emotional intent**: Danger and beauty coexist. The player feels the world is alive but threatening.

### 2. Sky Light — Lumen Blue Ambient
- **Intensity**: 1.2
- **Color**: Cool blue `(0.6, 0.75, 1.0)` — sky dome fill, counterbalances warm sun
- **Cast Shadows**: Enabled
- **Lumen integration**: Real-time global illumination via Lumen GI
- **Emotional intent**: The sky is vast and indifferent. The player is small.

### 3. Sky Atmosphere — Cretaceous Atmosphere
- **Rayleigh Scattering Scale**: 0.0331 — slightly denser atmosphere than modern Earth
- **Mie Scattering Scale**: 0.003996 — dust and particulates in prehistoric air
- **Mie Absorption Scale**: 0.000444
- **Result**: Slightly more orange/amber horizon, deeper blue zenith
- **Scientific basis**: Cretaceous atmosphere had higher CO2 and different particulate composition

### 4. Exponential Height Fog — Volumetric Golden Haze
- **Fog Density**: 0.035
- **Height Falloff**: 0.18 — fog hugs the ground, thins with altitude
- **Inscattering Color**: `(0.85, 0.72, 0.55)` — warm amber ground haze
- **Max Opacity**: 0.85
- **Start Distance**: 200cm — begins close to player for immersion
- **Volumetric Fog**: ENABLED
  - Scattering Distribution: 0.35 — forward scatter for god rays
  - Albedo: `(0.9, 0.85, 0.75)` — warm fog color
  - Extinction Scale: 1.2
  - View Distance: 6000cm
- **Emotional intent**: The world disappears into haze. Distance is mystery. Danger hides in fog.

### 5. Campfire Point Lights (3x)
| Label | Position | Intensity | Color | Radius |
|-------|----------|-----------|-------|--------|
| Campfire_Light_01 | (800, 600, 50) | 2500 lm | `(1.0, 0.55, 0.15)` | 600cm |
| Campfire_Light_02 | (-1200, 900, 50) | 2500 lm | `(1.0, 0.55, 0.15)` | 600cm |
| Campfire_Light_03 | (300, -1500, 50) | 2500 lm | `(1.0, 0.55, 0.15)` | 600cm |

- **Purpose**: Warm fill lights suggesting human habitation / survival camps
- **Emotional intent**: Fire = safety. The player is drawn toward warmth.

### 6. Post Process Volume — Cinematic Grade
- **Unbound**: True (affects entire scene)
- **Auto Exposure Bias**: 0.8 — slightly underexposed for cinematic feel
- **Auto Exposure Range**: 0.1 → 2.0
- **Color Saturation**: `(1.1, 1.05, 0.95)` — warm saturation boost
- **Color Contrast**: `(1.05, 1.02, 0.98)` — slight contrast lift
- **Color Gamma**: `(0.98, 0.97, 0.95)` — warm gamma curve
- **Bloom Intensity**: 0.4 — subtle bloom on bright sources
- **Bloom Threshold**: 1.0
- **Vignette Intensity**: 0.4 — frame edges darken, focus on center
- **Ambient Occlusion Intensity**: 0.6 — contact shadows in crevices
- **AO Radius**: 80cm

---

## Lighting Philosophy

> "The player doesn't notice good lighting. They only notice bad lighting."
> — Roger Deakins principle applied to game development

### Emotional Mapping by Time of Day

| Time | Sun Angle | Mood | Threat Level |
|------|-----------|------|-------------|
| Dawn | -5° | Hope, vulnerability | HIGH — predators active |
| Morning | 30° | Energy, exploration | MEDIUM |
| Midday | 75° | Harsh, exposed | LOW — heat danger |
| Golden Hour | -18° | Beauty, urgency | HIGH — predators hunting |
| Dusk | -8° | Dread, survival | VERY HIGH |
| Night | N/A (moon) | Terror, stealth | EXTREME |

### Color Language
- **Warm amber/orange**: Safety (fire), beauty, golden hour
- **Cool blue/cyan**: Danger (night), cold, isolation
- **Deep red/crimson**: Extreme danger, blood, volcano
- **Green**: Life, vegetation, relative safety
- **Grey/desaturated**: Death, exhaustion, starvation

---

## Lumen Configuration Notes

For optimal Lumen performance in MinPlayableMap:

```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.HardwareRayTracing 0
r.Lumen.TraceMeshSDFs 1
r.Lumen.DiffuseIndirect.Allow 1
r.RayTracing.Shadows 0
r.Shadow.Virtual.Enable 1
```

Hardware Ray Tracing disabled for performance. Lumen software path provides
sufficient quality for this game's art direction.

---

## Day/Night Cycle — Future Implementation

The day/night cycle will be driven by a Blueprint timeline that:
1. Rotates the DirectionalLight actor over 24 real-minutes (= 1 game day)
2. Interpolates fog color from warm (day) to cool blue (night)
3. Adjusts SkyLight intensity (1.2 day → 0.15 night)
4. Enables/disables moon light (secondary DirectionalLight)
5. Triggers audio events at dawn/dusk for ambient soundscape changes

This system is ready for Agent #16 (Audio) to hook into.

---

## Files Modified This Cycle
- `/Game/Maps/MinPlayableMap` — lighting actors configured/spawned, map saved
- `Docs/Lighting/CinematicLightingSpec_Cycle019.md` — this document

## Dependencies
- **Requires from #05/#06**: Terrain and foliage in place for fog/AO to interact with geometry
- **Provides to #09**: Lighting rig ready for character rendering tests
- **Provides to #16**: Day/night cycle hook points defined

---

*Agent #8 — Lighting & Atmosphere | Cycle PROD_CYCLE_AUTO_20260617_012*
