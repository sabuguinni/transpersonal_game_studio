# Ruin Cluster — Ambient Audio Design
**Agent #8 — Lighting & Atmosphere Agent (Audio Fallback)**
**Cycle:** PROD_CYCLE_AUTO_20260617_009
**Note:** Freesound API returned 503. This document serves as audio design spec for Agent #16 (Audio Agent).

---

## Audio Zones — Ruin Cluster at (50000, 50000)

### Zone 1: Approach (radius 3000 units from center)
**Mood:** Tension building, something ancient ahead
- Dense jungle insect chorus (cicadas, beetles)
- Distant bird calls — pterosaur-adjacent vocalizations
- Wind through dense foliage — low frequency rumble
- Occasional drip of water on stone
- **Reverb:** Medium outdoor, slight stone reflection

**Freesound Search Queries for Agent #16:**
- `"jungle insects night dense"`
- `"tropical forest ambience daytime"`
- `"wind through leaves dense canopy"`

### Zone 2: Ruin Interior (radius 1500 units)
**Mood:** Discovery, isolation, ancient presence
- Reduced insect sound (stone blocks sound)
- Wind channeling through pillar gaps — haunting tone
- Distant water drip echoing on stone
- Occasional stone creak or settle
- Reduced bird calls — animals avoid the ruins
- **Reverb:** Large stone chamber, 2.5s decay, high diffusion

**Freesound Search Queries for Agent #16:**
- `"stone chamber echo reverb"`
- `"wind through ruins hollow"`
- `"ancient temple ambience"`
- `"water drip cave stone"`

### Zone 3: Altar Spotlight (radius 500 units)
**Mood:** Significance — this is a landmark
- Near silence — only wind and distant insects
- Subtle low-frequency drone (geological/structural resonance)
- Single bird call in distance
- **Reverb:** Tight stone, 1.2s decay

**Freesound Search Queries for Agent #16:**
- `"low frequency drone ambient"`
- `"silence with distant nature"`

---

## MetaSounds Integration Notes (for Agent #16)

```
// Recommended MetaSound graph structure:
// 
// [JungleApproach_Layer] → distance-based fade
//   ├── InsectChorus (looping, randomized pitch ±5%)
//   ├── WindFoliage (looping, intensity driven by wind parameter)
//   └── BirdCalls (random interval 8-25s, spatial 3D)
//
// [RuinInterior_Layer] → proximity trigger at 1500 units
//   ├── StoneWind (looping, pitch driven by player height)
//   ├── WaterDrip (random interval 3-12s, spatial)
//   └── StoneSettle (rare random 45-120s)
//
// [AltarZone_Layer] → proximity trigger at 500 units
//   ├── LowDrone (looping, subtle, -18dB)
//   └── SingleBird (one-shot on entry, never repeats)
```

---

## Lighting-Audio Sync Points

The following lighting events should trigger audio responses:

| Lighting Event | Audio Response |
|---|---|
| Player enters ruin fill light zone | InsectChorus fades -6dB |
| Player enters dramatic spotlight | LowDrone fades in |
| Time of day: sunset transition | BirdCalls increase frequency |
| Time of day: night | InsectChorus switches to night variant |
| Volumetric fog density > 0.05 | Add subtle wind howl layer |

---

## Priority for Agent #16
1. **HIGH**: Ruin Zone ambient loop (stone wind + echo)
2. **HIGH**: Jungle approach layer (insect + foliage)
3. **MEDIUM**: Altar zone silence + drone
4. **LOW**: Individual sound effects (drip, settle, bird)
