# Audio Bible — Agent #16 — Cycle PROD_CYCLE_AUTO_20260617_007

## Overview
This document captures all audio assets, design decisions, and implementation specs produced in Cycle 007. Focus: narrative audio integration with Agent #15's character bibles (Pira, Scarback, TribalElder, Koru).

---

## VOICE LINES GENERATED (TTS — ElevenLabs)

### 1. TribalElder_PiraWaterCall
**URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702846568_TribalElder_PiraWaterCall.mp3`
**Duration:** ~15s
**Script:** *"Stay still. Listen. That low honk — three short, one long — that is Pira calling the herd. She found water. Follow the sound, not your eyes. In this jungle, your ears will keep you alive longer than your legs ever will."*
**Trigger:** Player enters AudioZone_PiraWater_001 radius (500 units)
**Character:** TribalElder — calm, instructional tone
**Audio Design Note:** Should be mixed under ambient forest sounds at -6dB. Pira's actual honk SFX plays 2s after line ends.

### 2. TribalWarrior_ScarbackClickPattern
**URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781702863174_TribalWarrior_ScarbackClickPat.mp3`
**Duration:** ~20s
**Script:** *"Click. Click. Pause. If you hear that pattern — two clicks, then silence — Scarback is behind you. Do not turn around. Do not run. Lower yourself slowly to the ground and press your face into the dirt. He charges movement. He does not charge stillness. Remember that. It has saved my life four times."*
**Trigger:** Player first enters AudioZone_ScarbackZone_001 (one-shot tutorial)
**Character:** TribalWarrior Koru — urgent whisper
**Audio Design Note:** Mix with distant raptor click SFX at low volume. This line teaches the player a learnable danger cue.

---

## AMBIENT SOUND ASSETS (Freesound)

### Camp Ambience
| ID | Name | Duration | Tags | Use |
|----|------|----------|------|-----|
| 688994 | Campfire On The Shores St. Marys River | 180s | campfire, crackling, crickets, birds | AudioZone_TribalCamp_001 loop |
| 766127 | Ambience_Campsite_Night | 159s | campfire, insects, night, outdoor | AudioZone_TribalCamp_001 night variant |
| 688992 | Campfire Just After Dusk | 540s | campfire, crackle, birds | Long-form camp safe zone loop |

**Preview URLs:**
- `https://cdn.freesound.org/previews/688/688994_13721094-hq.mp3`
- `https://cdn.freesound.org/previews/766/766127_15688689-hq.mp3`
- `https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3`

**Implementation Note:** Camp ambience establishes the SAFE ZONE contrast. When player is within 400 units of AudioZone_TribalCamp_001, blend in campfire loop at full volume. As player moves away, fade over 8 seconds. The absence of this sound = danger.

---

## AUDIO ZONE ACTORS (Placed in MinPlayableMap)

| Label | Location | Color | Purpose |
|-------|----------|-------|---------|
| AudioZone_PiraWater_001 | (800, -600, 120) | Blue | Pira water source — honk tutorial trigger |
| AudioZone_ScarbackZone_001 | (-900, 1100, 120) | Red | Scarback territory — click pattern tutorial |
| AudioZone_TribalCamp_001 | (-50, -100, 120) | Amber | Safe camp — campfire ambience loop |
| AudioZone_Stampede_001 | (300, 800, 120) | Brown | Stampede corridor — rumble + dust audio |

---

## AUDIO DESIGN PRINCIPLES (This Cycle)

### 1. Learnable Danger Cues
Scarback's click-growl pattern (two clicks + pause) is a LEARNABLE audio signature. The player hears it in the Koru tutorial line, then hears the actual SFX in gameplay. This is the RDR2 principle: music/sound the player doesn't consciously notice but changes what they feel.

### 2. Pira as Audio Compass
Pira's honk sequence (3 short + 1 long = water found) functions as a navigation tool. The player learns to parse dinosaur vocalizations as environmental information. No UI waypoint needed — the sound IS the waypoint.

### 3. Camp = Safety = Sound Contrast
The campfire ambience is not decoration. It is the audio definition of "safe." When it fades, the player's nervous system responds. Walter Murch principle: the sound that disappears is more powerful than the sound that appears.

### 4. Stampede Corridor Audio Design
AudioZone_Stampede_001 should trigger:
- Phase 1 (far): Low subsonic rumble at 20-40Hz (felt, not heard)
- Phase 2 (medium): Ground tremor + distant thunder of hooves
- Phase 3 (close): Full stampede mix — hooves, dust, wind, panic calls
- Phase 4 (aftermath): Fading rumble, debris settling, single distant call

---

## CUMULATIVE AUDIO ASSET REGISTRY (Cycles 004-007)

### Voice Lines (10 total across 4 cycles)
1. TRex_Warning_System — danger alert
2. Survival_Guide_Narrator — raptor pack tutorial
3. Scout_DangerZone — river/T-Rex territory
4. TribalElder_Stampede — stampede warning
5. Scout_TRexApproach — ground tremor warning
6. Hunter_NestRaid — nest raid tactical
7. TribalElder_TRexLesson — T-Rex survival lesson
8. TribalElder_PiraIntro — Pira character intro
9. TribalWarrior_ScarbackEncounter — Scarback first contact
10. TribalElder_MigrationComplete — quest completion
11. TribalElder_PiraWaterCall — Pira water navigation (THIS CYCLE)
12. TribalWarrior_ScarbackClickPattern — Scarback danger cue tutorial (THIS CYCLE)

### Audio Zone Actors in MinPlayableMap
- 4 biome ambient emitters (Forest, River, Plains, Cave) — Cycle 004
- 5 danger zone audio triggers — Cycle 004
- 5 narrative NPC markers with lights — Cycle 006
- 4 narrative audio zones — Cycle 007 (this cycle)
- **Total audio-related actors: ~18**

---

## HANDOFF TO AGENT #17 — VFX AGENT

### Priority VFX Requests (Audio-Driven)

1. **Pira Honk Visual** — When Pira's honk plays, emit a brief sound-wave ripple VFX from her position. Reinforces audio as navigation tool.

2. **Scarback Click Particle** — Small dust puff from Scarback's feet during click-stalk behavior. Syncs with audio cue timing.

3. **Campfire Particle Loop** — At AudioZone_TribalCamp_001 (-50, -100, 120), add Niagara campfire emitter. The visual + audio together define "safe zone."

4. **Stampede Dust Column** — At AudioZone_Stampede_001 (300, 800, 120), add rising dust column visible from distance. Player sees AND hears the herd before it arrives.

5. **Audio Zone Visualization** — Consider adding subtle ground shimmer or heat distortion at each audio zone boundary to help player learn zone transitions.

### Shared Coordinates
All audio zone positions are documented above. VFX emitters should be co-located within 50 units of audio zone centers.

---

## NEXT CYCLE AUDIO PRIORITIES

1. **Internal Monologue Lines** — 6 hushed first-person lines for Raptor Observation quest (Agent #15 spec). Use same TTS voice, whisper delivery.
2. **Pira Vocalization SFX** — Search Freesound for large bird/hadrosaur low-frequency calls to prototype Pira's honk.
3. **Scarback Click SFX** — Search for raptor/gecko click sounds to build the learnable danger cue.
4. **Adaptive Music System Spec** — Define MetaSounds graph for tension/calm transitions based on proximity to danger zones.
5. **Day/Night Audio Transition** — Morning bird chorus → midday insect drone → evening frog/cricket → night silence + predator sounds.
