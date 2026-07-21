# Cinematic Lighting System — Agent #8 Specification
## Transpersonal Game Studio — Prehistoric Survival Game

### Overview
Full cinematic lighting pipeline for the Cretaceous biome using UE5 Lumen + Atmosphere.
All systems applied via Python to MinPlayableMap.

---

## Lighting Components Deployed

### 1. Directional Sun Light — `Sun_GoldenHour`
- **Intensity**: 8.5 lux
- **Color Temperature**: 3200K warm amber (RGB 0.98, 0.85, 0.65)
- **Rotation**: Pitch -35°, Yaw 45° (late afternoon golden hour angle)
- **Volumetric Scattering**: 2.5x intensity
- **Shadow Cascades**: 4 (high quality)
- **Atmosphere Sun Light**: Enabled (drives sky atmosphere color)
- **Cast Volumetric Shadow**: Enabled

### 2. Sky Atmosphere — `SkyAtmosphere_Cretaceous`
- UE5 physical sky simulation
- Driven by Directional Light angle
- Produces realistic sunrise/sunset color gradients
- Rayleigh scattering for blue sky, Mie scattering for haze

### 3. Volumetric Clouds — `VolumetricClouds_Cretaceous`
- Physical cloud simulation
- Casts dynamic shadows on terrain
- Interacts with Directional Light for dramatic cloud lighting

### 4. Sky Light — `SkyLight_Ambient`
- **Intensity**: 1.2
- **Color**: Soft blue-white (0.7, 0.85, 1.0) — sky ambient fill
- **Cast Shadows**: Enabled
- Captures sky atmosphere for realistic ambient occlusion

### 5. Exponential Height Fog — `HeightFog_Cretaceous`
- **Fog Density**: 0.035 (subtle ground haze)
- **Height Falloff**: 0.2 (thicker near ground, clears at altitude)
- **Inscattering Color**: Cool blue-grey (0.55, 0.65, 0.85)
- **Max Opacity**: 0.85
- **Start Distance**: 200 units
- **Volumetric Fog**: ENABLED
  - Scattering Distribution: 0.35 (forward scattering for god rays)
  - Albedo: Light blue-white (0.75, 0.82, 0.9)
  - Extinction Scale: 1.2
  - View Distance: 6000 units

### 6. Post Process Volume — `PostProcess_Cinematic`
- **Unbound**: True (affects entire level)
- **Lumen Final Gather Quality**: 2.0 (high)
- **Lumen Scene Lighting Quality**: 2.0 (high)
- **Lumen Scene Detail**: 1.0

---

## Emotional Intent by Time of Day

### Golden Hour (Current Setup)
- Warm amber sun at low angle
- Long dramatic shadows across terrain
- God rays through jungle canopy
- Emotional tone: **Wonder + Danger** — the world is beautiful but the predators are active

### Dawn (Future Implementation)
- Cool blue-pink sky gradient
- Rising mist from swamps
- Soft diffuse lighting — low contrast
- Emotional tone: **Hope + Vulnerability** — new day, player is exposed

### Midday (Future Implementation)
- Harsh overhead sun, deep shadows
- High contrast, bleached highlights
- Heat shimmer (distortion VFX)
- Emotional tone: **Survival pressure** — heat, exposure, predator activity peak

### Night (Future Implementation)
- Moon as secondary directional light (cool blue, 0.3 lux)
- Stars via sky sphere
- Bioluminescent ferns (point lights, green-blue)
- Emotional tone: **Terror + Awe** — darkness hides predators, stars are beautiful

---

## Lumen Configuration Notes

Lumen Global Illumination requires:
- `r.Lumen.Enabled 1` (console command)
- `r.Lumen.Reflections.Enabled 1`
- `r.RayTracing.Enabled 0` (Lumen uses software GI, not RT)
- Hardware RT optional for higher quality reflections

Apply via console: `r.Lumen.DiffuseIndirect.Allow 1`

---

## Integration with Other Agents

| Agent | Dependency |
|-------|-----------|
| #05 World Generator | Terrain height data needed for fog height calibration |
| #06 Environment Artist | Foliage density affects volumetric fog visibility |
| #07 Architecture | Interior lighting requires separate point/spot lights |
| #17 VFX | Niagara particle emitters (campfire, dust) interact with Lumen |
| #16 Audio | Time-of-day state drives audio transitions (dawn chorus → night insects) |

---

## Performance Budget

| Component | GPU Cost (est.) | Notes |
|-----------|----------------|-------|
| Lumen GI | ~2.5ms | Quality 2.0 |
| Volumetric Fog | ~1.2ms | 6000 unit range |
| Volumetric Clouds | ~0.8ms | Standard quality |
| Shadow Cascades (4) | ~1.5ms | Directional light |
| **Total** | **~6ms** | Within 60fps budget (16.6ms frame) |

---

## Files Modified
- `/Game/Maps/MinPlayableMap` — Lighting actors placed and configured
- `Docs/Lighting/CinematicLightingSystem_Spec.md` — This document

## Next Steps for Agent #9 (Character Artist)
- Characters will be lit by this golden hour setup
- Skin shaders should use Subsurface Scattering for warm sun interaction
- Character ambient occlusion will be driven by Lumen
- Recommend testing MetaHuman under this lighting before finalizing skin tones
