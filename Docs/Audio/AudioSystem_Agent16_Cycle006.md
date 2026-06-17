# Audio Agent #16 — Cycle 006 Audio System Documentation

## Voice Lines Generated (ElevenLabs TTS)

| ID | Character | Line | URL | Duration |
|----|-----------|------|-----|----------|
| VL-001 | Scout_TRexApproach | "Stay low. That trembling in the ground means something massive is close. If you hear the trees crack — do not wait. Run." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698314926_Scout_TRexApproach.mp3 | ~8s |
| VL-002 | Hunter_NestRaid | "The nest is unguarded. Move fast, take what you need, and be gone before the mother returns. You have maybe two minutes. Do not waste them." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698331491_Hunter_NestRaid.mp3 | ~10s |

### Previous Cycle Voice Lines (still active)
- Scout_DangerZone: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781693935338_Scout_DangerZone.mp3
- TribalElder_Stampede: (Cycle 005)
- TribalElder_Stealth: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698230225_TribalElder_Stealth.mp3
- Scout_NestDiscovery: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698241124_Scout_NestDiscovery.mp3
- Hunter_RiverBriefing: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781698254038_Hunter_RiverBriefing.mp3

---

## Audio Zone Markers (UE5 PointLight placeholders)

| Label | Location | Biome | Ambient Profile |
|-------|----------|-------|-----------------|
| AudioZone_Camp_001 | (200, 100, 60) | Tribal Camp | Fire crackling, low voices, stone tools |
| AudioZone_River_001 | (3000, 1500, 40) | River Bend | Flowing water, frogs, wind through reeds |
| AudioZone_Forest_001 | (-800, 2200, 80) | Dense Forest | Insects, birds, leaf rustling |
| AudioZone_Plains_001 | (5000, 0, 50) | Open Savanna | Wind, distant herd rumble |
| AudioZone_Nest_001 | (4000, 3500, 90) | Nest Area | Tension sting trigger, egg cracking SFX |

---

## Tension Trigger Volumes (TriggerBox actors)

| Label | Location | Scale | Purpose |
|-------|----------|-------|---------|
| TensionTrigger_Nest_001 | (4000, 3500, 90) | 5×5×3 | Nest discovery — danger music sting |
| TensionTrigger_TRex_001 | (2500, 2000, 60) | 8×8×4 | T-Rex proximity — heartbeat + low drone |
| TensionTrigger_River_001 | (3000, 1500, 40) | 6×6×3 | River crossing — ambient shift to water |

---

## Dialogue Trigger Integration (from Agent #15)

Agent #15 placed these TriggerBox actors in MinPlayableMap:
- `Dialogue_TribalElder_Camp` — connects to VL-001 (TribalElder_Stealth)
- `Dialogue_Scout_Outpost` — connects to Scout_DangerZone / Scout_TRexApproach
- `Dialogue_Hunter_River` — connects to Hunter_RiverBriefing / Hunter_NestRaid
- `Dialogue_Elder_NestWarning` — connects to Scout_NestDiscovery

**MetaSounds Integration Plan:**
Each TriggerBox should fire a MetaSound cue on `OnActorBeginOverlap`:
1. Load voice line URL as external audio source
2. Apply attenuation: `Attenuation_Dialogue` (inner 300cm, outer 800cm, falloff linear)
3. Apply reverb: outdoor = none, cave = heavy reverb, forest = medium reverb

---

## Audio Design Notes

### Tension System
- **Low danger** (no predators nearby): ambient only — insects, wind, water
- **Medium danger** (predator detected, >100m): add low-frequency drone, reduce ambient volume 20%
- **High danger** (predator <50m): heartbeat layer, music stops, only environmental sounds
- **Combat**: percussion hit — bone drum, stone strike, sharp breath

### Biome Ambient Layers
```
Camp:    fire_crackle.wav + low_wind.wav + distant_voices.wav
River:   water_flow.wav + frog_chorus.wav + reed_wind.wav
Forest:  insect_layer.wav + bird_calls.wav + leaf_rustle.wav
Plains:  open_wind.wav + herd_distant.wav + grass_movement.wav
Nest:    silence + tension_drone.wav + egg_crack_occasional.wav
```

### Voice Line Attenuation Settings
```
Inner Radius:  300 cm  (full volume)
Outer Radius:  1200 cm (fade to silence)
Falloff:       Linear
Spatialization: 3D (binaural recommended)
Reverb Send:   0.15 (subtle outdoor reverb)
```

---

## Handoff to Agent #17 (VFX)

Audio zones are placed. VFX Agent should:
1. Add **dust particle emitter** near `AudioZone_Camp_001` (fire sparks rising)
2. Add **mist/fog particle** near `AudioZone_River_001` (water surface mist)
3. Add **leaf particle system** near `AudioZone_Forest_001` (falling leaves)
4. Add **screen shake** component trigger at `TensionTrigger_TRex_001` (ground tremor)
5. Add **red vignette flash** at `TensionTrigger_Nest_001` (danger alert visual)

Audio + VFX zones are co-located — same world positions, same trigger volumes.

---

*Generated: Cycle PROD_CYCLE_AUTO_20260617_006 | Agent #16 Audio Agent*
