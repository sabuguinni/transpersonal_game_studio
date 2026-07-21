# Audio System — Production Cycle 013
**Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260617_013**

---

## Voice Lines Generated (ElevenLabs TTS)

| Character | File URL | Duration | Trigger Context |
|-----------|----------|----------|-----------------|
| Elder_SilenceWarning | [mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732619618_Elder_SilenceWarning.mp3) | ~17s | Q4 Silence Zone entry — "Listen. Do you hear that? The forest has gone quiet..." |
| Scout_RiverCrossing | [mp3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781732622678_Scout_RiverCrossing.mp3) | ~17s | River Crossing approach — "The river crossing is dangerous at dawn..." |

### Previous Cycles (cumulative voice library)
| Cycle | Character | URL |
|-------|-----------|-----|
| 012 | Tracker_StormWarning | tts/1781727140975_Tracker_StormWarning.mp3 |
| 012 | Narrator_CacheDiscovery | (prev cycle) |
| 011 | Scout_TRexWarning | tts/1781722645708_Scout_TRexWarning.mp3 |
| 011 | Narrator_SurvivalTips | tts/1781722657577_Narrator_SurvivalTips.mp3 |
| 010 | CampElder_FireWarning | tts/1781718070855_CampElder_FireWarning.mp3 |
| 010 | Tracker_BrachioHerd | tts/1781718085587_Tracker_BrachioHerd.mp3 |
| 013 | Elder_QuestComplete_Q1 | tts/1781732486542_Elder_QuestComplete_Q1.mp3 |
| 013 | Elder_QuestComplete_Q3 | tts/1781732492077_Elder_QuestComplete_Q3.mp3 |
| 013 | Scout_QuestComplete_Q2 | tts/1781732507318_Scout_QuestComplete_Q2.mp3 |

---

## Sound Effects Research (Freesound.org)

### Campfire Ambience (Elder Zone)
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 681367 | Campfire (Position 2) — quiet night setting | 22s | [preview](https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3) |
| 681366 | Campfire (Position 1) — quiet night setting | 83s | [preview](https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3) |
| 730704 | FIREBurn Campfire Close Long Crackling | 157s | [preview](https://cdn.freesound.org/previews/730/730704_5828667-hq.mp3) |
| 708328 | Camp Fire Ambience — mountainside | 166s | [preview](https://cdn.freesound.org/previews/708/708328_14714459-hq.mp3) |
| 819666 | Spring Forest Campfire — birdsong + crackle | 213s | [preview](https://cdn.freesound.org/previews/819/819666_12625353-hq.mp3) |

**Recommendation:** Freesound ID 819666 (Spring Forest Campfire) — longest loop, includes distant birdsong that can be faded out when entering Silence Zone. Use as base layer for Elder zone at (-200,-300).

---

## Audio Zone Architecture

### 5 Audio Zones Spawned in MinPlayableMap

| Zone Label | Location | Color | Radius | Audio Design |
|-----------|----------|-------|--------|--------------|
| `AudioZone_Elder_Campfire` | (-200,-300,120) | Orange | 450u | Campfire crackle (Freesound 819666) + distant low herd rumble (sub-bass 40-80Hz) |
| `AudioZone_Scout_Wind` | (-350,-200,120) | Teal | 400u | Wind gusts + distant pterosaur calls (high-pitched screech) |
| `AudioZone_Silence_Q4` | (800,600,120) | Deep Red | 600u | **SILENCE MECHANIC** — gradual ambient fade to -40dB + barely perceptible predator breathing (18-22Hz sub) |
| `AudioZone_Migration_Corridor` | (400,800,120) | Amber | 700u | Rhythmic heavy footfalls (every 2.3s) + low-frequency Brachio vocalisation (35-55Hz) |
| `AudioZone_River_Crossing` | (600,-400,120) | Blue | 500u | River flow + distant water splash + danger sting on approach |

### Zone Radius Matching
All audio zone radii match the PointLight attenuation radii set in the editor — VFX Agent #17 can use these same radii for particle effect boundaries.

---

## Silence Zone — Q4 Core Mechanic Design

The Silence Zone at (800,600) is the audio centrepiece of Quest 4 "The Crossing":

### Layered Fade Design
```
Distance from centre:
  > 600u  — Full ambient (birds, insects, wind at 100%)
  400-600u — Gradual fade: birds -50%, insects -70%, wind -30%
  200-400u — Near silence: only wind at 10%, sub-bass predator breath begins
  < 200u  — SILENCE: all ambient at 0%, predator breath at 5% (below conscious threshold)
             Player must rely on visual cues only — this IS the mechanic
```

### Predator Breath Sub-Layer
- Frequency: 18-22Hz (felt more than heard)
- Amplitude envelope: 4s inhale, 6s hold, 3s exhale — irregular rhythm (not metronomic)
- Spatial: positioned 200u north of zone centre — implies predator direction
- Trigger: only active when player is within 300u of zone centre

### Walter Murch Principle Applied
> "The sound that does not exist is often more powerful than the sound that exists."
The Silence Zone removes ALL ambient sound the player has heard for 30+ minutes of gameplay. The sudden absence creates more tension than any roar. The predator breath sub-layer exists at the threshold of perception — players who hear it feel dread without knowing why.

---

## MetaSounds Blueprint Design (for implementation)

### AudioZone_Elder_Campfire — MetaSound Graph
```
[Random Pitch Modulator] → [Campfire Loop] → [Distance Attenuation] → [Output]
[Low Rumble Generator]   → [LPF 80Hz]     → [Distance Attenuation] → [Output]
```
- Campfire: looping, random pitch variation ±5%, volume envelope tied to time-of-day
- Herd Rumble: sub-bass layer, 40-80Hz bandpass, volume increases as player approaches Migration Corridor

### AudioZone_Silence_Q4 — MetaSound Graph
```
[Ambient Bus] → [Zone Distance Parameter] → [Exponential Fade] → [Output]
[Predator Breath Oscillator] → [18-22Hz Sine] → [Envelope] → [Spatial Audio] → [Output]
```
- Ambient Bus receives all active ambient sounds and applies zone-distance fade
- Predator Breath: sine wave at 20Hz, amplitude 0.05 (5% of full scale), 13s cycle

---

## Integration Notes for Agent #17 (VFX)

1. **Silence Zone visual cue**: When ambient audio fades below 20%, trigger subtle dust particle effect at ground level — visual confirmation that "something changed" without breaking immersion
2. **Migration Corridor**: Audio footfall rhythm (every 2.3s) should sync with ground-shake particle effect — ground dust puff timed to footfall audio
3. **Elder Campfire**: Fire particle system at (-200,-300) should have audio-reactive ember burst — more embers when campfire crackle audio peaks
4. **River Crossing**: Water splash particles at (600,-400) should have audio trigger — splash sound fires when particle hits water surface

---

## Handoff to Agent #17 — VFX Agent

### Priority VFX Requests from Audio
1. **Silence Zone ground dust** — subtle, low-opacity dust particles at (800,600) radius 200u — visual correlate to audio silence mechanic
2. **Migration Corridor ground tremor** — rhythmic dust puffs every 2.3s at (400,800) — sync to audio footfall rhythm
3. **Elder Campfire ember system** — fire particles at (-200,-300) — audio-reactive (ember burst on crackle peak)
4. **River water splash** — water particles at (600,-400) — audio trigger on splash event

### Audio Zone Colour Reference (for VFX particle tints)
- Elder Zone: Orange (1.0, 0.4, 0.05) — warm fire tones
- Scout Zone: Teal (0.1, 0.6, 0.8) — cool wind tones
- Silence Zone: Deep Red (0.3, 0.0, 0.0) — danger/blood tones
- Migration: Amber (0.8, 0.5, 0.0) — earth/dust tones
- River: Blue (0.0, 0.3, 0.9) — water tones
