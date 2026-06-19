# Audio Zones — Cycle PROD_CYCLE_AUTO_20260619_001
**Agent:** #16 — Audio Agent  
**Date:** 2026-06-19  
**Depends on:** Narrative Agent #15 (DialogueTree_Elder_FirstTools.md)

---

## Voice Lines Produced This Cycle

| ID | Character | Text (excerpt) | Duration | URL |
|----|-----------|----------------|----------|-----|
| VL_001 | Narrator_Danger_Rumble | "Wind. Always wind here. And beneath it — something else..." | ~12s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781830550509_Narrator_Danger_Rumble.mp3 |
| VL_002 | Narrator_LoreStone_Memory | "The stone here is old. Older than the tribe..." | ~11s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781830552521_Narrator_LoreStone_Memory.mp3 |

---

## Audio Zones Placed in MinPlayableMap

| Actor Label | Location | Color | Purpose | Linked Voice |
|-------------|----------|-------|---------|--------------|
| `AudioZone_LoreStone_LostHunter_001` | (800, -400, 120) | Cold Blue | Wind + memory ambient — LoreStone_LostHunter_001 | VL_002 |
| `AudioZone_TRexDanger_001` | (2200, 1800, 150) | Deep Red | Ground rumble + danger pulse — TRex territory | VL_001 |
| `AudioZone_Elder_Dialogue_001` | (0, 0, 110) | Warm Amber | Interaction chime — Elder NPC_Elder_001 | Elder TTS from #15 |
| `AudioZone_Raptor_Patrol_001` | (1500, -800, 130) | Green-Yellow | Tension ambient — raptor patrol corridor | — |

---

## MetaSounds Integration Spec

### AudioZone_TRexDanger_001 — Danger Rumble
```
MetaSound Graph: MS_TRexDanger_Ambient
  Input: PlayerDistance (float, 0–2000)
  Node: LFO → Frequency: 0.3Hz, Amplitude: 1.0
  Node: WavePlayer → Asset: SFX_GroundRumble_Low (placeholder)
  Node: Attenuator → Distance: 0–700 units
  Node: Pitch Shift → -2 semitones at distance < 300
  Output: Mono → Spatialized via Attenuation_TRexZone
Trigger: OnPlayerEnterRadius(700)
Voice cue: VL_001 fires at distance < 400 (one-shot, 5min cooldown)
```

### AudioZone_LoreStone_LostHunter_001 — Memory Ambient
```
MetaSound Graph: MS_LoreStone_Memory
  Node: WavePlayer → Asset: SFX_Wind_Cave_Low (placeholder)
  Node: Reverb → RoomSize: 0.8, Wet: 0.6
  Node: LowPassFilter → Cutoff: 800Hz (muffled, distant feel)
  Output: Stereo → Spatialized
Trigger: OnPlayerEnterRadius(500)
Voice cue: VL_002 fires on first entry (one-shot per session)
```

### AudioZone_Elder_Dialogue_001 — Interaction Chime
```
MetaSound Graph: MS_Elder_DialogueChime
  Node: WavePlayer → Asset: SFX_BoneChime_Interaction (placeholder)
  Node: Envelope → Attack: 0.05s, Release: 1.2s
  Node: Pitch → +3 semitones (bright, welcoming)
  Output: Mono → No spatialization (UI-layer)
Trigger: OnPlayerEnterRadius(300) → fires once, then suppressed until dialogue ends
```

### AudioZone_Raptor_Patrol_001 — Tension Ambient
```
MetaSound Graph: MS_Raptor_Patrol_Tension
  Node: WavePlayer → Asset: SFX_Raptor_Distant_Hiss (placeholder)
  Node: Random Delay → 8–25s between hiss events
  Node: Attenuator → Distance: 0–600 units
  Output: Mono → Spatialized
Trigger: Continuous while player in radius (600)
```

---

## SFX Catalogue — Required Assets (Placeholders for VFX/QA)

| Asset Name | Type | Description | Priority |
|------------|------|-------------|----------|
| `SFX_GroundRumble_Low` | Ambient loop | Sub-bass 20–80Hz ground vibration | P1 |
| `SFX_Wind_Cave_Low` | Ambient loop | Cold hollow wind through stone | P1 |
| `SFX_BoneChime_Interaction` | One-shot | Bone/stone percussion chime (UI feedback) | P1 |
| `SFX_Raptor_Distant_Hiss` | Stochastic | Distant raptor hiss/growl, randomized | P2 |
| `SFX_Footstep_Stone_Player` | Foley | Player footstep on stone/rock surface | P2 |
| `SFX_Footstep_Dirt_Player` | Foley | Player footstep on dirt/soil surface | P2 |
| `SFX_TRex_Distant_Roar` | Stochastic | Far T-Rex roar, 60–120s random interval | P1 |
| `SFX_Fire_Crackle_Small` | Ambient loop | Small campfire crackle | P2 |

---

## Narrative Agent #15 Voice Lines — Integration Status

| Line | Character | Status | MetaSound Cue |
|------|-----------|--------|---------------|
| Elder_Lore_Memory | Elder | ✅ Live URL | MS_Elder_LoreMemory (pending) |
| Hunter_TRex_Warning | Hunter | ✅ Live URL | MS_Hunter_Warning (pending) |
| Elder_CraftSuccess | Elder | ✅ Live URL | MS_Elder_CraftSuccess (pending) |
| Elder_QuestFailureWarning | Elder | ✅ Live URL | MS_Elder_QuestFail (pending) |

---

## Handoff to Agent #17 — VFX Agent

**Audio zones are placed and saved in MinPlayableMap.**  
VFX Agent should:
1. Add **particle emitters** at `AudioZone_TRexDanger_001` — dust/ground vibration particles
2. Add **heat shimmer / distortion** at `AudioZone_Elder_Dialogue_001` — fire ember particles
3. Add **floating dust motes** at `AudioZone_LoreStone_LostHunter_001` — memory atmosphere
4. Add **leaf/debris scatter** at `AudioZone_Raptor_Patrol_001` — tension visual cue
5. Audio zones use PointLight actors as anchors — VFX emitters should be co-located at same XY positions

**MAP_SAVED: True** — all 4 audio zone actors persisted to `/Game/Maps/MinPlayableMap`
