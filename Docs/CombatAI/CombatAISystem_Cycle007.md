# Combat & Enemy AI System — Agent #12 — Cycle PROD_CYCLE_AUTO_20260617_007

## Overview
This document defines the complete combat AI design for dinosaur enemies in the prehistoric survival game.
All systems are implemented as UE5 map actors (light markers for zones) and documented here for Blueprint/BT wiring.

---

## 1. Raptor Pack Combat System

### Pack Composition
| Role | Label | Position | Behavior |
|------|-------|----------|----------|
| Alpha | `CombatPos_RaptorAlpha_001` | (-800, 1200, 100) | Frontal charge — draws player attention |
| Beta | `CombatPos_RaptorBeta_001` | (-1100, 1500, 100) | Left flank — circles to player's left |
| Gamma | `CombatPos_RaptorGamma_001` | (-500, 1500, 100) | Right flank — circles to player's right |

### Trigger Volume
- **Label**: `CombatTrigger_RaptorPack_001`
- **Position**: (-800, 1000, 120)
- **Detection Radius**: 600 units
- **Trigger Condition**: Player enters radius AND is not crouching AND has not applied scent mask

### Combat State Machine (per raptor)
```
PATROL → [player detected within 600u] → HUNT_APPROACH
HUNT_APPROACH → [within 200u of player] → FLANK_EXECUTE (Beta/Gamma) OR CHARGE (Alpha)
CHARGE → [hit player OR missed] → REPOSITION
REPOSITION → [cooldown 3s] → HUNT_APPROACH
FLANK_EXECUTE → [in flank position] → LEAP_ATTACK
LEAP_ATTACK → [hit OR missed] → REPOSITION
```

### Pack Coordination Logic
- **Alpha activates first**: charges within 1.5s of detection
- **Beta activates at T+0.8s**: begins wide left arc (radius 400u)
- **Gamma activates at T+1.2s**: begins wide right arc (radius 400u)
- **Flanking complete when**: Beta and Gamma are both within 60° arc of player's sides
- **Pack retreat trigger**: Alpha health < 30% OR 2 raptors incapacitated

### Blackboard Keys (BB_Raptor)
```
TargetActor: Object — the player pawn
PatrolIndex: Int — current waypoint index (0-2, uses Waypoint_Floresta_011/012/013)
bIsHunting: Bool — pack hunt mode active
LastKnownPos: Vector — last confirmed player position
PackRole: Enum (Alpha/Beta/Gamma) — determines behavior branch
bFlankComplete: Bool — has reached flank position
CooldownTimer: Float — seconds until next attack attempt
```

### Behavior Tree Structure (BT_Raptor_Pack)
```
Root (Selector)
├── Sequence [bIsHunting == true]
│   ├── BTTask_MoveToFlankPosition (Beta/Gamma only)
│   ├── BTTask_WaitForAlphaCharge
│   └── BTTask_LeapAttack
├── Sequence [TargetActor != null]
│   ├── BTTask_MoveToTarget (Alpha)
│   └── BTTask_ChargeAttack
└── Sequence [Patrol]
    ├── BTTask_MoveToWaypoint
    └── BTTask_WaitAtWaypoint (3-5s random)
```

---

## 2. T-Rex Ambush System

### Ambush Markers
| Label | Position | Purpose |
|-------|----------|---------|
| `Ambush_TRex_Origin_001` | (1800, 500, 100) | T-Rex spawn/lurk position |
| `Ambush_TRex_DetectCone_001` | (1400, 600, 100) | Player detection trigger (1200u radius) |
| `Retreat_Corridor_TRex_001` | (1200, 200, 100) | Safe narrow gap — T-Rex cannot follow |

### T-Rex Combat Behavior
```
LURK (at Origin) → [player enters DetectCone] → ROAR (1.5s stagger) → CHARGE
CHARGE (speed: 1400 u/s) → [player in retreat corridor] → BLOCKED (T-Rex stops)
CHARGE → [player in open] → STOMP_ATTACK
STOMP_ATTACK → [hit] → FEED (player incapacitated)
STOMP_ATTACK → [miss] → SCAN (T-Rex looks around 8s)
SCAN → [player visible] → CHARGE
SCAN → [player hidden] → PATROL_RETURN
```

### T-Rex Combat Parameters
- **Detection radius**: 1200 units (visual cone 120°)
- **Hearing radius**: 800 units (triggered by running footsteps)
- **Charge speed**: 1400 units/second
- **Stomp damage**: 85 HP (instant kill if player < 20 HP)
- **Roar stagger**: 1.5s player movement penalty
- **Retreat corridor width**: 200 units (T-Rex body width: 350 units — cannot follow)

### Ambush Trigger Conditions
1. Player enters `Ambush_TRex_DetectCone_001` radius (1200u)
2. Player is running (footstep audio triggers hearing)
3. Time of day: Dawn or Dusk (T-Rex most active)
4. Player has NOT used scent-masking item

---

## 3. Ankylosaurus Defensive Combat

### Combat Zones
| Label | Position | Radius | Purpose |
|-------|----------|--------|---------|
| `CombatZone_Ankylo_Defensive_001` | (-200, -1500, 100) | 500u | Full engagement zone |
| `DangerZone_Ankylo_TailSwipe_001` | (-350, -1500, 100) | 350u | Rear 180° tail danger |

### Ankylosaurus Combat Behavior
```
GRAZE (passive) → [player within 300u] → WARNING_DISPLAY (stomp + low rumble)
WARNING_DISPLAY → [player retreats > 400u] → GRAZE (return to passive)
WARNING_DISPLAY → [player stays < 300u for 3s] → CHARGE_HEADBUTT
CHARGE_HEADBUTT → [hit] → KNOCKBACK (player 600u, 40 HP damage)
CHARGE_HEADBUTT → [miss] → TURN_AROUND
TURN_AROUND → [player behind Ankylo] → TAIL_SWIPE
TAIL_SWIPE → [hit] → KNOCKBACK (player 800u, 60 HP damage, bone break chance 25%)
TAIL_SWIPE → [miss] → GRAZE (de-escalate)
```

### Ankylosaurus Combat Parameters
- **Warning threshold**: 300 units
- **Headbutt damage**: 40 HP + 2s stun
- **Tail swipe damage**: 60 HP + knockback 800u + 25% bone fracture
- **Tail swipe arc**: 180° rear, 350u radius
- **Armor**: Frontal attacks deal 50% reduced damage
- **Weak point**: Underbelly (2x damage multiplier)

---

## 4. Combat Audio Integration

### Voice Lines (from Agent #12 TTS)
| File | URL | Trigger |
|------|-----|---------|
| ElderNPC_CombatWarning | `tts/1781702256198_ElderNPC_CombatWarning.mp3` | Player first enters raptor zone |
| NPC_Scout_CombatAlert | `tts/1781702269884_NPC_Scout_CombatAlert.mp3` | T-Rex detected within 500u of camp |

### Trigger Wiring
```
OnActorBeginOverlap(CombatTrigger_RaptorPack_001)
  → Play ElderNPC_CombatWarning (if not played in last 120s)
  → Set bIsHunting = true on all raptor BB
  → Activate Alpha charge sequence

OnActorBeginOverlap(Ambush_TRex_DetectCone_001)
  → Play NPC_Scout_CombatAlert (if NPC_Scout_001 alive and within 800u)
  → Trigger T-Rex roar animation
  → Begin charge countdown (1.5s)
```

---

## 5. Combat Balance Design

### "30 Seconds of Fun" Principle (Jaime Griesemer)
Each encounter is designed around a 30-second core loop:
- **Raptors**: Detect → Flank setup (10s) → Coordinated attack (8s) → Player counter or retreat (12s)
- **T-Rex**: Detect → Roar stagger (1.5s) → Charge (3s) → Player reaches corridor OR dies (25s)
- **Ankylosaurus**: Warning (5s) → Headbutt (3s) → Turn (2s) → Tail swipe (2s) → Resolution (18s)

### Fairness Rules
1. Player always has 1.5s warning before any attack lands (roar, stomp sound, visual tell)
2. Every encounter has a viable escape route (retreat corridor, climb, water)
3. T-Rex cannot enter narrow corridors (< 300u width)
4. Raptors cannot swim — rivers are safe zones
5. Ankylosaurus de-escalates if player retreats — not every encounter must be combat

---

## 6. Handoff to Agent #13 (Crowd & Traffic Simulation)

### Data for Crowd Simulation
- **Raptor pack size**: 3-5 raptors (Alpha + 2-4 Betas/Gammas)
- **T-Rex territory radius**: 2000 units — other dinosaurs avoid this zone
- **Ankylosaurus herd**: 2-4 individuals, maintain 600u spacing
- **Combat zone exclusion**: Crowd agents should NOT path through active combat zones
- **Flee behavior**: Herbivores within 800u of T-Rex combat should trigger flee stampede

### Zone Data for Mass AI
```
TRex_Territory: center=(1800, 500), radius=2000 — EXCLUSION ZONE for crowd agents
RaptorPack_Hunt: center=(-800, 1000), radius=600 — DANGER ZONE (crowd flee)
Ankylo_Graze: center=(-200, -1500), radius=500 — PASSIVE (crowd avoid but don't flee)
```
