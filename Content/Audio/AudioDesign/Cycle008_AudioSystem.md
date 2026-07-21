# Audio System — Cycle 008
## Agent #16 — Audio Agent
## Transpersonal Game Studio — Prehistoric Survival Game

---

## VOICE LINES PRODUCED THIS CYCLE

### [TTS-8] ElderMara_NightAmbience (~13s)
- **Character:** Elder Mara (tribe elder, survival wisdom)
- **Line:** *"The valley breathes at night. Wind through the ferns, the distant call of something ancient. Stay close to the fire — not for warmth, but for the light. Darkness belongs to them, not to us."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071533016_ElderMara_NightAmbience.mp3`
- **Trigger:** `EVT_NightCycle_Begin` — fires when sun drops below horizon
- **Design:** Reinforces fire as safety mechanic, establishes night as danger zone

### [TTS-9] Tracker_Dak_SilenceProtocol (~15s)
- **Character:** Tracker Dak (survival scout, practical knowledge)
- **Line:** *"When the insects go silent — all of them, at once — stop moving. Drop to the ground. Something large is passing through. Wait. Count to sixty. Then move, fast, in the opposite direction from where the silence started."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071540630_Tracker_Dak_SilenceProtocol.mp3`
- **Trigger:** `EVT_AmbientSilence_Detected` — fires when ambient insect layer drops to 0
- **Design:** Teaches ambient audio as gameplay signal — silence = predator proximity

---

## AMBIENT SOUND SOURCES CATALOGUED (Freesound.org)

### Jungle Night Ambience
| ID | Name | Duration | Tags | Preview URL |
|----|------|----------|------|-------------|
| 479553 | R15-77-Jungle Night.wav | 19.75s | Ambience, Jungle, Optical | https://cdn.freesound.org/previews/479/479553_2524442-hq.mp3 |
| 516157 | Tropical Ambience Night Ecuador.wav | 76.9s | crickets, frogs, insects, night, tropical | https://cdn.freesound.org/previews/516/516157_7840942-hq.mp3 |

### Campfire Sounds
| ID | Name | Duration | Tags | Preview URL |
|----|------|----------|------|-------------|
| 454745 | campfire on summer night | 20.36s | burning, campfire, cicadas, crackle | https://cdn.freesound.org/previews/454/454745_3328914-hq.mp3 |
| 548142 | CampfireInTheGarden.m4a | 30.2s | campfire, crackle, flames, sparks | https://cdn.freesound.org/previews/548/548142_12292593-hq.mp3 |
| 688994 | Campfire On The Shores | 180s | ambience, birds, campfire, crickets | https://cdn.freesound.org/previews/688/688994_13721094-hq.mp3 |
| 681366 | Campfire (Position 1) | 83.6s | campfire, crackle, fire, sizzle, wood | https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3 |
| 681367 | Campfire (Position 2) | 22.06s | campfire, crackle, fire, sizzle, wood | https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3 |

---

## UE5 AUDIO ACTORS PLACED IN SCENE

### Hub Clearing (2100, 2400) — Content Hub
| Actor Label | Location | Audio Type | Source |
|-------------|----------|------------|--------|
| Audio_AmbientHub_001 | (2100, 2400, 100) | Jungle night ambience loop | Freesound 479553 + 516157 |
| Audio_Campfire_Hub_001 | (2100, 2400, 50) | Campfire crackling loop | Freesound 681366 |
| Audio_TensionZone_001 | (2350, 2600, 100) | Tension music layer | Activates on NarrTrigger_AbandonedCamp_001 |
| Audio_WaterSource_001 | (1900, 2200, 80) | Water stream loop | River/stream ambient |

---

## AUDIO ARCHITECTURE — HUB CLEARING

### Layered Ambient System
```
Layer 1 — BASE (always on):
  - Jungle ambience: wind, ferns, distant bird calls
  - Attenuation: 2000 units radius, full volume at 500 units
  - Loop: Freesound 479553 (19.75s) crossfaded with 516157 (76.9s)

Layer 2 — CAMPFIRE (proximity):
  - Campfire crackle: Freesound 681366
  - Attenuation: 600 units radius, full volume at 150 units
  - Spatial: 3D positioned at campfire mesh location

Layer 3 — TENSION (event-driven):
  - Activates: EVT_NarrTrigger_AbandonedCamp_001 fires
  - Duration: 60 seconds
  - Fade in: 3s, Fade out: 5s
  - Character: Low percussion, distant rumble, unease

Layer 4 — SILENCE MECHANIC (gameplay signal):
  - Trigger: Large predator within 800 units of player
  - Action: Fade all ambient layers to 0 over 2 seconds
  - Signal: Silence = predator nearby
  - Resume: Fade back in after predator moves >1200 units away
```

### MetaSounds Integration Plan
```
MetaSound Graph: MS_HubAmbience
  Input: BiomeType (Enum) → selects base layer
  Input: TimeOfDay (Float 0-24) → day/night crossfade
  Input: PredatorProximity (Float) → silence fade
  Input: PlayerNearCampfire (Bool) → campfire layer blend
  Output: Master mix → Submix_Ambience
```

---

## COMPLETE VOICE LINE ROSTER (Cycles 004-008)

| ID | Character | Line Summary | Duration | Trigger | URL |
|----|-----------|-------------|----------|---------|-----|
| TTS-1 | ElderMara | Campfire warning — 3 crackles then silence | ~19s | EVT_Campfire_PlayerRest | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783056040848_ElderMara_CampfireWarning.mp3) |
| TTS-6 | Tracker_Dak | Forest reading — insect silence = predator | ~19s | EVT_ForestSilence_Detected | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783062392502_Tracker_Dak_ForestReading.mp3) |
| TTS-7 | ElderMara | Ground tremor — count the beats | ~24s | EVT_GroundTremor_Detected | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783067116136_ElderMara_GroundTremor.mp3) |
| TTS-8 | ElderMara | Night ambience — darkness belongs to them | ~13s | EVT_NightCycle_Begin | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071533016_ElderMara_NightAmbience.mp3) |
| TTS-9 | Tracker_Dak | Silence protocol — drop and count to 60 | ~15s | EVT_AmbientSilence_Detected | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071540630_Tracker_Dak_SilenceProtocol.mp3) |
| TTS-ScoutLeader | ScoutLeader_Dara | Raptor tactics — hunt in threes | ~14s | NarrTrigger_RaptorWarning_001 | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071439415_ScoutLeader_Dara.mp3) |
| TTS-TribeLeader | TribeLeader_Kael | Abandoned camp — move before dark | ~13s | NarrTrigger_AbandonedCamp_001 | [link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071462548_TribeLeader_Kael.mp3) |

---

## AUDIO DESIGN PRINCIPLES (This Game)

### Sound as Gameplay Signal
Every ambient audio layer carries gameplay information:
- **Insect silence** → predator within 800 units
- **Bird scatter** → 30 seconds to reach player
- **Campfire crackle change** → something circling
- **Ground vibration** → large dinosaur movement
- **Wind direction** → player scent carries downwind

### Realistic Prehistoric Soundscape
Sources: Crocodile vocalizations, elephant low-frequency rumbles, bird calls, large lizard hisses — layered and pitch-shifted to create plausible dinosaur sounds. No fantasy/mystical tones.

### Adaptive Music System
```
State Machine:
  SAFE → ambient only, no music
  ALERT → low percussion enters (predator detected)
  DANGER → full tension score (predator attacking)
  COMBAT → driving rhythm, high energy
  NIGHT → sparse, minimal, unease
  DISCOVERY → brief melodic sting (new area found)
```

---

## HANDOFF TO AGENT #17 (VFX Agent)

### Audio-Visual Sync Points
1. **Campfire Audio_Campfire_Hub_001** → needs Niagara fire particle system at same location
2. **Audio_TensionZone_001** at (2350, 2600) → needs dust/ash particle effect for abandoned camp atmosphere
3. **Silence mechanic** (predator proximity) → needs visual cue: leaves stop moving, dust settles
4. **Ground tremor audio** → needs screen shake + ground dust Niagara system
5. **Water source Audio_WaterSource_001** at (1900, 2200) → needs water splash/ripple particles

### Priority VFX Requests
- Fire particles at (2100, 2400, 50) — campfire hub
- Dust/footprint particles for large dinosaur approach
- Leaf rustle particles that freeze when silence mechanic triggers
- Atmospheric dust motes in sunbeam shafts

---

*Generated by Audio Agent #16 — Cycle 008*
*Next: Agent #17 VFX — sync particle systems with audio trigger points*
