# Audio System Design — Transpersonal Game Studio
**Agent:** Audio Agent #16  
**Cycle:** PROD_CYCLE_AUTO_20260623_008  
**Priority:** P7 — Audio & VFX

---

## Philosophy (Walter Murch Principle)
> "The sound that does not exist is often more powerful than the sound that does."

The audio system for this prehistoric survival game operates on **absence and presence**. Silence before a T-Rex appears is more terrifying than any roar. The player should feel the world through sound before they see it.

**Red Dead Redemption 2 Principle:** Music the player never consciously notices but that changes what they feel is the peak of interactive composition. Our adaptive music system must be invisible — the player thinks "I feel tense" not "the music changed."

---

## Architecture Overview

### Layer 1: World Ambience (Always On)
- Base environmental loop per biome (jungle, open plain, riverbank, cave)
- Wind intensity driven by weather system
- Insect/bird density driven by time of day
- Implemented as: AmbientSound actors with MetaSound Sources

### Layer 2: Proximity Audio (Spatial)
- Dinosaur vocalisations with attenuation curves
- Water sources (rivers, waterfalls) — spatial anchors
- Fire (campfire, volcanic vents) — crackling + heat shimmer
- Implemented as: Spatialized AmbientSound actors near gameplay zones

### Layer 3: Adaptive Music (Invisible)
- **State machine:** Calm → Aware → Tense → Combat → Aftermath
- Transitions driven by: nearest predator distance, player health, quest state
- Instrument palette: bone percussion, hollow wood, stone resonance, breath
- NO orchestral instruments — only materials that existed in the Cretaceous
- Implemented as: MetaSound parameter modulation + Blueprint state machine

### Layer 4: Quest Stingers (Event-Driven)
- Non-spatialized 2D cues triggered by quest events
- Short, earned, not celebratory
- See: `AudioZoneConfig.json` → `quest_stingers`

### Layer 5: Voice Lines (NPC Dialogue)
- ElevenLabs TTS — quota pending reset
- Voice profiles defined in `AudioZoneConfig.json` → `voice_lines_pending_tts`
- Priority 1: TribalElder_Kara (QUEST_RAPTOR_HUNT_01)
- Priority 2: Scout_Davan (QUEST_HERD_MIGRATION_01)

---

## Instrument Palette (Cretaceous-Authentic)
All musical elements must be producible from materials available in the prehistoric era:

| Instrument | Material | Sound Character |
|---|---|---|
| `bone_drum` | Large hollow bone, hide membrane | Deep, resonant, irregular |
| `bone_flute` | Hollow bird/small animal bone | High, breathy, pentatonic |
| `hollow_wood_knock` | Fallen log, stone striker | Dry, sharp, rhythmic |
| `resonant_stone_tap` | Flat stone on stone | Metallic-adjacent, clear |
| `breath_drone` | Human breath, throat | Intimate, primal, unsettling |
| `low_drone_fall` | Bone + breath combined | Descending, grave, final |

---

## Audio Zones Deployed (MinPlayableMap)

### AudioAmbient_RaptorHunt (1800, 1200, 100)
- Linked to: `NarrDialogue_RaptorHunt`, `QUEST_RAPTOR_HUNT_01`
- Layers: raptor clicks, jungle wind, raptor alert bark
- Music: bone drum tension ramp, 72 BPM

### AudioAmbient_HerdMigration (-2000, 800, 100)
- Linked to: `NarrDialogue_HerdMigration`, `QUEST_HERD_MIGRATION_01`
- Layers: herd rumble, sauropod contact calls, open plain wind
- Music: sparse wonder theme, 48 BPM, bone flute

### AudioAmbient_Stampede (500, -1500, 100)
- Linked to: `NarrDialogue_StampedeSurvival`, `QUEST_STAMPEDE_SURVIVAL_01`
- Layers: ground tremor, footfall impacts, panic calls
- Music: driving percussion, 140 BPM, no melody

---

## Quest Stingers Deployed (MinPlayableMap)

| Actor Label | Event | Duration | Character |
|---|---|---|---|
| `AudioStinger_QuestStart` | OnQuestAccepted | 2.5s | Bone drum + flute rise |
| `AudioStinger_ObjComplete` | OnObjectiveComplete | 1.2s | Stone tap + exhale |
| `AudioStinger_QuestComplete` | OnQuestComplete | 4.0s | Full resolution phrase |
| `AudioStinger_QuestFail` | OnQuestFailed | 3.0s | Low descending drone |

---

## Voice Lines Manifest

### Priority 1 — TribalElder_Kara (QUEST_RAPTOR_HUNT_01)
- `RH_INTRO_01`: "Stay low. The great lizard hunts at dusk..."
- `RH_INTRO_02`: "Three raptors. Moving fast. They circle before they strike..."
- Voice profile: elder_female_raspy_authoritative

### Priority 2 — Scout_Davan (QUEST_HERD_MIGRATION_01)
- `HM_INTRO_01`: "Look at them. Hundreds. Moving south like they do every dry season..."
- `HM_WAYPOINT_A`: "They crossed the river here. See the tracks?..."
- Voice profile: young_male_eager_observational

**Status:** Pending ElevenLabs quota reset. All IDs defined and ready for import.

---

## MetaSound Integration Path
1. Replace AmbientSound actor placeholders with MetaSound Source components
2. Wire quest state (from QuestManager) to MetaSound parameter bus
3. Wire predator proximity (from DinosaurAI) to music intensity parameter
4. Implement adaptive crossfade between music states (4-8s transitions)
5. Add reverb zones: cave (high reverb), open plain (minimal), jungle (mid + flutter)

---

## Dependencies
- **From #15 (Narrative):** Quest IDs, dialogue zone locations, NPC speaker profiles ✅
- **From #12 (Combat AI):** Predator proximity events for music state machine
- **From #14 (Quest):** Quest event dispatchers (OnQuestAccepted, OnObjectiveComplete, etc.)
- **To #17 (VFX):** Audio-driven VFX triggers — raptor alert bark → particle burst; stampede rumble → screen shake magnitude

---

## API Fallbacks Applied This Cycle
- `text_to_speech` FAIL (quota exceeded) → voice lines documented in manifest, IDs defined, ready for next quota reset
- `search_sounds` FAIL (service degraded) → audio design documented procedurally, MetaSound implementation path defined
