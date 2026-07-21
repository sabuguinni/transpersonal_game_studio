# Audio Agent — Cycle PROD_CYCLE_AUTO_20260703_002

## Voice Lines Generated

### 1. Survivor_Journal_Tension (~16s)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783045327222_Survivor_Journal_Tension.mp3
- **Text:** "The valley fell silent. No wind. No insects. Just the distant crack of something massive moving through the tree line. I gripped my spear tighter and pressed my back against the rock. Whatever it was — it was close. And it was hunting."
- **Trigger:** `EVT_TRex_ProximityEnter` — plays when player enters Audio_TRex_ProximityZone_001
- **Design Note:** Diegetic journal narration. No music underneath — silence IS the tension. Fade in low sub-bass rumble at 8s mark.

### 2. Tracker_Dak_EscapeCommand (~10s)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783045333697_Tracker_Dak_EscapeCommand.mp3
- **Text:** "Run. Do not look back. Do not stop. The moment you hesitate — you are dead. Move to the river. The water will break your scent trail. Go. Now."
- **Trigger:** `EVT_CombatFlee_TRex` — plays when player health < 30% and T-Rex is within 600 units
- **Design Note:** Urgent, commanding. Short sentences = panic rhythm. This is the audio cue that teaches the player the river mechanic.

## Sound Effects Found (Freesound)

### Campfire Ambience
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 681367 | Campfire (Position 2) | 22s | https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3 |
| 681366 | Campfire (Position 1) | 83s | https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3 |
| 688992 | Campfire Just After Dusk | 540s | https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3 |
| 802195 | Fire-Nature Sounds | 247s | https://cdn.freesound.org/previews/802/802195_17223245-hq.mp3 |

**Usage:** Loop `681366` (83s, long loop) at `Audio_Campfire_Hub_001` actor position. Attenuate to 0 at 500 units radius. Layer `802195` (nature sounds) as background forest ambient.

## UE5 Actors Spawned

| Label | Type | Location | Purpose |
|-------|------|----------|---------|
| Audio_Campfire_Hub_001 | PointLight | (150, 80, 20) | Campfire warm light + audio anchor |
| Audio_Campfire_Hub_002 | PointLight | (155, 85, 15) | Secondary campfire flicker light |
| Audio_TRex_ProximityZone_001 | TriggerBox | (2100, 2400, 50) | T-Rex audio alert zone (scale 8x8x3) |
| Audio_ForestAmbient_Raptor_001 | TriggerBox | (800, 1200, 30) | Raptor territory ambient zone (scale 20x20x5) |
| Audio_RiverAmbient_001 | TriggerBox | (-500, 1800, 10) | River water ambient zone (scale 15x40x3) |
| Audio_LoreMarker_TrackerDak_001 | TriggerBox | (300, 200, 25) | Tracker Dak dialogue trigger zone |

## Audio Zone Design

### Zone Architecture (MetaSounds Blueprint)
```
PlayerEnters(Audio_TRex_ProximityZone_001)
  → StopCurrentAmbient()
  → PlayOneShot(Survivor_Journal_Tension.mp3)
  → StartLoop(TRex_Distant_Rumble_SFX, volume=0.3)
  → TriggerCameraShake(LightShake, radius=1200)

PlayerEnters(Audio_ForestAmbient_Raptor_001)
  → CrossfadeTo(Forest_Dense_Insects_Loop, duration=3s)
  → StartLoop(Raptor_Distant_Calls_Loop, volume=0.15, random_interval=8-25s)

PlayerEnters(Audio_RiverAmbient_001)
  → CrossfadeTo(River_Flow_Loop, duration=2s)
  → StopLoop(Raptor_Distant_Calls_Loop)

PlayerEnters(Audio_LoreMarker_TrackerDak_001)
  → PlayOneShot(Tracker_Dak_SurvivalLesson.mp3)  [from previous cycle]
  → SetFlag(LoreTrigger_TrackerDak_Played = true)
```

## Campfire Light Design
- **Primary (Hub_001):** Orange-amber (1.0, 0.45, 0.1), 800 intensity, 400 radius
- **Secondary (Hub_002):** Deep orange (1.0, 0.3, 0.05), 300 intensity, 200 radius
- **Blueprint animation needed:** Flicker both lights with random intensity variation (600-900 / 200-400) at 8-15 fps for realistic campfire effect

## Adaptive Music System Design

### State Machine (to be implemented in MetaSounds)
```
BIOME_OPEN_PLAINS:
  - Base layer: Low percussion (bone drums, 60 BPM)
  - Tension layer: Muted (0%)
  - Alert layer: Muted (0%)

BIOME_DENSE_FOREST:
  - Base layer: Insect ambience + distant bird calls
  - Tension layer: 20% (subtle string tension)
  - Alert layer: Muted (0%)

PREDATOR_NEARBY (< 800 units):
  - Base layer: Fade to 30%
  - Tension layer: 80%
  - Alert layer: 40%

PREDATOR_ATTACKING (< 200 units):
  - Base layer: 0%
  - Tension layer: 100%
  - Alert layer: 100% (full percussion, fast tempo)

SAFE_CAMP (near campfire):
  - Base layer: Campfire crackle + soft percussion
  - Tension layer: 0%
  - Alert layer: 0%
```

## Technical Notes
- All TriggerBox zones use scale multiplied by 100 (UE5 default box = 100x100x100 units)
- Audio_TRex_ProximityZone_001 covers ~800x800 units — appropriate for T-Rex detection range
- River zone is elongated (15x40) to match river corridor geometry
- Campfire lights positioned at PlayerStart hub (150, 80) — visible from spawn point

## Dependencies for Next Agent (#17 VFX)
- `Audio_Campfire_Hub_001` and `Audio_Campfire_Hub_002` need Niagara fire particle system attached
- `Audio_TRex_ProximityZone_001` overlap should trigger screen shake VFX
- `Audio_ForestAmbient_Raptor_001` should have subtle particle dust/pollen effect
- Campfire Blueprint needs light flicker animation (random intensity variation)
