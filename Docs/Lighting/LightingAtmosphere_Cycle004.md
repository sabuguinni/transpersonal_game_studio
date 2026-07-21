# Lighting & Atmosphere System — Agent #8 Cycle 004
## Production Date: PROD_CYCLE_AUTO_20260619_004

---

## Overview

This document records the lighting and atmosphere configuration applied to `MinPlayableMap` in Cycle 004.
The visual direction follows Roger Deakins' principle: **light does not illuminate — light means.**
Every lighting decision serves the emotional intent of the prehistoric survival world.

---

## Emotional Intent: Golden Hour Prehistory

The primary lighting mood is **late-afternoon golden hour** — the most dangerous hunting time in the Cretaceous.
Long shadows from the sun reveal terrain texture and give the ruin cluster dramatic depth.
Volumetric fog creates mystery and distance, suggesting unseen threats beyond the tree line.
Warm torchlight at the ruin cluster implies ancient human presence — someone was here before.

---

## Systems Configured This Cycle

### 1. DirectionalLight (Sun_Main)
| Property | Value | Rationale |
|----------|-------|-----------|
| Intensity | 10.0 lux | Strong enough to illuminate terrain clearly |
| Color | RGB(1.0, 0.92, 0.78) | Warm golden-hour amber, not harsh white noon |
| Pitch | **-45°** | CRITICAL: negative pitch = sun points DOWN = terrain lit |
| Yaw | 45° | Southwest angle creates long diagonal shadows on ruins |
| Cast Shadows | True | Essential for Lumen GI shadow casting |
| Dynamic Shadow Distance | 50,000 cm | Full map coverage |

**Memory Rule Enforced:** `hugo_sun_pitch_negative_proven` — pitch=-45 confirmed by screenshot test.
Positive pitch (+45) causes terrain to be completely black. Never use positive pitch.

### 2. Lumen Global Illumination
| Console Command | Value | Effect |
|-----------------|-------|--------|
| r.Lumen.DiffuseIndirect.Allow | 1 | Enable Lumen GI for indirect lighting |
| r.Lumen.Reflections.Allow | 1 | Enable Lumen reflections on wet surfaces |
| r.DynamicGlobalIlluminationMethod | 1 | Set to Lumen (not SSGI or DFAO) |
| r.ReflectionMethod | 1 | Lumen reflections active |

Lumen GI ensures the ruin stone materials receive correct indirect bounce light from the golden sun.
When the Meshy GLB ruin assets are imported, Lumen will automatically pick up their PBR materials.

### 3. SkyAtmosphere
- Actor: `SkyAtmosphere_Main` (spawned if not present)
- Provides physically-based sky rendering tied to DirectionalLight direction
- At pitch=-45°, sky shows warm orange/amber horizon with blue zenith — correct for golden hour
- Works in conjunction with SkyLight for ambient fill

### 4. SkyLight
| Property | Value | Rationale |
|----------|-------|-----------|
| Intensity | 1.5 | Soft ambient fill, not overpowering the directional |
| Real Time Capture | True | Captures SkyAtmosphere changes dynamically |

Real-time capture ensures the sky color changes propagate to ambient lighting as day/night cycle evolves.

### 5. ExponentialHeightFog (Volumetric)
| Property | Value | Rationale |
|----------|-------|-----------|
| Fog Density | 0.02 | Subtle — visible at distance, not oppressive nearby |
| Fog Height Falloff | 0.2 | Fog concentrates near ground level |
| Volumetric Fog | **True** | 3D volumetric scattering, not flat 2D fog |
| Scattering Distribution | 0.2 | Slight forward scattering for god rays |
| Fog Albedo | RGB(0.75, 0.8, 0.85) | Cool blue-grey, contrasts warm sun |
| Extinction Scale | 1.0 | Standard visibility |
| Inscattering Color | RGB(0.6, 0.7, 0.9) | Blue atmospheric haze at distance |
| Start Distance | 100 cm | Fog starts just beyond player, not in their face |

The cool fog color against the warm directional light creates the classic "golden hour haze" look.
Volumetric fog enables god rays (light shafts) when the sun angle aligns with openings in the canopy.

### 6. Ruin Cluster Accent Lights (Torchlight)
Three warm point lights placed around the ruin cluster at (50000, 50000):

| Label | Location | Color | Intensity | Radius |
|-------|----------|-------|-----------|--------|
| RuinLight_Torch1 | (50200, 50200, 150) | RGB(1.0, 0.55, 0.1) | 3000 | 800 cm |
| RuinLight_Torch2 | (49800, 50300, 150) | RGB(1.0, 0.5, 0.08) | 2800 | 750 cm |
| RuinLight_Torch3 | (50100, 49700, 120) | RGB(1.0, 0.6, 0.15) | 2500 | 700 cm |

These warm orange-amber lights suggest ancient torches or fire remnants at the ruin site.
They create visual interest and guide the player toward the ruin cluster as a point of interest.
The warm torchlight contrasts with the cool volumetric fog for depth and atmosphere.

### 7. Overexposed Light Reduction
All PointLights and SpotLights with intensity > 5000 were reduced to 2000.
This prevents the "blown out" overexposed look that destroys atmosphere.

---

## Lighting Philosophy for Prehistoric Survival

### Time of Day Emotional Mapping
| Time | Mood | Lighting State |
|------|------|----------------|
| Dawn (5-7am) | Hope / Vulnerability | Soft pink-orange, low contrast |
| Morning (7-11am) | Active / Alert | Cool white, sharp shadows |
| Noon (11am-2pm) | Harsh / Dangerous | Bright white, short shadows, high contrast |
| Golden Hour (4-6pm) | Beautiful / Deadly | Warm amber, long shadows, fog |
| Dusk (6-8pm) | Tense / Transition | Deep orange-red, silhouettes |
| Night (8pm-5am) | Terror / Survival | Dark blue, point light dependency |

**Current State:** Golden Hour (4-6pm) — most cinematic, most dangerous.

### Lumen GI Impact on Gameplay
- Stone ruins receive warm bounce light from sun — readable and visually rich
- Dense jungle areas stay darker — natural threat zones
- Open clearings are bright — safe zones visually communicated through lighting
- Player can read danger level from lighting alone — no HUD needed for basic orientation

---

## Integration Notes for Agent #9 (Character Artist)

- Character skin should be set up with subsurface scattering for warm golden-hour skin tones
- MetaHuman materials will automatically receive Lumen GI bounce from terrain
- Avoid pure-black or pure-white materials — they break Lumen GI energy conservation
- Character shadow length at pitch=-45° is approximately 1:1 with character height

---

## Integration Notes for Agent #17 (VFX)

- Volumetric fog is active — Niagara particle emitters should account for fog depth
- God ray opportunities: gaps in canopy above ruin cluster at (50000, 50000)
- Fire/torch VFX at ruin lights: `RuinLight_Torch1/2/3` positions are torch spawn points
- Dust mote particles will be visible in volumetric fog light shafts

---

## Files Modified
- `/Game/Maps/MinPlayableMap` — lighting actors configured and saved
- `Docs/Lighting/LightingAtmosphere_Cycle004.md` — this document

## Status
- `bridge_ok` ✅
- `CAP_SAFE` ✅  
- `DLIGHT_FIXED` ✅ (pitch=-45, intensity=10, warm color)
- `LUMEN_ENABLED` ✅
- `SKYATM_FOUND/SPAWNED` ✅
- `SKYLIGHT_FOUND/SPAWNED` ✅
- `FOG_UPDATED/SPAWNED` ✅ (volumetric=True)
- `RUIN_LIGHTS_SPAWNED` ✅ (3 torchlight points)
- `ATM_SAVED` ✅
