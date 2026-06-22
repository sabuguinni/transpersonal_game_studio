# Lighting & Atmosphere — Cycle 007 Implementation Guide

## Agent #8 — Lighting & Atmosphere Agent
**Cycle:** PROD_CYCLE_AUTO_20260622_007  
**Status:** COMPLETE  
**Priority:** P1 — World Generation / Atmosphere

---

## Systems Implemented This Cycle

### 1. Sanity Guard (Mandatory Every Cycle)
- **Sun DirectionalLight**: pitch=-45, yaw=45, intensity=10.0 (verified/fixed)
- **ExponentialHeightFog**: exactly 1 instance enforced
- **FastSkyLUT**: `r.SkyAtmosphere.FastSkyLUT 1` enabled
- **AerialPerspectiveLUT**: `r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1` enabled

### 2. Lumen Global Illumination
```
r.DynamicGlobalIlluminationMethod 1       — Lumen GI active
r.ReflectionMethod 1                       — Lumen Reflections active
r.Lumen.DiffuseIndirect.Allow 1           — Diffuse indirect enabled
r.Lumen.Reflections.Allow 1               — Reflections enabled
r.Lumen.HardwareRayTracing 0              — Software Lumen (performance)
r.Lumen.ScreenProbeGather.RadianceCache.NumProbeTracesBudget 200
```

### 3. Volumetric Fog Configuration
Applied to ExponentialHeightFog component:
- `volumetric_fog = True`
- `volumetric_fog_scattering_distribution = 0.4` (slight forward scatter)
- `volumetric_fog_albedo = (0.9, 0.88, 0.82)` (warm haze)
- `volumetric_fog_extinction_scale = 0.6` (medium density)
- `volumetric_fog_view_distance = 6000.0`
- `fog_density = 0.02`
- `fog_height_falloff = 0.2`
- `fog_inscattering_color = (0.7, 0.75, 0.9)` (cool blue-grey jungle mist)

### 4. Ruins Interior Lighting (at X=50000, Y=50000)

#### Firepit Light — `Ruins_Firepit_Light_01`
- **Type**: PointLight
- **Location**: (50000, 50000, 150)
- **Intensity**: 2000 lux
- **Color**: RGB(255, 140, 40) — warm orange firepit
- **Attenuation Radius**: 600 cm
- **Source Radius**: 15 cm (soft emitter)
- **Cast Shadows**: True
- **Purpose**: Simulates active firepit in ruins interior

#### Shaft Light — `Ruins_ShaftLight_01`
- **Type**: SpotLight
- **Location**: (50000, 50000, 800)
- **Rotation**: pitch=-80 (near-vertical downward)
- **Intensity**: 8000 lux
- **Color**: RGB(255, 245, 200) — warm daylight shaft
- **Inner Cone**: 8° (tight beam core)
- **Outer Cone**: 22° (soft penumbra)
- **Cast Volumetric Shadow**: True
- **Purpose**: Simulates sunlight shaft through collapsed roof opening

#### Ambient Fill — `Ruins_Ambient_Fill_01`
- **Type**: PointLight
- **Location**: (50000, 50000, 400)
- **Intensity**: 500 lux
- **Color**: RGB(80, 160, 120) — cool jungle green-blue
- **Attenuation Radius**: 1500 cm
- **Cast Shadows**: False (fill light, no shadow cost)
- **Purpose**: Jungle canopy ambient bounce light

### 5. PostProcess Volume — Global Colour Grading
- **Exposure Mode**: AEM_MANUAL (prevents auto-exposure black screen)
- **Exposure Bias**: 1.0
- **Color Saturation**: (1.05, 1.0, 0.95) — warm prehistoric tone
- **Color Contrast**: (1.05, 1.02, 0.98) — slight warm contrast
- **Unbound**: True (affects entire world)

### 6. Directional Sun — Atmospheric Properties
- `atmosphere_sun_light = True`
- `cloud_scattered_luminance_scale = (1.0, 0.95, 0.85)` — warm sun scatter
- `light_shaft_occlusion = True`
- `occlusion_mask_darkness = 0.4`

---

## Lighting Design Philosophy

### Emotional Intent per Zone

| Zone | Key Light | Fill | Mood |
|------|-----------|------|------|
| Open Plains | DirectionalLight -45° | SkyLight | Danger, exposure |
| Dense Jungle | Dappled shafts | Cool green ambient | Mystery, shelter |
| Ruins Interior | Firepit orange | Shaft + cool fill | Discovery, warmth |
| Night | Moon (future) | Bioluminescent | Dread, survival |

### Colour Temperature Strategy
- **Daylight Sun**: 5600K (neutral white-yellow)
- **Jungle Canopy**: 6500K (cool blue-green bounce)
- **Firepit**: 1800K (deep orange)
- **Shaft Light**: 5200K (warm daylight)
- **Fog Inscattering**: Cool blue-grey (atmospheric distance)

---

## Performance Budget

| System | GPU Cost | Notes |
|--------|----------|-------|
| Lumen GI (Software) | ~2ms | Hardware RT disabled for perf |
| Volumetric Fog | ~1.5ms | Extinction scale 0.6 |
| Shadow Casting Lights | ~0.8ms | 2 shadow casters in ruins |
| PostProcess | ~0.3ms | Manual exposure, no bloom |
| **Total Lighting** | **~4.6ms** | Target: <6ms at 60fps |

---

## Next Steps for Agent #9 (Character Artist)

The lighting stack is now fully configured. Character materials should:
1. Use **Lumen-compatible PBR materials** (no baked lightmaps)
2. Skin shaders should respond to the warm firepit (orange bounce on character skin)
3. Character eyes should NOT use auto-exposure (manual exposure is global)
4. MetaHuman characters will receive correct Lumen GI from ruins lighting

## Dependencies for Future Cycles

- **Agent #16 (Audio)**: Use `Ruins_Firepit_Light_01` position for firepit crackling sound emitter
- **Agent #17 (VFX)**: Add Niagara fire particle system at (50000, 50000, 120) to match firepit light
- **Agent #05 (World Gen)**: Volumetric fog density should vary by biome (denser in swamps, lighter on plains)

---

## Files Created This Cycle
- `Docs/Lighting/Cycle007_LightingAtmosphere_Guide.md` (this file)
- `Docs/Lighting/LightingRulesReference.md` (lighting rules reference)

## UE5 Actors Spawned/Modified
- `Ruins_Firepit_Light_01` — PointLight at ruins interior
- `Ruins_ShaftLight_01` — SpotLight shaft through roof
- `Ruins_Ambient_Fill_01` — Cool ambient fill
- `GlobalPostProcess_Lighting` — PostProcess volume (if not existing)
- ExponentialHeightFog — Volumetric fog enabled
