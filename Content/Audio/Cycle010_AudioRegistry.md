# Audio Asset Registry — PROD_CYCLE_AUTO_20260618_010
**Agent #16 — Audio Agent**
**Date:** 2026-06-18

---

## Voice Lines Generated (TTS — ElevenLabs)

| ID | Character | Duration | Supabase URL | Trigger Zone |
|----|-----------|----------|--------------|--------------|
| 1 | Narrator_Survival | ~13s | `tts/1781806358571_Narrator_Survival.mp3` | Predator proximity (T-Rex approach) |
| 2 | Elder_Mara_Firecamp | ~12s | `tts/1781806365927_Elder_Mara_Firecamp.mp3` | BaseCamp campfire zone |

### Narrator_Survival — Full Text
> "Danger. Something large moves through the eastern marshes. The ground trembles — low, rhythmic. A predator. Stay still. Do not run. Running triggers the chase. Breathe slow. Wait for it to pass."

**Use case:** Triggered when T-Rex enters proximity radius (~800 units) of player. One-shot per session. Plays over ambient audio (duck -12dB during voice).

### Elder_Mara_Firecamp — Full Text
> "Fire means safety tonight. Keep it burning. The large predators avoid the light — they hunt by movement and heat, not flame. Gather wood before dark. Never let it die."

**Use case:** Triggered when player first enters `Ambient_BaseCamp_Campfire` zone. Tutorial hint for campfire survival mechanic.

---

## Freesound Assets Catalogued

### Campfire SFX (for BaseCamp zone)
| Freesound ID | Name | Duration | Preview URL |
|---|---|---|---|
| 729396 | Campfire 02 | 267s | https://cdn.freesound.org/previews/729/729396_12863902-hq.mp3 |
| 729395 | Campfire 01 | 109s | https://cdn.freesound.org/previews/729/729395_12863902-hq.mp3 |
| 852107 | Fireplace (loop) | 8.5s | https://cdn.freesound.org/previews/852/852107_18387771-hq.mp3 |
| 856943 | Campfire + Forest Birds | 60s | https://cdn.freesound.org/previews/856/856943_12846320-hq.mp3 |

**Recommended:** ID 852107 (8.5s loop, clean crackling) for `Ambient_BaseCamp_Campfire` zone.

### River Ambience SFX (for RiverCrossing zone)
| Freesound ID | Name | Duration | Preview URL |
|---|---|---|---|
| 446019 | Gentle Stream | 63s | https://cdn.freesound.org/previews/446/446019_7241289-hq.mp3 |
| 442478 | Forest River Over Rocks | 116s | https://cdn.freesound.org/previews/442/442478_5902878-hq.mp3 |
| 433589 | Stream River Up Close | 68s | https://cdn.freesound.org/previews/433/433589_5618682-hq.mp3 |

**Recommended:** ID 442478 (forest river + rocks, 116s, natural loop point) for `Ambient_RiverCrossing_Loop` zone.

---

## Audio Trigger Zones Spawned in MinPlayableMap

| Actor Label | Location | Scale | Purpose |
|---|---|---|---|
| `Ambient_RaptorRidge_Loop` | (2800, 1200, 150) | 8×8×4 | Raptor territory ambient — pairs with `DialogueTrigger_ScoutElder_RaptorRidge` |
| `Ambient_RiverCrossing_Loop` | (500, -2800, 120) | 10×5×3 | River water ambience — pairs with `DialogueTrigger_RiverCrossing_Warning` |
| `Ambient_CacheZone_Wind` | (-1500, 3200, 150) | 6×6×3 | Wind/isolation ambient — pairs with `DialogueTrigger_Tracker_CacheFind` |
| `Ambient_BaseCamp_Campfire` | (0, 0, 120) | 5×5×3 | Campfire crackling loop — player base area |

---

## Audio Design Spec — Ambient Layering System

### Zone Audio Stack (per zone, 3 layers)
```
Layer 1 — BASE (always on, -18dB):
  Global: wind, insects, distant bird calls
  
Layer 2 — ZONE (on zone enter, fade in 2s, -12dB):
  RaptorRidge: raptor distant calls, rustling undergrowth
  RiverCrossing: water flow, splashing, frogs
  CacheZone: wind through rocks, silence (tension)
  BaseCamp: campfire crackling, wood pop

Layer 3 — EVENT (triggered, -6dB, ducks layers 1+2):
  Dialogue voice lines (from Agent #15 TTS)
  Predator proximity growls
  Combat stings
```

### Attenuation Settings (3D Spatial Audio)
```
Type: Sphere attenuation
Inner radius: 200 units (full volume)
Outer radius: 800 units (fade to 0)
Falloff: Logarithmic
Spatialization: HRTF enabled
Occlusion: enabled (walls block sound)
```

### MetaSound Blueprint Spec (for UE5 implementation)
```
Input: bZoneActive (bool)
Input: fDangerLevel (float 0.0–1.0)
Output: AudioOutput (stereo)

Graph:
  BaseLoop → LowPassFilter(cutoff=fDangerLevel*8000+200) → Volume(1.0)
  ZoneLoop → Envelope(attack=2s, release=3s) → Volume(0.8)
  EventOneShot → Trigger(bZoneActive) → Volume(1.0)
  Mix3 → Compressor → AudioOutput
```

---

## Handoff to Agent #17 (VFX Agent)

### Audio-Driven VFX Triggers
The following audio zones should drive visual effects:

1. **`Ambient_RaptorRidge_Loop`** → Niagara: dust particles + rustling grass when raptor calls play
2. **`Ambient_RiverCrossing_Loop`** → Niagara: water splash particles + mist at river surface
3. **`Ambient_BaseCamp_Campfire`** → Niagara: fire embers + smoke column (already exists from prior cycles — verify label `Campfire_BaseCamp` or similar)
4. **Predator proximity (T-Rex)** → Screen shake + low-frequency rumble particle on ground

### Audio Reactive Materials
- Ground material near campfire: warm orange tint when `bCampfireActive = true`
- Sky material: desaturate + blue shift when `fDangerLevel > 0.7`

---

## Cumulative Audio Asset Count (Cycles 001–010)

| Category | Count | Notes |
|---|---|---|
| Voice lines (TTS) | ~20 | Across all cycles |
| Ambient zones in map | ~12 | TriggerBox actors |
| Freesound assets catalogued | ~40 | Preview URLs logged |
| MetaSound specs written | 3 | Campfire, Raptor, River |
| Audio trigger zones (this cycle) | 4 | Spawned + MAP_SAVED |

---

*Generated by Audio Agent #16 — PROD_CYCLE_AUTO_20260618_010*
