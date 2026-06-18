# Audio Agent #16 — Audio System Design — Cycle 003
**PROD_CYCLE_AUTO_20260618_003**

---

## Voice Lines Produced This Cycle

| Character | File | Duration | Content | URL |
|-----------|------|----------|---------|-----|
| Elder_Hunter_CampFire | Elder_Hunter_CampFire.mp3 | ~17s | "The fire speaks tonight. Listen — the crackle, the pop of wet wood. This is the sound of survival..." | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756638340_Elder_Hunter_CampFire.mp3) |
| Elder_Hunter_TRex_Warning | Elder_Hunter_TRex_Warning.mp3 | ~13s | "Move. Now. Do not look back. When the ground shakes like that — when the birds go silent all at once..." | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756641213_Elder_Hunter_TRex_Warning.mp3) |

---

## Complete Voice Line Index (All Cycles)

| Cycle | Character | URL | Trigger |
|-------|-----------|-----|---------|
| 001 | Elder_Hunter_Q1_Briefing | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/...) | Q1 start |
| 002 | Elder_Hunter_Q2_Briefing | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/...) | Q2 start |
| 003 | Elder_Hunter_Q3_Briefing | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756491228_Elder_Hunter_Q3.mp3) | Q3 start |
| 003 | Elder_Hunter_JournalIntro | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756493668_Elder_Hunter_JournalIntro.mp3) | Journal zone enter |
| 003 | Elder_Hunter_Q2_Success | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756503057_Elder_Hunter_Q2_Success.mp3) | Q2 complete |
| 003 | JournalEntry_LostHunter_Day12 | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756506565_JournalEntry_LostHunter_Day12.mp3) | Journal Entry 4 |
| 003 | Elder_Hunter_CampFire | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756638340_Elder_Hunter_CampFire.mp3) | Camp arrival |
| 003 | Elder_Hunter_TRex_Warning | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781756641213_Elder_Hunter_TRex_Warning.mp3) | T-Rex proximity |

---

## Audio Zone Actors Spawned in MinPlayableMap

All actors use PointLight as spatial anchor (visible in editor, defines audio trigger radius).

| Actor Label | Location | Color | Intensity | Radius | Audio Purpose |
|-------------|----------|-------|-----------|--------|---------------|
| `AudioZone_CampFire_Elder_001` | (0, -800, 120) | Amber | 800 | 600u | Campfire crackle + Elder voice |
| `AudioZone_Journal_Entry01` | (1000, 500, 100) | Warm Yellow | 400 | 350u | Journal discovery SFX + voice |
| `AudioZone_Journal_Entry02` | (2200, 1800, 100) | Warm Yellow | 400 | 350u | Journal discovery SFX + voice |
| `AudioZone_Journal_Entry03` | (3500, 3000, 100) | Warm Yellow | 400 | 350u | Journal discovery SFX + voice |
| `AudioZone_Journal_Entry04` | (5000, 4200, 100) | Warm Yellow | 400 | 350u | Journal discovery SFX + voice |
| `AudioZone_Journal_FinalSpear` | (6500, 5800, 100) | Deep Red | 600 | 500u | Melancholic underscore trigger |
| `AudioZone_TRex_Proximity_001` | (3000, 0, 120) | Red | 1200 | 1200u | Heartbeat + rumble + screen shake |
| `AudioZone_RaptorPack_001` | (-1500, 2000, 100) | Green | 700 | 800u | Raptor chittering + tension layer |
| `AudioZone_WateringHole_Q3` | (4500, -2000, 80) | Blue | 500 | 700u | Water ambient + peaceful underscore |
| `AudioZone_QuestSuccess_Q1` | (2000, 1000, 100) | Gold | 600 | 400u | Tribal percussion fanfare |
| `AudioZone_QuestSuccess_Q2` | (-500, 3500, 100) | Gold | 600 | 400u | Tribal percussion fanfare |
| `AudioZone_QuestSuccess_Q3` | (5500, -1500, 100) | Gold | 600 | 400u | Tribal percussion fanfare |

---

## Audio Design System — Full Specification

### 1. Adaptive Music Layers

The soundtrack uses a **3-layer adaptive system** based on threat level:

```
Layer 0 — SAFE (default):
  - Low drone: bone flute, wind through grass
  - Tempo: ~60 BPM
  - Instruments: reed flute, hollow log percussion, ambient wind

Layer 1 — ALERT (predator nearby, radius 800u):
  - Add: low string tension, irregular heartbeat percussion
  - Tempo: ~80 BPM, irregular
  - Trigger: AudioZone_RaptorPack_001 or AudioZone_TRex_Proximity_001

Layer 2 — DANGER (predator within 400u):
  - Add: rapid percussion, dissonant bone horn stabs
  - Tempo: ~110 BPM
  - Trigger: proximity < 400u to any dinosaur

Layer 3 — COMBAT:
  - Full percussion, frantic rhythm, no melody
  - Tempo: ~140 BPM
  - Trigger: player takes damage
```

### 2. Environmental Ambient Zones

```
CAMP ZONE (AudioZone_CampFire_Elder_001):
  - Primary: fire crackling loop (looping, 3D spatialized)
  - Secondary: distant night insects
  - Music: slow, warm percussion — safety theme
  - Voice: Elder_Hunter_CampFire.mp3 on first arrival

JOURNAL TRAIL (AudioZone_Journal_Entry01–04):
  - Primary: wind through grass, distant bird calls
  - Discovery SFX: stone scraping sound (150ms, non-looping)
  - Voice: corresponding journal entry MP3
  - Music: melancholic single-note bone flute phrase

FINAL SPEAR (AudioZone_Journal_FinalSpear):
  - Primary: complete silence for 2 seconds
  - Then: single low drum hit
  - Voice: JournalEntry_LostHunter_Day12.mp3
  - Music: sustained low drone, no rhythm

T-REX PROXIMITY (AudioZone_TRex_Proximity_001):
  - Primary: ground rumble (sub-bass, felt not heard)
  - Secondary: birds fleeing (sudden silence then wingbeats)
  - Music: Layer 2 trigger
  - Screen shake: 0.3 intensity, 0.5s duration

WATERING HOLE (AudioZone_WateringHole_Q3):
  - Primary: water flowing, gentle splashing
  - Secondary: large animal breathing (distant)
  - Music: peaceful, open — rare safe moment
  - Voice: Elder_Hunter_Q3.mp3 on first visit
```

### 3. Sound Effects Catalogue

| SFX Name | Trigger | Duration | Notes |
|----------|---------|----------|-------|
| `SFX_Fire_Crackle_Loop` | Camp zone enter | Loop | 3D spatialized, radius 600u |
| `SFX_Stone_Scrape` | Journal discovery | 150ms | Non-looping, single shot |
| `SFX_Ground_Rumble` | T-Rex proximity | Loop | Sub-bass, felt not heard |
| `SFX_Birds_Flee` | T-Rex alert | 2s | Sudden silence → wingbeats |
| `SFX_Water_Flow_Loop` | Watering hole zone | Loop | 3D spatialized |
| `SFX_Quest_Fanfare` | Quest success | 3s | Tribal percussion, non-looping |
| `SFX_Footstep_Dirt` | Player walk | 200ms | Per-step, 4 variants |
| `SFX_Footstep_Rock` | Player on stone | 200ms | Per-step, 4 variants |
| `SFX_Raptor_Chitter` | Raptor zone | 0.5s | Random interval 3–8s |
| `SFX_Heartbeat` | Danger zone | Loop | Syncs with music Layer 2 |

### 4. Voice Line Trigger Logic

```
ON_FIRST_ENTER(AudioZone_CampFire_Elder_001):
  → Wait 2s
  → Play Elder_Hunter_CampFire.mp3
  → Flag: CampFireVoicePlayed = true

ON_FIRST_ENTER(AudioZone_TRex_Proximity_001):
  → Play SFX_Birds_Flee immediately
  → Wait 1s
  → Play Elder_Hunter_TRex_Warning.mp3
  → Flag: TRexWarningPlayed = true

ON_INTERACT(AudioZone_Journal_Entry01..04):
  → Play SFX_Stone_Scrape
  → Wait 0.3s
  → Play corresponding journal MP3
  → Unlock journal entry in UI

ON_QUEST_COMPLETE(Q1/Q2/Q3):
  → Play SFX_Quest_Fanfare
  → Wait 1s
  → Play Elder_Hunter_Q2_Success.mp3 (Q2) or equivalent
```

### 5. MetaSounds Blueprint Architecture (UE5)

```
MetaSound_AdaptiveMusic:
  - Input: ThreatLevel (float 0.0–3.0)
  - Input: BiomeType (int 0=Savanna, 1=Forest, 2=Swamp)
  - Nodes: 4 parallel audio streams, crossfade by ThreatLevel
  - Output: Stereo mix to AudioComponent

MetaSound_AmbientZone:
  - Input: ZoneType (enum: Camp, Journal, TRex, Raptor, Water)
  - Input: PlayerDistance (float)
  - Nodes: distance-based volume attenuation + random interval triggers
  - Output: 3D spatialized audio

MetaSound_VoiceLine:
  - Input: VoiceLineID (string)
  - Input: PlayOnce (bool)
  - Nodes: SoundWave player + played-flag setter
  - Output: 2D audio (no spatialization for dialogue)
```

---

## Freesound API Status
Freesound returned 502/503 errors this cycle. Fallback executed:
- UE5 audio zone actors spawned procedurally (12 zones)
- Full audio design spec documented above
- Recommended Freesound queries for next cycle:
  - `campfire crackling wood burning` → SFX_Fire_Crackle_Loop
  - `stone scraping flint impact` → SFX_Stone_Scrape
  - `jungle night insects ambience` → ambient underscore
  - `tribal drums percussion short` → SFX_Quest_Fanfare
  - `ground rumble low frequency` → SFX_Ground_Rumble

---

## Handoff to Agent #17 — VFX Agent

### Audio-Driven VFX Requests

1. **Campfire particle system** at `AudioZone_CampFire_Elder_001` (0, -800, 120)
   - Niagara fire emitter, warm orange/amber
   - Syncs visually with `SFX_Fire_Crackle_Loop` rhythm

2. **Journal discovery sparkle** at all 4 `AudioZone_Journal_Entry` actors
   - Brief dust/particle burst on interaction (matches `SFX_Stone_Scrape` 150ms)
   - Subtle — not magical, just disturbed dust

3. **T-Rex ground impact dust** at `AudioZone_TRex_Proximity_001`
   - Ground shockwave ring expanding outward
   - Syncs with `SFX_Ground_Rumble` sub-bass pulse

4. **Raptor zone tension fog** at `AudioZone_RaptorPack_001`
   - Low ground fog, slight green tint
   - Opacity increases as player enters zone

5. **Quest success burst** at all 3 `AudioZone_QuestSuccess_Q*` actors
   - Brief golden particle burst (not magical — think dust in sunlight)
   - Duration matches `SFX_Quest_Fanfare` (3s)

6. **Screen shake system** — triggered by T-Rex proximity
   - Shake intensity: 0.3, duration: 0.5s
   - Trigger radius: 800u from `AudioZone_TRex_Proximity_001`

---

*Audio Agent #16 — PROD_CYCLE_AUTO_20260618_003 — MAP_SAVED:True*
