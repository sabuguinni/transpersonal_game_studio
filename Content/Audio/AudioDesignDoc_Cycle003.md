# Audio Design Document — Cycle AUTO_20260704_003
## Agent #16 — Audio Agent

---

## VOICE LINES PRODUCED THIS CYCLE

### [TTS-15] ElderMara_TRexEncounter (~19s)
- **Text:** *"The ground shakes before you hear it. That is how you know a Rex is close — the earth speaks first. Drop everything. Find the nearest tree with a trunk wider than your arms. Press against it. Do not run. They see movement. They smell fear. But they cannot smell stone. Be stone."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783136253406_ElderMara_TRexEncounter.mp3`
- **Trigger:** Player enters T-Rex proximity zone (radius 2000 units) for first time
- **Gameplay function:** Teaches hide-don't-run mechanic vs T-Rex
- **MetaSound cue:** `Audio/Cues/ElderMara/SC_ElderMara_TRexEncounter`

### [TTS-16] ElderMara_NightSurvival (~24s)
- **Text:** *"Night is not your enemy. Night is your shield. The large predators hunt by sight and smell — but they tire. They sleep. The small ones, the ones with feathers, those hunt by sound in the dark. So at night: no fire, no talking, no movement. Dig into the earth if you must. Become part of the ground. Dawn is six hours away. You will survive if you are patient."*
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783136264055_ElderMara_NightSurvival.mp3`
- **Trigger:** First time player survives until nightfall (sun below horizon)
- **Gameplay function:** Teaches night survival — no fire, no movement, patience mechanic
- **MetaSound cue:** `Audio/Cues/ElderMara/SC_ElderMara_NightSurvival`

---

## PREVIOUS VOICE LINES (cumulative library)

| ID | Character | Line Name | Duration | URL |
|----|-----------|-----------|----------|-----|
| TTS-11 | ElderMara | ForestSilence | ~19s | `...1783096294453_ElderMara_MovementSilence.mp3` |
| TTS-12 | ElderMara | WaterSurvival | ~19s | `...1783096294453_ElderMara_WaterSurvival.mp3` |
| TTS-13 | Narrator | NestWarning | ~15s | `...ElderMara_NestWarning.mp3` |
| TTS-14 | Narrator | GreatOneDown | ~18s | `...ElderMara_GreatOneDown.mp3` |
| TTS-15 | ElderMara | TRexEncounter | ~19s | See above |
| TTS-16 | ElderMara | NightSurvival | ~24s | See above |

---

## AUDIO ZONES PLACED IN SCENE (UE5)

### Audio_Hub_AmbientZone_001
- **Location:** X=2100, Y=2400, Z=200
- **Type:** SphereReflectionCapture (visual proxy for ambient audio zone)
- **Radius:** 5x scale = 500 unit influence radius
- **Audio content:** Jungle ambience — insects, birds, wind through canopy
- **Behaviour:** Always-on background layer at hub clearing

### Audio_TRex_ProximityTrigger_001
- **Location:** X=2100, Y=2700, Z=100
- **Type:** TriggerBox (20x20x5 scale)
- **Audio content:** Ground rumble SFX + ElderMara_TRexEncounter voice line
- **Behaviour:** One-shot trigger on first player overlap

### Audio_Campfire_Light_001
- **Location:** X=2050, Y=2380, Z=120
- **Type:** PointLight (warm orange, 3000 lux, 500 unit radius)
- **Audio content:** Fire crackling loop (MetaSound procedural)
- **Visual:** Warm orange glow — campfire presence marker at hub

---

## SOUND DESIGN PHILOSOPHY

### Adaptive Music System (MetaSounds)
The game uses a 3-layer adaptive music system:

**Layer 1 — Tension Base (always playing)**
- Slow percussion: bone on hollow log, 60-80 BPM
- Subsonic rumble: 20-40Hz, felt not heard
- Triggers: none — constant environmental presence

**Layer 2 — Danger Proximity (fades in with predator approach)**
- Faster percussion: 90-120 BPM
- High-frequency insect silence (absence of sound = danger signal)
- Triggers: predator within 1500 units

**Layer 3 — Combat/Chase (full intensity)**
- Driving percussion: 140+ BPM
- Dissonant strings/bones
- Triggers: predator aggro state, player health < 30%

### Sound Absence as Design Tool (Walter Murch principle)
- **Forest goes silent** = predator nearby (insects stop first)
- **Wind drops** = storm incoming
- **No birdsong** = player is in wrong biome (danger zone)
- **Echo changes** = player near cave/enclosed space

---

## FREESOUND QUERIES (for future integration)
Freesound API returned empty results this cycle — likely API key configuration.
Recommended manual searches:
- "prehistoric jungle ambience" → ambient bed
- "heavy footstep ground shake" → T-Rex approach
- "raptor bird screech" → Raptor attack
- "stone knapping flint" → crafting SFX
- "campfire crackling" → base camp ambience
- "jungle night insects" → night ambience layer

---

## NEXT CYCLE PRIORITIES (for Agent #17 VFX)

1. **Campfire VFX** — Niagara particle system at Audio_Campfire_Light_001 location (X=2050, Y=2380)
2. **T-Rex footstep dust** — Ground impact particles when T-Rex walks (sync with audio trigger zone)
3. **Screen shake** — Camera shake Blueprint when player enters Audio_TRex_ProximityTrigger_001
4. **Damage flash** — Red vignette post-process material when player health drops
5. **Day/night VFX** — Sky color transition particles (dust motes in sunbeams at hub clearing)

---

## TECHNICAL NOTES

### Sun Configuration (CAP-compliant)
- Pitch: -55° (bright daylight, no over-exposure)
- Yaw: 45° (dramatic side-lighting for hub clearing)
- Intensity: 10.0 lux
- Color: Warm white (1.0, 0.97, 0.88) — Cretaceous afternoon

### Audio MetaSound Blueprint Structure
```
SC_ElderMara_TRexEncounter:
  Input: bTriggerLine (bool)
  → WavePlayer (TTS-15 MP3)
  → OutputMixer
  → AudioComponent on Player

SC_Ambient_HubClearing:
  Input: fPlayerProximity (float 0-1)
  → WavePlayer_Insects (loop)
  → WavePlayer_Birds (random interval)
  → WavePlayer_Wind (loop)
  → Modulator (fPlayerProximity → volume)
  → OutputMixer
```
