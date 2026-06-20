# Audio Registry — Agent #16 — PROD_CYCLE_AUTO_20260620_008

## TTS Voice Lines Generated This Cycle

| ID | Character | Line Summary | URL | Duration |
|----|-----------|-------------|-----|----------|
| A16_008_01 | ElderHunter_FireCaution | "Fire keeps night creatures back — keep it small" | [mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781953812678_ElderHunter_FireCaution.mp3) | ~10s |
| A16_008_02 | ScoutWarrior_RaptorWarning | "Raptor pack — three or four — run for rocks not trees" | [mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781953837534_ScoutWarrior_RaptorWarning.mp3) | ~11s |
| A16_008_03 | GathererNPC_WaterLocation | "Fresh water two ridges east — go at dawn not midday" | [mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781953874694_GathererNPC_WaterLocation.mp3) | ~11s |

## Freesound Assets Identified

### Campfire (use for Audio_CampfireAmbient actor)
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 729395 | Campfire 01 | 109s | https://cdn.freesound.org/previews/729/729395_12863902-hq.mp3 |
| 729396 | Campfire 02 | 268s | https://cdn.freesound.org/previews/729/729396_12863902-hq.mp3 |
| 852107 | Fireplace crackling loop | 8.6s | https://cdn.freesound.org/previews/852/852107_18387771-hq.mp3 |
| 856943 | Campfire + Forest Birds | 60s | https://cdn.freesound.org/previews/856/856943_12846320-hq.mp3 |

**Recommended**: ID 856943 "FIREBurn_Campfire Forest Birds" — 60s loop with natural ambience baked in, ideal for camp centre.

## Ambient Audio Actors Placed in MinPlayableMap

| Actor Label | Position | Purpose |
|-------------|----------|---------|
| Audio_CampfireAmbient | (700, 100, 110) | Camp centre fire — attach Freesound 856943 |
| Audio_NightInsects | (500, 300, 150) | Wide-radius insect loop |
| Audio_RaptorVocalLoop_East | (1500, -1200, 200) | Eastern valley raptor calls — danger zone audio cue |
| Audio_WindAmbient | (0, 0, 800) | Global wind — covers full map |
| Audio_TRexDistantRumble | (-800, 200, 300) | North ridge T-Rex presence — low-frequency rumble |

## NPC Dialogue Audio Assignments

Matches NPC markers placed by Agent #15 (Narrative):

| NPC Actor | Position | TTS Line to Attach | Trigger |
|-----------|----------|--------------------|---------|
| NPC_ElderHunter | (800, 200, 100) | A16_008_01 (FireCaution) | Player within 400 units |
| NPC_ScoutWarrior | (600, -400, 100) | A16_008_02 (RaptorWarning) | Player within 350 units |
| NPC_Gatherer | (400, 600, 100) | A16_008_03 (WaterLocation) | Player within 300 units |

## Audio Design Notes

### Spatial Audio Strategy
- **Camp zone** (radius ~600 units centred on 700,100): Campfire dominant, insects secondary, wind tertiary
- **Eastern valley** (X>1200, Y<-800): Raptor vocal loop activates — signals danger zone
- **North ridge** (X<-600): T-Rex rumble — low frequency, felt more than heard
- **Global**: Wind ambient at height 800 — non-attenuated, always present

### Adaptive Audio Intent
- Campfire volume inversely tied to time-of-day (louder at night, quieter at dawn)
- Raptor vocal loop pitch-shifts slightly when player health < 50% (stress response)
- T-Rex rumble increases in volume when TRex dino actor is within 2000 units of player
- All ambient actors use UE5 Attenuation with logarithmic falloff, inner radius 200, outer radius 1500

### Freesound Licence Notes
All identified Freesound assets are CC0 or CC-BY. Attribution required for:
- ID 729395/729396: Attribution to uploader (12863902)
- ID 856943: Attribution to KVV AUDIO

## Previous Cycle Audio Assets (Cumulative)
From cycles 005-007:
- TrackerNPC_WindWarning TTS
- ElderNPC_FireWarning TTS  
- TRex proximity warning TTS
- Tyrannosaurus roar search results
- Dinosaur footstep search results
