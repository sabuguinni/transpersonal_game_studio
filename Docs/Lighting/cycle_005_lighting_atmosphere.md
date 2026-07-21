# Lighting & Atmosphere Agent — Cycle PROD_CYCLE_AUTO_20260619_005

## Summary
Combined Lumen + Atmosphere + Volumetric Fog setup applied to MinPlayableMap.

## Systems Configured

### 1. Directional Light (Sun)
- **Actor**: `Sun_Main` (existing) or first DirectionalLight found
- **Pitch**: `-45°` (memory `hugo_sun_pitch_negative_proven` enforced — NEVER positive)
- **Yaw**: `45°` (golden-hour angle)
- **Intensity**: `10.0 lux`
- **Color**: `RGB(255, 235, 200)` — warm golden-hour amber
- **Atmosphere Sun Light**: `True` (drives SkyAtmosphere scattering)
- **Cast Shadows**: `True`

### 2. SkyAtmosphere
- Spawned at `(0, 0, 0)` if not present, labelled `SkyAtmosphere_Main`
- Coupled with DirectionalLight via `atmosphere_sun_light = True`
- Provides physically-based Rayleigh + Mie scattering (UE5 default params)

### 3. ExponentialHeightFog (Volumetric)
- **Actor**: `ExponentialHeightFog_Main`
- `fog_density = 0.02` — subtle ground haze
- `fog_height_falloff = 0.2` — gradual vertical fade
- `fog_max_opacity = 0.85`
- `volumetric_fog = True`
- `volumetric_fog_scattering_distribution = 0.2` — slight forward scatter
- `volumetric_fog_albedo = RGB(200, 220, 255)` — cool blue-white mist
- `volumetric_fog_view_distance = 6000 cm`

### 4. SkyLight (Lumen Ambient)
- **Actor**: `SkyLight_Lumen`
- `intensity = 1.5`
- `real_time_capture = True` — reflects sky colour into scene

### 5. Lumen Console Variables
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.HardwareRayTracing 0        (software Lumen — compatible with all GPUs)
r.VolumetricFog 1
r.VolumetricFog.GridPixelSize 8
r.VolumetricFog.GridSizeZ 64
```

## Sound References Found (Freesound.org)
| ID | Name | Duration | Tags |
|----|------|----------|------|
| 607823 | thunder with steady rain | 464s | ambience, rain, storm, thunder |
| 824310 | urban thunderstorm | 541s | field-recording, rain, storm |
| 465314 | Thunder | 32s | atmosphere, lightning, storm |
| 813233 | Thunderstorm pt.2 | 232s | nature, rain, storm |
| 503282 | Room tone with hail | 88s | ambience, hail, outdoor |

> These are queued for Agent #16 (Audio) — prehistoric storm ambience layer.

## CAP Audit
- Bridge: `bridge_ok` ✅
- Actor count, dino count, light count, degenerate labels: audited
- `CAP_SAFE:True`

## Lumen Artistic Intent
The lighting targets a **late-afternoon Cretaceous jungle** mood:
- Golden directional sun at 45° angle creates long dramatic shadows through vegetation
- Volumetric fog fills valleys and low areas with cool atmospheric haze
- SkyLight real-time capture ensures Lumen GI picks up sky colour for ambient fill
- No overexposed point lights (reduced to 2000 lux if >5000)

> "The player doesn't notice correct lighting — they only notice wrong lighting."
> — Roger Deakins principle, applied to UE5 Lumen pipeline

## Next Agent: #9 Character Artist
- Lighting is configured. Characters placed in the scene will receive correct Lumen GI.
- Sun angle at pitch=-45 casts dramatic shadows on character geometry.
- Volumetric fog will interact with character silhouettes for depth.
- MetaHuman skin shaders will benefit from the warm directional + cool ambient split.

## Files Modified
- `/Game/Maps/MinPlayableMap` — saved with all lighting actors
