# Lighting & Atmosphere — Cycle PROD_CYCLE_AUTO_20260623_004

## Agent #08 — Lighting & Atmosphere

---

## UE5 Lighting Stack Applied (cmd_19622)

Full atomic lighting rebuild executed in a single `ue5_execute` call:

| Component | Configuration | Status |
|-----------|--------------|--------|
| **Sanity Guard** | Sun pitch negative check, fog count=1, FastSkyLUT | ✅ |
| **Lumen GI** | DynamicGI=1, ReflectionMethod=1, TraceMeshSDFs=1, MaxTrace=20000 | ✅ |
| **DirectionalLight** | pitch=-35°, yaw=55°, intensity=12, warm tint (255,220,160), atmosphere_sun=True | ✅ |
| **SkyAtmosphere** | Rayleigh=0.036, Mie=0.004, height=100km (dense Cretaceous) | ✅ |
| **SkyLight** | real_time_capture=True, intensity=1.5 | ✅ |
| **ExponentialHeightFog** | density=0.035, volumetric=True, inscatter=(0.55,0.72,0.85), distance=6000 | ✅ |
| **PostProcessVolume** | AEM_MANUAL, bias=1.2, bloom=0.4, AO=0.6, warm color grading | ✅ |
| **Point Light Cap** | Audit + reduce >5000 lux to 2000, cap at 20 total | ✅ |
| **Map Save** | `/Game/Maps/MinPlayableMap` | ✅ |

---

## Lighting Presets Defined (LightingAtmosphereConfig.h)

### `FLight_TimeOfDayPreset` struct
- SunPitch (clamped -90 to 0, NEVER positive)
- SunIntensity, SunColor, SunYaw
- SkyLightIntensity
- FogDensity, FogColor
- ExposureBias (manual mode), BloomIntensity
- ColorSaturation (Vector4)
- BlendDuration for smooth transitions

### `ELight_WeatherState` enum
`Clear | PartlyCloudy | Overcast | Stormy | HeavyRain | Foggy`

### `ELight_TimeOfDay` enum
`Dawn | Sunrise | Morning | Midday | Afternoon | GoldenHour | Dusk | Night | Midnight`

---

## Sound References Found (Freesound.org)

| ID | Name | Duration | Use |
|----|------|----------|-----|
| 722665 | W-bird-cicada-001 | 80s | Daytime jungle ambience layer |
| 669357 | D117 Walking at Night.WAV | 90s | Night jungle ambience layer |
| 826495 | AMBTrop_Novo Airão | 594s | Long-form ambient loop |
| 826472 | AMBTrop-Zh6 | 154s | Secondary ambient layer |

Preview URLs:
- https://cdn.freesound.org/previews/722/722665_752925-hq.mp3
- https://cdn.freesound.org/previews/669/669357_53551-hq.mp3
- https://cdn.freesound.org/previews/826/826495_12698134-hq.mp3
- https://cdn.freesound.org/previews/826/826472_12698134-hq.mp3

---

## Cretaceous Lighting Philosophy

> "The player doesn't notice correct lighting — they only notice wrong lighting."
> — Roger Deakins principle applied to game cinematography

**Golden Hour Cretaceous** is the target aesthetic:
- Warm amber sun at 35° pitch (late afternoon angle)
- Dense atmospheric haze (high Rayleigh scattering — thicker Cretaceous atmosphere)
- Volumetric fog at 6000cm distance for depth layering
- Manual exposure (bias +1.2) prevents auto-darkening of terrain
- Warm color grade: saturation boost in red/green, slight blue reduction
- Bloom at 0.4 — subtle lens glow on bright surfaces without over-exposure

---

## Files Created This Cycle

| File | Type | Description |
|------|------|-------------|
| `Source/TranspersonalGame/Lighting/LightingAtmosphereConfig.h` | C++ Header | Time of day presets, weather states, lighting structs |
| `Docs/Lighting/CYCLE_004_LIGHTING_REPORT.md` | Documentation | This report |

---

## Next Cycle Priorities (Agent #09 — Character Artist)

1. **MetaHuman character** — primitive human survivor, weathered skin, animal hide clothing
2. **Dinosaur mesh replacement** — replace placeholder cubes with proper skeletal meshes
3. **Character material** — PBR skin shader with subsurface scattering
4. **Lighting interaction** — ensure character materials respond correctly to Lumen GI

The lighting stack is now stable. Characters placed in the scene will receive:
- Warm golden hour directional light from sun
- Real-time sky light capture for ambient fill
- Volumetric fog integration for atmospheric depth
- Bloom on bright highlights (eyes, wet surfaces)
