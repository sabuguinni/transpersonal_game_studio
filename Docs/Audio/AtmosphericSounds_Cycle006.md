# Atmospheric Sound References — Cycle 006
**Agent:** #08 Lighting & Atmosphere Agent  
**Cycle:** PROD_CYCLE_AUTO_20260618_006

---

## Freesound References for Prehistoric Atmosphere

### 1. Forest Rainstorm 01 — ID: 648474
- **Duration:** 697s (11+ minutes loop-ready)
- **Preview:** https://cdn.freesound.org/previews/648/648474_2968542-hq.mp3
- **Use case:** Heavy rain weather event — Cretaceous storm system
- **Tags:** ambience, atmosphere, forest, wind, rain
- **Notes:** No thunder — pure wind/rain texture, ideal for layering

### 2. Forest Rainstorm 02 — ID: 648475
- **Duration:** 718s (alternate perspective of same storm)
- **Preview:** https://cdn.freesound.org/previews/648/648475_2968542-hq.mp3
- **Use case:** Secondary layer for storm ambience — stereo width
- **Notes:** Pair with #648474 for full storm atmosphere

### 3. Blizzard in the Forest at Midnight — ID: 558664
- **Duration:** 1598s (26 minutes — binaural)
- **Preview:** https://cdn.freesound.org/previews/558/558664_111413-hq.mp3
- **Use case:** Extreme weather event / night storm / danger atmosphere
- **Notes:** Binaural — use for headphone players, intense dramatic moments

### 4. Wind in Winter Forest (Weaker) — ID: 835136
- **Duration:** 127s
- **Preview:** https://cdn.freesound.org/previews/835/835136_5828667-hq.mp3
- **Use case:** Gentle breeze baseline — calm prehistoric day ambience
- **Notes:** Synthesized — clean loop, no artifacts

### 5. Wind in Winter Forest (Moderate) — ID: 852707
- **Duration:** 122s
- **Preview:** https://cdn.freesound.org/previews/852/852707_5828667-hq.mp3
- **Use case:** Building wind before storm event
- **Notes:** Synthesized — pairs with #835136 for wind intensity ramp

---

## Audio-Lighting Sync Design

The day/night cycle (BP_DayNightCycle) should drive audio layers:

| Time of Day | Lighting State | Audio Layer |
|-------------|---------------|-------------|
| Dawn (0.1–0.2) | Low sun, orange sky | Bird calls rising, light wind |
| Morning (0.2–0.4) | Bright, warm | Full jungle ambience, insects |
| Noon (0.4–0.6) | Harsh overhead | Peak insect chorus, heat shimmer |
| Afternoon (0.6–0.7) | Golden hour | Wind building, birds settling |
| Dusk (0.7–0.8) | Red sky, long shadows | Nocturnal creatures beginning |
| Night (0.8–1.0) | Dark, moonlight | Crickets, distant roars, danger |
| Storm event | Fog density +0.04 | Freesound #648474 + #648475 |

## For Agent #16 (Audio Agent)
- Download Freesound IDs: 648474, 648475, 558664, 835136, 852707
- Implement MetaSounds blueprint with parameter-driven layer blending
- Sync audio intensity parameter to BP_DayNightCycle timeline value
- Storm trigger: when fog_density > 0.035, crossfade to storm layers
