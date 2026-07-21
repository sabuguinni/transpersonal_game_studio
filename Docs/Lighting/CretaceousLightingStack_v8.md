# Cretaceous Lighting Stack v8 — Agent #08 Lighting & Atmosphere
**Cycle:** PROD_CYCLE_AUTO_20260624_008  
**Status:** ✅ DEPLOYED to MinPlayableMap

---

## Overview

The Cretaceous Lighting Stack v8 is the definitive atomic lighting configuration for the Transpersonal Game Studio prehistoric survival game. It establishes the visual language of the Cretaceous era — golden hour warmth, dense atmospheric haze, volumetric god rays, and dynamic sky capture.

---

## Components Applied (Atomic Execution)

### 1. Lumen GI — 15 Console Variables
| Variable | Value | Purpose |
|----------|-------|---------|
| `r.Lumen.Reflections.Allow` | 1 | Enable Lumen reflections |
| `r.Lumen.DiffuseIndirect.Allow` | 1 | Enable Lumen GI |
| `r.Lumen.HardwareRayTracing` | 0 | Software Lumen (performance) |
| `r.Lumen.ScreenProbeGather.DownsampleFactor` | 1 | Full-res probe gather |
| `r.Lumen.ScreenProbeGather.TracingOcclusionSlop` | 0.1 | Soft occlusion |
| `r.Lumen.FinalGather.LightingUpdateSpeed` | 4 | Fast GI updates |
| `r.Lumen.FinalGather.NumSamplesScale` | 2 | 2× sample quality |
| `r.VolumetricFog` | 1 | Enable volumetric fog |
| `r.VolumetricFog.GridPixelSize` | 8 | High-res fog grid |
| `r.VolumetricFog.GridSizeZ` | 128 | Tall fog volume |
| `r.AtmosphereTransmittanceTextureWidth` | 256 | Atmosphere quality |
| `r.AtmosphereTransmittanceTextureHeight` | 64 | Atmosphere quality |
| `r.Shadow.DistanceScale` | 2.0 | Extended shadow distance |
| `r.Shadow.RadiusThreshold` | 0.01 | Fine shadow detail |
| `r.MotionBlurQuality` | 4 | Cinematic motion blur |

### 2. DirectionalLight (Sun)
- **Pitch:** -38° (late afternoon golden hour)
- **Yaw:** 45° (south-west direction)
- **Color:** `#FFE09E` (warm golden, RGB 255/224/158)
- **Intensity:** 12.0 lux
- **atmosphere_sun_light:** True (drives SkyAtmosphere)
- **cast_shadows:** True
- **dynamic_shadow_distance_movable_light:** 50,000 cm
- **shadow_amount:** 0.85
- **indirect_lighting_intensity:** 1.5

### 3. SkyAtmosphere
- Spawned at origin if not present
- Driven by DirectionalLight with `atmosphere_sun_light=True`
- Provides physically-based Rayleigh scattering

### 4. SkyLight
- **real_time_capture:** True (captures live sky for reflections)
- **intensity:** 2.0
- **cast_shadows:** True
- Fixes black sky in SceneCapture2D screenshots

### 5. ExponentialHeightFog
- **fog_density:** 0.025 (subtle but present)
- **fog_height_falloff:** 0.2 (gradual height fade)
- **fog_inscattering_color:** RGB(0.42, 0.55, 0.72) — Cretaceous blue-grey haze
- **fog_max_opacity:** 0.85
- **start_distance:** 500 cm
- **fog_cutoff_distance:** 200,000 cm
- **volumetric_fog:** True
- **volumetric_fog_scattering_distribution:** 0.35 (forward scattering for god rays)
- **volumetric_fog_albedo:** RGB(0.75, 0.82, 0.90)
- **volumetric_fog_extinction_scale:** 1.2
- **volumetric_fog_distance:** 6,000 cm

### 6. PostProcessVolume (Infinite Extent)
**Exposure:**
- Method: Histogram auto-exposure
- Bias: +0.5 EV (slightly bright)
- Min brightness: 0.3 | Max brightness: 3.0

**Bloom:**
- Intensity: 0.6 (subtle bloom on bright highlights)
- Threshold: 0.8

**Ambient Occlusion:**
- Intensity: 0.7
- Radius: 120 cm
- Quality: 75%

**Color Grading (Warm Cretaceous Palette):**
- Saturation: (1.1, 1.05, 0.95, 1.0) — boosted warm tones
- Contrast: (1.05, 1.0, 0.98, 1.0) — slight contrast lift
- Gamma: (0.98, 0.97, 0.95, 1.0) — warm midtones
- Gain: (1.08, 1.02, 0.92, 1.0) — warm highlights, cool shadows

### 7. VolumetricCloud
- Spawned if not present
- Provides physical cloud layer for sky realism

### 8. Point Light Cap
- Maximum 20 point lights enforced
- Excess lights culled automatically

---

## Ambient Audio References (Freesound.org)

For Cretaceous jungle atmosphere, the following sounds are suitable:

| ID | Name | Duration | Tags |
|----|------|----------|------|
| 669357 | Amazon Rainforest Night Ambience | 90s | amazon, insects, birds, cicadas |
| 653743 | Peruvian Amazon birds frogs daytime | 58s | amazon, birds, frogs, jungle |
| 749737 | Dense Forest with Birds | 101s | dense forest, birds, ambient |
| 346225 | Forest Ambience (Madagascar Rainforest) | 125s | tropical, birds, jungle |
| 423804 | Exotic Birds Sound | 64s | exotic birds, ambient, forest |
| 468241 | Thailand Jungle Waterfall | 37s | water, jungle, cicadas, birds |

**Integration:** These should be placed as AmbientSound actors in UE5 with:
- Attenuation radius: 5000-15000 cm
- Spatialization: enabled
- Loop: True
- Volume: 0.3-0.6 (layered under music)

---

## Visual Target

> "If an 11-year-old opened the game right now, they would see: dense tropical jungle with warm golden sunlight piercing through the canopy, a T-Rex visible in a clearing, volumetric haze rolling through the valley, and a dramatic Cretaceous sky."

### Key Visual Metrics
- ✅ Sun at golden hour angle (-38° pitch)
- ✅ Warm color temperature (#FFE09E)
- ✅ Volumetric atmospheric haze (blue-grey Cretaceous)
- ✅ SkyLight real_time_capture (no black sky in screenshots)
- ✅ Lumen GI active (indirect bounce lighting)
- ✅ PostProcess color grade (warm highlights, cool shadows)

---

## Dependencies

| System | Status | Agent |
|--------|--------|-------|
| MinPlayableMap terrain | ✅ Active | #05 World Generator |
| Tropical_Jungle_Pack vegetation | ✅ Active | #06 Environment Artist |
| Dinosaur_Pack skeletal meshes | ✅ Active | #09/#12 Character/Combat |
| TranspersonalCharacter | ✅ Active | #03 Core Systems |

---

## Next Agent (#09 Character Artist)

The lighting stack is complete. Character Artist should:
1. Ensure dinosaur scales are correct (T-Rex: 3.0, Raptor: 1.5, Brachio: 2.5)
2. Position dinosaurs in clearings where sunlight reaches them (not under dense canopy)
3. Verify skeletal mesh materials respond correctly to Lumen GI
4. Add ambient occlusion capsule components to large dinosaurs for ground contact shadows

---

*Generated by Agent #08 — Lighting & Atmosphere | Transpersonal Game Studio*
