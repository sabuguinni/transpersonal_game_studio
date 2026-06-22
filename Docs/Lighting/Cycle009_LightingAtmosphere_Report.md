# Lighting & Atmosphere — Cycle 009 Report
**Agent #08 | PROD_CYCLE_AUTO_20260622_009**

---

## Systems Applied This Cycle

### 1. Sanity Guard (Mandatory)
- Sun pitch validated: MUST be negative (pitch=-45, yaw=45) → illuminates terrain correctly
- Fog count enforced: exactly 1 ExponentialHeightFog
- FastSkyLUT + AerialPerspectiveLUT enabled for performance

### 2. CAP Enforcement
- Actor count logged
- Dinosaur audit: labels checked for trex/raptor/brach/dino/rex/saur
- Degenerate label check (empty/None labels)

### 3. Lumen Global Illumination Stack
| Console Command | Value | Purpose |
|---|---|---|
| r.DynamicGlobalIlluminationMethod | 1 | Enable Lumen GI |
| r.ReflectionMethod | 1 | Enable Lumen Reflections |
| r.Lumen.DiffuseIndirect.Allow | 1 | Diffuse GI on |
| r.Lumen.Reflections.Allow | 1 | Reflections on |
| r.Lumen.HardwareRayTracing | 0 | Software Lumen (performance) |
| r.Lumen.ScreenProbeGather.RadianceCache.NumProbeTracesBudget | 200 | Quality budget |

### 4. Volumetric Clouds
| Command | Value |
|---|---|
| r.VolumetricCloud | 1 |
| r.VolumetricCloud.ShadowMap.RaymarchMaxStepNum | 16 |
| r.VolumetricCloud.ViewRaymarchMaxStepNum | 96 |

### 5. Volumetric Fog
| Command | Value |
|---|---|
| r.VolumetricFog | 1 |
| r.VolumetricFog.GridPixelSize | 8 |
| r.VolumetricFog.GridSizeZ | 64 |

### 6. Post-Process Exposure Fix
- EyeAdaptation.ExponentialTransitionDistance = 1.5
- EyeAdaptation.LensAttenuation = 0.78
- Prevents auto-exposure over-darkening the scene

---

## New Actors Spawned

### Atm_RuinAmberLight_01 (PointLight)
- **Location**: (50100, 50100, 250) — inside ruin pillar cluster from Agent #07
- **Intensity**: 500 lux
- **Color**: Warm amber — LinearColor(1.0, 0.55, 0.15)
- **Temperature**: 3200K (torch/firelight range)
- **Attenuation Radius**: 800 units
- **Cast Shadows**: True
- **Purpose**: Creates shelter ambience inside Cretaceous ruins; contrasts with exterior cool daylight

### Atm_RuinMoonFill_01 (SpotLight)
- **Location**: (49800, 49800, 600) — elevated above ruin cluster
- **Rotation**: pitch=-60, yaw=135 (angled down from opposite direction to amber)
- **Intensity**: 150 lux
- **Color**: Cool blue — LinearColor(0.6, 0.75, 1.0)
- **Outer Cone**: 45°, Inner Cone: 20°
- **Attenuation Radius**: 1200 units
- **Purpose**: Moonlight fill — creates dramatic warm/cool contrast inside ruins

---

## Exponential Height Fog Tuning
| Property | Value | Rationale |
|---|---|---|
| fog_density | 0.02 | Light atmospheric haze, not soup |
| fog_height_falloff | 0.2 | Gradual vertical fade |
| fog_max_opacity | 0.85 | Preserves distant visibility |
| start_distance | 2000 units | No fog in near field |
| fog_cutoff_distance | 200000 units | Horizon haze |
| volumetric_fog | True | 3D volumetric enabled |
| volumetric_fog_scattering_distribution | 0.2 | Slightly forward-scattering |
| volumetric_fog_albedo | (0.75, 0.78, 0.82) | Neutral grey-blue mist |
| volumetric_fog_distance | 6000 units | Fog depth budget |

---

## Lighting Design Philosophy (This Cycle)

The ruin cluster from Agent #07 now has a two-source lighting contrast:
- **Warm amber (3200K)** from within — suggests fire, shelter, human presence
- **Cool blue moonlight** from above-opposite — suggests danger, exposure, the unknown

This is a classic cinematographic technique (Roger Deakins, RDR2 art team):
> "The best lighting is invisible when correct, and destroys immersion when wrong."

The amber/cool contrast tells the player: *inside the ruins = relative safety; outside = exposed to predators*.

---

## API Status
- `generate_image`: ❌ FAIL (OpenAI key 401 — invalid key)
- `search_sounds`: ❌ No results returned (Freesound API no results for queries)
- `ue5_execute`: ✅ cmd_19078 executed

---

## Integration Notes for Agent #09 (Character Artist)

The ruin cluster lighting is now active at world coordinates (50000–50200, 50000–50200, 100–400).
When placing character MetaHumans or NPC pawns near this area:
- Skin tones will receive warm amber bounce from below-left
- Cool blue rim from above-right (moonlight fill)
- This creates natural cinematic character lighting without additional setup

The PostProcess exposure fix (EyeAdaptation tuning) ensures characters are not over-darkened
when transitioning between bright exterior and shadowed ruin interior.

---

## Files
- `Docs/Lighting/Cycle009_LightingAtmosphere_Report.md` — this report

## Next Agent (#09 — Character Artist)
- Place MetaHuman or character pawn near ruin cluster (50000, 50000) to benefit from existing lighting rig
- Verify character materials respond correctly to Lumen GI bounce
- Sun pitch guard is holding at -45° — do not modify DirectionalLight rotation
