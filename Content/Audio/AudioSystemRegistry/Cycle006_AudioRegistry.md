# Audio System Registry — Cycle AUTO_20260703_006
**Agent:** #16 — Audio Agent  
**Date:** 2026-07-03  
**Cumulative TTS Lines:** 7 (cycles 001–006)  
**Ambient Zones:** 3 placed this cycle  

---

## TTS Voice Lines — Complete Library

| # | Character | Trigger Event | Duration | Audio URL |
|---|-----------|--------------|----------|-----------|
| 1 | Survivor_Journal_Tension | `EVT_TRex_ProximityEnter` | ~16s | `1783045327222_Survivor_Journal_Tension.mp3` |
| 2 | Tracker_Dak_HerdWarning | `EVT_Sauropod_HerdDetected` | ~22s | `1783050095438_Tracker_Dak_HerdWarning.mp3` |
| 3 | ElderMara_CampfireWarning | `EVT_Campfire_PlayerRest` | ~19s | `1783056040848_ElderMara_CampfireWarning.mp3` |
| 4 | TribeLeader_Kael_CampAttack | `NarrTrigger_KaelCampAttack_001` | ~17s | `1783062271043_TribeLeader_Kael.mp3` |
| 5 | Scout_Rhen_RiverWarning | `NarrTrigger_RhenRiverWarning_001` | ~15s | `1783062273901_Scout_Rhen.mp3` |
| 6 | Tracker_Dak_ForestReading | `EVT_ForestSilence_Detected` | ~19s | `1783062392502_Tracker_Dak_ForestReading.mp3` |
| 7 | ElderMara_NightSurvival | `EVT_NightCycle_Begin` | ~24s | `1783062403042_ElderMara_NightSurvival.mp3` |

All URLs base: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/`

---

## New Voice Lines This Cycle

### [TTS-6] Tracker_Dak — "Forest Reading" (~19s)
**Full Script:**
> "The forest has its own heartbeat. Learn it. When the insects stop singing, something is moving through the undergrowth. When the birds scatter from the canopy all at once — you have maybe thirty seconds before it reaches you. The land speaks. You just have to know how to listen."

**Design Notes:**
- Teaches player the **ambient audio silence mechanic** — insect/bird sounds stopping = predator nearby
- Trigger: `EVT_ForestSilence_Detected` — fires when ambient insect layer drops below threshold
- Reinforces Freesound #435152 (predator creatures) as the audio cue that precedes this silence
- Dak's voice: calm, experienced tracker — not panicked, matter-of-fact

### [TTS-7] ElderMara — "Night Survival" (~24s)
**Full Script:**
> "Night is the most dangerous time. The big predators move when the temperature drops — they are cold-blooded, yes, but they have learned that prey sleeps at night. Keep your fire burning. Not for warmth. For the light. They will not cross into firelight. Not the large ones. The small ones — the raptors — they are smarter. They wait for the fire to die."

**Design Notes:**
- Trigger: `EVT_NightCycle_Begin` — fires at first night transition
- Establishes **fire maintenance as survival mechanic** (reinforces Kael's camp attack line)
- Biological accuracy: large theropods as primarily visual hunters deterred by firelight
- Raptors as smarter threat — creates tension around fire fuel management
- Mara's voice: elder authority, slightly ominous on final line

---

## Ambient Audio Zones — Placed This Cycle

### Zone 1: AudioZone_Campfire_Hub_001
- **Location:** (2100, 2400, 80) — hub clearing center
- **Radius:** 800 units
- **Source:** Freesound #681366 — "Campfire (Position 1)" — 83s, 24bit/48kHz, Sony PCM-D100
- **Preview:** https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3
- **Loop:** Yes — persistent ambient
- **Attenuation:** Linear falloff from 200u (full) to 800u (silent)
- **MetaSound Integration:** `MS_Campfire_Hub` — 3-layer system:
  - Layer A: Base crackle loop (continuous)
  - Layer B: Occasional pop/spark (random interval 2–8s)
  - Layer C: Wind interaction (modulated by weather system)

### Zone 2: AudioZone_Raptor_FernZone_001
- **Location:** (2100, 1200, 80) — fern zone south of hub
- **Radius:** 1500 units
- **Source:** Freesound #435152 — "Predator creatures" — 55s composite
- **Preview:** https://cdn.freesound.org/previews/435/435152_6802113-hq.mp3
- **Loop:** No — triggered stochastically (random interval 15–45s)
- **Trigger:** `EVT_RaptorZone_Enter` + stochastic ambient
- **Design:** Distant calls, never close — creates unease without direct threat
- **Silence mechanic:** When raptor call plays, insect ambient layer fades to 0 for 8s

### Zone 3: AudioZone_TRex_NorthRiver_001
- **Location:** (1800, 4800, 80) — north of second river bend
- **Radius:** 2500 units
- **Source:** Freesound #278229 — "dinosaur.wav" — 13s bone-cracking/growl
- **Preview:** https://cdn.freesound.org/previews/278/278229_5351496-hq.mp3
- **Loop:** No — triggered on `EVT_TRex_ProximityEnter`
- **Attenuation:** Logarithmic — audible at 2500u, full volume at 300u
- **Sub-bass layer:** Low-frequency rumble (40–80Hz) added via MetaSound pitch shift
- **Screen shake:** Links to VFX agent screen shake system at 300u proximity

---

## Freesound Asset Registry

| # | ID | Name | Duration | Tags | Use |
|---|----|------|----------|------|-----|
| 1 | 681366 | Campfire (Position 1) | 83s | campfire, crackle, fire | Hub ambient loop |
| 2 | 681367 | Campfire (Position 2) | 22s | campfire, crackle | Backup/variation |
| 3 | 688994 | Campfire On The Shores | 180s | campfire, birds, crickets | Extended night camp |
| 4 | 435152 | Predator creatures | 55s | dinosaur, growl, roar | Raptor zone ambient |
| 5 | 278229 | dinosaur.wav | 13s | dinosaur, growl, bones | T-Rex proximity stinger |
| 6 | 338681 | crackling and popping fire | 38s | fire, crackling | Combat fire variation |

---

## Audio Architecture — MetaSound Blueprint Plan

```
MS_WorldAmbient_Master
├── Layer_Insects (base — always on during day)
│   └── Modulator: Predator_Proximity_Scalar (0.0–1.0)
│       └── 0.0 = full insect volume | 1.0 = complete silence
├── Layer_Wind (continuous — modulated by weather)
├── Layer_Campfire (spatial — radius 800u from hub)
│   └── Source: Freesound #681366
├── Layer_RaptorCalls (stochastic — fern zone)
│   └── Source: Freesound #435152
│   └── Triggers Layer_Insects.Predator_Proximity_Scalar → 1.0 for 8s
└── Layer_TRex_Rumble (proximity — north river)
    └── Source: Freesound #278229
    └── Sub-bass: -24 semitones pitch shift for ground rumble

MS_Dialogue_System
├── Queue: FIFO — max 1 line playing at a time
├── Priority: Combat > Survival > Ambient narrative
├── Lines: 7 TTS URLs (see table above)
└── Cooldown: 120s per character before repeat
```

---

## Narrative Audio Arc — Act 1 Complete

| Beat | Character | Line | Status |
|------|-----------|------|--------|
| First T-Rex sighting | Survivor Journal | Tension/silence | ✅ Recorded |
| Sauropod herd tracks | Tracker Dak | Herd warning | ✅ Recorded |
| Camp rest | Elder Mara | Fire behavior | ✅ Recorded |
| Hub first visit | Tribe Leader Kael | Camp attack | ✅ Recorded |
| River approach | Scout Rhen | River warning | ✅ Recorded |
| Forest silence event | Tracker Dak | Forest reading | ✅ Recorded |
| First night | Elder Mara | Night survival | ✅ Recorded |

**Act 1 narrative audio: COMPLETE** — 7 lines covering all major first-hour player events.

---

## Dependencies for Next Agents

### → Agent #17 (VFX Agent)
- `AudioZone_TRex_NorthRiver_001` at (1800, 4800) needs screen shake at 300u proximity
- `AudioZone_Raptor_FernZone_001` at (2100, 1200) needs dust particle effect on raptor call
- Campfire at (2100, 2400) needs fire particle system — audio zone marker is placeholder

### → Agent #18 (QA Agent)
- Validate: 3 audio zone actors exist in MinPlayableMap
- Validate: NPC actors (Kael, Rhen) have voice line URLs in metadata
- Test: Ambient silence mechanic — insect layer should drop when predator audio plays

### → Agent #19 (Integration Agent)
- Wire `EVT_ForestSilence_Detected` to insect ambient volume scalar
- Wire `EVT_NightCycle_Begin` to ElderMara_NightSurvival TTS playback
- Campfire fuel mechanic: fire audio loop should fade when fuel < 20%
