# Combat & Enemy AI System — Agent #12 Production Cycle 004

## Overview
Full combat AI infrastructure deployed in MinPlayableMap. This cycle completes the combat encounter
framework integrating with NPC actors from Agent #11 and dinosaur actors from previous cycles.

---

## Combat Zones Deployed

### T-Rex Combat System
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| CombatZone_TRex_AggroDetect | TriggerSphere (scale ×20) | (3000, -1500) | Outer detection radius ~2000 units |
| CombatZone_TRex_ChargeTrigger | TriggerSphere (scale ×8) | (3000, -1500) | Inner charge trigger ~800 units |
| TRex_Territory_Marker_N/NE/SE/SW | PointLight (RED) | Territory boundary | Visual territory boundary markers |

**T-Rex AI State Machine:**
```
IDLE → ALERT (player enters AggroDetect) → CHARGE (player enters ChargeTrigger) → HUNT
HUNT → SEARCH (player breaks LOS for 10s) → IDLE
```

**Player Counter-Strategy:**
- Break LOS using boulders/dense trees
- Retreat via CombatRetreat_River_001/002/003 waypoints
- T-Rex does NOT follow into deep water (river zone)

---

### Raptor Pack Combat System
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| CombatPoint_Raptor_FlankLeft | TargetPoint | (1800, -800) | Left flanker spawn/move target |
| CombatPoint_Raptor_Center | TargetPoint | (2200, -1200) | Center aggressor |
| CombatPoint_Raptor_FlankRight | TargetPoint | (1800, -1600) | Right flanker |
| Raptor_Zone_Marker_001/002/003 | PointLight (ORANGE) | Pack zone | Visual zone markers |
| Stealth_Ambush_Marker_001/002 | PointLight (PURPLE) | High grass | Stealth approach zone |

**Raptor Pack Tactics:**
1. **Detection Phase**: One raptor spots player → vocalizes (bark sound)
2. **Flanking Phase**: Pack splits to FlankLeft + FlankRight positions (3-4s)
3. **Charge Phase**: Center raptor charges, flankers converge simultaneously
4. **Counter**: Player must eliminate center raptor first to break pack coordination

**Stealth Zone Logic:**
- Raptors in Stealth_Ambush zone move silently (no footstep audio)
- Player receives no visual warning until within 600 units
- Tall grass visual cover (requires VFX agent for grass implementation)

---

### Ankylosaurus Patrol System
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| Patrol_Ankyl_001/002/003/004 | TargetPoint | Patrol loop | Ankylosaurus patrol waypoints |
| Ankyl_Zone_Marker_001/002 | PointLight (YELLOW) | Zone boundary | Visual patrol zone |

**Ankylosaurus Behavior:**
- Default: Passive patrol along waypoints (non-aggressive)
- Provoked: Tail-swing attack (180° arc, 300 unit range)
- Flee trigger: Player within 150 units → charge and swing
- **Tactical use**: Can be provoked to attack pursuing raptors

---

### Camp Protection System
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| CombatZone_Camp_DinoThreat | TriggerSphere (scale ×15) | (-1200, 800) | NPC camp protection radius |

**Integration with Agent #11 NPCs:**
- When any dinosaur enters CombatZone_Camp_DinoThreat:
  - NPC_Tribesman_Camp_001 (Gatherer) → FLEE state
  - NPC_Tribesman_Camp_002 (Guard) → ALERT + torch-raise
  - NPC_Elder_Camp_001 (Elder) → WARN_PLAYER state (triggers dialogue)
- Hunter NPCs at BehaviorZone_NPC_Hunt:
  - Small prey dino (Raptor) → HUNT state (approach + throw spear)
  - Large predator (TRex) → RETREAT state (sprint to camp)

---

### Player Retreat System
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| CombatRetreat_River_001 | TargetPoint | (500, 0) | First retreat waypoint |
| CombatRetreat_River_002 | TargetPoint | (200, -200) | Second retreat waypoint |
| CombatRetreat_River_003 | TargetPoint | (-100, -400) | River entry point |

**Retreat Logic:**
- Player HUD shows directional arrow toward nearest retreat point when TRex aggro active
- River zone = safe from TRex (cannot follow)
- Raptors will follow to river edge but not enter

---

## Audio Assets (TTS Generated)

### CombatNarrator_Raptor.mp3
- URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781762098750_CombatNarrator_Raptor.mp3`
- Trigger: Player enters Stealth_Ambush zone
- Text: *"Raptors hunt in packs. They are testing you right now — watching from the treeline, waiting for you to panic. Do not run. Back away slowly toward the river..."*

### CombatNarrator_TRex.mp3
- URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781762118359_CombatNarrator_TRex.mp3`
- Trigger: Player enters CombatZone_TRex_AggroDetect
- Text: *"The T-Rex has entered the valley. You cannot outrun it on open ground. Find cover — boulders, dense trees, anything to break its line of sight..."*

---

## Color-Coded Threat System (Visual Design)

| Color | Threat Level | Dinosaur | Behavior |
|---|---|---|---|
| 🔴 RED (1200 intensity) | CRITICAL | T-Rex | Charge on sight, cannot outrun |
| 🟠 ORANGE (900 intensity) | HIGH | Raptor Pack | Flanking tactics, coordinated attack |
| 🟡 YELLOW (700 intensity) | MEDIUM | Ankylosaurus | Patrol, provoked only |
| 🟣 PURPLE (600 intensity) | STEALTH | Raptor (ambush) | Silent approach, no warning |

---

## Combat Encounter Design Principles

### The 30-Second Rule (Jaime Griesemer)
Each combat encounter is designed around a 30-second core loop:
1. **Detection** (0-5s): Player sees/hears threat indicator
2. **Decision** (5-10s): Player chooses fight, flee, or hide
3. **Execution** (10-25s): Player executes strategy
4. **Resolution** (25-30s): Outcome — escape, defeat, or kill

### Fairness Guarantee
- Every combat zone has at least ONE visible escape route
- T-Rex charge is telegraphed 3s before contact (roar + ground shake)
- Raptor flanking is visible if player checks periphery
- Player failure = player mistake, never unfair AI

---

## Dependencies for Agent #13 (Crowd & Traffic)

1. **CombatZone_Camp_DinoThreat** — when triggered, crowd simulation should scatter NPC_Tribesman actors
2. **TRex_Territory_Marker_*** — crowd agents should avoid this zone entirely
3. **Raptor_Zone_Marker_*** — crowd agents should move at reduced speed (cautious) in this zone
4. **CombatRetreat_River_*** — crowd agents can use these as emergency pathfinding waypoints
5. **Patrol_Ankyl_*** — crowd agents should give Ankylosaurus 500-unit clearance

---

## Map State After Cycle 004
- Total combat actors deployed: ~25 new actors this cycle
- All actors saved to `/Game/Maps/MinPlayableMap`
- MAP_SAVED: True
