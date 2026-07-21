# Ruin Cluster Lighting Setup — Agent #8 Cycle 022

## Overview
This document describes the complete lighting setup applied to the Cretaceous Ruin Cluster
at coordinates (50000, 50000) in `/Game/Maps/MinPlayableMap`.

## Actors Placed This Cycle

### 1. Firepit_Light_RuinCluster (PointLight)
- **Location**: (50150, 50150, 150)
- **Intensity**: 3000 lm
- **Color**: Warm orange `(R:1.0, G:0.45, B:0.1)` — firepit/ember tone
- **Attenuation Radius**: 800 UU
- **Source Radius**: 15 UU (soft penumbra)
- **Cast Shadows**: Yes
- **Purpose**: Primary warm light source inside the ruin cluster, simulates a prehistoric firepit

### 2. BioLight_ArchGodray_01 (PointLight)
- **Location**: (50150, 49800, 200)
- **Intensity**: 1500 lm
- **Color**: Bioluminescent green `(R:0.3, G:0.9, B:0.6)`
- **Attenuation Radius**: 500 UU
- **Cast Shadows**: No (fill light only)
- **Purpose**: Bioluminescent plant/moss accent light near archway opening, cool contrast to firepit

### 3. GodRay_Archway_01 (SpotLight)
- **Location**: (50150, 49800, 600)
- **Rotation**: Pitch=-75° (pointing steeply downward through archway gap)
- **Intensity**: 8000 lm
- **Color**: Warm sunlight `(R:1.0, G:0.95, B:0.8)`
- **Inner Cone**: 8° | **Outer Cone**: 22°
- **Attenuation Radius**: 1200 UU
- **Cast Shadows**: Yes
- **Purpose**: Simulates god-ray shaft of sunlight piercing through the archway opening

## Global Lighting Configuration

### Lumen GI (applied via console commands)
```
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.Lumen.Reflections.Allow 1
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.TraceMeshSDFs 1
r.Lumen.ScreenProbeGather.RadianceCache.NumProbeTracesBudget 200
```

### Volumetric Fog (ExponentialHeightFog component)
- **Volumetric Fog**: Enabled
- **Scattering Distribution**: 0.2 (slightly forward-scattering for god-ray visibility)
- **Albedo**: `(R:0.85, G:0.88, B:0.92)` — cool misty atmosphere
- **Extinction Scale**: 0.4 (light fog, not soup)
- **View Distance**: 6000 UU
- **Fog Density**: 0.02
- **Height Falloff**: 0.2

### PostProcess Exposure
- **Mode**: AEM_MANUAL (prevents auto-darken crushing the scene)
- **Exposure Bias**: 1.0
- **Min/Max Brightness**: 1.0 (locked)

### SkyAtmosphere
- Verified present in scene
- FastSkyLUT enabled: `r.SkyAtmosphere.FastSkyLUT 1`
- AerialPerspectiveLUT fast apply: `r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1`

### DirectionalLight (Sun)
- **Pitch**: -45° (confirmed negative — illuminates terrain)
- **Yaw**: 45°
- **Intensity**: 10.0 lux

## Lighting Design Intent
The ruin cluster uses a three-point lighting approach:
1. **Key light** — GodRay SpotLight (warm sunlight shaft, dramatic)
2. **Fill light** — Firepit PointLight (warm orange, ground-level)
3. **Rim/accent** — BioLight (cool green, separates ruins from background)

The volumetric fog makes all three light sources visible as volumetric shafts,
creating a cinematic prehistoric atmosphere consistent with the game's tone.

## Dependencies
- Ruin mesh actors placed by Agent #7 (Architecture) in Cycle 021
- GLB asset: `cretaceous_stone_ruin_archway` (Meshy task `019eecad-9116-7eaa-990f-503c69522e55`)
- Agent #16 (Audio) should add: crackling fire audio at (50150, 50150, 150), jungle ambience loop

## Next Steps for Agent #9 (Character Artist)
- Character should be visible under the Lumen GI lighting
- Ensure MetaHuman skin materials use Lumen-compatible shading model
- Test character silhouette against the god-ray shaft at (50150, 49800, 200)
