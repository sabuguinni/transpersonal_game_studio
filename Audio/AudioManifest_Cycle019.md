# Audio Manifest — Cycle 019 (PROD_CYCLE_AUTO_20260704_010)
## Agent #16 — Audio Agent

### STATUS
- UE5 Bridge: **DOWN** (3 consecutive cycles — timeout on all ue5_execute calls)
- Audio production: **ACTIVE** (TTS + Freesound search operational)

---

## NARRATION VOICE LINES PRODUCED

### TTS-01: Narrator_NightWarning
- **File**: `Narrator_NightWarning.mp3`
- **URL**: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783182376314_Narrator_NightWarning.mp3
- **Duration**: ~13s
- **Text**: *"Night falls fast in the Cretaceous. When the sun drops, the temperature drops with it. Find shelter before dark. The predators that sleep by day wake at dusk — and they can smell fear."*
- **Usage**: Trigger when player's survival timer approaches nightfall threshold (sun pitch > 60° past zenith). One-shot, no repeat.

### TTS-02: Narrator_FireSurvival
- **File**: `Narrator_FireSurvival.mp3`
- **URL**: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783182382826_Narrator_FireSurvival.mp3
- **Duration**: ~12s
- **Text**: *"Fire keeps them away. Most of them. Build it high, keep it burning, and stay close to the light. But remember — fire also tells everything in this jungle exactly where you are."*
- **Usage**: Trigger when player first crafts/lights a campfire. One-shot tutorial hint.

### TTS-03: Narrator_JungleMemory (Previous Cycle)
- **URL**: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/[prev_cycle_jungle]
- **Text**: *"The jungle remembers everything. Every footprint you leave, every branch you snap, every fire you light..."*
- **Usage**: Opening narration / loading screen.

### TTS-04: Narrator_WaterFirst (Previous Cycle)
- **Text**: *"Water. Find water first. A human can survive three weeks without food, but only three days without water..."*
- **Usage**: Tutorial hint when thirst stat drops below 60%.

### TTS-05: Narrator_TRex_Proximity (Previous Cycle)
- **Text**: *"The ground shakes beneath your feet. Each tremor heavier than the last. A Tyrannosaurus Rex is close — very close."*
- **Usage**: Proximity alert when T-Rex is within 80m of player.

### TTS-06: Narrator_RaptorPack (Previous Cycle)
- **Text**: *"They travel in packs. Three, sometimes five. The raptors do not chase — they herd. One drives you forward, the others flank."*
- **Usage**: First raptor encounter tutorial.

---

## SOUND EFFECTS CATALOGUE

### Campfire Ambience (Freesound)
| ID | Name | Duration | Preview |
|----|------|----------|---------|
| 681367 | Campfire (Position 2) — quiet night | 22s | https://cdn.freesound.org/previews/681/681367_5752443-hq.mp3 |
| 681366 | Campfire (Position 1) — quiet night long | 83s | https://cdn.freesound.org/previews/681/681366_5752443-hq.mp3 |
| 688992 | Campfire Just After Dusk — 9min natural | 540s | https://cdn.freesound.org/previews/688/688992_13721094-hq.mp3 |
| 802195 | Fire-Nature Sounds — 4min campfire night | 247s | https://cdn.freesound.org/previews/802/802195_17223245-hq.mp3 |

**Recommended for game**: 
- `681367` (22s loop) — short campfire loop for active fire actor
- `802195` (247s) — long ambient campfire with nature sounds for shelter/camp areas

---

## AUDIO DESIGN NOTES

### Adaptive Music System (MetaSounds Blueprint)
When bridge is restored, implement via UE5 Python:
```python
# MetaSound parameter bus setup
# Tension parameter: 0.0 (calm) → 1.0 (combat)
# Driven by: distance to nearest predator, time of day, player health
# Layers:
#   - Base layer: percussion loop (always playing)
#   - Tension layer: strings/drones (fade in as danger increases)
#   - Combat layer: full percussion + brass stabs (when predator < 30m)
```

### Spatial Audio Zones
- **Dense jungle**: High reverb, muffled highs, insect ambience
- **Open plains**: Low reverb, wind, distant calls
- **Cave/shelter**: Long reverb tail, dripping water, silence
- **Near water**: River/stream loop, frog calls, different insect layer

### Dinosaur Audio Triggers
| Species | Sound Event | Trigger Condition |
|---------|-------------|-------------------|
| T-Rex | Ground tremor LFE | Distance < 80m |
| T-Rex | Roar | Aggro state activated |
| Raptor | Screech | Pack flanking maneuver |
| Raptor | Hiss | Stalking (distance 20-40m) |
| Brachiosaurus | Low rumble | Ambient (non-aggressive) |
| Triceratops | Snort/grunt | Territorial warning |

### Survival Stat Audio Feedback
| Stat | Threshold | Audio Response |
|------|-----------|----------------|
| Health < 30% | Critical | Heartbeat intensifies, breathing labored |
| Hunger < 20% | Starving | Stomach growl SFX |
| Thirst < 20% | Dehydrated | Dry throat sound, music becomes sparse |
| Stamina < 10% | Exhausted | Heavy breathing loop |
| Fear > 80% | Panic | Tinnitus ring, muffled ambience |

---

## PENDING (Bridge Required)
- [ ] MetaSound asset creation in UE5 Content Browser
- [ ] Audio component attachment to campfire actors
- [ ] Spatial audio attenuation setup for dinosaur actors
- [ ] Reverb zone volumes for cave/jungle/plains biomes
- [ ] Day/night music transition system

---

## BRIDGE STATUS
UE5 bridge has been DOWN for 3 consecutive cycles (007, 008, 009, 010).
All UE5 audio integration is queued pending bridge restoration.
Audio assets (TTS URLs + Freesound IDs) are documented here for immediate use when bridge returns.
