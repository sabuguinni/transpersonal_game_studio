# Audio Catalogue — Cycle 009 (PROD_CYCLE_AUTO_20260703_009)
## Agent #16 — Audio Agent

---

## VOICE LINES (ElevenLabs TTS)

### TTS-10: ElderMara_TRexWarning (~16s)
- **Line:** *"The T-Rex does not hunt by sight alone. It reads the air — your sweat, your fear, the blood on your hands from yesterday's kill. Stand downwind. Stay still. Breathe shallow. If it turns toward you, do not run. Running tells it you are prey."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783080989664_ElderMara_TRexWarning.mp3`
- **Trigger:** `EVT_TRex_Proximity_50m` — fires when player enters 50m radius of T-Rex actor
- **Audio Zone:** `AudioZone_TRex_Hub_001` at X=2100, Y=2400, Z=100

### TTS-11: ElderMara_StormWarning (~16s)
- **Line:** *"Rain is coming. Three days, maybe four. Fill every vessel you have — the river will flood and turn brown and bitter. Seal the dried meat under bark and stone. A wet camp in the cold season has killed more of our people than any predator."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783080993112_ElderMara_StormWarning.mp3`
- **Trigger:** `EVT_Weather_StormApproaching` — fires when storm weather state begins
- **Audio Zone:** `AudioZone_Storm_Hub_001` at X=2300, Y=2200, Z=100

---

## PREVIOUS VOICE LINES (Cycles 006-008)

### TTS-07: ElderMara_GroundTremor (~24s)
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783067116136_ElderMara_GroundTremor.mp3`
- **Trigger:** `EVT_TRex_Footstep_Proximity_100m`

### TTS-08: ElderMara_NightAmbience (~13s)
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783071533016_ElderMara_NightAmbience.mp3`
- **Trigger:** `EVT_NightCycle_Begin`

### TTS-09: ElderMara_CraftingWisdom (~12s)
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783076176004_ElderMara_CraftingWisdom.mp3`
- **Trigger:** `EVT_Player_OpenCraftingMenu`

---

## SOUND EFFECTS (Freesound.org)

### SFX-1: Wildlife Creature Warning Howl
- **Freesound ID:** 743078
- **Duration:** 10.7s
- **Preview:** `https://cdn.freesound.org/previews/743/743078_71257-hq.mp3`
- **Tags:** Dinosaur, creature, danger, attack, alien
- **Usage:** T-Rex proximity alert — plays at 50-100m range, attenuated with distance
- **MetaSound Trigger:** `EVT_TRex_Proximity_100m`

### SFX-2: Forest Valley Ambience (Long)
- **Freesound ID:** 825739
- **Duration:** 12002.3s (~3.3 hours)
- **Preview:** `https://cdn.freesound.org/previews/825/825739_5287430-hq.mp3`
- **Tags:** Field recording, forest, valley, ambiance
- **Usage:** Dense forest biome ambient loop — seamless background for forested areas
- **Audio Zone:** `AudioZone_ForestAmbience_Hub_001` at X=1900, Y=2600

### SFX-3: Forest Noise (Short Burst)
- **Freesound ID:** 825696
- **Duration:** 46.3s
- **Preview:** `https://cdn.freesound.org/previews/825/825696_5287430-hq.mp3`
- **Tags:** Field recording, forest, noise
- **Usage:** Predator movement cue — random stochastic trigger when predator is nearby but unseen
- **MetaSound Trigger:** `EVT_Predator_Unseen_Nearby`

---

## UE5 AUDIO TRIGGER ZONES DEPLOYED

| Label | World Location | Scale | Trigger Event |
|-------|---------------|-------|---------------|
| `AudioZone_TRex_Hub_001` | X=2100, Y=2400, Z=100 | 8x8x4 | EVT_TRex_Proximity_50m |
| `AudioZone_Storm_Hub_001` | X=2300, Y=2200, Z=100 | 6x6x3 | EVT_Weather_StormApproaching |
| `AudioZone_ForestAmbience_Hub_001` | X=1900, Y=2600, Z=100 | 10x10x4 | EVT_Biome_Forest_Enter |
| `AudioZone_CampPerimeter_Hub_001` | X=2100, Y=2700, Z=100 | 7x7x3 | EVT_Camp_Perimeter_Breach |

---

## ELDER MARA — VOICE CHARACTER PROFILE

**Character:** Elder Mara — the tribe's oldest surviving hunter. Speaks in short, precise sentences. No metaphors. No comfort. Pure survival intelligence distilled from decades of close calls.

**Voice Tone:** Low, gravelly, unhurried. The voice of someone who has already survived everything the world has thrown at them.

**Line Count Total:** 11 lines (TTS-01 through TTS-11)

**Trigger Coverage:**
- T-Rex detection: TTS-10 (proximity warning)
- T-Rex footstep: TTS-07 (ground tremor)
- Night cycle: TTS-08 (darkness warning)
- Crafting: TTS-09 (tool wisdom)
- Storm: TTS-11 (weather preparation)
- Raptor behavior: TTS (previous cycles)
- Stone crafting: TTS (previous cycles)
- Naming/fear: TTS (previous cycles)
- Northern Forest lore: TTS (previous cycles)

---

## METASOUND INTEGRATION NOTES

All voice lines should be imported as USoundWave assets and wrapped in MetaSound patches:
- **Attenuation:** Use `SoundAttenuation_NPC_Voice` with falloff 200-800cm
- **Priority:** Voice lines override ambient SFX (priority 90 vs ambient priority 30)
- **Cooldown:** Each line has 120s cooldown to prevent repetition
- **Subtitles:** All lines require subtitle text in `DataTable_Subtitles`

---

## NEXT CYCLE RECOMMENDATIONS

**For Agent #17 (VFX):**
- T-Rex proximity zone (`AudioZone_TRex_Hub_001`) should trigger Niagara dust/ground shake VFX
- Storm warning zone should trigger rain particle system onset
- Camp perimeter breach should trigger torch flicker VFX

**Audio gaps remaining:**
- Crafting sound effects (stone-on-stone, bone snapping, fire ignition)
- Footstep system (dirt, grass, mud, rock surface variants)
- Dinosaur herd ambient vocalizations (non-threat, distance)
- Water sounds (river, rain on leaves, puddle splashes)
