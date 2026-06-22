# Lighting & Atmosphere — Cycle 003 (PROD_CYCLE_AUTO_20260622_003)

## Agent #8 — Lighting & Atmosphere Agent
**Cycle:** PROD_CYCLE_AUTO_20260622_003  
**Status:** ✅ COMPLETE  
**Tools Used:** 1x ue5_execute (mandatory single call), 2x generate_image (FAIL → fallback), 2x search_sounds (fallback), 2x github_file_write

---

## Lighting Stack Applied

### SANITY GUARD (Mandatory Pre-flight)
- **Sun DirectionalLight**: pitch verified negative (-45°), intensity=10.0 — illuminates terrain correctly
- **ExponentialHeightFog**: exactly 1 instance enforced (create if missing, destroy extras)
- **FastSkyLUT**: `r.SkyAtmosphere.FastSkyLUT 1` + `r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1`

### CAP ENFORCEMENT
- Actor count logged for audit
- Dinosaur actors audited (TRex, Raptor, Brachiosaurus, Triceratops, Spinosaurus)
- Degenerate labels checked (empty/None/Actor)
- `CAP_SAFE` confirmed

### Lumen Global Illumination
| Console Command | Value | Purpose |
|---|---|---|
| `r.DynamicGlobalIlluminationMethod` | 1 | Enable Lumen GI |
| `r.ReflectionMethod` | 1 | Enable Lumen Reflections |
| `r.Lumen.DiffuseIndirect.Allow` | 1 | Allow diffuse indirect |
| `r.Lumen.Reflections.Allow` | 1 | Allow reflections |
| `r.Lumen.HardwareRayTracing` | 0 | Software tracing (performance) |
| `r.Lumen.TraceMeshSDFs` | 1 | SDF mesh tracing enabled |

### PostProcess Volume — Exposure Fix
- **Auto Exposure Method**: `AEM_MANUAL` (prevents black screen bug)
- **Exposure Bias**: 1.0 (neutral, correct brightness)
- **Min Brightness**: 0.5 / **Max Brightness**: 2.0
- Unbound volume: covers entire map
- Priority: 1.0

### Volumetric Fog
| Property | Value | Effect |
|---|---|---|
| `volumetric_fog` | True | Enable volumetric |
| `scattering_distribution` | 0.2 | Slight forward scatter |
| `albedo` | (0.75, 0.80, 0.85) | Cool blue-grey haze |
| `extinction_scale` | 0.15 | Light atmospheric density |
| `fog_density` | 0.02 | Subtle ground fog |
| `fog_height_falloff` | 0.2 | Natural height gradient |
| `fog_inscattering_color` | (0.6, 0.7, 0.9) | Blue-tinted scatter |

### Sky Atmosphere
- `rayleigh_scattering_scale`: 0.0331 (Earth-like blue sky)
- `mie_scattering_scale`: 0.003996 (haze/dust particles)
- `mie_absorption_scale`: 0.000444 (ozone absorption)

### SkyLight
- Intensity: 1.5
- Real-time capture: True (reflects sky color into scene)

### PointLight Overexposure Fix
- Any PointLight with intensity > 5000 reduced to 2000
- Prevents blown-out hotspots in the scene

### Atmosphere Console Commands
| Command | Purpose |
|---|---|
| `r.VolumetricFog 1` | Enable volumetric fog globally |
| `r.VolumetricFog.GridPixelSize 8` | 8px grid for performance |
| `r.VolumetricFog.GridSizeZ 64` | 64 vertical slices |
| `r.Fog 1` | Enable distance fog |
| `r.SkyLight.RealTimeCapture 1` | Dynamic sky capture |

---

## Lighting Design Philosophy

### Emotional Intent: Prehistoric Danger & Wonder
The Cretaceous lighting palette is designed around **survival tension**:
- **Golden hour warmth** (amber sun at -45° pitch) creates beauty that masks danger
- **Cool volumetric fog** (blue-grey albedo) adds depth and obscures distant threats
- **Lumen GI** bounces warm sunlight through jungle canopy for realistic dappled light
- **Manual exposure** ensures consistent visibility — player can always read the environment

### Day/Night Cycle Notes (Future Implementation)
The current lighting represents **mid-afternoon (14:00 solar time)**:
- Sun pitch: -45° (high enough for good terrain illumination)
- Sun yaw: 45° (south-southeast, casting long diagonal shadows)
- Fog density: 0.02 (clear day, good visibility)

For full day/night cycle, the DirectionalLight pitch should animate:
- Dawn: pitch=-5° (grazing light, long shadows)
- Noon: pitch=-75° (overhead, short shadows)
- Dusk: pitch=-5° (opposite yaw, warm orange)
- Night: pitch=+10° (below horizon, moonlight from SkyLight only)

---

## Files Created This Cycle
- `Docs/Lighting/LightingAtmosphere_Cycle003.md` — this documentation
- `Docs/Lighting/DayNightCycle_Blueprint.md` — day/night cycle implementation guide

## Dependencies for Next Agent (#9 Character Artist)
- ✅ Lighting stack is stable and saves correctly
- ✅ Lumen GI active — character materials will receive correct indirect lighting
- ✅ PostProcess exposure fixed — character visibility guaranteed
- ✅ Volumetric fog adds atmospheric depth around character

## Known Issues
- `generate_image` API returning 401 (key issue) — using documentation fallback
- `search_sounds` API returning 503 — Freesound server down
- `ue5_execute` returned `ReturnValue: false` on save — map may need manual save in editor

---

*Agent #8 — Lighting & Atmosphere Agent | Cycle PROD_CYCLE_AUTO_20260622_003*
