# Audio Registry — Cycle 005
**Agent:** #16 Audio Agent  
**Cycle:** PROD_CYCLE_AUTO_20260703_004  
**World Hub:** X=2100, Y=2400 (Cretaceous forest clearing)

---

## VOICE LINES (Cycle 005)

### TTS-1: ElderMara_CampfireWarning (~19s)
- **File:** `1783056040848_ElderMara_CampfireWarning.mp3`
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783056040848_ElderMara_CampfireWarning.mp3
- **Trigger:** `Audio_Campfire_Hub_001` zone at (2100, 2400, 90)
- **Event:** `EVT_Campfire_PlayerRest` — plays when player sits at campfire at night
- **Design:** Practical survival wisdom — fire behavior as predator proximity indicator. Ends with "The fire will return at dawn" — psychological anchor.

### TTS-2: TrackerDak_StormApproach (~20s)
- **File:** `1783056049075_TrackerDak_StormApproach.mp3`
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783056049075_TrackerDak_StormApproach.mp3
- **Trigger:** `Audio_Ambient_Storm_001` zone at (1800, 2800, 120)
- **Event:** `EVT_Storm_Incoming` — plays when storm weather flag activates
- **Design:** Pterosaur behavior as storm predictor. Key survival mechanic: large predators hunt during storm noise cover. Motivates player to seek shelter.

---

## AMBIENT SOUND EFFECTS (Freesound)

### SFX-1: Campfire Hub Loop
- **Freesound ID:** 856943
- **Name:** FIREBurn_Campfire Forest Birds 01_KVV AUDIO_FREE
- **Duration:** 60s (loopable)
- **Preview:** https://cdn.freesound.org/previews/856/856943_12846320-hq.mp3
- **Zone:** `Audio_Campfire_Hub_001` at (2100, 2400)
- **Usage:** Primary campfire ambience — crackling fire + background forest birds. Loop continuously at hub.

### SFX-2: Campfire Extended Loop
- **Freesound ID:** 729395
- **Name:** Campfire 01
- **Duration:** 109s (loopable)
- **Preview:** https://cdn.freesound.org/previews/729/729395_12863902-hq.mp3
- **Zone:** `Audio_Campfire_Hub_001` at (2100, 2400)
- **Usage:** Secondary campfire layer — deeper roar. Crossfade with SFX-1 for variation.

### SFX-3: Forest Rainstorm Loop
- **Freesound ID:** 648474
- **Name:** Forest Rainstorm 01
- **Duration:** 697s (loopable)
- **Preview:** https://cdn.freesound.org/previews/648/648474_2968542-hq.mp3
- **Zone:** `Audio_Ambient_Storm_001` at (1800, 2800)
- **Usage:** Storm approach ambience — wind + rain in trees. Fade in as storm_flag activates.

### SFX-4: Forest Rainstorm Alt
- **Freesound ID:** 648475
- **Name:** Forest Rainstorm 02
- **Duration:** 719s (loopable)
- **Preview:** https://cdn.freesound.org/previews/648/648475_2968542-hq.mp3
- **Zone:** `Audio_Ambient_Storm_001` at (1800, 2800)
- **Usage:** Storm intensity layer 2 — alternate perspective, crossfade for storm escalation.

---

## UE5 AUDIO ZONE MARKERS (In-World Actors)

| Label | Location | Color | Radius | Represents |
|-------|----------|-------|--------|------------|
| `Audio_Campfire_Hub_001` | (2100, 2400, 90) | Warm orange | 600 | Campfire crackling loop + ElderMara voice |
| `Audio_Ambient_TRexZone_001` | (2400, 1600, 100) | Deep red | 1200 | Low-frequency TRex rumble loop |
| `Audio_Ambient_Storm_001` | (1800, 2800, 120) | Cool blue | 1500 | Wind/thunder storm buildup |
| `Audio_Ambient_Forest_001` | (1600, 2200, 80) | Soft green | 2000 | Jungle insects/birds baseline |

---

## CUMULATIVE VOICE LINE REGISTRY (Cycles 001–005)

| Cycle | File | Character | Trigger Event | World Location |
|-------|------|-----------|---------------|----------------|
| 001 | `*_Survivor_Journal_Day7.mp3` | Survivor Journal | `EVT_DayStart_7` | Hub (2100, 2400) |
| 001 | `*_ProximityAlert_TRex.mp3` | AI Alert System | `EVT_TRex_ProximityEnter` | TRex zone (2400, 1600) |
| 002 | `*_Survivor_Journal_Tension.mp3` | Survivor Journal | `EVT_TRex_ProximityEnter` | TRex zone (2400, 1600) |
| 003 | `*_Tracker_Dak_HerdWarning.mp3` | Tracker Dak | `EVT_Sauropod_HerdDetected` | Herd area (1800, 2800) |
| 004 | `*_ElderMara_HerdReading.mp3` | Elder Mara | `NarrTrigger_HerdObservation_001` | (1800, 2800) |
| 004 | `*_TrackerDak_TRexStealth.mp3` | Tracker Dak | `NarrTrigger_TRexStealthZone_001` | (2400, 1600) |
| 005 | `1783056040848_ElderMara_CampfireWarning.mp3` | Elder Mara | `EVT_Campfire_PlayerRest` | Hub (2100, 2400) |
| 005 | `1783056049075_TrackerDak_StormApproach.mp3` | Tracker Dak | `EVT_Storm_Incoming` | Storm zone (1800, 2800) |

---

## AUDIO DESIGN PRINCIPLES (This Project)

1. **Silence as signal** — The absence of insects/birds = predator proximity. Never fill silence with music.
2. **Layered ambience** — Base layer (insects) + mid layer (wind/water) + event layer (predator sounds).
3. **Voice lines are survival tools** — Every spoken line contains actionable information, not atmosphere.
4. **Storm = danger escalation** — Weather changes trigger audio zone crossfades, not just visual changes.
5. **Campfire = safety anchor** — Hub audio is warm, consistent, predictable. Psychological safe zone.

---

## HANDOFF TO AGENT #17 (VFX Agent)

### Audio-VFX Sync Points
1. **Campfire at (2100, 2400)** — `Audio_Campfire_Hub_001` orange PointLight needs Niagara fire particle system. VFX should pulse in sync with crackling audio envelope.
2. **TRex zone at (2400, 1600)** — `Audio_Ambient_TRexZone_001` red light needs ground dust/vibration Niagara effect. Particle intensity should scale with TRex proximity.
3. **Storm zone at (1800, 2800)** — `Audio_Ambient_Storm_001` blue light needs rain particle system + wind-blown leaf debris. Activate when `storm_flag` = true.
4. **Forest ambient at (1600, 2200)** — `Audio_Ambient_Forest_001` green light needs floating pollen/spore particles for visual depth.

### Priority VFX Request
- **Campfire Niagara system** at hub (2100, 2400) is highest priority — it's the hero screenshot anchor point per `hugo_hub_quality_v2_fix` memory. A visible, animated fire at the hub dramatically improves the Cretaceous forest composition.
