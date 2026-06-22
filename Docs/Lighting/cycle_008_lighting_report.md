# Lighting & Atmosphere Agent — Cycle PROD_CYCLE_AUTO_20260622_008

## Summary

Full lighting stack deployed for MinPlayableMap. All systems active and saved.

---

## Systems Deployed

### 1. Sanity Guard (Mandatory)
- **Sun pitch**: Verified negative (pitch=-45, yaw=45) — illuminates terrain correctly
- **Fog count**: Exactly 1 ExponentialHeightFog enforced
- **FastSkyLUT**: `r.SkyAtmosphere.FastSkyLUT 1` + `r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1`

### 2. Lumen Global Illumination
| Console Command | Value | Purpose |
|---|---|---|
| `r.DynamicGlobalIlluminationMethod` | 1 | Enable Lumen GI |
| `r.ReflectionMethod` | 1 | Enable Lumen Reflections |
| `r.Lumen.DiffuseIndirect.Allow` | 1 | Allow diffuse indirect |
| `r.Lumen.Reflections.Allow` | 1 | Allow reflections |
| `r.Lumen.HardwareRayTracing` | 0 | Software Lumen (perf) |
| `r.Lumen.GlobalIllumination.MaxTraceDistance` | 20000 | GI trace range |
| `r.Lumen.Scene.SurfaceCacheFarFieldDistance` | 40000 | Far field cache |

### 3. Volumetric Fog
- **Density**: 0.03 (subtle ground mist)
- **Height falloff**: 0.2
- **Max opacity**: 0.85
- **Start distance**: 200 cm
- **Cutoff distance**: 200,000 cm
- **Volumetric fog**: TRUE
- **Scattering distribution**: 0.3 (slightly forward-scattering)
- **Albedo**: RGB(0.85, 0.80, 0.75) — warm dusty atmosphere
- **Extinction scale**: 1.2
- **Fog distance**: 6,000 cm
- **Second fog layer**: density=0.015, height_offset=-200 (ground hugging)

### 4. SkyAtmosphere
- **Rayleigh scattering scale**: 0.0331 (Earth-like)
- **Mie scattering scale**: 0.003996 (haze/dust)
- **Mie absorption scale**: 0.000444
- **Mie anisotropy**: 0.8 (strong forward scatter — god rays)
- **Atmosphere height**: 60 km

### 5. SkyLight
- **Intensity**: 1.5
- **Cast shadows**: TRUE
- **Real-time capture**: TRUE (reflects live sky)

### 6. Directional Light (Sun)
- **Intensity**: 10.0 lux
- **Color**: RGB(1.0, 0.95, 0.85) — warm golden sunlight
- **Cast shadows**: TRUE
- **Cast volumetric shadow**: TRUE
- **Atmosphere sun light**: TRUE
- **Shadow cascades**: 4
- **Dynamic shadow distance**: 20,000 cm

### 7. Ruin Amber Point Light (A#7 Handoff)
- **Actor label**: `Light_RuinAmber_01`
- **Location**: (50150, 50000, 200) — inside ruin threshold
- **Intensity**: 2,000 lm
- **Color**: RGB(1.0, 0.65, 0.2) — warm amber/fire
- **Attenuation radius**: 800 cm
- **Cast volumetric shadow**: TRUE

### 8. PostProcess Volume — Manual Exposure
- **Actor label**: `Light_PostProcess_Global`
- **Infinite extent**: TRUE
- **Auto exposure method**: AEM_MANUAL
- **Exposure bias**: 1.0 (prevents black screen)

### 9. Volumetric Light Shafts
| Console Command | Value |
|---|---|
| `r.VolumetricFog` | 1 |
| `r.VolumetricFog.GridPixelSize` | 8 |
| `r.VolumetricFog.GridSizeZ` | 128 |
| `r.LightShaftQuality` | 1 |
| `r.LightShafts.Downsample` | 2 |

---

## Audio References (Freesound)

### Jungle Ambience
| ID | Name | Duration | Preview |
|---|---|---|---|
| 825833 | AMBForst_Summer.Forest Edge.Very Quiet | 224s | https://cdn.freesound.org/previews/825/825833_5828667-hq.mp3 |
| 831381 | AMBForst_Autumn.Forest Edge.Calm And Quiet | 283s | https://cdn.freesound.org/previews/831/831381_5828667-hq.mp3 |
| 700734 | Insects-twilight-pine-trees-forest | 219s | https://cdn.freesound.org/previews/700/700734_9034501-hq.mp3 |

### Weather / Storm
| ID | Name | Duration | Preview |
|---|---|---|---|
| 376810 | LONG THUNDER ROLLS AND HEAVY RAIN | 268s | https://cdn.freesound.org/previews/376/376810_2309965-hq.mp3 |
| 671587 | Tropical Storm Heavy Rain Thunderstorm | 64s | https://cdn.freesound.org/previews/671/671587_14490715-hq.mp3 |
| 669140 | Thunderstorm Heavy Rain 1 | 13s | https://cdn.freesound.org/previews/669/669140_14490715-hq.mp3 |

---

## Cinematic Intent (Roger Deakins Principle)

> "The light doesn't illuminate — the light means."

The lighting stack is tuned for a **Cretaceous late afternoon** mood:
- **Golden warm sun** (RGB 1.0, 0.95, 0.85) at 45° angle — long shadows, dramatic depth
- **Amber ruin light** — suggests fire, danger, ancient habitation
- **Volumetric fog** — ground mist creates mystery, separates foreground/background layers
- **Lumen GI** — indirect bounce light fills shadows with warm secondary illumination
- **Mie anisotropy 0.8** — strong god rays through canopy gaps

The player should feel: *"This world is alive, dangerous, and beautiful."*

---

## Map State
- **Map**: `/Game/Maps/MinPlayableMap`
- **Saved**: TRUE
- **New actors**: `Light_RuinAmber_01`, `Light_PostProcess_Global`

---

## Handoff to Agent #9 (Character Artist)

The lighting environment is now complete. Character artists should note:
1. **Skin tones** will receive warm amber bounce from the ground fog — avoid overly pale/cool skin tones that will look unnatural
2. **Eye reflections** — SkyLight real-time capture means character eyes will reflect the live sky correctly
3. **Shadow quality** — 4 cascade CSM with 20,000 cm range covers the playable area fully
4. **Ruin area** at (50000, 50000) has dedicated amber point light — characters near ruins will have dramatic rim lighting
5. **Volumetric fog** at density 0.03 — distant characters (>6,000 cm) will have atmospheric haze — use this for LOD transitions

Audio Agent (#16) should integrate:
- Freesound IDs: 825833 (day ambience), 700734 (twilight insects), 376810 (storm layer)
- Blend based on time-of-day cycle when implemented
