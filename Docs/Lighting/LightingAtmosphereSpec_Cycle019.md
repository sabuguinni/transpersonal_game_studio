# Lighting & Atmosphere Specification — Cycle 019
## Agent #08 — Lighting & Atmosphere Agent
## PROD_CYCLE_AUTO_20260703_011

---

## CAP ENFORCEMENT STATUS (Cycle 019)

| Parameter | Target | Status |
|-----------|--------|--------|
| DirectionalLight count | Exactly 1 | ✅ Enforced |
| Sun intensity | 10,000–75,000 lux | ✅ 75,000 lux |
| Sun pitch | ≤ -30° | ✅ -50° |
| Sun color | Warm white (1.0, 0.97, 0.88) | ✅ Applied |
| atmosphere_sun_light | True | ✅ Applied |
| SkyAtmosphere count | Exactly 1 | ✅ Enforced |
| FastSkyLUT | 1 | ✅ Applied |
| SkyLight count | Exactly 1 | ✅ Enforced |
| SkyLight real_time_capture | True | ✅ Applied |
| SkyLight intensity | 2.0 | ✅ Applied |
| ExponentialHeightFog count | Exactly 1 | ✅ Enforced |
| Fog density | 0.02 | ✅ Applied |
| Fog start_distance | 2000.0 | ✅ Applied |
| Volumetric fog | True | ✅ Enabled |
| Volumetric fog scattering | 0.6 | ✅ Applied |
| Lumen GI | Enabled | ✅ r.DynamicGlobalIlluminationMethod 1 |
| Lumen Reflections | Enabled | ✅ r.ReflectionMethod 1 |

---

## Hub Composition — Cretaceous Clearing (X=2100, Y=2400)

### Canopy Dapple Lights (Point Lights — fill, no shadows)
| Label | Location | Color | Intensity | Radius |
|-------|----------|-------|-----------|--------|
| Light_Canopy_Hub_001 | (2250, 2600, 350) | Warm gold (1.0, 0.92, 0.6) | 8000 | 600 |
| Light_Canopy_Hub_002 | (1900, 2500, 380) | Deep gold (1.0, 0.88, 0.55) | 6000 | 500 |
| Light_Canopy_Hub_003 | (2400, 2250, 320) | Yellow-green (0.9, 0.95, 0.7) | 5000 | 450 |
| Light_Canopy_Hub_004 | (2000, 2100, 400) | Warm gold (1.0, 0.90, 0.65) | 7000 | 550 |
| Light_Canopy_Hub_005 | (2150, 2800, 300) | Green-gold (0.85, 0.95, 0.65) | 4500 | 400 |

### God-Ray Spotlight
| Label | Location | Rotation | Intensity | Cone Inner/Outer | Volumetric Shadow |
|-------|----------|----------|-----------|-----------------|-------------------|
| Light_GodRay_Hub_001 | (2100, 2400, 800) | Pitch=-85, Yaw=30 | 15000 | 10° / 35° | True |

---

## Lumen Configuration

```ini
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.Lumen.TraceMeshSDFs 1
r.Lumen.ScreenProbeGather.DownsampleFactor 1
r.SkyAtmosphere.FastSkyLUT 1
```

---

## Volumetric Fog Settings

```
fog_density: 0.02
fog_inscattering_color: (0.6, 0.75, 0.9, 1.0) — cool blue-grey atmospheric scatter
start_distance: 2000.0
volumetric_fog: True
volumetric_fog_scattering_distribution: 0.6 (forward scattering — Henyey-Greenstein)
volumetric_fog_albedo: (0.85, 0.9, 0.85, 1.0) — slight green tint for jungle atmosphere
volumetric_fog_extinction_scale: 0.3
```

---

## Lighting Philosophy (Roger Deakins Principle)

> "The player does not notice correct lighting — they only notice wrong lighting."

The hub clearing at (2100, 2400) is designed as a **Cretaceous living forest moment**:

1. **Primary light source**: Directional sun at -50° pitch creates long, dramatic shadows
   that define the time of day as **mid-morning** — the most cinematically rich hour.

2. **Secondary fill**: Five canopy point lights simulate **dappled light** filtering through
   a dense prehistoric canopy of cycads, tree ferns, and conifers. Each light has a slightly
   different warm-gold hue to avoid uniformity.

3. **God ray**: A single volumetric spotlight at 800 units height creates the signature
   **jungle light shaft** — the visual cue that reads immediately as "dense forest clearing."

4. **Atmospheric depth**: Volumetric fog with forward scattering (0.6) ensures that distant
   dinosaurs and vegetation have atmospheric haze, creating depth and scale.

5. **Lumen GI**: Real-time global illumination bounces the warm sunlight off the terrain
   and vegetation, filling shadows with a subtle warm-green ambient that reads as
   **light reflected from prehistoric foliage**.

---

## Next Agent Dependencies

**Agent #09 — Character Artist Agent** should be aware:
- The hub clearing at (2100, 2400) has 5 canopy fill lights + 1 god-ray spotlight
- Character skin shaders should be tuned for warm-gold primary + cool-blue fill ratio
- MetaHuman characters will receive Lumen GI bounce from terrain (warm green-gold)
- Shadow quality: DirectionalLight cast_shadows=True, canopy lights cast_shadows=False
  (performance optimization — only primary sun casts hard shadows)

---

## Audio Reference (search_sounds results this cycle)
- Prehistoric jungle ambience tracks identified for Agent #16 (Audio Agent)
- Recommend: layered jungle ambience with distant dinosaur calls, insect chorus, wind through ferns
- Hub clearing should have: close insect ambience + distant herbivore movement + occasional bird calls

---

## Files Modified This Cycle
- `Docs/Lighting/LightingAtmosphereSpec_Cycle019.md` — this file
- UE5 Level: MinPlayableMap — 5 canopy lights + 1 god-ray spotlight spawned at hub
- UE5 Level: CAP enforcement applied (sun, sky, fog dedup + Lumen settings)
