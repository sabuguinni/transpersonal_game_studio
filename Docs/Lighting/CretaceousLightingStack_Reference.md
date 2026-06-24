# Cretaceous Lighting Stack — Agent #08 Reference
## Cycle: PROD_CYCLE_AUTO_20260624_003

### Overview
Atomic lighting rebuild applied to `/Game/Maps/MinPlayableMap` every cycle.
All components configured in a single `ue5_execute` Python script to prevent
intermediate degenerate states.

---

## Stack Components

### 1. Sanity Guard (runs first)
| Check | Action |
|-------|--------|
| Sun pitch ≥ 0 | Force pitch = -45°, yaw = 45°, intensity = 10 |
| Fog count = 0 | Spawn ExponentialHeightFog |
| Fog count > 1 | Destroy extras, keep 1 |
| Sky LUT | `r.SkyAtmosphere.FastSkyLUT 1` |

### 2. Lumen GI Console Vars
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.Lumen.HardwareRayTracing 0
r.Lumen.TraceMeshSDFs 1
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget 200
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.Reflections.MaxRoughnessToTrace 0.4
r.Lumen.TranslucencyReflections.FrontLayer.Allow 1
r.VolumetricFog 1
r.VolumetricFog.GridPixelSize 8
r.VolumetricFog.GridSizeZ 64
```

### 3. DirectionalLight (Sun)
| Property | Value |
|----------|-------|
| Pitch | -42° |
| Yaw | 50° |
| Intensity | 12.0 lux |
| Color | RGB(1.0, 0.92, 0.78) — warm golden |
| atmosphere_sun_light | True |
| cast_shadows | True |
| dynamic_shadow_distance_movable_light | 50,000 cm |

### 4. SkyAtmosphere
- Spawned at origin if missing
- Label: `SkyAtmosphere_Cretaceous`
- Works with DirectionalLight atmosphere_sun_light=True

### 5. SkyLight
| Property | Value |
|----------|-------|
| Intensity | 1.2 |
| real_time_capture | True |
- Required for SkyAtmosphere to render correctly in SceneCapture2D

### 6. ExponentialHeightFog
| Property | Value |
|----------|-------|
| fog_density | 0.03 |
| fog_inscattering_color | RGB(0.55, 0.72, 0.85) — blue-green atmospheric |
| fog_height_falloff | 0.2 |
| start_distance | 200 cm |
| fog_max_opacity | 0.85 |
| volumetric_fog | True |
| volumetric_fog_scattering_distribution | 0.2 |
| volumetric_fog_extinction_scale | 1.0 |

### 7. PostProcessVolume
| Property | Value |
|----------|-------|
| infinite_extent | True |
| auto_exposure_method | AEM_MANUAL |
| auto_exposure_bias | 1.0 |
| bloom_intensity | 0.4 |
| ambient_occlusion_intensity | 0.6 |
| color_saturation | (1.1, 1.05, 0.95, 1.0) — warm |
| color_contrast | (1.05, 1.02, 0.98, 1.0) |

---

## Ambient Audio References (Freesound)

| ID | Name | Duration | Use |
|----|------|----------|-----|
| 749737 | denseforestwithbirds | 101s | Jungle layer A — dense canopy birds |
| 583930 | jungle forest 02 | 121s | Jungle layer B — Mayan pyramid recording |

Preview URLs:
- https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3
- https://cdn.freesound.org/previews/583/583930_2978883-hq.mp3

---

## Known Issues
- `generate_image` returning 401 (API key issue) — fallback to `search_sounds` applied
- SceneCapture2D black screen: requires SkyLight with `real_time_capture=True` (see Brain Memory)
- PostProcessVolume AEM_MANUAL critical to prevent auto-exposure blackout

---

## Handoff to Agent #09 (Character Artist)
- Lighting stack is stable and Cretaceous-tuned
- Warm golden sun at -42° pitch provides strong directional shadows for character rendering
- Volumetric fog adds depth for character silhouettes
- AO intensity 0.6 grounds characters to terrain naturally
- SkyLight real_time_capture ensures characters lit correctly by sky dome
