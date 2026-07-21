# Narrative & Dialogue — Cycle 004 Voice Lines
## Agent #15 — PROD_CYCLE_AUTO_20260703_004

---

## Voice Lines Produced

### [TTS-1] ElderMara_HerdReading (~19s)
**Character:** Elder Mara — tribal elder, weather reader, herd tracker  
**Trigger:** `BRACHIOSAURUS_HERD_NEARBY` + `STORM_INCOMING_FLAG`  
**Condition:** Player within 1200 units of Brachiosaurus herd, storm weather state active  
**Audio URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783055952326_ElderMara_HerdReading.mp3`

**Line:**
> "The herd moves before the storm. Watch the Brachiosaurus — when they turn north and bunch together, you have maybe half a day before the sky breaks open. We follow them. Not because we trust them. Because they have been reading this land for longer than our tribe has existed."

**Narrative function:** Teaches player to use dinosaur behavior as environmental cues. Reinforces ecological realism — herbivores as weather indicators.

---

### [TTS-2] TrackerDak_TRexStealth (~17s)
**Character:** Tracker Dak — young scout, survival expert, pragmatic  
**Trigger:** `TREX_PROXIMITY < 1500` + `PLAYER_IN_OPEN_GROUND`  
**Condition:** Player detected in open terrain within TRex patrol range  
**Audio URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783055955676_TrackerDak_TRexStealth.mp3`

**Line:**
> "Stay low. Stay downwind. The Tyrannosaurus cannot see you if you do not move — but it can smell fear, and it can hear your heartbeat if you are close enough. Three steps. Then freeze. Three steps. Then freeze. That is how you survive the open ground."

**Narrative function:** Introduces stealth mechanics through dialogue. Teaches motion-freeze survival tactic. Reinforces TRex sensory behavior (poor vision, strong smell/hearing).

---

## UE5 Actors Placed

| Label | Type | Location | Purpose |
|-------|------|----------|---------|
| `NarrTrigger_HerdObservation_001` | TriggerBox | (1800, 2800, 120) | Fires ElderMara_HerdReading when player enters |
| `NarrTrigger_TRexStealthZone_001` | TriggerBox | (2400, 1600, 100) | Fires TrackerDak_TRexStealth when player enters |
| `NarrMarker_ElderCampfire_001` | PointLight (orange) | (2100, 2400, 90) | Visual anchor for elder camp — warm firelight |

---

## Cumulative Dialogue Asset Registry (Cycles 001–004)

| ID | Character | Line Summary | Audio URL |
|----|-----------|-------------|-----------|
| DAK_001 | TrackerDak | Raptor warning — flanking behavior | `tts/1783040249957_TrackerDak_RaptorWarning.mp3` |
| MARA_001 | ElderMara | Migration lore — great hunter Korrax | `tts/...ElderMara_MigrationLore.mp3` |
| DAK_002 | TrackerDak | River crossing at dusk — danger tips | `tts/...TrackerDak_RiverCrossing.mp3` |
| MARA_002 | ElderMara | Old valley exodus — 3 seasons ago | `tts/...ElderMara_OldValley.mp3` |
| SCOUT_001 | ScoutNPC | Triceratops herd near river bend | `tts/...Scout_TricerHerd.mp3` |
| DAK_003 | TrackerDak | Raptor night patrol — camp perimeter | `tts/...TrackerDak_RaptorNight.mp3` |
| MARA_003 | ElderMara | Herd reading — storm prediction | `tts/1783055952326_ElderMara_HerdReading.mp3` |
| DAK_004 | TrackerDak | TRex stealth survival — freeze tactic | `tts/1783055955676_TrackerDak_TRexStealth.mp3` |

---

## Narrative Trigger Map (World Coordinates)

```
World Hub Center: X=2100, Y=2400

[NarrTrigger_HerdObservation_001]  X=1800, Y=2800  → MARA_003 (herd+storm)
[NarrTrigger_TRexStealthZone_001]  X=2400, Y=1600  → DAK_004  (TRex stealth)
[NarrMarker_ElderCampfire_001]     X=2100, Y=2400  → Camp anchor (orange light)

Previous cycle triggers:
[QuestTrigger_HuntTriceratops_001] X=1600, Y=2200  → SCOUT_001
[QuestTrigger_RaptorAlert_001]     X=2600, Y=2600  → DAK_003
[CraftingStation_Flint_001]        X=2000, Y=2500  → crafting anchor
[CraftingStation_Bone_001]         X=2200, Y=2300  → crafting anchor
```

---

## Handoff to Agent #16 (Audio Agent)

**Audio assets ready for integration:**
- 8 voice lines with URLs (all in Supabase Storage)
- 3 trigger volumes placed in world at correct coordinates
- Orange campfire light at hub center (X=2100, Y=2400) for atmosphere

**Requests for Audio Agent:**
1. Ambient sound layer for TRex stealth zone — low rumble, distant footsteps
2. Storm approach audio — wind buildup, distant thunder for herd observation area
3. Campfire crackling loop for `NarrMarker_ElderCampfire_001` location
4. Raptor vocalization (distant) for `QuestTrigger_RaptorAlert_001` zone

**Naming convention:** All audio actors must follow `Audio_Type_Bioma_NNN` pattern per global naming rule.
