# Audio Design Document — Cycle 008
## Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260617_008

---

## VOICE LINES PRODUCED THIS CYCLE

### Scout_RaptorWarning
- **URL**: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781708174015_Scout_RaptorWarning.mp3`
- **Duration**: ~17s
- **Text**: *"Listen. That is a Raptor call. Short, sharp, three times. They are coordinating. When you hear that, they already see you. Do not run in a straight line — they are faster. Find a tree, find a rock, break their line of sight. You have maybe ten seconds."*
- **Trigger**: Player enters Raptor territory zone for first time
- **Character**: Experienced scout NPC companion

### TribalElder_WinterWarning
- **URL**: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781708187102_TribalElder_WinterWarning.mp3`
- **Duration**: ~17s
- **Text**: *"Winter is coming. I can feel it in the mud — it is getting harder each morning. The big ones will move south, following the warmth. But the small ones, the clever ones, they will stay. And they will be hungry. That is when the real danger begins."*
- **Trigger**: Seasonal transition to winter (day/night cycle milestone)
- **Character**: Tribal Elder — knowledge keeper of migration patterns

---

## SOUND EFFECTS CATALOGUE (Freesound.org)

### Campfire Sounds (Found)
| ID | Name | Duration | Tags | Preview |
|----|------|----------|------|---------|
| 157187 | Campfire Crackle 3 | 2.0s | burning, campfire, crackling, fire | [Preview](https://cdn.freesound.org/previews/157/157187_2840005-hq.mp3) |
| 394952 | crackling campfire.wav | 1805s (30min loop) | campfire, crackle, field-recording | [Preview](https://cdn.freesound.org/previews/394/394952_7037-hq.mp3) |
| 790790 | Dry Pine Needles Burning | 62s | ASMR, campfire, crackling, nature | [Preview](https://cdn.freesound.org/previews/790/790790_16993478-hq.mp3) |
| 660297 | Campfire deer camp - wet wood | 37.8s | bonfire, campfire, crackling | [Preview](https://cdn.freesound.org/previews/660/660297_13721094-hq.mp3) |
| 620007 | Wood_Firestove.wav | 307s | campfire, fireplace, crackling | [Preview](https://cdn.freesound.org/previews/620/620007_13601611-hq.mp3) |

**Recommended for game**: ID 394952 (30min seamless loop) for campfire ambient zone.

---

## UE5 ACTORS PLACED IN MinPlayableMap

### AmbientSound Actors (4 zones)
| Label | Location | Purpose |
|-------|----------|---------|
| AmbientSound_Campfire_001 | (0, 0, 120) | Base camp fire crackling loop |
| AmbientSound_River_001 | (3000, 1500, 80) | River flow ambient |
| AmbientSound_Wind_Savanna_001 | (-2000, 2000, 200) | Open savanna wind |
| AmbientSound_Forest_Night_001 | (1500, -2500, 150) | Night insect/frog chorus |

### Audio Trigger Volumes (3 zones)
| Label | Location | Scale | Music State |
|-------|----------|-------|-------------|
| AudioTrigger_TRex_Territory_001 | (4000, 3000, 100) | 15×15×5 | DANGER — heavy percussion, low drones |
| AudioTrigger_Safe_Camp_001 | (0, 0, 100) | 10×10×4 | SAFE — minimal, warm, low tension |
| AudioTrigger_River_Crossing_001 | (3000, 1500, 100) | 8×8×4 | EXPLORATION — flowing, curious |

---

## CUMULATIVE VOICE LINE LIBRARY (Cycles 005–008)

| Cycle | File | Character | Trigger |
|-------|------|-----------|---------|
| 005 | Scout_DangerZone.mp3 | Scout | Enter T-Rex river territory |
| 005 | TribalElder_Stampede.mp3 | Elder | Ground tremor detected |
| 006 | Scout_TRexApproach.mp3 | Scout | T-Rex proximity alert |
| 006 | Hunter_NestRaid.mp3 | Hunter | Near dinosaur nest |
| 007 | TribalElder_PiraWaterCall.mp3 | Elder | Herd water migration |
| 007 | TribalWarrior_Scarb... | Warrior | Combat encounter |
| 008 | Scout_RaptorWarning.mp3 | Scout | Raptor territory entry |
| 008 | TribalElder_WinterWarning.mp3 | Elder | Winter season transition |

---

## AUDIO SYSTEM ARCHITECTURE

### Music State Machine (Adaptive)
```
STATES:
  SAFE_CAMP     → minimal percussion, bone flute, low tension
  EXPLORATION   → flowing rhythms, natural instruments, curious mood
  DANGER_NEAR   → heavy drums, low drones, heartbeat pulse
  COMBAT        → intense percussion, dissonant tones, high energy
  DEATH_NEARBY  → silence + single sustained tone + heartbeat

TRANSITIONS:
  SAFE → EXPLORATION: player moves >50m from camp
  EXPLORATION → DANGER: predator within 80m radius
  DANGER → COMBAT: predator aggro triggered
  COMBAT → SAFE: predator dead or player escaped >200m
  ANY → DEATH_NEARBY: player health <20%
```

### Spatial Audio Zones
- Campfire: 800 unit radius, attenuates to silence at 2000 units
- River: 1200 unit radius (flowing water carries further)
- Wind: global ambient, no attenuation
- Forest Night: 1500 unit radius, strongest at night cycle

### Dinosaur Audio Cues (Design Spec)
| Species | Idle Sound | Alert Sound | Attack Sound |
|---------|-----------|-------------|--------------|
| T-Rex | Deep rumble every 8-12s | Roar at 150m detection | Massive impact + roar |
| Raptor | Clicking/chirping | Sharp 3-call coordination | Screech + impact |
| Brachiosaurus | Low hum | Loud honk | Stomp + ground shake |
| Carnotaurus | Guttural growl | Snort + stomp | Charge + impact |

---

## NEXT CYCLE PRIORITIES (for Agent #17 VFX)

1. **Screen shake system** — T-Rex footsteps should trigger camera shake at <100m
2. **Damage flash VFX** — Red screen overlay when player takes damage
3. **Footstep dust particles** — Dirt puff on each heavy dinosaur step
4. **Fire particle system** — Campfire visual at AmbientSound_Campfire_001 location (0,0,120)
5. **Blood/impact particles** — Combat hit feedback

Audio trigger volumes are placed — VFX agent should add particle emitters near same locations.
