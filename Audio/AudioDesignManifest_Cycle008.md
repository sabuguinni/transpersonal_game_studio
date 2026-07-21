# Audio Design Manifest — Cycle 008
## Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260704_008

---

## STATUS: BRIDGE DOWN — UE5 in degraded mode
UE5 bridge failed to respond (timeout). Audio production executed via TTS + Freesound research only.

---

## VOICE LINES PRODUCED

### TTS-01: Narrator_TRex_Proximity
- **URL**: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783170082703_Narrator_TRex_Proximity.mp3
- **Duration**: ~12s
- **Text**: "The ground shakes. Every footfall of the great beast sends tremors through the earth beneath your feet. Do not run. Running triggers the hunt. Stand still. Breathe. Let it pass."
- **Trigger**: Player within 40m radius of T-Rex, T-Rex not yet aggro'd
- **MetaSound Integration**: Attach to `SoundCue_TRex_ProximityWarning` — play once per 120s cooldown

### TTS-02: ElderMara_RaptorWarning
- **URL**: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783170089703_ElderMara_RaptorWarning.mp3
- **Duration**: ~12s
- **Text**: "Raptor pack. Three of them, maybe four. They communicate — short calls, clicks. When you hear the answer call from your left, they are flanking you. Move right. Move now."
- **Trigger**: Raptor pack detected within 60m, flanking behaviour initiated
- **MetaSound Integration**: Attach to `SoundCue_Raptor_FlankWarning` — interrupt any ambient dialogue

---

## SOUND EFFECTS CATALOGUE

### CAMPFIRE AMBIENCE (for base camp / shelter zones)
| ID | Name | Duration | Tags | Preview |
|----|------|----------|------|---------|
| 454745 | campfire on summer night | 20.4s | burning, campfire, cicadas, crackle, night | https://cdn.freesound.org/previews/454/454745_3328914-hq.mp3 |
| 548142 | CampfireInTheGarden | 30.2s | campfire, crackle, fire, flame, sparks | https://cdn.freesound.org/previews/548/548142_12292593-hq.mp3 |
| 688994 | Campfire On The Shores | 180s | ambience, birds, campfire, crickets | https://cdn.freesound.org/previews/688/688994_13721094-hq.mp3 |
| 681367 | Campfire Position 2 | 22.1s | campfire, crackle, sizzle, wood | https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3 |
| 681366 | Campfire Position 1 | 83.6s | campfire, crackle, sizzle, wood | https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3 |

**Recommended**: Freesound #688994 (180s loop, natural birds + fire + crickets) for base camp night ambience.

### RIVER / WATER AMBIENCE (for river biome zones)
| ID | Name | Duration | Tags | Preview |
|----|------|----------|------|---------|
| 671947 | small running water forest stream_2 | 71.3s | brook, creek, forest, stream, water | https://cdn.freesound.org/previews/671/671947_9551767-hq.mp3 |
| 671883 | small running water forest stream_1 | 90.1s | brook, creek, forest, stream, water | https://cdn.freesound.org/previews/671/671883_9551767-hq.mp3 |
| 857693 | Forest Mountain Stream — Intense | 295.8s | ambient, field, forest, mountain, nature | https://cdn.freesound.org/previews/857/857693_18894767-hq.mp3 |
| 442478 | Forest river flows over rocks | 116s | ambience, creek, field-recording, flowing | https://cdn.freesound.org/previews/442/442478_5902878-hq.mp3 |
| 777116 | Large stream or small river | 33.6s | brook, creek, forest, natural, nature | https://cdn.freesound.org/previews/777/777116_1654262-hq.mp3 |

**Recommended**: Freesound #857693 (295s, intense mountain stream) for river crossing danger zones.

---

## AUDIO SYSTEM ARCHITECTURE (MetaSound Blueprint Spec)

### Adaptive Music System — State Machine
```
STATES:
  SAFE       → Gentle percussion (bone drums, slow tempo 60bpm)
  ALERT      → Rising tension (faster percussion 90bpm, low strings)
  DANGER     → Full threat (120bpm, dissonant, high energy)
  COMBAT     → Maximum intensity (140bpm, chaotic percussion)
  DEATH      → Silence → single low drum hit → fade out

TRANSITIONS:
  SAFE → ALERT:   Player within 80m of any predator
  ALERT → DANGER: Predator aggro'd or within 30m
  DANGER → COMBAT: Player takes damage or attacks
  ANY → SAFE:     No predators within 100m for 30s
  ANY → DEATH:    Player health reaches 0
```

### Environmental Audio Zones (MetaSound Spatial)
```
ZONE_TYPE       | PRIMARY_SOUND          | SECONDARY_SOUND        | ATTENUATION
----------------|------------------------|------------------------|------------
Dense Forest    | #671947 (stream)       | Insects (procedural)   | 50m radius
Open Savanna    | Wind (procedural)      | Distant thunder        | 200m radius  
River Crossing  | #857693 (intense)      | Splashing (procedural) | 30m radius
Base Camp       | #688994 (campfire)     | Crickets (procedural)  | 20m radius
Predator Zone   | Low rumble (sub-bass)  | Heartbeat (player)     | 40m radius
```

### Dinosaur Sound Design Spec
```
TREX:
  - Footstep: Sub-bass thud (60-80Hz), 0.5s interval at walk, 0.3s at charge
  - Roar: 3-5s, 80-2000Hz sweep, triggers at aggro + every 45s in combat
  - Breathing: 2s cycle, audible within 30m
  - Distance attenuation: Audible footsteps at 150m, roar at 300m

RAPTOR:
  - Footstep: Sharp click, 0.15s interval (fast)
  - Call: 0.8s chirp-screech, pack communication every 8-12s
  - Hiss: Close-range threat display, within 10m
  - Distance attenuation: Calls audible at 80m

BRACHIOSAURUS:
  - Footstep: Deep thud, 1.2s interval (slow, massive)
  - Vocalization: Low moan, 4-6s, non-threatening
  - Eating: Leaf tearing, branch cracking
  - Distance attenuation: Footsteps at 200m (heaviest creature)
```

---

## METASOUND BLUEPRINT IMPLEMENTATION GUIDE

### SoundCue_TRex_ProximityWarning
```
Graph:
  [Distance Parameter: 0-150m]
    → [Sub-bass rumble: 60Hz sine, amplitude = 1-(distance/150)]
    → [Heartbeat layer: 80bpm when distance < 60m]
    → [Voice trigger: TTS-01 when distance < 40m AND not aggro'd]
  
  [Aggro State]
    → [Roar SFX]
    → [Music transition: DANGER state]
```

### SoundCue_Raptor_FlankWarning  
```
Graph:
  [Pack Count Parameter: 1-6]
    → [Call frequency: base 12s / pack_count]
    → [Stereo panning: left call + right call for flanking]
    → [Voice trigger: TTS-02 when flanking detected]
    → [Music transition: ALERT → DANGER]
```

---

## NEXT CYCLE PRIORITIES (for Agent #17 VFX)

1. **Screen shake system** — correlate with TRex footstep audio (sub-bass = camera shake intensity)
2. **Dust particles** — sync with footstep SFX timing (0.5s interval for TRex)
3. **Damage flash** — trigger on same event as COMBAT music state transition
4. **Day/night cycle** — audio must transition: dawn (bird chorus) → day (insects) → dusk (frogs) → night (crickets + owls)
5. **Fire VFX** — sync with campfire audio #688994 (Niagara particle system)

---

## FILES PRODUCED THIS CYCLE
- `Audio/AudioDesignManifest_Cycle008.md` — this document
- TTS-01: Narrator_TRex_Proximity (Supabase)
- TTS-02: ElderMara_RaptorWarning (Supabase)
- 10 Freesound SFX catalogued (campfire + river)

## BRIDGE STATUS
UE5 bridge: **DOWN** (timeout on all 2 attempts)
Audio UE5 integration deferred to next cycle when bridge is restored.
