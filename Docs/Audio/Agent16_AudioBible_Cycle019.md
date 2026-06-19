# Audio Agent #16 — Audio Bible
## PROD_CYCLE_AUTO_20260619_003

---

## Voice Lines Generated

| ID | Character | Line | Duration | URL |
|----|-----------|------|----------|-----|
| 1 | Tribe_Survivor_Night | "Something is moving in the dark. Stay close to the fire. Raptors do not like the light — but they remember where we sleep." | ~9s | tts/1781839453291_Tribe_Survivor_Night.mp3 |
| 2 | Tribe_Elder_TRex_Warning | "Run. Do not look back. Do not stop. The ground shakes because something very large is very close. RUN." | ~7s | tts/1781839459060_Tribe_Elder_TRex_Warning.mp3 |

---

## Sound Assets Sourced (Freesound.org)

### Raptor Vocalization
- **ID:** 320345 — "Dino Hiss Dragon Roar"
- **Duration:** 5.7s
- **Tags:** dino, dinosaur, growl, hiss, beast, creature
- **Preview:** https://cdn.freesound.org/previews/320/320345_5286377-hq.mp3
- **Use:** Raptor idle/alert state — loop with pitch variation per individual

### Campfire Ambient (Camp Zone)
- **ID:** 681367 — "Campfire (Position 2)" — 22s, quiet night setting
- **ID:** 681366 — "Campfire (Position 1)" — 83s, quiet night setting
- **ID:** 802195 — "Fire-Nature Sounds" — 247s, campfire + nature at night
- **Preview:** https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3
- **Use:** Camp zone ambient loop — crossfade between positions based on player distance

---

## Audio Zones Placed in MinPlayableMap

| Label | Location | Color | Purpose |
|-------|----------|-------|---------|
| Audio_Camp_FireAmbient | (-500,-800,120) | Amber | Campfire crackle + tribe murmur loop |
| Audio_Raptor_Territory | (1200,1500,120) | Red | Raptor idle calls + rustling brush |
| Audio_TRex_Approach | (2500,800,120) | Purple | Sub-bass rumble + ground vibration cue |
| Audio_Cave_Entrance | (-1200,-400,120) | Blue | Dripping water + echo + wind whistle |
| Audio_Night_Perimeter | (0,-1500,120) | Dark blue | Night insects + distant dino calls |

---

## Audio Design Spec

### Camp Zone (-500,-800)
**Layers (distance-based mix):**
1. **0–200 units:** Campfire crackle (Freesound 681367) + NPC murmur (low-pass filtered)
2. **200–500 units:** Wind through trees + distant owl-like calls
3. **500+ units:** Silence broken only by distant dino movement

**Reactive triggers:**
- Player crouches near fire → murmur volume increases (NPCs sense safety)
- Night phase → insect layer fades in, fire becomes primary warmth cue
- Raptor proximity → all ambient cuts to silence (prey silence response)

### Raptor Territory (1200,1500)
**Layers:**
1. **Idle state:** Freesound 320345 (hiss/growl) at 0.3 volume, random interval 8–20s
2. **Alert state:** Pitch up 20%, interval drops to 2–5s, stereo spread widens
3. **Attack state:** Full volume, no interval, rapid chittering added

**Design note:** Silence IS the warning. When raptor ambient stops → player should feel dread.

### T-Rex Approach Zone (2500,800)
**Layers:**
1. **Far (>1500 units):** Sub-bass rumble 40–60Hz, felt not heard
2. **Mid (500–1500 units):** Ground thud rhythm (footstep cadence 1.2s interval)
3. **Near (<500 units):** Full roar + breath + displacement wind
4. **Screen shake trigger:** At <800 units, camera shake begins (0.5 magnitude)

**Design note:** Player should feel T-Rex before they see it. Audio is the early warning system.

### Cave Entrance (-1200,-400)
**Layers:**
1. **Entrance:** Wind whistle through rock (high-frequency, directional)
2. **Interior:** Water drip (irregular, 3–8s interval) + deep echo reverb
3. **Deep cave:** Complete silence except player footsteps — maximum tension

**Design note:** Cave = safety from large predators. Audio must communicate SHELTER.

### Night Perimeter (0,-1500)
**Layers:**
1. **Dusk transition:** Bird calls fade → insect chorus rises
2. **Full night:** Continuous insect layer + random distant roar (far, 3000+ units)
3. **Dawn transition:** Insect chorus fades → bird calls return

---

## Adaptive Music System Design

### State Machine (5 states)
```
SAFE_CAMP → EXPLORATION → ALERT → COMBAT → DEATH
```

**SAFE_CAMP:** Slow percussion (bone on wood), 60 BPM, warm low tones
**EXPLORATION:** Medium percussion, 80 BPM, tension builds with distance from camp
**ALERT:** Fast irregular rhythm, 100+ BPM, dissonant tones, no melody
**COMBAT:** Percussion only, maximum intensity, no pitch — pure rhythm
**DEATH:** Silence, then single low tone fade

### Transition Rules
- SAFE→EXPLORATION: Crossfade 4s when player moves >500 units from camp
- EXPLORATION→ALERT: Instant cut when dino enters 800-unit radius
- ALERT→COMBAT: Instant cut on first hit
- COMBAT→SAFE: 8s fade after 10s of no dino contact
- Any→DEATH: Instant silence on health=0

---

## Handoff to Agent #17 (VFX)

### Audio-VFX Sync Points
1. **T-Rex footstep:** Audio thud at 1.2s interval → VFX ground dust particle at same timing
2. **Raptor attack:** Audio chittering peak → VFX claw slash particle
3. **Fire ambient:** Audio campfire loop → VFX fire particle system (already in map)
4. **Cave drip:** Audio water drip → VFX water droplet splash particle
5. **Player damage:** Audio impact grunt → VFX red screen flash overlay

### Priority VFX Requests
1. Ground impact dust for T-Rex footsteps (sync with audio rumble)
2. Raptor claw slash trail (sync with attack audio state)
3. Campfire ember particles (sync with Freesound 802195 loop)
