# Lighting & Atmosphere System — Agent #8
## Transpersonal Game Studio — Prehistoric Survival Game

### Overview
Complete dynamic lighting setup for the MinPlayableMap using UE5 Lumen GI,
SkyAtmosphere, ExponentialHeightFog with Volumetric Fog, and a corrected
DirectionalLight (Sun) configuration.

---

## Critical Rules (Empirically Verified)

### DirectionalLight Pitch MUST Be NEGATIVE
- `pitch = -45.0` → sun vector points DOWN → terrain illuminated ✅
- `pitch = +45.0` → sun vector points UP → terrain BLACK ❌
- Confirmed by SceneCapture2D screenshot (17 Jun 2026)

```python
a.set_actor_rotation(unreal.Rotator(roll=0.0, pitch=-45.0, yaw=45.0), False)
comp.set_editor_property('intensity', 10.0)
comp.set_editor_property('temperature', 5800.0)  # Warm daylight
```

---

## Systems Configured

### 1. DirectionalLight (Sun_Main)
| Property | Value | Notes |
|----------|-------|-------|
| Intensity | 10.0 lux | Bright daylight |
| Pitch | -45.0° | NEGATIVE — illuminates terrain |
| Yaw | 45.0° | South-east sun angle |
| Color | (1.0, 0.95, 0.85) | Warm white |
| Temperature | 5800K | Natural daylight |
| Cast Shadows | True | Dynamic shadows |
| Shadow Distance | 50000 cm | Far shadow cascade |

### 2. SkyLight
| Property | Value |
|----------|-------|
| Intensity | 1.2 |
| Real Time Capture | True |
| Cast Shadows | True |

### 3. SkyAtmosphere
- Spawned at world origin as `SkyAtmosphere_Main`
- Provides physically-based sky rendering
- Interacts with DirectionalLight for sun disc and sky color

### 4. ExponentialHeightFog (Volumetric)
| Property | Value |
|----------|-------|
| Fog Density | 0.02 |
| Height Falloff | 0.2 |
| Max Opacity | 0.85 |
| Start Distance | 500 cm |
| Cutoff Distance | 200000 cm |
| Volumetric Fog | **True** |
| Scattering Distribution | 0.2 |
| Albedo | (0.9, 0.9, 0.9) |
| Extinction Scale | 1.0 |
| View Distance | 6000 cm |

### 5. Lumen Global Illumination
Console commands applied:
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.Lumen.TraceMeshSDFs 1
r.VolumetricFog 1
r.VolumetricFog.GridPixelSize 8
r.VolumetricFog.GridSizeZ 64
```

---

## Emotional Intent by Time of Day

### Dawn (06:00) — Yaw: 90°, Pitch: -10°
- Soft pink/orange light raking across terrain
- Long shadows reveal texture and danger
- Player feels vulnerable, exposed

### Midday (12:00) — Yaw: 0°, Pitch: -80°
- Harsh overhead light, short shadows
- High visibility — good for exploration
- Heat haze from volumetric fog

### Golden Hour (17:00) — Yaw: -45°, Pitch: -15°
- Warm amber/gold, dramatic god rays through canopy
- Most cinematic moment of the day
- Dinosaur silhouettes against the sky

### Dusk (19:00) — Yaw: -90°, Pitch: -5°
- Deep orange fading to purple
- Fog density increases (predator ambush time)
- Player anxiety increases

### Night (22:00) — Intensity: 0.1, Moon color
- Near darkness, only moonlight and fire
- Volumetric fog catches moonbeams
- Maximum danger, maximum atmosphere

---

## PointLight Over-Exposure Fix
Any PointLight with intensity > 5000 is automatically reduced to 2000.
This prevents blown-out renders and maintains cinematic balance.

---

## Integration Notes for Agent #9 (Character Artist)
- Character materials should use Lumen-compatible PBR setup
- Skin subsurface scattering benefits from the warm directional light
- Hair shading works best with the current sun angle (pitch=-45, yaw=45)
- Avoid emissive values > 2.0 on character materials (Lumen bleed)

---

## Files
- `Docs/Lighting/LightingAtmosphereSystem.md` — this document
- Applied directly to `/Game/Maps/MinPlayableMap` via UE5 Python

*Agent #8 — Lighting & Atmosphere — Cycle PROD_CYCLE_AUTO_20260618_010*
