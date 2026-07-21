# Cretaceous Lighting & Atmosphere — Cycle 002 Guide

## Overview
This document describes the complete lighting setup deployed to `MinPlayableMap` across cycles 001–002.
All lighting is implemented via UE5 Python (Remote Control API) — no C++ required.

---

## Lighting Architecture

### Philosophy (Roger Deakins Approach)
> "The player doesn't notice correct lighting — they only notice wrong lighting."

Every light in this world serves an **emotional function**, not just a technical one:
- **Warm amber** = safety, warmth, life (near player shelter, daytime savanna)
- **Cool blue/teal** = danger, unknown, ancient mystery (ruins, swamp, night)
- **Green bounce** = dense jungle canopy, biological abundance
- **Silver moonlight** = isolation, vulnerability, nocturnal predator territory

---

## Deployed Actors (MinPlayableMap)

### Cycle 001 — Core Lighting
| Actor Label | Type | Purpose |
|---|---|---|
| `Sun_Directional` | DirectionalLight | Primary sun — warm amber (255,224,165), intensity 10.0, atmosphere-coupled |
| `SkyAtmosphere_Cretaceous` | SkyAtmosphere | Realistic atmosphere scattering |
| `VolumetricCloud_Cretaceous` | VolumetricCloud | Dynamic cloud layer |
| `SkyLight_Lumen_Cretaceous` | SkyLight | Lumen sky contribution, real-time capture |
| `Fog_Volumetric_Cretaceous` | ExponentialHeightFog | Ground-level volumetric fog |

### Cycle 002 — Ruin Cluster & Biome Fills
| Actor Label | Type | Location | Color | Purpose |
|---|---|---|---|---|
| `Ruin_Light_Amber_001` | PointLight | (49600, 50000, 180) | Warm amber | Dramatic ruin pillar key light |
| `Ruin_Light_Amber_002` | PointLight | (50400, 50000, 180) | Warm amber | Ruin pillar fill |
| `Ruin_Light_Cool_001` | PointLight | (50000, 49600, 220) | Cool blue | Mysterious ruin shadow fill |
| `Ruin_Light_Cool_002` | PointLight | (50000, 50400, 200) | Cool blue | Ruin atmosphere |
| `Ruin_Light_Bounce_001` | PointLight | (50000, 50000, 80) | Warm bounce | Ground bounce for Lumen |
| `Fill_Forest_N` | PointLight | (20000, 20000, 300) | Green-tinted | North forest biome ambient |
| `Fill_Forest_S` | PointLight | (-20000, -20000, 300) | Green-tinted | South forest biome ambient |
| `Fill_Savanna_E` | PointLight | (30000, -10000, 400) | Warm yellow | East savanna ambient |
| `Fill_Savanna_W` | PointLight | (-30000, 10000, 400) | Warm yellow | West savanna ambient |
| `Fill_Swamp_001` | PointLight | (5000, 35000, 150) | Teal-green | Swamp biome ambient |
| `Fill_River_001` | PointLight | (15000, 5000, 200) | Cool blue | River reflection ambient |

---

## Lumen Configuration

### Recommended Project Settings (apply via Editor Preferences)
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.Lumen.HardwareRayTracing 1
r.Lumen.ScreenProbeGather.RadianceCache.NumProbesToTraceBudget 200
r.Lumen.GlobalIllumination.MaxTraceDistance 180000
r.Lumen.Scene.SurfaceCacheResolution 1.0
```

### Volumetric Fog Settings (deployed)
- `fog_density`: 0.025 — subtle ground mist
- `fog_height_falloff`: 0.18 — hugs terrain
- `fog_max_opacity`: 0.85 — visible but not blinding
- `volumetric_fog`: True — enables Lumen volumetric scattering
- `volumetric_fog_scattering_distribution`: 0.55 — forward scattering (god rays)
- `volumetric_fog_albedo`: (0.72, 0.80, 0.75) — slightly greenish jungle mist
- `volumetric_fog_distance`: 8000.0 cm — 80m volumetric depth

---

## Day/Night Cycle (Planned — Cycle 003+)

### Blueprint Architecture
```
BP_DayNightCycle
├── TimeOfDay (float 0.0–24.0)
├── SunAngle (mapped from TimeOfDay)
├── DirectionalLight reference → set rotation each tick
├── SkyAtmosphere → auto-updates from DirectionalLight
├── ExponentialHeightFog → density curve (denser at dawn/dusk)
└── SkyLight → recapture every 30s (performance budget)
```

### Emotional Time Zones
| Time | Mood | Sun Color | Fog Density | Danger Level |
|---|---|---|---|---|
| 05:00–07:00 | Dawn — hope | Pink-orange | High (0.04) | Medium |
| 07:00–12:00 | Morning — safe | Warm white | Low (0.01) | Low |
| 12:00–15:00 | Midday — harsh | Bright white | Minimal | Low |
| 15:00–18:00 | Golden hour | Deep amber | Medium (0.02) | Medium |
| 18:00–20:00 | Dusk — danger | Red-orange | High (0.05) | High |
| 20:00–05:00 | Night — survival | Moonblue | Very high (0.08) | Critical |

---

## Biome Lighting Palette Reference

### Forest Biome
- Primary: Dappled green-gold (#8DB56A tint)
- Shadow: Deep forest green (#2A4A2A)
- Atmosphere: Dense canopy blocks 60% direct light

### Savanna Biome
- Primary: Harsh amber-white (#FFE8A0)
- Shadow: Long purple-blue shadows at dawn/dusk
- Atmosphere: Heat shimmer at midday (distortion post-process)

### Swamp Biome
- Primary: Diffuse teal-grey (#4A7A6A)
- Shadow: Almost no hard shadows — overcast feel
- Atmosphere: Dense ground fog, bioluminescent accents at night

### Ruin Cluster (50000, 50000)
- Primary: Warm amber torchlight (deployed cycle 002)
- Secondary: Cool blue mystery fill (deployed cycle 002)
- Atmosphere: Dramatic contrast, god rays through broken roof

---

## Next Steps (Cycle 003)

1. **Day/Night Blueprint** — `BP_DayNightCycle` with timeline-driven sun rotation
2. **Weather System** — rain/storm/clear state machine affecting fog density
3. **Torch/Fire Emissive Lights** — point lights attached to VFX campfire actors
4. **Post-Process Volume** — cinematic color grading per biome zone
5. **Lumen Console Commands** — apply via `run_console_command` for optimal quality

---

## Dependencies
- Agent #7 (Architecture): Ruin pillar actors at (50000, 50000) — ✅ confirmed placed
- Agent #6 (Environment): Tree/rock placement for Lumen bounce surfaces — needed
- Agent #17 (VFX): Campfire/torch emitters for dynamic light sources — needed
- Agent #5 (World Gen): Biome boundaries for zone-specific post-process volumes — needed

---

*Lighting & Atmosphere Agent #8 — Cycle 002 — Transpersonal Game Studio*
