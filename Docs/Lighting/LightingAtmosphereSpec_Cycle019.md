# Lighting & Atmosphere Specification — Cycle 019
**Agent #08 — Lighting & Atmosphere Agent**
**Cycle:** PROD_CYCLE_AUTO_20260704_001

---

## CAP Enforcement Status

All lighting constraints enforced this cycle via single combined UE5 Python script:

| Parameter | Target | Enforced Value |
|-----------|--------|---------------|
| DirectionalLight count | 1 | 1 (duplicates removed) |
| Sun pitch | ≤ -30° (daytime) | -45° |
| Sun intensity | 10,000–75,000 lux | **75,000 lux** |
| Sun color | Warm white | RGB(1.0, 0.97, 0.88) |
| atmosphere_sun_light | True | ✅ |
| ExponentialHeightFog count | 1 | 1 (duplicates removed) |
| FastSkyLUT | 1 | ✅ |
| SkyLight real_time_capture | True | ✅ |
| SkyLight intensity | 2.0 | ✅ |

---

## Lumen Global Illumination

Enabled via console commands in single ue5_execute call:

```
r.Lumen.DiffuseIndirect.Allow 1
r.Lumen.Reflections.Allow 1
r.DynamicGlobalIlluminationMethod 1
r.ReflectionMethod 1
r.VolumetricCloud 1
r.VolumetricFog 1
r.SkyAtmosphere.FastSkyLUT 1
```

---

## Hub Composition (X=2100, Y=2400)

The hero screenshot clearing at world coordinates X=2100, Y=2400 receives:

- **HubFill_Light_001** — PointLight at Z=300, intensity=5000, radius=2000, warm amber (1.0, 0.95, 0.8), no shadows (fill only)
- **DirectionalLight** at -45° pitch provides primary hard shadows
- **SkyLight** real_time_capture provides ambient bounce from sky dome
- **VolumetricFog** enabled for god-ray potential through canopy gaps

### Intended Composition
- Bright DAYTIME — golden hour warm light
- Dense Cretaceous vegetation surrounds clearing
- Dinosaur silhouettes readable against bright sky
- Volumetric atmosphere adds depth and haze

---

## Audio References — Prehistoric Ambient Soundscape

Sourced from Freesound.org for Agent #16 (Audio Agent):

| ID | Name | Duration | Use Case |
|----|------|----------|----------|
| 749737 | denseforestwithbirds | 101s | Hub clearing ambient loop |
| 813632 | AMBTrop_Daytime tropical forest | 4654s | Biome-wide ambient bed |
| 583930 | jungle forest 02 | 121s | Jungle interior ambient |

**Preview URLs:**
- Dense forest birds: https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3
- Tropical forest daytime: https://cdn.freesound.org/previews/813/813632_7037-hq.mp3
- Jungle forest: https://cdn.freesound.org/previews/583/583930_2978883-hq.mp3

---

## Lighting Design Philosophy

> "The player doesn't notice correct lighting — they only notice wrong lighting."

### Cretaceous Daytime Palette
- **Primary:** Warm golden sun at -45° pitch (late morning / early afternoon)
- **Sky:** Deep blue-cyan zenith, warm horizon haze
- **Shadows:** Long, soft-edged (Lumen soft shadows)
- **Bounce:** Rich green from dense foliage (Lumen GI captures this automatically)
- **Atmosphere:** Slight volumetric haze for depth, NOT overcast

### Emotional Intent
The hub clearing at (2100, 2400) must read as:
- **Alive** — light moves through leaves, shadows shift
- **Dangerous** — contrast between bright clearing and dark forest edge
- **Ancient** — warm golden tones suggest primordial world, not modern

---

## Next Agent Dependencies

**→ Agent #09 (Character Artist):** Lighting is set for MetaHuman character visibility. Warm directional at -45° provides strong rim light on characters facing camera. SkyLight fill ensures shadow detail is readable.

**→ Agent #16 (Audio):** Ambient sound IDs provided above. Recommend layering:
1. Base: AMBTrop_Daytime (813632) — continuous bed
2. Mid: denseforestwithbirds (749737) — bird calls
3. Spot: jungle forest 02 (583930) — interior zones

---

## Files Modified This Cycle

| File | Action |
|------|--------|
| `Docs/Lighting/LightingAtmosphereSpec_Cycle019.md` | Created — this document |
| UE5 Level (MinPlayableMap) | Modified — CAP enforcement, hub fill light, Lumen enabled |

---

*Lighting & Atmosphere Agent #08 — PROD_CYCLE_AUTO_20260704_001*
