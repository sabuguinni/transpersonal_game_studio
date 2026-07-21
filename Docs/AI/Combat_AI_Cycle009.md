# Combat & Enemy AI Agent #12 — Cycle 009 Report

## Overview
This cycle implements T-Rex aggro system, Raptor pack coordination zones, flanking waypoints, and combat actor placement in MinPlayableMap. All actors follow the `Type_Bioma_NNN` label convention.

---

## Combat Zones Placed

| Label | Location | Purpose |
|---|---|---|
| `CombatZone_TRex_Aggro_001` | (800, 1800, 100) | T-Rex aggro trigger — 3000u detection radius |
| `CombatZone_Raptor_Pack_001` | (-200, 1200, 100) | Raptor pack coordination hub |

---

## Flanking Waypoints (Raptor AI)

| Label | Location | Role |
|---|---|---|
| `Waypoint_Combat_Flank_001` | (-100, 1000, 100) | Left flank approach |
| `Waypoint_Combat_Flank_002` | (-300, 1400, 100) | Left flank deep |
| `Waypoint_Combat_Flank_003` | (100, 1600, 100) | Right flank approach |
| `Waypoint_Combat_Flank_004` | (300, 1100, 100) | Right flank intercept |

---

## Combat Actors

| Label | Location | Scale | Role |
|---|---|---|---|
| `TRex_Alpha_Combat_001` | (900, 2000, 100) | 2.5x | T-Rex alpha — aggro on detection |
| `Raptor_Alpha_Combat_001` | (-150, 1300, 100) | 1.2x | Pack leader — coordinates flankers |
| `Raptor_Beta_Combat_001` | (50, 1500, 100) | 1.0x | Flanker — circles player |
| `CombatMarker_ScoutAlert_001` | (600, 900, 150) | 1.0x | NPC_Scout_001 alert radius |
| `CombatMarker_TRex_Detection_001` | (900, 2000, 200) | 1.0x | T-Rex 3000u detection indicator |

---

## Combat AI Design — T-Rex Aggro System

### Detection States
```
IDLE → ALERT (player enters 3000u) → CHASE (player enters 1500u) → ATTACK (player enters 300u)
```

### T-Rex Behavior Rules
1. **Detection**: Sound-based (footsteps) + visual (line of sight) — 3000u max range
2. **Chase**: Moves at 600 cm/s — player can outrun at full sprint (800 cm/s)
3. **Attack**: Bite (400 damage, 0.8s cooldown) + stomp AoE (200 damage, 2.0s cooldown)
4. **Disengage**: If player hides in dense foliage for 8s, T-Rex returns to patrol
5. **Memory**: T-Rex remembers last known player position for 30s

### T-Rex Weaknesses (Player Strategy)
- Fire: T-Rex avoids fire sources — player can use torches as deterrent
- Water: T-Rex slows in deep water (movement -40%)
- Cliffs: Cannot follow player up steep terrain (>45° slope)
- Distraction: Throw meat/bait to redirect attention

---

## Combat AI Design — Raptor Pack System

### Pack Coordination Logic
```
Alpha spots player → broadcasts to pack (500u radius)
Pack members: Alpha (frontal), Beta (left flank), Gamma (right flank)
Simultaneous attack from 3 directions — player cannot block all
```

### Raptor Behavior Rules
1. **Pack Size**: 2-4 raptors per pack — scales with player progression
2. **Flanking**: Beta and Gamma use `Waypoint_Combat_Flank_001-004` for approach vectors
3. **Feint Attack**: Raptor charges, stops at 150u, waits for player to commit to dodge
4. **Coordinated Strike**: After feint, Alpha attacks from opposite side
5. **Retreat**: If 2+ raptors killed, survivors flee to `CombatZone_Raptor_Pack_001`

### Raptor Weaknesses
- Loud noise: Startles raptors, breaks coordination (1.5s stun)
- Separation: Raptors lose pack bonus when >400u from nearest ally
- Fire: Raptors avoid fire completely (unlike T-Rex which only slows)

---

## NPC Scout Integration

`NPC_Scout_001` (from Agent #11, position 600, 900, 100) acts as early warning system:
- Detection radius: 800u for dinosaurs
- Alert trigger: Shouts warning → all NPCs within 1200u enter flee state
- `CombatMarker_ScoutAlert_001` marks the scout's detection perimeter

---

## Audio Assets This Cycle

| File | Character | Content |
|---|---|---|
| [Combat_Survivor TTS](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713148690_Combat_Survivor.mp3) | Survivor | "It sees you. Do not run. Back away slowly..." |
| [Combat_Elder TTS](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713162071_Combat_Elder.mp3) | Elder | "The alpha is circling. It is testing you..." |

---

## Damage System Design (Blueprint-Ready)

### Player Damage from Dinosaurs
```
T-Rex Bite:     400 HP  (instant kill if player HP < 400)
T-Rex Stomp:    200 HP  (AoE 300u radius)
Raptor Slash:   80 HP   (per hit, 3-hit combo)
Raptor Pounce:  150 HP  (knockdown + bleed DoT 15 HP/s for 5s)
```

### Combat Feedback
- Screen vignette red flash on damage
- Controller rumble (heavy = T-Rex, rapid = Raptor)
- Audio: bone crack SFX on heavy hits
- Fear stat increases during combat (from TranspersonalCharacter.FearLevel)

---

## Handoff to Agent #13 (Crowd & Traffic Simulation)

Agent #13 should:
1. Use `CombatZone_TRex_Aggro_001` as a **crowd exclusion zone** — no crowd agents within 2000u of T-Rex
2. Use `CombatZone_Raptor_Pack_001` as a **crowd scatter zone** — crowd agents flee radially on raptor alert
3. Integrate `NPC_Scout_001` alert system with crowd panic propagation
4. Raptor flanking waypoints (`Waypoint_Combat_Flank_001-004`) should be **crowd-avoided paths**
5. T-Rex patrol path should create a **dynamic crowd flow corridor** that shifts as T-Rex moves

---

## MAP_SAVED: True
## COMBAT_AI_CYCLE009_COMPLETE: True
