# Lighting & Atmosphere System — Cycle 006
**Agent #8 — Lighting & Atmosphere Agent**
**Cycle:** PROD_CYCLE_AUTO_20260617_006

---

## System Overview

Complete cinematic lighting pipeline for the Cretaceous prehistoric survival world.
Designed to serve the emotional tone of survival and danger — not beauty for its own sake.

> "The player doesn't notice correct lighting — they only notice wrong lighting."
> — Roger Deakins principle applied to game cinematography

---

## Implemented Components

### 1. Directional Sun Light (`Sun_GoldenHour`)
- **Intensity:** 8.5 lux (late afternoon golden hour)
- **Color Temperature:** 3200K warm amber (RGB 250, 217, 166)
- **Angle:** -35° pitch, 45° yaw (low horizon sun)
- **Shadow Cascades:** 4 (high quality dynamic shadows)
- **Volumetric Scattering:** 2.5x (god rays through foliage)
- **Atmosphere Sun Light:** True (physically accurate sky coupling)

### 2. Sky Atmosphere (`SkyAtmosphere_Cretaceous`)
- Physically-based atmospheric scattering
- Coupled with directional sun for accurate sky color
- Rayleigh + Mie scattering for haze and horizon glow

### 3. Volumetric Clouds (`VolumetricClouds_Cretaceous`)
- Dynamic cloud system
- Interacts with Lumen GI for accurate cloud shadowing
- Prehistoric overcast/partly cloudy aesthetic

### 4. Sky Light (`SkyLight_Ambient`)
- **Intensity:** 1.2 (soft ambient fill)
- **Color:** Cool blue-white (200, 220, 255) — sky bounce light
- Balances warm sun with cool sky for depth

### 5. Exponential Height Fog (`ExponentialFog_Cretaceous`)
- **Density:** 0.035 (atmospheric haze, not pea soup)
- **Height Falloff:** 0.2 (ground-hugging prehistoric mist)
- **Inscattering Color:** Warm amber (0.85, 0.72, 0.55) — matches sun
- **Start Distance:** 500 units (clear foreground)
- **Cutoff Distance:** 80,000 units (distant mountains visible)
- **Volumetric Fog:** Enabled (3D light scattering)
- **Scattering Distribution:** 0.35 (forward scatter for god rays)
- **Extinction Scale:** 1.2 (slight density boost for atmosphere)

### 6. Post Process Volume (`PostProcess_Cinematic`)
- **Unbound:** True (affects entire world)
- **Priority:** 1.0
- Lumen Global Illumination enabled via project settings
- Lumen Reflections for wet surfaces, water, eyes

---

## Emotional Intent by Time of Day

| Time | Sun Angle | Color Temp | Fog Density | Emotional Tone |
|------|-----------|------------|-------------|----------------|
| Dawn | -5° | 2800K orange | 0.06 | Vulnerability, new day |
| Morning | 45° | 5500K white | 0.02 | Clarity, hunting time |
| Noon | 75° | 6500K blue-white | 0.01 | Harsh, exposed, danger |
| Golden Hour | -35° | 3200K amber | 0.035 | Epic, cinematic, beauty |
| Dusk | -15° | 2200K deep orange | 0.08 | Urgency, find shelter |
| Night | -90° | 4000K moonlight | 0.05 | Fear, predator time |

---

## Sound References (Freesound.org)

For weather/atmosphere audio integration (Agent #16):

| ID | Name | Duration | Use Case |
|----|------|----------|----------|
| 699907 | Thunderstorm Sound FX | 1187s | Storm weather state |
| 317481 | Rainstorm with distant thunder | 245s | Rain transition |
| 362138 | Rolling thunder | 30s | Thunder punctuation |
| 844068 | Munich Thunderstorm Long | 3657s | Extended storm ambience |

---

## Lumen Configuration

```ini
# r.Lumen settings for MinPlayableMap
r.Lumen.Reflections.Allow 1
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.TraceMeshSDFs 1
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget 200
r.Lumen.FinalGather.LightingUpdateSpeed 4
```

---

## Dependencies

- **Requires from Agent #5 (World Generator):** Terrain mesh for fog height reference
- **Requires from Agent #6 (Environment):** Foliage density for volumetric scattering
- **Provides to Agent #9 (Character Artist):** Lighting conditions for skin shader calibration
- **Provides to Agent #16 (Audio):** Weather state system hooks for adaptive music

---

## Files Modified in UE5

All changes applied directly via Python Remote Control to `/Game/Maps/MinPlayableMap`:
- `Sun_GoldenHour` — DirectionalLight actor configured
- `SkyAtmosphere_Cretaceous` — Sky atmosphere actor
- `VolumetricClouds_Cretaceous` — Cloud volume actor
- `SkyLight_Ambient` — Ambient sky light
- `ExponentialFog_Cretaceous` — Volumetric height fog
- `PostProcess_Cinematic` — Cinematic post process volume
- Map saved: `/Game/Maps/MinPlayableMap`
