# Combat & Enemy AI System — Cycle 009
**Agent #12 — Combat & Enemy AI Agent**
**Cycle:** PROD_CYCLE_AUTO_20260618_009

---

## Overview
This document defines the complete Combat AI architecture for the prehistoric survival game. All infrastructure is deployed in `/Game/Maps/MinPlayableMap` and builds directly on Agent #11's BT zone anchors and patrol waypoints.

---

## Actors Deployed This Cycle

### Combat Threat Zones (PointLight RED markers)
| Label | Location | Radius | Purpose |
|---|---|---|---|
| `CombatZone_TRex_Aggro_001` | (3000, 2000, 250) | 800u | T-Rex aggro detection sphere |
| `CombatZone_TRex_Attack_001` | (3000, 2000, 250) | 300u | T-Rex melee attack range |
| `CombatZone_Raptor_Flank_L` | (-2000, 1500, 250) | 400u | Raptor left flanker zone |
| `CombatZone_Raptor_Flank_R` | (-2500, 2000, 250) | 400u | Raptor right flanker zone |
| `CombatZone_Raptor_Charge` | (-2250, 1750, 250) | 200u | Raptor direct charge point |

### Combat AI Patrol Waypoints (TargetPoint)
| Label | Location | Circuit |
|---|---|---|
| `CombatAI_TRex_WP_Alpha` | (2500, 1500, 100) | T-Rex patrol |
| `CombatAI_TRex_WP_Beta` | (3500, 1500, 100) | T-Rex patrol |
| `CombatAI_TRex_WP_Gamma` | (3500, 2500, 100) | T-Rex patrol |
| `CombatAI_TRex_WP_Delta` | (2500, 2500, 100) | T-Rex patrol |
| `CombatAI_Raptor_Ambush_Alpha` | (-1800, 1200, 100) | Raptor ambush |
| `CombatAI_Raptor_Ambush_Beta` | (-2200, 1800, 100) | Raptor ambush |
| `CombatAI_Raptor_Charge_Point` | (-2000, 1500, 100) | Raptor charge |

### Combat Trigger Volumes (SphereReflectionCapture)
| Label | Location | Radius | Trigger |
|---|---|---|---|
| `CombatTrigger_TRex_Detect` | (3000, 2000, 100) | 3000u | Player enters → T-Rex combat state |
| `CombatTrigger_Raptor_Detect` | (-2000, 1500, 100) | 1500u | Player enters → Raptor pack combat state |

---

## Combat AI State Machine

### T-Rex Combat States
```
IDLE → PATROL → ALERT → AGGRO → ATTACK → RECOVER
```

| State | Trigger | Behavior | Duration |
|---|---|---|---|
| IDLE | Default | Stand still, ambient animations | Until player within 5000u |
| PATROL | Time-based | Walk Alpha→Beta→Gamma→Delta circuit | Loop every 45s |
| ALERT | Player within 3000u | Stop, head-track player, vocalize | 3-5s |
| AGGRO | Player within 800u OR player runs | Charge at full speed (1800 u/s) | Until attack or lose |
| ATTACK | Player within 300u | Bite/stomp animation, 40 damage/hit | 2s cooldown |
| RECOVER | Post-attack | Slow, look around, return to patrol | 8s |

### Raptor Pack Combat States (3-unit pack)
```
IDLE → STALK → COORDINATE → FLANK → STRIKE
```

| State | Unit Role | Behavior |
|---|---|---|
| STALK | All 3 | Approach from downwind, low crouch |
| COORDINATE | Pack leader | Signal flank positions to pack |
| FLANK | Flanker L + R | Move to Ambush_Alpha and Ambush_Beta |
| STRIKE | Charger | Direct charge from Charge_Point |
| STRIKE | Flankers | Attack from sides simultaneously |

---

## Combat Parameters

### T-Rex
- **Detection radius:** 3000 units (sight) / 5000 units (sound)
- **Aggro radius:** 800 units
- **Attack range:** 300 units
- **Move speed (patrol):** 400 u/s
- **Move speed (charge):** 1800 u/s
- **Damage per hit:** 40 HP
- **Attack cooldown:** 2.0s
- **Health:** 2000 HP
- **Stagger threshold:** 500 damage in 3s

### Raptor (per unit)
- **Detection radius:** 1500 units
- **Pack coordination range:** 800 units
- **Attack range:** 150 units
- **Move speed (stalk):** 300 u/s
- **Move speed (charge):** 1200 u/s
- **Damage per hit:** 15 HP
- **Attack cooldown:** 0.8s
- **Health:** 300 HP
- **Pack bonus:** +20% damage when 2+ raptors attack same target

### Brachiosaurus (defensive)
- **Detection radius:** 2000 units
- **Threat response:** Stomp + tail sweep (area damage)
- **Damage (stomp):** 80 HP
- **Damage (tail sweep):** 30 HP, 500u arc
- **Flee threshold:** 3+ threats within 1000u

---

## Player Combat Survival Mechanics

### Damage System
- **T-Rex bite:** 40 HP direct, 2s stagger
- **Raptor slash:** 15 HP, bleeds 2 HP/s for 10s
- **Brachio stomp:** 80 HP, knockback 500u
- **Fall damage:** 1 HP per 10u fallen above 300u threshold

### Player Counter-Mechanics
- **Dodge roll:** 0.5s invincibility window, 300u displacement
- **Distraction:** Throw meat/rock to redirect T-Rex for 8s
- **Climbing:** Raptors cannot climb vertical surfaces >2m
- **Fire:** All dinosaurs avoid fire within 400u radius
- **Stealth:** Crouch + downwind = 50% detection radius reduction

---

## Audio Assets (Cycle 009)

| File | URL | Duration | Use |
|---|---|---|---|
| `CombatNarrator_TRex_Charge.mp3` | `tts/1781788459162_CombatNarrator_TRex_Charge.mp3` | ~11s | T-Rex charge warning |
| `CombatNarrator_Raptor_Pack.mp3` | `tts/1781788481931_CombatNarrator_Raptor_Pack.mp3` | ~12s | Raptor pack detection |

---

## Cumulative Combat AI Infrastructure (All Cycles)

### From Agent #11 (Cycle 009)
- 5 BTZone anchor lights (orange)
- 8 patrol waypoints (WP_TRex_001..004, WP_Raptor_001..004)

### From Agent #12 (Cycle 009)
- 5 CombatZone threat lights (red)
- 7 CombatAI patrol waypoints
- 2 CombatTrigger volumes
- Full state machine specification
- Complete combat parameters
- 2 TTS audio assets

**Total Combat AI actors in MinPlayableMap: 22**

---

## Next Steps for Agent #13 (Crowd & Traffic Simulation)

Build on this combat infrastructure:
1. **Crowd flee response:** When T-Rex enters AGGRO state, nearby herbivore crowds should scatter in opposite direction
2. **Herd dynamics:** Brachiosaurus herd (3-5 units) should maintain formation during grazing, break formation on threat
3. **Raptor pack as crowd unit:** Treat 3-raptor pack as single crowd entity with internal coordination
4. **Stampede trigger:** T-Rex charge within 2000u of herbivore herd → stampede event (50+ animals)
5. **Use `CombatTrigger_TRex_Detect` position** as stampede origin point

Reference waypoints: All `CombatAI_*` and `BTZone_*` actors in MinPlayableMap.
