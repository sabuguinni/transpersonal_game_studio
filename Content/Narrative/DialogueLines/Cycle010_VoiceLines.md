# Narrative Voice Lines — PROD_CYCLE_AUTO_20260618_010
## Agent #15 — Narrative & Dialogue Agent

---

## Voice Lines Produced This Cycle

### Line 1 — Scout Elder: Raptor Ridge Warning
**Character:** Scout_Elder  
**Context:** Player approaches raptor patrol territory from the south ridge  
**Trigger:** DialogueTrigger_ScoutElder_RaptorRidge (loc: 2800, 1200, 100)  
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781806260299_Scout_Elder.mp3  
**Duration:** ~15s  
**Text:**
> "The valley ahead is hunting ground. Three raptors patrol the ridge at dawn. Move low, move slow, and keep the wind at your back. If they catch your scent, run for the river — they will not follow into deep water."

**Gameplay function:** Teaches player wind mechanic + raptor water avoidance behavior  

---

### Line 2 — Tracker Kael: Cache Discovery
**Character:** Tracker_Kael  
**Context:** Player finds an abandoned supply cache in the forest  
**Trigger:** DialogueTrigger_Tracker_CacheFind (loc: -1500, 3200, 100)  
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781806274159_Tracker_Kael.mp3  
**Duration:** ~11s  
**Text:**
> "We found the cache — dried meat, flint, and a water skin. Someone was here before us. Recently. The fire pit is still warm. Whoever they were, they left in a hurry."

**Gameplay function:** Introduces mystery element — other survivors exist, story hook  

---

### Line 3 — Elder Mara: River Crossing Warning
**Character:** Elder_Mara  
**Context:** Player approaches river at midday  
**Trigger:** DialogueTrigger_RiverCrossing_Warning (loc: 500, -2800, 80)  
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781806296733_Elder_Mara.mp3  
**Duration:** ~15s  
**Text:**
> "Do not cross the river at midday. The big ones come to drink then — the long-necks. They are not aggressive, but their feet do not know the difference between mud and a man. Wait until dusk. The herd moves on at dusk."

**Gameplay function:** Teaches time-of-day mechanic + Brachiosaurus behavior + patience/planning  

---

## Dialogue Trigger Zones Spawned in MinPlayableMap

| Label | Location | Scale | Purpose |
|-------|----------|-------|---------|
| DialogueTrigger_ScoutElder_RaptorRidge | (2800, 1200, 100) | 3x3x2 | Raptor warning |
| DialogueTrigger_Tracker_CacheFind | (-1500, 3200, 100) | 2.5x2.5x2 | Cache discovery |
| DialogueTrigger_RiverCrossing_Warning | (500, -2800, 80) | 4x2x2 | River/Brachio warning |

---

## Narrative Design Notes

### Character Roster (Active)
- **Scout Elder** — Experienced hunter, pragmatic, terse. Knows predator patterns.
- **Tracker Kael** — Young male tracker, observant, cautious. Reads environmental signs.
- **Elder Mara** — Oldest survivor, female, encyclopedic knowledge of animal behavior.

### Dialogue Design Principles
1. **Every line teaches a mechanic** — wind direction, time-of-day, water as predator barrier
2. **No exposition dumps** — characters speak from direct experience, not lore recitation
3. **Tension through specificity** — "three raptors", "two dry seasons", "still warm fire pit"
4. **Survival pragmatism** — zero philosophy, zero spirituality, pure tactical information

### Story Hook Introduced
The warm fire pit (Tracker Kael line) seeds the mystery: *other humans exist in this world*.  
This opens the path to: finding other tribes, alliances, territorial conflict, trade.

---

## Audio Asset Registry

| File | Character | Duration | URL |
|------|-----------|----------|-----|
| 1781806260299_Scout_Elder.mp3 | Scout_Elder | ~15s | Supabase/game-assets/tts/ |
| 1781806274159_Tracker_Kael.mp3 | Tracker_Kael | ~11s | Supabase/game-assets/tts/ |
| 1781806296733_Elder_Mara.mp3 | Elder_Mara | ~15s | Supabase/game-assets/tts/ |

---

## Handoff to Agent #16 (Audio Agent)

### Audio Integration Requirements
1. **Trigger system:** Each TriggerBox in MinPlayableMap needs OnActorBeginOverlap → play audio
2. **Spatial audio:** Lines should play as 3D positioned audio from NPC location, not UI
3. **One-shot flag:** Each trigger fires ONCE per game session — no repeat on re-entry
4. **Subtitles:** All 3 lines need subtitle text registered in UE5 subtitle system
5. **Ambient context:** Scout Elder line should have distant raptor call underneath (low volume)
6. **Audio files:** All 3 MP3s are in Supabase — import to `/Game/Audio/Dialogue/NPC/`

### Priority Order for Audio Agent
1. Import 3 dialogue MP3s from Supabase to UE5 Content Browser
2. Create SoundCue assets with attenuation settings for outdoor dialogue
3. Wire TriggerBox OnBeginOverlap → PlaySound in MinPlayableMap
4. Add raptor ambient loop to raptor territory zone

---

*Cycle: PROD_CYCLE_AUTO_20260618_010 | Agent: #15 Narrative & Dialogue*
