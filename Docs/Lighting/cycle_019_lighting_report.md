# Lighting & Atmosphere — Cycle 019 Report
**Agent:** #08 — Lighting & Atmosphere Agent  
**Cycle:** PROD_CYCLE_AUTO_20260703_010  
**Date:** 2026-07-03

---

## What Was Done This Cycle

### 1. CAP Enforcement (Bridge Validation + Scene Guard)
- **Bridge validated:** `bridge_ok` confirmed, world loaded ✅
- **DirectionalLight guard:** Enforced exactly ONE sun at 75,000 lux, pitch ≤ -45°, warm white (1.0, 0.97, 0.88), `atmosphere_sun_light=True`
- **Fog dedup:** Exactly ONE ExponentialHeightFog kept; volumetric fog enabled (`r.VolumetricFog 1`), density=0.02, inscattering blue-sky color
- **SkyAtmosphere dedup:** Duplicates removed, single instance retained
- **SkyLight:** `real_time_capture=True`, intensity=1.5 — ensures Lumen captures sky color correctly

### 2. Lumen GI Console Commands Applied
```
r.Lumen.Reflections.Allow 1
r.Lumen.GlobalIllumination.Allow 1
r.FastSkyLUT 1
r.Lumen.HardwareRayTracing 0
r.VolumetricFog 1
r.VolumetricFog.GridPixelSize 8
r.VolumetricFog.GridSizeZ 64
r.Atmosphere 1
r.SkyAtmosphere.AerialPerspectiveLUT.FastApplyOnOpaque 1
```

### 3. Hub Lighting (X=2100, Y=2400 — Content Hub)
- **GodRay_Hub_001** — RectLight at Z=800, pitch=-80°, 8000 intensity, warm golden color (1.0, 0.92, 0.7), source 200×200, casts shadows → simulates shaft of sunlight piercing canopy
- **WarmFill_Hub_001** — PointLight at Z=300, 3000 intensity, warm amber (1.0, 0.85, 0.6), radius=1500, no shadows → fills the clearing with warm ambient bounce

### 4. Audio Research (Freesound)
Found high-quality prehistoric jungle ambience candidates:
| ID | Name | Duration | Notes |
|----|------|----------|-------|
| 749737 | denseforestwithbirds | 101s | Dense forest, multiple bird species, field recording |
| 813632 | AMBTrop_Daytime tropical forest | 4654s | Panamanian rainforest, insects + birds, professional MKH80 |
| 583930 | jungle forest 02 | 121s | Mayan jungle, morning birds, calm |

**Recommended for MinPlayableMap:** Sound ID 813632 (professional quality, long loop, tropical daytime)

### 5. generate_image FAIL (401) — Fallback Executed
- `generate_image` returned 401 (API key issue) — fallback procedural visual executed in UE5 (GodRay + WarmFill lights at hub)
- No concept art generated this cycle due to API unavailability

---

## Lighting Design Intent (Director of Photography Notes)

The hub clearing at (2100, 2400) must read as **bright Cretaceous midday**:
- **Key light:** DirectionalLight (sun) at 75k lux, pitch -45° → hard shadows, strong directionality
- **Fill:** WarmFill PointLight → simulates ground bounce from warm soil/vegetation
- **Accent:** GodRay RectLight → vertical shaft through canopy, creates depth and drama
- **Atmosphere:** Volumetric fog at low density → haze in distance, god rays visible
- **Sky:** SkyAtmosphere + real-time SkyLight → correct sky color reflected on all surfaces

This creates the **Roger Deakins "invisible lighting"** — the player feels the heat of the Cretaceous sun without consciously noticing the light setup.

---

## Files Created/Modified
- `Docs/Lighting/cycle_019_lighting_report.md` — this report

## UE5 Changes Applied
- Sun: 75k lux, pitch=-45°, warm white, atmosphere_sun_light=True
- Fog: volumetric, density=0.02, blue-sky inscattering
- SkyLight: real_time_capture=True, intensity=1.5
- Lumen GI: all console commands applied
- GodRay_Hub_001: RectLight at hub (2100, 2400, 800)
- WarmFill_Hub_001: PointLight at hub (2100, 2400, 300)
- Level saved

---

## Dependencies for Next Agent (#09 — Character Artist)
- Hub lighting is configured for bright daylight — character models will be lit correctly
- Volumetric fog density is LOW (0.02) — characters will be clearly visible at medium range
- Warm fill light at hub ensures no harsh underlit faces on characters near PlayerStart
- SkyLight real_time_capture ensures MetaHuman skin shaders receive correct sky IBL

## NEXT: Agent #09 should
1. Place/configure the player character (TranspersonalCharacter) at PlayerStart (hub area)
2. Ensure character mesh is visible and correctly lit by the hub lighting setup
3. Add 1-2 NPC dinosaur actors near hub for the hero screenshot composition
