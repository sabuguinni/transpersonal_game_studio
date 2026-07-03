# Audio Agent — Cycle 012 Deliverables
## PROD_CYCLE_AUTO_20260703_012

---

## Voice Lines Generated (ElevenLabs TTS)

### [TTS-13] ElderMara_MovementSilence (~28s)
- **Character:** Elder Mara (survival mentor NPC)
- **Trigger:** Player enters dense forest biome for first time
- **Line:** *"I have seen men die because they made noise. Not because they were slow, not because they were weak — because they cracked a branch at the wrong moment, or coughed when the wind dropped. Sound travels far out here. The predators hear what you cannot. Move like water. Step where the ground is soft. When you must cross dry leaves, go slow — one foot, then wait, then the other. Silence is the oldest weapon we have."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096294453_ElderMara_MovementSilence.mp3`
- **Use:** Plays once when player first enters dense vegetation zone. Teaches stealth mechanic.

### [TTS-14] ElderMara_FireCraft (~27s)
- **Character:** Elder Mara (survival mentor NPC)
- **Trigger:** Player crafts first fire OR approaches campfire at hub
- **Line:** *"Fire is life. Fire is also death — yours, if you are careless. A fire that smokes draws everything within two valleys. Keep it small. Feed it dry wood, not green. Build it against rock so the light does not spread. And when you sleep, bank the coals — do not let it die, but do not let it breathe too freely either. The night belongs to things that see better than us. Do not give them a beacon."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096299419_ElderMara_FireCraft.mp3`
- **Use:** Plays when player first crafts/interacts with fire. Teaches fire management mechanic.

---

## Sound Effects Catalogued (Freesound.org)

### Campfire Ambience
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 681367 | Campfire (Position 2) | 22s | [MP3](https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3) |
| 681366 | Campfire (Position 1) | 83s | [MP3](https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3) |
| 688992 | Campfire Just After Dusk | 540s | [MP3](https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3) |
| 802195 | Fire-Nature Sounds | 247s | [MP3](https://cdn.freesound.org/previews/802/802195_17223245-hq.mp3) |

**Recommended:** ID 681366 (83s loop, clean campfire, no music) for `Audio_CampfireHub_001` AmbientSound actor.

### Storm / Rain / Thunder
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 728687 | Amb_Forest_Thunder_Rain_Night | 118s | [MP3](https://cdn.freesound.org/previews/728/728687_8407191-hq.mp3) |
| 344975 | Stormy Night | 41s | [MP3](https://cdn.freesound.org/previews/344/344975_4034520-hq.mp3) |
| 639478 | Rain Downpour Thunder | 180s | [MP3](https://cdn.freesound.org/previews/639/639478_5828667-hq.mp3) |

**Recommended:** ID 728687 (forest thunder + rain + birds, 118s) for `Audio_StormZone_Trigger_001` — plays when storm weather state activates.

---

## UE5 Audio Zones Placed (MinPlayableMap)

All zones centred on hub area X=2100, Y=2400 (hero screenshot composition):

| Actor Label | Type | Location | Purpose |
|-------------|------|----------|---------|
| `Audio_CampfireHub_001` | AmbientSound | (2100, 2400, 50) | Campfire crackling loop at hub centre |
| `Audio_ForestNight_001` | AmbientSound | (2350, 2600, 80) | Night insects/cicadas at forest edge |
| `Audio_WindTrees_001` | AmbientSound | (1800, 2200, 100) | Wind through canopy, west treeline |
| `Audio_RiverDistant_001` | AmbientSound | (2500, 2100, 60) | Distant water/river approach |
| `Audio_StormZone_Trigger_001` | TriggerVolume | (2100, 2400, 200) | 30x30 — activates storm audio layer |
| `Audio_TRexProximity_Trigger_001` | TriggerVolume | (2100, 2400, 150) | 50x50 — activates T-Rex proximity heartbeat |
| `Light_CampfireHub_001` | PointLight | (2100, 2400, 55) | Orange 800lux, radius 600 — campfire glow |

---

## Elder Mara Dialogue Arc — Complete Catalogue (Cycles 009-012)

| TTS ID | File Name | Topic | Duration | URL |
|--------|-----------|-------|----------|-----|
| TTS-10 | ElderMara_TRexWarning | T-Rex scent/stealth | ~16s | [Link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/...) |
| TTS-11 | ElderMara_ForestHeartbeat | Reading nature signs | ~16s | [Link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/...) |
| TTS-12 | ElderMara_StormSurvival | Storm as cover | ~18s | [Link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/...) |
| TTS-13 | ElderMara_MovementSilence | Stealth movement | ~28s | [Link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096294453_ElderMara_MovementSilence.mp3) |
| TTS-14 | ElderMara_FireCraft | Fire management | ~27s | [Link](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783096299419_ElderMara_FireCraft.mp3) |

**Remaining Elder Mara lines to generate (next cycles):**
- ElderMara_WaterFinding — locating water sources in dry season
- ElderMara_PackHunters — raptor pack behaviour and escape tactics
- ElderMara_WinterComing — seasonal survival preparation
- ElderMara_FirstKill — emotional/practical guidance after first hunt
- ElderMara_Shelter — building effective shelter against predators

---

## Audio Design Philosophy (Walter Murch Principle)

> *"The sound that does not exist is often more powerful than the sound that exists."*

Applied to this game:
- **T-Rex zone:** Heartbeat SFX + low-frequency rumble plays BEFORE T-Rex is visible. Player hears the danger before seeing it.
- **Storm warning:** Bird silence + distant thunder plays 30s before storm hits. Teaches player to read audio cues.
- **Night transition:** Daytime insects fade → silence → night insects layer in. The silence between is the emotional beat.
- **Campfire hub:** Fire crackle is the ONLY warm sound in the world. It anchors the hub as safe space emotionally.

---

## Next Cycle Priorities (for Audio Agent #16)

1. **ElderMara_WaterFinding** — next TTS line in arc
2. **ElderMara_PackHunters** — raptor-specific survival dialogue
3. **Search sounds:** raptor vocalizations, bone/stone crafting impacts
4. **UE5:** Configure AmbientSound actors with actual sound cue assets once imported
5. **UE5:** Add reverb volumes inside cave areas for acoustic differentiation
