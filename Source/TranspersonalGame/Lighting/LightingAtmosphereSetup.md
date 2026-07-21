# Lighting & Atmosphere Setup — Agent #08
## Cycle: PROD_CYCLE_AUTO_20260704_012

### STATUS: BRIDGE DOWN — DEGRADED MODE

The UE5 bridge (Remote Control API port 30010) is non-responsive for the 5th consecutive cycle.
All ue5_execute commands timeout. generate_image API returning 401 auth errors.

---

## Intended Lighting Configuration (Ready to Execute When Bridge Restores)

### Primary Sun — DirectionalLight
- **Intensity**: 75,000 lux (bright midday daylight)
- **Color**: Warm white (R=1.0, G=0.95, B=0.85)
- **Rotation**: Pitch=-45°, Yaw=45° (sun from upper-right)
- **atmosphere_sun_light**: True
- **cast_shadows**: True
- **Label**: Sun_Primary_001

### Sky System
- **SkyAtmosphere**: 1x instance (SkyAtmosphere_001)
- **SkyLight**: 1x instance with real_time_capture=True, intensity=3.0 (SkyLight_001)

### Lumen Global Illumination
```
r.Lumen.Enabled 1
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.MaxTraceDistance 20000
r.Lumen.Scene.SurfaceCacheResolution 1
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
```

### Volumetric Fog — ExponentialHeightFog
- **fog_density**: 0.02 (subtle atmospheric haze)
- **fog_height_falloff**: 0.2
- **fog_inscattering_color**: Sky blue (R=0.6, G=0.75, B=0.9)
- **start_distance**: 5000 units
- **volumetric_fog**: True
- **volumetric_fog_scattering_distribution**: 0.2
- **volumetric_fog_extinction_scale**: 0.5
- **Label**: AtmosphericFog_001

### Hub Area Fill Lights (X=2100, Y=2400)
1. **FillLight_Hub_Canopy_001** — Green-tinted canopy bounce
   - Location: (2100, 2400, 300)
   - Intensity: 5000 lux
   - Color: (R=0.8, G=1.0, B=0.6) — jungle green bounce
   - Radius: 3000 units, no shadows

2. **FillLight_Hub_Sky_001** — Cool sky fill
   - Location: (2300, 2200, 400)
   - Intensity: 3000 lux
   - Color: (R=0.7, G=0.85, B=1.0) — sky blue
   - Radius: 2500 units, no shadows

### Post Process Volume
- **Unbound**: True (affects entire level)
- **Label**: PostProcess_Hub_001
- **Location**: (2100, 2400, 100)

---

## Emotional Intent (Roger Deakins Principle)

The hub clearing at (2100, 2400) must read as:
- **ALIVE** — dappled light through canopy, god rays, green bounce
- **DANGEROUS** — deep shadows at forest edges, contrast between clearing and jungle
- **PREHISTORIC** — warm golden sun, thick atmospheric haze, no artificial sources
- **CINEMATIC** — single strong key light (sun), fill from sky, bounce from vegetation

The player must feel small and exposed in the clearing, with the forest edge feeling like it hides predators.

---

## CAP Rules (Enforced Every Cycle)
1. EXACTLY 1 DirectionalLight — destroy duplicates
2. Sun pitch MUST be between -30° and -60° (never flat/night)
3. Intensity MUST be >= 10,000 lux (server guard enforces floor)
4. atmosphere_sun_light MUST be True
5. EXACTLY 1 SkyAtmosphere, EXACTLY 1 SkyLight
6. Fog density <= 0.05 (no thick fog that blocks daylight)

---

## Next Cycle Priority
When bridge restores:
1. Execute the Python script above (all in one ue5_execute call)
2. Verify hub area reads as bright daylight
3. Add SpotLight for T-Rex dramatic rim lighting at hub center
4. Configure PostProcessVolume with Bloom + Exposure settings
