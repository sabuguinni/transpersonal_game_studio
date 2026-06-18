# Lighting & Atmosphere System — Cycle 006
**Agent:** #08 Lighting & Atmosphere Agent  
**Cycle:** PROD_CYCLE_AUTO_20260618_006  
**Status:** COMPLETE

---

## Systems Implemented This Cycle

### 1. DirectionalLight (Sun) — Fixed & Configured
- **Intensity:** 8.0 lux (was near-zero causing deep terrain shadow)
- **Color:** Warm amber-white (1.0, 0.95, 0.85) — Cretaceous golden hour
- **Pitch:** -45° (correct sun angle, illuminates terrain from above)
- **Yaw:** 30° (slight east offset for dramatic shadow casting)
- **atmosphere_sun_light:** True — drives SkyAtmosphere scattering
- **cast_shadows:** True — Lumen shadow casting enabled

### 2. SkyAtmosphere — Spawned/Verified
- Label: `SkyAtmosphere_001`
- Physically-based atmosphere scattering
- Driven by DirectionalLight as atmosphere_sun_light
- Produces correct horizon glow and sky color gradient

### 3. SkyLight — Spawned/Verified
- Label: `SkyLight_001`
- Intensity: 1.5 (ambient fill from sky dome)
- cast_shadows: True
- Provides soft ambient illumination in shadowed areas

### 4. ExponentialHeightFog + Volumetric Fog
- Label: `HeightFog_001`
- fog_density: 0.02 (subtle prehistoric haze)
- fog_height_falloff: 0.2 (thicker near ground, thins at altitude)
- fog_inscattering_color: cool blue-grey (0.5, 0.6, 0.7) — atmospheric depth
- start_distance: 500 units
- **volumetric_fog: True** — god rays, light shafts through jungle canopy
- volumetric_fog_view_distance: 6000 units
- volumetric_fog_scattering_distribution: 0.2 (forward scattering)
- volumetric_fog_albedo: (0.75, 0.8, 0.85) — misty jungle atmosphere

### 5. PostProcessVolume (Lumen)
- Label: `PostProcess_Lumen_001`
- **unbound: True** — affects entire world
- Enables Lumen Global Illumination and Reflections
- Tone mapping for cinematic prehistoric look

### 6. Overexposed Light Reduction
- All PointLights/SpotLights with intensity > 5000 reduced to 2000
- Prevents blown-out renders and color bleeding

---

## Lighting Design Intent (Roger Deakins Principle)

> "The player doesn't notice correct lighting — only wrong lighting."

The Cretaceous world uses a **golden hour baseline** with:
- **Warm sun** (amber-white) from 45° elevation — dramatic terrain shadows
- **Cool sky fill** (blue-grey SkyLight) — separation between lit/shadow
- **Volumetric fog** — depth, atmosphere, god rays through canopy
- **Subtle haze** — distance falloff creates sense of vast prehistoric scale

This is NOT decorative. Each element serves survival gameplay:
- Fog limits visibility → tension, predator ambush potential
- God rays mark clearings → navigation cues
- Shadow contrast → player reads terrain depth for climbing/jumping

---

## UE5 Lumen Configuration Notes

For full Lumen GI (requires r.Lumen.* console vars):
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.HardwareRayTracing 0
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
```

These should be set in Project Settings → Rendering → Global Illumination.

---

## Day/Night Cycle — Architecture (Next Phase)

The lighting system is designed for dynamic day/night:
- DirectionalLight pitch: -90° (midnight) → -45° (golden hour) → 0° (noon) → +45° (sunset)
- SkyAtmosphere automatically updates sky color based on sun angle
- SkyLight should recapture every N minutes for accurate ambient
- Fog density: 0.04 (night/dawn) → 0.02 (day) → 0.03 (dusk)
- Point lights (campfires, torches) activate at night via Blueprint timeline

Blueprint class to implement: `BP_DayNightCycle`
- Timeline: 0.0 = midnight, 0.5 = noon, 1.0 = midnight
- Duration: 1200 seconds (20-minute real-time day)
- Drives DirectionalLight rotation + SkyLight recapture + fog density

---

## Files Created
- `Docs/Lighting/AtmosphereSystem_Cycle006.md` — this document

## UE5 Actors Configured
- `Sun_Directional_001` — DirectionalLight (fixed/spawned)
- `SkyAtmosphere_001` — physically-based sky
- `SkyLight_001` — ambient sky fill
- `HeightFog_001` — volumetric fog + height haze
- `PostProcess_Lumen_001` — Lumen GI + tone mapping

## Next Agent (#09 Character Artist)
- World lighting is now set up for character visibility
- Warm sun from 45° will illuminate character from above-right
- Volumetric fog provides atmospheric depth behind characters
- PostProcess Lumen is active — character materials will benefit from GI
- Recommend: character skin materials use Subsurface profile for Lumen compatibility
