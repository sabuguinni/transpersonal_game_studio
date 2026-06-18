# Lighting & Atmosphere Setup — Cycle 019 (PROD_CYCLE_AUTO_20260618_011)

## Agent #8 — Lighting & Atmosphere Agent

### Summary
Combined Lumen + Atmosphere + Volumetric Fog lighting setup applied to MinPlayableMap.

---

## Lighting Configuration Applied

### Sun (DirectionalLight: Sun_Main)
- **Pitch**: -45° (CRITICAL — negative pitch confirmed by screenshot test)
  - pitch=+45 → terrain completely black (sun points at sky)
  - pitch=-45 → terrain illuminated + SkyAtmosphere visible
- **Yaw**: 45° (golden hour angle)
- **Intensity**: 10.0 lux
- **Color**: Warm white (1.0, 0.95, 0.85) — slight warm tint for prehistoric golden hour
- **atmosphere_sun_light**: True — drives SkyAtmosphere scattering
- **cast_shadows**: True

### SkyAtmosphere
- **rayleigh_scattering_scale**: 0.0331 (standard Earth atmosphere)
- **mie_scattering_scale**: 0.003 (slight haze for prehistoric humidity)
- **mie_absorption_scale**: 0.000333

### SkyLight
- **Intensity**: 1.5
- **Color**: Cool blue (0.6, 0.7, 1.0) — sky bounce light complement to warm sun
- **cast_shadows**: True

### ExponentialHeightFog (Volumetric)
- **fog_density**: 0.02
- **fog_height_falloff**: 0.2
- **fog_inscattering_color**: Muted blue-grey (0.5, 0.6, 0.7)
- **volumetric_fog**: True
- **volumetric_fog_scattering_distribution**: 0.2
- **volumetric_fog_albedo**: (0.75, 0.75, 0.75)
- **volumetric_fog_extinction_scale**: 1.0

### PostProcessVolume (Lumen)
- **lumen_final_gather_quality**: 1.0
- **lumen_scene_detail**: 1.0
- **lumen_scene_lighting_quality**: 1.0
- **auto_exposure_method**: AEM_HISTOGRAM
- **auto_exposure_min_brightness**: 0.3
- **auto_exposure_max_brightness**: 3.0
- **bloom_intensity**: 0.5
- **ambient_occlusion_intensity**: 0.5

### Point/Spot Lights
- All lights with intensity > 5000 reduced to 2000 (prevents over-exposure)

---

## Sound References Found (Freesound.org)
- **BeforeTheRain(Wind).wav** (ID: 73418) — Heavy winds, forest foliage, storm approach
  - Duration: 108s | Tags: field-recording, nature, outdoor, storm, wind
  - Preview: https://cdn.freesound.org/previews/73/73418_97826-hq.mp3
- **urban thunderstorm** (ID: 824310) — Storm + heavy rain, late summer evening
  - Duration: 541s | Tags: ambience, rain, storm, thunder, outdoor
  - Preview: https://cdn.freesound.org/previews/824/824310_1433145-hq.mp3

---

## Cinematic Intent (Roger Deakins Principle)
The lighting serves the **emotional truth** of the prehistoric world:
- Warm golden sun = life, danger, exposed vulnerability on open terrain
- Cool blue sky bounce = depth, mystery, the vast indifferent sky
- Volumetric fog = atmospheric distance, scale, the unknown beyond
- Lumen GI = every surface catches and bounces real light — no flat areas

---

## Technical Notes
- Sun pitch MUST be negative (memory: hugo_sun_pitch_negative_proven)
- Never modify editor camera (memory: hugo_no_camera_modification)
- All operations in single ue5_execute script (CRITERIO 2 OBRIGATORIO)
- Map saved: ATM_SAVED confirmed

---

## Next Agent (#9 — Character Artist)
- Lighting is configured for character visibility at ground level
- Warm key light from yaw=45° — character faces should be lit from front-right
- Volumetric fog adds depth to character silhouettes
- Lumen GI means character materials will receive proper indirect lighting
- Recommend testing MetaHuman character placement at PlayerStart (0,0,0)
