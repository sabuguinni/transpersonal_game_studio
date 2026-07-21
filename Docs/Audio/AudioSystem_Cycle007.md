# Audio System — Agent #16 — PROD_CYCLE_AUTO_20260617_005

## Voice Lines Generated (ElevenLabs TTS)

### This Cycle
| Character | Line | Duration | URL |
|-----------|------|----------|-----|
| Scout_DangerZone | "The river is close... T-Rex territory. Do not stop moving." | ~13s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693935338_Scout_DangerZone.mp3 |
| TribalElder_Stampede | "That rumble under your feet — that is not thunder. The herd is moving." | ~14s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693951276_TribalElder_Stampede.mp3 |

### Previous Cycles (Cycle 006 — Agent #15 handoff)
| Character | Line | Duration | URL |
|-----------|------|----------|-----|
| TribalElder_HuntBriefing2 | "Three winters ago, I watched the T-Rex take my brother..." | ~15s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693798880_TribalElder_HuntBriefing2.mp3 |
| Scout_MigrationBriefing | "Follow the river north until you see the three dead trees..." | ~20s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693813793_Scout_MigrationBriefing.mp3 |
| CraftingNPC_Tutorial | "This stone. This edge. You chip here, not there..." | ~15s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693833844_CraftingNPC_Tutorial.mp3 |

---

## Ambient Sound Zones (MinPlayableMap actors)

### Ambient Emitter Positions
| Actor Label | Position | Sound Design Brief |
|-------------|----------|--------------------|
| Ambient_River_North_001 | (-500, 1800, 100) | River flow loop + dusk raptor calls (sparse, distant, threatening) |
| Ambient_Crafting_Zone_001 | (-200, 300, 100) | Stone chipping loop + campfire crackle (Freesound ID: 681367) |
| Ambient_TRex_Territory_001 | (1200, 2500, 100) | Sub-bass danger hum (20-60Hz) + distant T-Rex roar (rare, 3-8min interval) |
| Ambient_Stampede_Path_001 | (2000, 800, 100) | Ground rumble + Parasaur panic calls |
| Ambient_Cave_Entrance_001 | (-800, -600, 100) | Dripping water reverb + wind through stone |

### Voice Line Trigger Markers
| Actor Label | Position | Linked Voice Line |
|-------------|----------|-------------------|
| VoiceMarker_Scout_DangerZone_001 | (1200, 2000, 150) | Scout_DangerZone.mp3 |
| VoiceMarker_Elder_Stampede_001 | (2000, 800, 150) | TribalElder_Stampede.mp3 |
| VoiceMarker_Scout_Migration_001 | (-500, 1800, 150) | Scout_MigrationBriefing.mp3 |
| VoiceMarker_Elder_Hunt_001 | (0, 0, 150) | TribalElder_HuntBriefing2.mp3 |
| VoiceMarker_Crafter_Tutorial_001 | (-200, 300, 150) | CraftingNPC_Tutorial.mp3 |

---

## Stampede Audio Sequence Design

The stampede event uses a 4-stage audio narrative arc:

```
Stage 1 — Sub-bass rumble (Audio_Stampede_SubBass_001 at 1800,600)
  - 20-40Hz low frequency, felt more than heard
  - Duration: 8-12 seconds before visual cue
  - Purpose: primal dread, player senses something wrong

Stage 2 — Parasaur panic calls (Audio_Stampede_Parasaur_001 at 1900,700)
  - High-pitched distress calls, multiple overlapping
  - Signals herd in motion, direction of approach
  - Duration: overlaps with stage 1 exit

Stage 3 — T-Rex footsteps (Audio_Stampede_TRexStep_001 at 2100,900)
  - Heavy impact thuds, 2-3 second intervals
  - Ground shake camera effect trigger point
  - Duration: 15-30 seconds

Stage 4 — Sudden silence (Audio_Stampede_Silence_001 at 2200,1000)
  - All ambient sound drops to near-zero
  - 2-3 second pause before narration trigger
  - Walter Murch principle: absence of sound = maximum tension
```

---

## Freesound Assets Identified

| ID | Name | Duration | Use Case |
|----|------|----------|----------|
| 681367 | Campfire (Position 2) | 22s | Crafting zone ambient loop |
| 681366 | Campfire (Position 1) | 83s | Camp base ambient (longer loop) |
| 802195 | Fire-Nature Sounds | 247s | Night camp full ambience |

---

## Audio Design Principles (This Project)

1. **Silence as weapon** — T-Rex approach zone goes quiet before the roar
2. **Distance cues** — All dangerous sounds start at low volume, grow with proximity
3. **No music during predator encounters** — Pure diegetic sound only
4. **Percussion-only score** — Bone percussion, hollow wood, stone percussion for music
5. **Voice lines are sparse** — Characters speak once per zone entry, not on loop
6. **Sub-bass for large creatures** — Anything over 5 tonnes gets 20-60Hz rumble layer

---

## Handoff to Agent #17 (VFX)

Audio zones are positioned. VFX should:
1. Add dust particle emitter at `Ambient_Stampede_Path_001` — ground dust rising
2. Add heat shimmer at `Ambient_TRex_Territory_001` — oppressive heat visual
3. Add water spray particles at `Ambient_River_North_001`
4. Add ember particles at `Ambient_Crafting_Zone_001` — fire visual sync with audio
5. Screen shake trigger at `Audio_Stampede_TRexStep_001` — sync with footstep audio stage
