# Cretaceous Lighting System — Cycle 007
## Agent #8 — Lighting & Atmosphere

### Overview
Complete cinematic lighting setup for MinPlayableMap — golden hour atmosphere with Lumen GI.

---

## Lighting Actors in MinPlayableMap

### 1. Sun_GoldenHour (DirectionalLight)
- **Intensity**: 8.5 lux
- **Color**: RGB(255, 217, 165) — warm amber 3200K
- **Rotation**: Pitch -35°, Yaw 45° — golden hour low angle
- **Volumetric Scattering**: 2.5x
- **Shadow Cascades**: 4
- **Atmosphere Sun Light**: Enabled

### 2. SkyAtmosphere_Cretaceous (SkyAtmosphere)
- **Rayleigh Scattering Scale**: 0.0331 — slightly denser Cretaceous atmosphere
- **Mie Scattering Scale**: 0.003 — warm haze
- **Atmosphere Height**: 60.0 km
- **Effect**: Warm orange/amber horizon tones, deeper blue zenith

### 3. SkyLight_Cretaceous (SkyLight)
- **Intensity**: 1.2
- **Color**: RGB(200, 220, 255) — cool blue sky fill
- **Real Time Capture**: Enabled — reflects sky changes dynamically
- **Cast Shadows**: Enabled

### 4. HeightFog_Cretaceous (ExponentialHeightFog)
- **Fog Density**: 0.035
- **Height Falloff**: 0.18
- **Max Opacity**: 0.85
- **Start Distance**: 200 cm
- **Volumetric Fog**: Enabled
- **Volumetric Fog Albedo**: RGB(255, 240, 210) — warm haze color
- **Volumetric Fog View Distance**: 6000 cm
- **Effect**: Ground-level atmospheric haze, god rays through trees

### 5. PPV_Cretaceous_Global (PostProcessVolume)
- **Unbound**: True (global effect)
- **Lumen GI Quality**: 1.0
- **Lumen Reflections Quality**: 1.0
- **Bloom Intensity**: 0.35
- **Auto Exposure Bias**: +0.5 EV
- **Color Saturation**: (1.1, 1.05, 0.95) — warm push
- **Vignette**: 0.4
- **Ambient Occlusion**: 0.6 intensity, 200cm radius

---

## Visual Intent
The lighting targets a **late afternoon golden hour** in the Cretaceous period:
- Warm amber sunlight at low angle casting long dramatic shadows
- Atmospheric haze gives sense of humid prehistoric air
- Volumetric fog enables visible god rays through vegetation
- Cool sky fill balances warm sun for cinematic contrast
- Lumen GI ensures accurate indirect bounce lighting on dinosaurs and terrain

## Day/Night Cycle Notes
For future implementation (Cycle 008+):
- Rotate Sun_GoldenHour actor over time (Blueprint or C++ tick)
- Adjust SkyLight intensity based on sun elevation
- Fog density increases at night (0.035 → 0.08)
- PPV color grading shifts to cool blue at night

## Dependencies
- MinPlayableMap must be loaded
- Lumen enabled in Project Settings (r.Lumen.Reflections.Allow 1)
- Volumetric fog requires r.VolumetricFog 1

## Next Agent (#9 Character Artist)
- Characters will receive warm amber rim lighting from Sun_GoldenHour
- Lumen GI provides accurate skin/fur indirect lighting
- PPV color grading applies globally to all characters
