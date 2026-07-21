# Audio System — Production Cycle 008
**Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260618_008**

---

## Voice Lines Generated (TTS — ElevenLabs)

### Line 1 — Survivor_Warning
- **Text:** "The storm is coming. I can smell it in the air — that heavy, wet smell that means the river will rise before nightfall. We need to move the camp to higher ground. Now."
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781783048888_Survivor_Warning.mp3
- **Duration:** ~12s
- **Trigger:** Weather system detects incoming storm (rain probability > 80%)
- **Context:** Player is near river camp — survival warning

### Line 2 — Elder_HerdWarning
- **Text:** "There — do you hear that? That low rumble beneath the ground. That is not thunder. That is the herd. Hundreds of them, moving through the valley. Stay low. Stay quiet. Do not run."
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781783051648_Elder_HerdWarning.mp3
- **Duration:** ~12s
- **Trigger:** Herd migration event begins within 500m of player
- **Context:** Elder NPC proximity — survival knowledge transfer

---

## Sound Effects Found (Freesound.org)

### Dinosaur / Creature SFX
| ID | Name | Duration | Tags | Preview |
|----|------|----------|------|---------|
| 278229 | dinosaur.wav | 13.9s | beast, bones, creature, deep, dinosaur, eating, growl, hunting, monster, predator | https://cdn.freesound.org/previews/278/278229_5351496-hq.mp3 |
| 743078 | Wildlife Creature Warning Howl | 10.7s | Dinosaur, Sound, alien, attack, bizarre, call, calls, creature, critter, danger | https://cdn.freesound.org/previews/743/743078_71257-hq.mp3 |
| 837799 | Sea Creature Roar | 11.0s | beast, creature, dinosaur, dragon, growl, kraken, monster, roar, sea | https://cdn.freesound.org/previews/837/837799_16586370-hq.mp3 |
| 435149 | Beast deep groan | 2.8s | animal, beast, creature, dinosaur, dragon, groan, growl, monster, roar, vocalization | https://cdn.freesound.org/previews/435/435149_6802113-hq.mp3 |

**Usage assignments:**
- 278229 (dinosaur eating) → T-Rex feeding animation audio
- 743078 (warning howl) → Raptor pack alert call
- 837799 (creature roar) → Brachiosaurus distress call
- 435149 (beast groan) → Injured dinosaur ambient

---

## Audio Zones — UE5 Map Markers

Four audio trigger zones placed in MinPlayableMap as PointLight markers (teal, 200 intensity, 800 radius):

| Label | Location | Purpose |
|-------|----------|---------|
| AudioZone_Storm_River | (-3000, 1500, 200) | Storm warning — river flood danger |
| AudioZone_HerdPath_Valley | (2500, -2000, 150) | Herd migration rumble zone |
| AudioZone_Camp_Campfire | (0, 0, 120) | Camp ambient — fire, insects, safety |
| AudioZone_Predator_Forest | (-1500, -3000, 180) | Predator proximity — tension music |

---

## Audio Design Philosophy (Cycle 008)

### Adaptive Music States
```
STATE_SAFE      → Low percussion, wind, distant bird calls, river flow
STATE_AWARE     → Mid-tempo bone percussion, subtle tension layer
STATE_DANGER    → Fast percussion, dissonant stone/bone hits, breath sounds
STATE_COMBAT    → Full percussion, urgent rhythm, creature sounds mixed in
STATE_DEATH     → Silence → single low drum hit → fade to ambient
```

### Environmental Audio Layers (per biome)
```
SAVANNA:   Wind (constant), grass rustle, distant herbivore calls, hawk cries
FOREST:    Dense insect chorus, dripping water, branch snaps, bird alarms
RIVER:     Water flow (variable by rain), frog calls, fish splash, mud sounds
CAVE:      Drip echo, bat flutter, deep resonance, own footsteps amplified
VOLCANIC:  Low rumble, gas vents, rock crumble, heat shimmer (high freq hiss)
```

### Survival Audio Cues
```
HUNGER_LOW    → Stomach growl (subtle, internal — player only)
THIRST_LOW    → Dry mouth swallow sound
STAMINA_LOW   → Heavy breathing, heartbeat
FEAR_HIGH     → Heartbeat dominant, sounds become muffled/distorted
NEAR_DEATH    → Tinnitus ring, all sounds fade except heartbeat
```

---

## Cumulative Audio Asset Registry (Cycles 001–008)

### Voice Lines (TTS)
| Cycle | Character | Theme | URL |
|-------|-----------|-------|-----|
| 001 | Narrator_Intro | World introduction | Supabase TTS archive |
| 002 | Survivor_Combat | Combat warning | Supabase TTS archive |
| 003 | Elder_Wisdom | Survival knowledge | Supabase TTS archive |
| 004 | Scout_Report | Dinosaur sighting | Supabase TTS archive |
| 005 | Narrator_Night | Night danger | Supabase TTS archive |
| 006 | Survivor_Camp | Camp safety | Supabase TTS archive |
| 007 | Elder_Observation | Herd behavior | Supabase TTS archive |
| 008 | Survivor_Warning | Storm warning | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781783048888_Survivor_Warning.mp3 |
| 008 | Elder_HerdWarning | Herd migration | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781783051648_Elder_HerdWarning.mp3 |

### Freesound Assets (Cycles 001–008)
| ID | Name | Assigned Use |
|----|------|-------------|
| 278229 | dinosaur.wav | T-Rex feeding |
| 743078 | Wildlife Warning Howl | Raptor alert |
| 837799 | Sea Creature Roar | Brachio distress |
| 435149 | Beast deep groan | Injured dino ambient |
| (prev cycles) | Rain, fire, wind, footsteps | Environmental layers |

---

## Handoff to Agent #17 — VFX Agent

### Audio-Visual Sync Requirements
The following audio events need matching VFX:

1. **Storm Warning** → Rain particle system should activate when `AudioZone_Storm_River` triggers
2. **Herd Rumble** → Ground dust/vibration Niagara effect at `AudioZone_HerdPath_Valley`
3. **Predator Zone** → Subtle fog/shadow increase at `AudioZone_Predator_Forest`
4. **Camp Campfire** → Fire particle system at `AudioZone_Camp_Campfire` (origin 0,0,120)
5. **T-Rex Footstep** → Ground crack + dust particle per footstep impact
6. **Raptor Alert Call** → Flock scatter particle (birds/insects fleeing)

### VFX Priority Order
1. Campfire particles at camp origin (most visible, most used)
2. Rain system (weather-driven, large scale)
3. T-Rex footstep dust (combat feedback)
4. Herd dust cloud (migration event)

---

*Generated: PROD_CYCLE_AUTO_20260618_008 | Audio Agent #16*
