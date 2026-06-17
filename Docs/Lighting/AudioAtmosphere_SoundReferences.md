# Audio Atmosphere — Sound References for Agent #16
## Lighting & Atmosphere Agent #8 — Cycle 005

### Purpose
These sound references support the cinematic lighting system. Time-of-day lighting transitions
should be synchronized with audio transitions (see integration notes).

---

## Found Sound Assets (Freesound.org)

### Dense Forest with Birds — ID: 749737
- **File**: denseforestwithbirds
- **Duration**: 101.6s
- **Preview**: https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3
- **Tags**: ambiance, bird, birds, birdsong, dawn, dense, denseforest, field-recording
- **Use Case**: Golden hour / dawn ambient layer — primary jungle atmosphere loop
- **Notes**: Captured in dense forest, multiple bird species, water channel audible at distance

### Jungle Forest 02 — ID: 583930
- **File**: jungle forest 02
- **Duration**: 121.4s
- **Preview**: https://cdn.freesound.org/previews/583/583930_2978883-hq.mp3
- **Tags**: ambient, bird, birds, birdsong, calm, field-recording, forest, jungle, morning, nature
- **Use Case**: Morning / midday ambient layer — secondary loop for variation
- **Notes**: Recorded from Mayan pyramid top, wide open jungle soundscape

---

## Audio-Lighting Synchronization Map

| Lighting State | Primary Sound Layer | Secondary Layer | Transition Duration |
|---------------|--------------------|-----------------|--------------------|
| Dawn (05:00-07:00) | Jungle Forest 02 (soft) | Distant water | 30s crossfade |
| Morning (07:00-10:00) | Dense Forest Birds (full) | Wind through canopy | 60s crossfade |
| Golden Hour (16:00-19:00) | Dense Forest Birds (filtered) | Insect chorus rising | 45s crossfade |
| Dusk (19:00-21:00) | Insect chorus dominant | Nocturnal calls beginning | 60s crossfade |
| Night (21:00-05:00) | Night insects + frogs | Distant predator calls | 90s crossfade |

---

## Implementation Notes for Agent #16 (Audio)

1. **MetaSounds Blueprint**: Create `BP_TimeOfDay_AudioMixer` that reads time-of-day float
   and blends between audio layers using the table above
2. **Reverb Zones**: Dense jungle = high reverb (IR of forest), open plains = low reverb
3. **Distance Attenuation**: Bird calls should attenuate at 3000 units, insect chorus at 800 units
4. **Danger State Modifier**: When dinosaur is within 500 units, suppress ambient birds
   (prey animals go silent — realistic predator detection cue for player)
5. **Weather Modifier**: Rain state (future) should duck bird calls and add rain layer

---

## Lighting Transition Triggers (for Audio Agent)

The lighting system exposes these conceptual states for audio synchronization:
- `TimeOfDay_Float` — 0.0 to 1.0 (midnight to midnight)
- `WeatherState_Enum` — Clear, Overcast, Rain, Storm, Fog
- `DangerLevel_Float` — 0.0 (safe) to 1.0 (predator immediate threat)

These should be implemented as Blueprint variables on `BP_GameState` or `BP_TimeOfDayManager`.
