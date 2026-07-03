# Lighting & Atmosphere — Agent #08 | PROD_CYCLE_AUTO_20260703_008

## Cycle Summary

### CAP Enforcement Applied
- **DirectionalLight (Sun_Primary)**: 75,000 lux, pitch=-50°, warm white (1.0, 0.97, 0.88), atmosphere_sun_light=True, cast_shadows=True
- **SkyAtmosphere**: Exactly 1 instance (SkyAtmosphere_Primary), duplicates destroyed
- **SkyLight (SkyLight_Primary)**: real_time_capture=True, intensity=1.5
- **ExponentialHeightFog (HeightFog_Primary)**: density=0.02, volumetric=True, jungle-haze color (0.6, 0.75, 0.9), scattering=0.6, extinction=0.8
- **FastSkyLUT=1** applied via console command

### Lumen GI Console Commands Applied
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.HardwareRayTracing 0
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget 200
r.FastBlurThreshold 0
r.SkyLight.RealTimeReflectionCapture 1
r.VolumetricFog 1
r.VolumetricFog.GridPixelSize 8
r.VolumetricFog.GridSizeZ 64
sg.ShadowQuality 3
r.Shadow.CSM.MaxCascades 4
```

### Hub Area Lighting (X=2100, Y=2400)
- **RectLight_GodRay_Hub**: 8,000 lux, warm amber (1.0, 0.95, 0.75), 400x400 source, pitch=-80°, cast_shadows=True
- **FillLight_Hub_East**: PointLight, 2,000 lux, cool blue-white, radius=1,200
- **FillLight_Hub_West**: PointLight, 2,000 lux, cool blue-white, radius=1,200
- **FillLight_Hub_North**: PointLight, 2,000 lux, cool blue-white, radius=1,200

### Audio References Found (Freesound.org)
| ID | Name | Duration | Tags |
|----|------|----------|------|
| 749737 | denseforestwithbirds | 101s | dawn, dense forest, birdsong, ambiance |
| 813632 | AMBTrop_Daytime tropical forest | 4654s | rainforest, birds, insects, drip, nature |

Preview URLs:
- https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3
- https://cdn.freesound.org/previews/813/813632_7037-hq.mp3

### generate_image Status
- Both attempts returned FAIL (401 — API key invalid)
- Fallback: procedural UE5 lighting setup executed instead (hub fill lights + god-ray RectLight)

### Level State
- Level saved after all changes
- All CAP rules enforced: 1 sun, 1 atmosphere, 1 skylight, 1 fog

## Lighting Philosophy Applied
> "The player doesn't notice correct lighting — they only notice wrong lighting."
> — Roger Deakins principle

The hub clearing at (2100, 2400) is configured for **bright Cretaceous midday**:
- Overhead sun at -50° pitch creates strong directional shadows from vegetation
- Volumetric fog at low density (0.02) gives jungle atmospheric depth without obscuring visibility
- God-ray RectLight simulates canopy light shafts breaking through tree cover
- Cool fill lights from east/west/north simulate sky bounce light in open clearing
- Warm sun + cool fill = natural outdoor lighting contrast for cinematic depth

## Handoff to Agent #09 — Character Artist
The lighting environment is ready for character placement:
- Bright daylight (75k lux) ensures characters are clearly visible
- Volumetric fog adds depth separation between character and background
- Hub clearing is well-lit for hero screenshot composition
- All duplicate lights removed — no performance overhead from redundant actors
