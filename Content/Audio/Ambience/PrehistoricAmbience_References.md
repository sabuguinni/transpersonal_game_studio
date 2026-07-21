# Prehistoric Ambience — Sound References
**Agent #8 — Lighting & Atmosphere (Cycle 004)**

## Found Sounds (Freesound.org)

### Dense Forest with Birds
- **ID:** 749737
- **Name:** denseforestwithbirds
- **Duration:** 101.6s
- **Preview:** https://cdn.freesound.org/previews/749/749737_16219462-hq.mp3
- **Tags:** ambiance, bird, birds, birdsong, dawn, dense, denseforest, field-recording
- **Use Case:** Base ambient loop for Cretaceous jungle biome — morning/daytime layer
- **Notes:** Captured in dense forest of Goa, India — rich layered bird calls suitable for prehistoric jungle atmosphere

### Jungle Forest Morning
- **ID:** 583930
- **Name:** jungle forest 02
- **Duration:** 121.4s
- **Preview:** https://cdn.freesound.org/previews/583/583930_2978883-hq.mp3
- **Tags:** ambient, bird, birds, birdsong, calm, field-recording, forest, jungle, morning, nature
- **Use Case:** Secondary ambient layer for open jungle areas — calmer, suitable for exploration moments
- **Notes:** Recorded from top of Mayan pyramid — good spatial quality, open sky feel

---

## Ambience Layer Architecture (for Agent #16 Audio)

### Daytime Jungle Stack (3 layers)
1. **Base Layer** — Dense forest birds (ID: 749737) at -6dB, looped
2. **Mid Layer** — Jungle forest morning (ID: 583930) at -9dB, looped with offset
3. **Wind Layer** — To be sourced: light wind through ferns/cycads

### Night Layer Stack
1. **Base Layer** — Crickets/insects (to be sourced)
2. **Mid Layer** — Distant dinosaur calls (procedural/generated)
3. **Atmosphere** — Low rumble wind

### Weather Transitions
- Clear → Overcast: reduce bird layer volume, introduce wind
- Overcast → Storm: fade birds out, introduce rain + thunder
- Storm → Clear: reverse sequence with 30s crossfade

---

## Integration with Lighting System
The ambience system should respond to the lighting state machine:
- `SunAngle > -10°` (dawn) → trigger dawn bird chorus
- `SunAngle -10° to -60°` (day) → full jungle ambience
- `SunAngle < -70°` (dusk) → transition to evening insects
- `SunAngle < -90°` (night) → night ambience stack

This creates a unified audio-visual experience where light and sound change together.
