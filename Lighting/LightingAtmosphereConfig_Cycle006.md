# Lighting & Atmosphere Configuration — Agent #08 | PROD_CYCLE_AUTO_20260703_006

## Overview
Full lighting pass for the Cretaceous hub clearing at world coordinates X=2100, Y=2400.
All operations executed via single combined UE5 Python script (CAP-compliant, 1x ue5_execute).

---

## CAP Enforcement (Canonical Actor Policy)

### DirectionalLight (Sun)
- **Label**: Sun_Primary (first found, duplicates destroyed)
- **Intensity**: 75,000 lux (daytime floor enforced — never <10,000)
- **Pitch**: ≤ -45° (sun above horizon, bright daytime)
- **Color**: Warm white (1.0, 0.97, 0.88) — golden hour daylight
- **atmosphere_sun_light**: True
- **cast_shadows**: True
- **indirect_lighting_intensity**: 1.2

### SkyAtmosphere
- **Label**: SkyAtmosphere_Main
- **Policy**: Single instance enforced, duplicates destroyed

### SkyLight
- **Label**: SkyLight_Main
- **real_time_capture**: True
- **Intensity**: 3.0
- **Color**: Soft sky blue (0.6, 0.75, 1.0) — ambient fill

### ExponentialHeightFog
- **Label**: HeightFog_Main
- **fog_density**: 0.015 (subtle, not obscuring)
- **fog_height_falloff**: 0.2
- **inscattering_color**: Atmospheric blue-grey (0.55, 0.7, 0.9)
- **start_distance**: 2000.0 units
- **cutoff_distance**: 80,000 units
- **volumetric_fog**: True
- **scattering_distribution**: 0.6
- **albedo**: (0.85, 0.9, 0.95) — clean atmosphere
- **extinction_scale**: 0.8
- **view_distance**: 12,000 units

---

## Hub Lighting (X=2100, Y=2400)

### God-Ray RectLight
- **Label**: GodRay_Hub_001
- **Position**: (2100, 2400, 800)
- **Rotation**: Pitch -80° (angled down through canopy)
- **Intensity**: 8,000
- **Color**: Warm amber (1.0, 0.95, 0.7) — sunbeam through trees
- **Source**: 600x600 units (wide beam)
- **cast_shadows**: True

### Ambient Point Lights
| Label | Position | Intensity | Color | Notes |
|-------|----------|-----------|-------|-------|
| AmbientLight_Hub_N | (2100, 3200, 200) | 1500 | Sky blue | North fill |
| AmbientLight_Hub_S | (2100, 1600, 200) | 1500 | Warm amber | South fill |
| AmbientLight_Hub_E | (2900, 2400, 200) | 1500 | Forest green | East fill |

### PostProcessVolume
- **Label**: PostProcess_Hub_Main
- **infinite_extent**: True
- **priority**: 1.0

---

## Lumen Global Illumination Settings
```
r.SkyAtmosphere.FastSkyLUT 1
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.DynamicGlobalIlluminationMethod 1   (Lumen)
r.ReflectionMethod 1                   (Lumen reflections)
r.Lumen.HardwareRayTracing 0           (Software Lumen — performance)
```

---

## Audio Reference — Ambient Soundscape

### Freesound Assets Found
| ID | Name | Duration | Tags | Use |
|----|------|----------|------|-----|
| 749737 | denseforestwithbirds | 101.6s | birds, birdsong, dense forest, ambience | Hub clearing ambient loop |

**Recommended**: Loop `denseforestwithbirds` (ID: 749737) as base layer for hub clearing.
Preview: https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3

---

## Artistic Intent (Roger Deakins Principle)
> "The player doesn't notice correct lighting — they only notice wrong lighting."

The hub clearing at (2100, 2400) reads as:
- **Time of day**: Late afternoon golden hour (~4pm)
- **Mood**: Tense beauty — lush and alive, but with danger lurking
- **Key light**: Sun at -45° pitch, warm amber, casting long shadows through cycad fronds
- **Fill light**: Sky blue ambient from above, forest green from east
- **Atmosphere**: Subtle volumetric haze at distance, god rays piercing canopy
- **Emotional register**: Awe + threat — the Cretaceous is beautiful and deadly

---

## generate_image Status
- **Attempt 1**: FAIL (401 — API key invalid)
- **Fallback**: search_sounds executed immediately (atomic recovery)
- **Sound found**: ID 749737 — dense forest birds ambience

---

## Files Created This Cycle
- `Lighting/LightingAtmosphereConfig_Cycle006.md` — This document

## Next Agent (#09 — Character Artist)
The hub clearing at (2100, 2400) is now fully lit for bright Cretaceous daytime:
- Sun at 75,000 lux, pitch -45°, warm white
- Volumetric fog active, god rays from above
- 3 ambient fill lights surrounding the hub
- PostProcessVolume (infinite) for Lumen quality
- Level saved

Character meshes placed in this clearing will benefit from:
- Warm key light from upper-left (sun direction)
- Soft sky fill from above
- Subtle green bounce from forest floor
- God-ray highlight for dramatic hero shots
