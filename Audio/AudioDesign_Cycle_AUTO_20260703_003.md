# Audio Design — Cycle AUTO_20260703_003
**Agent:** #16 Audio Agent  
**Date:** 2026-07-03  
**Priority:** P7 — Audio & VFX (Narrative Audio / Ambient Soundscape)

---

## TTS Voice Lines Produced

### 1. Tracker_Dak_HerdWarning (~22s)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783050095438_Tracker_Dak_HerdWarning.mp3
- **Character:** Tracker Dak (experienced survivor, male, weathered)
- **Trigger:** `EVT_Sauropod_HerdDetected` — plays when player first encounters sauropod herd tracks near hub (2100, 2400)
- **Script:** *"The herd passed through here at dawn. Twenty, maybe thirty of them — the long-necks. You can tell by the depth of the prints. Each one weighs more than ten men. They move together, always. If you see one alone, something is very wrong. A lone long-neck means the herd was scattered. And only one thing scatters a herd that size."*
- **Design Intent:** Teaches player to read environmental signs. The final unfinished sentence creates dread — the player must infer the threat. Walter Murch principle: what is NOT said is more powerful.
- **UE5 Actor:** `AudioTrigger_Dak_HerdWarning_001` @ (1900, 2400, 50)

### 2. Elder_Mara_FireWarning (~19s)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783050104436_Elder_Mara_FireWarning.mp3
- **Character:** Elder Mara (tribe elder, female, authoritative)
- **Trigger:** `EVT_Player_FirstCampfire` — plays when player crafts/lights first campfire
- **Script:** *"Fire. You think it is your greatest tool. You are wrong. Fire is a beacon. Every predator within two kilometres will see it. Every scavenger will smell it. You cook your meat fast, you eat fast, and you kill the fire before the sky goes dark. The night belongs to them. Not to you."*
- **Design Intent:** Reframes fire as a double-edged survival tool. Teaches the core tension: comfort vs. danger. Reinforces the night danger system.
- **UE5 Actor:** `AudioTrigger_Mara_FireWarning_001` @ (2100, 2600, 100)

---

## Previous Cycle TTS (carried forward for reference)

### 3. Tracker_Dak_SurvivalLesson (Cycle 013)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783038498478_Tracker_Dak_SurvivalLesson.mp3
- **Trigger:** `QST_SurvivalLesson_Begin` → player first enters forest biome

### 4. Survivor_Journal_Tension (Cycle 002)
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783045327222_Survivor_Journal_Tension.mp3
- **Trigger:** `EVT_TRex_ProximityEnter` — T-Rex audio zone at (2100, 2400)

### 5. Day7_ForestSilence (Cycle 001)
- **Trigger:** `EVT_ForestSilence_Detected` — ambient silence state change

### 6. ProximityAlert_400m (Cycle 001)
- **Trigger:** `EVT_LargePredator_400m` — HUD audio alert

---

## UE5 Audio Zone Actors Placed

| Actor Label | Location | Purpose |
|---|---|---|
| `AudioZone_TRex_Hub_001` | (2100, 2400, 50) | T-Rex proximity audio trigger sphere |
| `AudioZone_Forest_Day_001` | (2100, 2200, 100) | Daytime forest ambient soundscape |
| `AudioZone_Night_Danger_001` | (2300, 2500, 100) | Night danger ambient soundscape |
| `AudioTrigger_Dak_HerdWarning_001` | (1900, 2400, 50) | Dak herd warning dialogue trigger |
| `AudioTrigger_Mara_FireWarning_001` | (2100, 2600, 100) | Mara fire warning dialogue trigger |

---

## Audio Design System Architecture

### Adaptive Music States (MetaSounds Design)
```
State Machine:
  SAFE_DAY     → gentle percussion, distant bird calls, wind
  ALERT_DAY    → tension percussion builds, bird calls cease
  DANGER_DAY   → aggressive percussion, low drone
  SAFE_NIGHT   → crickets, distant calls, minimal percussion
  ALERT_NIGHT  → silence (Walter Murch: silence = maximum tension)
  DANGER_NIGHT → heartbeat percussion, predator breathing
  
Transitions:
  SAFE → ALERT: triggered by predator within 200m
  ALERT → DANGER: triggered by predator within 80m or combat start
  DANGER → ALERT: triggered by predator fleeing or player hiding
  ALERT → SAFE: 30s after last threat detection
```

### Narrative Audio Trigger System
```
Trigger Types:
  EVT_ProximityEnter    — player enters radius of named actor
  EVT_FirstInteraction  — player first crafts/uses item type
  EVT_BiomeEnter        — player first enters biome type
  EVT_QuestStep         — quest milestone reached
  EVT_HerdDetected      — player finds herd tracks/scat/prints
  
Priority Queue:
  CRITICAL (combat, death) > NARRATIVE (dialogue) > AMBIENT (soundscape)
  Narrative lines never interrupt CRITICAL audio
  Max 1 narrative line per 90 seconds (no audio spam)
```

### Sound Design Principles (Walter Murch / RDR2)
1. **Silence as weapon** — forest going quiet = maximum danger signal
2. **Invisible music** — player feels emotion, never notices the score
3. **Diegetic priority** — world sounds (footsteps, wind, water) always audible
4. **Distance cues** — predator audio starts at -40dB, rises as threat increases
5. **No jump-scare audio** — tension builds slowly, never cheap shock

---

## Freesound Search Results
*Note: Freesound API returned empty results this cycle (likely rate limit or connectivity)*

**Queued searches for next cycle:**
- `dinosaur footsteps heavy ground impact prehistoric`
- `jungle ambience night insects crickets tropical forest`
- `heavy breathing creature growl predator ambience`
- `fire campfire crackling wood burning primitive`

---

## Dependencies
- **From #15 (Narrative):** Dialogue scripts for Tracker Dak, Elder Mara, other named characters
- **From #14 (Quest):** Quest event IDs for trigger binding (`QST_*` events)
- **From #12 (Combat AI):** Predator proximity events (`EVT_TRex_*`, `EVT_Raptor_*`)
- **To #17 (VFX):** Audio zone actor locations for VFX co-placement (dust particles, visual cues)

---

## Next Cycle Priorities
1. Generate TTS for **combat audio** — first raptor attack warning, player injury grunt
2. Generate TTS for **crafting audio** — first stone tool completion, first shelter built
3. Implement **MetaSounds Blueprint** for adaptive music state machine
4. Search Freesound for **sauropod movement** and **raptor vocalisation** SFX
