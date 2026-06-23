# Lighting & Atmosphere System — Cycle 013
**Agent #08 | PROD_CYCLE_AUTO_20260622_013**

## System Overview
Full dynamic lighting stack for the Cretaceous prehistoric survival world.
Implemented via UE5 Python (Remote Control) — all changes live in MinPlayableMap.

---

## Components Active in MinPlayableMap

### 1. Directional Light (Sun_Main)
- **Pitch**: -45° (negative = illuminates terrain downward)
- **Yaw**: 45° (southeast angle for dramatic shadows)
- **Intensity**: 10.0 lux
- **Rule**: pitch MUST be negative. Positive pitch = black terrain (confirmed by screenshot 17 Jun 2026)

### 2. Sky Atmosphere (SkyAtmosphere_Main)
- Rayleigh scattering enabled
- FastSkyLUT = 1 (performance optimization)
- AerialPerspectiveLUT.FastApply = 1

### 3. Sky Light (SkyLight_Main)
- Real-time capture enabled
- Intensity: 1.0
- Captures sky color for ambient bounce light

### 4. Exponential Height Fog
- Fog density: 0.02
- Inscattering color: cool blue-grey (0.4, 0.6, 0.8)
- Height falloff: 0.2
- **Volumetric fog**: ENABLED
- Scattering distribution: 0.2 (slight forward scatter)
- Albedo: 0.75 (neutral grey)
- Extinction scale: 1.0

### 5. Lumen Global Illumination
- `r.DynamicGlobalIlluminationMethod 1` — Lumen enabled
- `r.ReflectionMethod 1` — Lumen reflections
- `r.Lumen.DiffuseIndirect.Allow 1`
- `r.Lumen.Reflections.Allow 1`
- `r.Lumen.TraceMeshSDFs 1` — accurate SDF tracing

### 6. PostProcess Volume (MainPostProcess_Lighting)
- Infinite extent: true
- Priority: 1.0
- **Auto Exposure Method**: AEM_MANUAL (prevents auto-darkening)
- **Exposure Bias**: 1.0
- Min/Max brightness: 0.5 / 2.0

---

## Cinematic Intent (Roger Deakins Principle)
> "The player doesn't notice correct lighting — they only notice wrong lighting."

### Time of Day: Golden Hour (Default)
- Sun at -45° pitch creates long diagonal shadows
- Warm directional light + cool sky ambient = natural contrast
- Volumetric fog adds depth and atmospheric perspective
- God rays visible through dense canopy geometry

### Emotional Tone: Dangerous Beauty
- The world looks alive and vast
- Shadows hide predators (gameplay tension)
- Warm light = false sense of safety
- Fog = unknown distance = fear of the unknown

---

## Sanity Guard Rules (Auto-enforced each cycle)
1. Sun pitch MUST be negative (< 0)
2. Exactly 1 ExponentialHeightFog actor
3. FastSkyLUT = 1 always
4. PostProcess exposure = AEM_MANUAL
5. Point lights > 5000 intensity → clamped to 2000

---

## Known Issues & Next Steps
- `generate_image` API returning 401 (OpenAI key invalid) — fallback to search_sounds
- Freesound API returning empty results — audio pipeline blocked
- Next cycle: implement day/night cycle blueprint via UE5 Python timeline
- Next cycle: add sunrise/sunset color temperature shift (warm→cool→warm)

---

## Files
| File | Purpose |
|------|---------|
| `Docs/Lighting/LightingAtmosphereSystem_Cycle013.md` | This document |
| `Source/TranspersonalGame/Lighting/DayNightCycle.h` | Day/night cycle C++ header |
| `Source/TranspersonalGame/Lighting/DayNightCycle.cpp` | Day/night cycle implementation |

---

## CAP Audit (Cycle 013)
- Total actors in MinPlayableMap: reported by UE5 bridge
- Dinosaur actors: audited (trex, raptor, brach, dino labels)
- Map saved: MinPlayableMap
