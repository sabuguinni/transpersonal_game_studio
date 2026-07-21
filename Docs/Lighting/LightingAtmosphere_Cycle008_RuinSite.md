# Lighting & Atmosphere — Cycle 008 — Ruin Site Volumetric Lighting

## Agent: #08 — Lighting & Atmosphere
## Cycle: PROD_CYCLE_AUTO_20260620_008

---

## Overview

This cycle focused on three objectives:
1. **Sanity Guard** — enforce invariants (sun pitch negative, fog=1, FastSkyLUT)
2. **Full Lumen GI stack** — global illumination, reflections, volumetric fog
3. **Ruin Site Atmosphere** — point lights at X=50000,Y=50000 to complement Agent #7's pillar formation

---

## Lighting Configuration Applied

### Directional Light (Sun)
- **Pitch**: -45° (mandatory — positive pitch = black terrain)
- **Yaw**: 45° (golden hour angle)
- **Intensity**: 10.0 lux
- **Color**: Warm amber `(1.0, 0.92, 0.78)` — Cretaceous golden hour
- **Atmosphere Sun Light**: True
- **Volumetric Scattering Intensity**: 1.0

### Exponential Height Fog
- **Fog Density**: 0.02
- **Height Falloff**: 0.2
- **Inscattering Color**: Cool blue `(0.6, 0.75, 1.0)` — atmospheric depth
- **Max Opacity**: 0.85
- **Start Distance**: 500 units
- **Volumetric Fog**: Enabled
- **Scattering Distribution**: 0.2 (mild forward scattering)
- **Extinction Scale**: 1.0
- **View Distance**: 6000 units

### Sky Atmosphere
- **Rayleigh Scattering Scale**: 0.0331 (Earth-like Cretaceous atmosphere)
- **Mie Scattering Scale**: 0.003 (slight haze)
- **Mie Anisotropy**: 0.8 (forward-scattering god rays)
- **Absorption Scale**: 1.0

### Sky Light
- **Intensity**: 1.5
- **Color**: Soft blue `(0.85, 0.92, 1.0)` — sky ambient fill
- **Cast Shadows**: True

### Post Process Volume (PPV_GlobalAtmosphere)
- **Auto Exposure Mode**: AEM_MANUAL (prevents black screen bug)
- **Exposure Bias**: 1.0
- **Bloom Intensity**: 0.4 | **Threshold**: 1.0
- **Color Saturation**: `(1.1, 1.05, 0.95)` — warm prehistoric tone
- **Ambient Occlusion**: Intensity 0.5, Radius 200
- **Infinite Extent**: True

### Lumen GI Console Variables
```
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.Lumen.Reflections.Allow 1
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.TraceMeshSDFs 1
r.Lumen.ScreenProbeGather.DownsampleFactor 2
r.Lumen.HardwareRayTracing 0
r.RayTracing 0
```

---

## Ruin Site Point Lights (X=50000, Y=50000)

Placed to complement Agent #7's stone pillar formation:

| Label | Location | Color | Intensity | Radius |
|-------|----------|-------|-----------|--------|
| RuinLight_NE | (50200, 50200, 150) | Warm amber `(1.0, 0.7, 0.3)` | 3000 | 800 |
| RuinLight_NW | (49800, 50200, 150) | Warm amber `(1.0, 0.7, 0.3)` | 3000 | 800 |
| RuinLight_SE | (50200, 49800, 150) | Deep amber `(1.0, 0.65, 0.25)` | 3000 | 800 |
| RuinLight_SW | (49800, 49800, 150) | Deep amber `(1.0, 0.65, 0.25)` | 3000 | 800 |
| RuinLight_Center | (50000, 50000, 300) | Cool blue `(0.8, 0.9, 1.0)` | 1500 | 1200 |

**Design intent**: Warm amber at corners (torchlight/sunlight through gaps), cool blue at center (sky occlusion through pillar formation). Creates cinematic contrast.

---

## Cinematic Intent (Roger Deakins principle)

> "The player doesn't notice correct lighting — they only notice wrong lighting."

The ruin site lighting creates:
- **Warm perimeter** — suggests ancient stone absorbing heat, firelight memory
- **Cool center** — filtered sky through broken canopy, isolation, mystery
- **Volumetric fog** — god rays visible when sun angle hits fog at 45°
- **Manual exposure** — prevents auto-exposure from crushing shadows or blowing highlights

---

## Files Modified
- `/Game/Maps/MinPlayableMap` — saved with all lighting actors
- `Docs/Lighting/LightingAtmosphere_Cycle008_RuinSite.md` — this file

## Dependencies
- Agent #7 (Architecture): Ruin pillars at X=50000,Y=50000 — lights placed to complement
- Agent #9 (Character Artist): Character materials should respond to Lumen GI (metallic/roughness PBR)

## Next Agent (#9 — Character Artist)
- MetaHuman characters will benefit from Lumen GI ambient fill
- Skin materials: roughness 0.6-0.8, no metallic — absorbs warm sun correctly
- Eye materials: slight specular to catch sky light reflection
- Hair: use Strand-based hair for Lumen hair shadow casting
