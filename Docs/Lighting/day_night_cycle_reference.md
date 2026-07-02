# Day/Night Cycle — Complete Reference

**Agent**: #08 — Lighting & Atmosphere Agent  
**Last Updated**: PROD_CYCLE_AUTO_20260702_009

---

## Overview

The Cretaceous world uses a 4-phase day/night cycle with distinct emotional tones.
Each phase has been implemented across cycles 006–009 and is fully documented here
for reference by all agents (especially #09 Character Artist, #10 Animation, #16 Audio, #17 VFX).

---

## Phase 1: Night (Cycle 006)

**Emotional tone**: Danger, isolation, primal fear  
**Time of day**: 22:00 – 04:00

| Parameter | Value |
|-----------|-------|
| Sun pitch | -72° (below horizon) |
| Sun color | Deep blue RGB(80, 100, 160) |
| Sun intensity | 0.3 lux |
| Fog density | 0.042 |
| Fog albedo | Indigo RGB(0.15, 0.12, 0.28) |
| Volumetric fog | Enabled |
| SkyLight intensity | 0.4 |
| Special lights | Bioluminescent point lights (blue-green, 300–500 intensity) |

**Gameplay implications**:
- Predators more active — T-Rex and raptors hunt at night
- Player visibility severely reduced — torch/fire essential
- Bioluminescent plants provide navigation landmarks
- Temperature drops — cold survival mechanic activates

---

## Phase 2: Dawn (Cycle 009)

**Emotional tone**: Discovery, hope, cautious optimism  
**Time of day**: 04:00 – 07:00

| Parameter | Value |
|-----------|-------|
| Sun pitch | -8° (just above horizon) |
| Sun yaw | 45° (NE) |
| Sun color | Rose-gold RGB(255, 200, 160) |
| Sun intensity | 2.5 lux |
| Fog density | 0.022 |
| Fog albedo | Warm dawn RGB(0.92, 0.85, 0.78) |
| Volumetric fog | Enabled |
| Fog extinction scale | 1.4 |
| SkyLight intensity | 1.2 |
| Special lights | God-ray shafts through ruin gaps (8000/5500 intensity SpotLights) |

**Gameplay implications**:
- Safest transition period — nocturnal predators retreating
- Herbivore herds begin morning grazing
- Morning mist (fog density 0.022) provides stealth opportunities
- Best time for exploration and resource gathering

---

## Phase 3: Golden Hour (Cycle 007)

**Emotional tone**: Warmth, activity, urgency (day ending)  
**Time of day**: 07:00 – 10:00 and 17:00 – 19:00

| Parameter | Value |
|-----------|-------|
| Sun pitch | -18° |
| Sun color | Amber RGB(255, 180, 80) |
| Sun intensity | 8.0 lux |
| Fog density | 0.028 |
| Fog albedo | Warm haze RGB(0.95, 0.88, 0.72) |
| Volumetric fog | Enabled |
| SkyLight intensity | 2.0 |
| Special lights | 5× warm fill lights (800–1200 intensity) |

**Gameplay implications**:
- Peak activity for all dinosaur species
- Long shadows create dramatic gameplay moments
- Warm light makes environment feel alive and vibrant
- Ideal for cinematic story moments

---

## Phase 4: Dusk (Cycle 008)

**Emotional tone**: Tension, transition, impending danger  
**Time of day**: 19:00 – 22:00

| Parameter | Value |
|-----------|-------|
| Sun pitch | -22° |
| Sun color | Orange-amber RGB(255, 160, 60) |
| Sun intensity | 1.8 lux |
| Fog density | 0.035 |
| Fog albedo | Warm purple-orange RGB(0.88, 0.72, 0.65) |
| Volumetric fog | Enabled |
| SkyLight intensity | 0.8 |
| Special lights | Warm rim (west) + cool purple fill (east) |

**Gameplay implications**:
- Nocturnal predators beginning to stir
- Player must find shelter before full dark
- Dramatic silhouette lighting for T-Rex encounters
- Last chance to gather resources before night

---

## Lumen Configuration (All Phases)

These settings are constant across all phases:

```
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.Reflections.Allow 1
r.Lumen.TraceMeshSDFs 1
r.VolumetricFog 1
r.SkyAtmosphere.FastSkyLUT 1
```

---

## CAP Enforcement Rules (Immutable)

These rules are enforced at the start of every lighting cycle:

1. **Sun pitch guard**: Sun pitch must be ≤ -8° (never above horizon in gameplay)
2. **Fog dedup**: Maximum 1 ExponentialHeightFog actor in scene
3. **FastSkyLUT**: Always enabled for performance
4. **SkyLight**: Always real_time_capture=True
5. **Lumen**: Always enabled (GI + Reflections)
6. **Map save**: Always save after lighting changes

---

## Ruin Area Lighting (Permanent)

The ruin cluster at **(50000, 50000)** has permanent atmospheric lighting:

| Actor | Type | Purpose |
|-------|------|---------|
| RuinShaft_Altar_001 | SpotLight | Vertical god-ray over altar |
| RuinShaft_NW_001 | SpotLight | Angled dawn shaft from NW |
| RuinFill_East_001 | PointLight | Cool blue sky fill |
| RuinEmber_Altar_001 | PointLight | Warm ground glow |
| RuinLight_*** (×4) | PointLight | General ambient fill |

These lights are designed to work across all day phases — they shift in perceived color
as the ambient light changes, maintaining visual interest at any time of day.

---

## Audio Integration Notes (for Agent #16)

Recommended Freesound.org assets per phase:

| Phase | Primary Sound | Freesound ID |
|-------|--------------|--------------|
| Night | Nocturnal insects + distant roars | TBD |
| Dawn | Mountain forest birds + mist | 860231 |
| Golden Hour | Active jungle + pterosaur calls | TBD |
| Dusk | Evening insects + predator distant calls | TBD |

Dawn reference confirmed: ID 860231 — Taiwan Alishan Early Morning Forest Birds (40s, crisp mountain dawn)

---

## VFX Integration Notes (for Agent #17)

| Phase | Recommended Niagara Effects |
|-------|---------------------------|
| Night | Firefly particles, bioluminescent spore drift |
| Dawn | Mist particle wisps rising from ground, dew drops |
| Golden Hour | Dust motes in light shafts, pollen drift |
| Dusk | Ember particles near fire sources, bat swarms |

The volumetric fog in all phases supports Niagara particle interaction —
particles will be occluded by fog correctly with r.VolumetricFog 1 active.
