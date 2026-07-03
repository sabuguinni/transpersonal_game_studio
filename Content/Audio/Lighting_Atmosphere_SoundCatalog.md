# Lighting & Atmosphere — Sound Catalog
## Agent #08 | PROD_CYCLE_AUTO_20260703_007

This catalog documents curated Freesound.org audio assets for the atmospheric
audio layer of the prehistoric survival world. These sounds complement the
visual lighting system (Lumen GI, volumetric fog, god-rays) with matching
audio atmosphere.

---

## AMBIENT SOUNDSCAPE LAYER

### Primary Jungle Ambience
| ID | Name | Duration | Use Case |
|----|------|----------|----------|
| 813632 | AMBTrop_Daytime tropical forest (Tim Kahn, MKH80208030st) | 77m 33s | **Main daytime hub ambience** — late morning Panamanian rainforest, insects + birds + water drops on leaves. Perfect for the (2100,2400) Cretaceous clearing. |

**Preview:** https://cdn.freesound.org/previews/813/813632_7037-hq.mp3
**Tags:** ambience, birds, drip, field-recording, forest, insects, nature
**Integration:** Loop as base ambient layer in MetaSounds. Attenuate with distance from hub center. Blend with dinosaur vocalizations at 30% mix.

---

## WEATHER / STORM LAYER

### Distant Thunder (Approaching Storm)
| ID | Name | Duration | Use Case |
|----|------|----------|----------|
| 802401 | THUN_The Storm Is Coming Fast — Distant Rolling Thunder Rumble 2 | 6m 57s | Dynamic weather system — distant storm approaching, triggers before rain event |
| 743019 | THUN_The Storm Is Coming Fast — Distant Rumble + Close Crackling Hit | 3m 12s | Storm climax moment — close lightning strike, high drama |
| 686816 | Thunderstorm with ground strikes — BIG dynamic range | 29m 4s | Full thunderstorm event loop — apocalyptic intensity for extreme weather |
| 668772 | August afternoon thundery shower merging with thunderstorm | 35m 44s | Transitional weather — shower becoming storm, for weather system blending |
| 817001 | September early morning thunderstorm with robins singing | 57m 47s | Dawn storm clearing — post-storm atmosphere with birdsong return |

**Preview (802401):** https://cdn.freesound.org/previews/802/802401_5828667-hq.mp3
**Preview (743019):** https://cdn.freesound.org/previews/743/743019_5828667-hq.mp3
**Preview (686816):** https://cdn.freesound.org/previews/686/686816_1531809-hq.mp3

---

## INTEGRATION NOTES

### MetaSounds Blueprint Architecture
```
[WeatherSystem] → triggers → [StormAmbience_MetaSound]
    ├── Clear (0.0): AMBTrop_Daytime (813632) at 100%
    ├── Approaching (0.3): AMBTrop + Thunder_Distant (802401) blend
    ├── Storm (0.7): Thunder_Full (686816) dominant, rain layer
    └── Clearing (1.0 → 0.0): Thunder_Morning (817001) fade-out
```

### Lighting-Audio Sync Points
- **Sun intensity > 50000 lux** → Daytime ambience active (813632)
- **Sun intensity 10000-50000 lux** → Dusk/dawn blend layer
- **Volumetric fog density > 0.05** → Muffled ambience filter (LPF 4kHz)
- **Storm approaching** → Thunder layer crossfade in over 30s
- **God-ray active** → Increase bird calls +6dB (bright clearing audio cue)

### Spatial Audio Setup
- Hub center (2100, 2400): Full ambience volume
- Falloff radius: 3000 units (linear)
- Reverb preset: Large Forest (long tail, 2.5s RT60)
- Occlusion: enabled (trees block direct sound path)

---

## NEXT STEPS FOR AUDIO AGENT (#16)
1. Import sound IDs 813632 and 802401 into /Game/Audio/Ambience/
2. Create MetaSound patch: `MS_Atmosphere_DayNight` with weather blend parameter
3. Place AmbientSound actor at hub (2100, 2400, 100) with 813632 as source
4. Wire WeatherSystem float parameter to MetaSound blend input
5. Add reverb volume covering the hub clearing area

---

## LIGHTING SYSTEM STATUS (this cycle)
- ✅ DirectionalLight: 75000 lux, pitch=-50°, warm white, atmosphere_sun_light=True
- ✅ SkyAtmosphere: 1x (deduped)
- ✅ SkyLight: real_time_capture=True, intensity=3.0
- ✅ ExponentialHeightFog: volumetric=True, density=0.02, scattering=0.7
- ✅ God-ray RectLight at hub: 8000 intensity, 400x400 source
- ✅ Ambient fill lights: 3x PointLight around hub
- ✅ Lumen GI: enabled via console (r.DynamicGlobalIlluminationMethod 1)
- ✅ FastSkyLUT: enabled (performance)
- ✅ Level saved
