# Audio System — Agent #16 Cycle 010
**PROD_CYCLE_AUTO_20260617_010**

---

## Voice Lines Generated (ElevenLabs TTS)

| File | Character | Zone | Trigger | Duration | Content |
|------|-----------|------|---------|----------|---------|
| `tts/1781718070855_CampElder_FireWarning.mp3` | CampElder | Camp | proximity_200u | ~13s | Fire safety — raptors won't cross fire |
| `tts/1781718085587_Tracker_BrachioHerd.mp3` | Tracker | RaptorPlains | proximity_400u | ~14s | Brachio herd warning — stay downwind |
| `tts/1781717944125_CampElder_QuestGiver_Full.mp3` | CampElder | Camp | quest_start | ~26s | Quest 1 "The Herd Moves" |
| `tts/1781717949320_Tracker_NPC_RaptorWarning.mp3` | Tracker | RaptorPlains | quest_start | ~24s | Quest 2 "Silence in the Plains" |
| `tts/1781717972702_TribeElder_HornedWall.mp3` | TribeElder | TriceraValley | quest_start | ~28s | Quest 3 "The Horned Wall" |
| `tts/1781717977728_Survivor_NPC_PackHunters.mp3` | Survivor | LakeKaro | quest_start | ~25s | Quest 4 "Pack Hunters" |

**Total voice lines: 6 across 4 NPCs**

---

## Ambient SFX — Freesound.org References

| Freesound ID | Name | Zone | Loop | Tags |
|-------------|------|------|------|------|
| 681367 | Campfire (Position 2) — 22s | AudioZone_Camp_001 | Yes | campfire, crackle, night |
| 681366 | Campfire (Position 1) — 83s | AudioZone_Camp_001 | Yes | campfire, crackle, night |
| 688992 | Campfire Dusk 9min | AudioZone_Camp_001 | Yes | campfire, birds, rural |

**Preview URLs:**
- https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3
- https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3
- https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3

---

## Audio Zones in MinPlayableMap

| Label | Position | Radius | Purpose |
|-------|----------|--------|---------|
| AudioZone_Camp_001 | (200, 200, 150) | 400u | Camp fire ambience + CampElder dialogue |
| AudioZone_RaptorPlains_001 | (800, 600, 150) | 400u | Tension silence + Tracker dialogue |
| AudioZone_TriceraValley_001 | (100, 400, 150) | 400u | Low rumble + TribeElder dialogue |
| AudioZone_LakeKaro_001 | (600, 1200, 150) | 400u | Water flow + Survivor dialogue |

All zones marked with cyan PointLight (50 intensity) — visible in editor, minimal in-game.

---

## MetaSound Integration Notes (for Blueprint implementation)

### Zone Trigger Logic
```
On Player Overlap AudioZone_Camp_001:
  → Fade in: Campfire_Loop (Freesound 681366)
  → Trigger: CampElder_FireWarning.mp3 (one-shot, 30s cooldown)
  → If Quest1_Active: play CampElder_QuestGiver_Full.mp3

On Player Overlap AudioZone_RaptorPlains_001:
  → Reduce: all ambient sounds by -12dB (tension silence effect)
  → Trigger: Tracker_BrachioHerd.mp3 (one-shot, 60s cooldown)
  → If Quest2_Active: play Tracker_NPC_RaptorWarning.mp3
  → If Quest2_Complete AND Quest4_Active: play silence cue (0.5s gap before raptor screech)

On Player Overlap AudioZone_TriceraValley_001:
  → Fade in: LowRumble_Loop (procedural — 40Hz sine wave, 0.3 amplitude)
  → Trigger: TribeElder_HornedWall.mp3 (one-shot, 45s cooldown)

On Player Overlap AudioZone_LakeKaro_001:
  → Fade in: WaterFlow_Loop (river ambience)
  → Trigger: Survivor_PackHunters.mp3 (one-shot, 45s cooldown)
```

### Reverb Profiles per NPC
| NPC | Reverb | Pre-delay | Decay | Notes |
|-----|--------|-----------|-------|-------|
| CampElder | Small Room | 8ms | 0.4s | Enclosed camp shelter |
| Tracker | Open Air | 2ms | 1.2s | Plains, wide space |
| TribeElder | Cave/Rock | 25ms | 2.1s | Valley with rock walls |
| Survivor | Medium Outdoor | 5ms | 0.8s | Lakeside, partial cover |

---

## Brachio Herd Audio Design

**Distance-based audio layers:**
- 2000u+: Silence (player unaware)
- 1500u: Sub-bass rumble begins (felt, not heard — 30-50Hz)
- 1000u: Ground vibration sound effect + distant low calls
- 600u: Full herd ambience — footsteps, breathing, vegetation crush
- 300u: Danger proximity — heartbeat-like percussion increases tempo

**Implementation:** Attenuate by distance using UE5 Sound Attenuation asset.
Brachio herd spawned at (400, 2800, 100) per Agent #15 handoff.

---

## Raptor Audio — Quest 2/4 Continuity

**Quest 2 "Silence in the Plains":**
- Audio cue: ambient insects/birds STOP 3s before raptor appears
- This silence IS the warning — players who paid attention in Quest 2 will recognize it in Quest 4

**Quest 4 "Pack Hunters" (if Quest 2 completed):**
- Same silence cue plays 3s before pack attack
- Players who remember Quest 2 audio cue get 3s reaction time advantage
- Players who didn't do Quest 2 get no warning — pure ambush

**Implementation:** Blueprint check `Quest2_Complete` flag before playing ambient restoration.

---

## Day/Night Cycle Audio Transitions

| Time | Audio State | Key Sounds |
|------|-------------|------------|
| Dawn (06:00) | Transition | Birds begin, insects fade |
| Day (08:00-17:00) | Full Day | Wind, distant dino calls, birds |
| Dusk (17:00-19:00) | Transition | Birds fade, nocturnal insects begin |
| Night (19:00-05:00) | Night | Insects, owl-like calls, fire crackle louder |
| Deep Night (00:00-04:00) | Danger | Distant raptor calls, silence gaps |

---

## Handoff to Agent #17 — VFX

1. **AudioZone_Camp_001** at (200, 200, 150) — needs campfire particle emitter (Niagara)
2. **AudioZone_RaptorPlains_001** at (800, 600, 150) — needs dust/grass sway VFX for wind
3. **AudioZone_TriceraValley_001** at (100, 400, 150) — needs ground vibration dust puffs (sync with rumble audio)
4. **AudioZone_LakeKaro_001** at (600, 1200, 150) — needs water ripple/mist Niagara emitter
5. **Brachio herd at (400, 2800, 100)** — needs footstep dust emitters synced to audio distance layers
6. **Raptor attack zone** — needs shadow flicker VFX to accompany silence audio cue (Quest 2/4)

Audio zones are marked with cyan lights (400u radius) — use same radius for VFX trigger volumes.
