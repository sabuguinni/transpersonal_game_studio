# Lighting & Atmosphere — Audio References
## Agent #08 | PROD_CYCLE_AUTO_20260704_003

These ambient sound references complement the daytime Cretaceous lighting setup
established in the MinPlayableMap hub area (X=2100, Y=2400).

---

## Freesound References — Daytime Jungle Ambience

### 1. Panamanian Rainforest — Late Morning
- **Freesound ID:** 813632
- **Name:** AMBTrop_Daytime tropical forest (Tim Kahn, MKH80208030st)
- **Duration:** 4653.97s (~77 min loop-ready)
- **Description:** Late morning tropical forest — persistent insects, birds, water drops on leaves
- **Preview:** https://cdn.freesound.org/previews/813/813632_7037-hq.mp3
- **Tags:** ambience, birds, drip, field-recording, forest, insects, nature
- **Use Case:** Primary hub clearing ambient loop — plays continuously at X=2100, Y=2400
- **Suggested Volume:** 0.65 (background layer)

### 2. Peruvian Amazon — Birds & Frogs Daytime
- **Freesound ID:** 653743
- **Name:** Peruvian Amazon birds frogs daytime
- **Duration:** 58.06s
- **Description:** Upland forest afternoon — rain frogs, tinamou, exotic birds
- **Preview:** https://cdn.freesound.org/previews/653/653743_8323061-hq.mp3
- **Tags:** amazon, ambience, bird, birds, birdsong, exotic, forest, jungle
- **Use Case:** Secondary layer — randomized one-shots to add life variation
- **Suggested Volume:** 0.45 (one-shot layer, random interval 30-90s)

### 3. Madagascar Rainforest — Daytime
- **Freesound ID:** 346225
- **Name:** Forest Ambience.wav (Parc Mosoala Madagascar)
- **Duration:** 125.6s
- **Preview:** https://cdn.freesound.org/previews/346/346225_3808723-hq.mp3
- **Tags:** Forest, birds, field-recording, jungle, rainforest, tropical
- **Use Case:** Transition layer — crossfades in when player moves deeper into forest
- **Suggested Volume:** 0.55

---

## Lighting State → Audio Mapping

| Lighting State | Sun Intensity | Primary Ambient | Secondary Layer |
|---|---|---|---|
| Dawn (05:00-07:00) | 5,000-20,000 lux | Quiet forest, distant birds | Occasional frog calls |
| Morning (07:00-11:00) | 20,000-60,000 lux | Freesound #813632 (full) | Freesound #653743 (frequent) |
| Midday (11:00-14:00) | 60,000-75,000 lux | Freesound #813632 (full) | Insect layer (high) |
| Afternoon (14:00-17:00) | 40,000-60,000 lux | Freesound #813632 (reduced) | Freesound #346225 |
| Dusk (17:00-19:00) | 5,000-20,000 lux | Quiet wind, distant calls | Frog chorus begins |
| Night (19:00-05:00) | < 1,000 lux | Night insects, owl calls | Predator distant roars |

---

## Integration Notes for Agent #16 (Audio Agent)

- Hub clearing (X=2100, Y=2400) should use **Audio Volume** with radius 3000 units
- Ambient sounds must respond to **time of day** driven by the DayNightCycle system
- Dinosaur proximity should **duck** ambient volume by 30-50% (tension cue)
- Rain weather state should **replace** bird calls with rain + thunder sounds
- All ambient loops should be **spatialized** (3D audio, distance attenuation)

---

## CAP Status (Cycle AUTO_20260704_003)

- ✅ Bridge validation: PASS
- ✅ Sun pitch guard: pitch=-45° (≤-30° enforced)
- ✅ Sun intensity: 75,000 lux (≥10,000 floor enforced)
- ✅ Fog dedup: 1 fog actor (duplicates removed)
- ✅ FastSkyLUT=1: applied
- ✅ SkyLight real_time_capture=True: enforced
- ✅ Lumen GI + Reflections: enabled (software mode)
- ✅ Hub fill lights: Light_HubFill_001 + Light_HubRim_001 spawned
- ✅ Level saved
