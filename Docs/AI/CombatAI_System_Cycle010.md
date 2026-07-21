# Combat AI System — Cycle 010 Documentation
**Agent #12 — Combat & Enemy AI**
**Cycle:** PROD_CYCLE_AUTO_20260618_010

---

## Overview
This cycle implements the core Combat AI architecture for the prehistoric survival game. The system covers raptor pack flanking behavior, T-Rex aggro zones, and AI state machine transition markers — all deployed as PointLight visual markers in MinPlayableMap.

---

## Raptor Pack Flanking System

### Formation Roles
| Actor Label | Role | Position | Behavior |
|---|---|---|---|
| `CombatRaptor_Alpha_001` | ALPHA_FRONTAL | (1800, 800) | Direct charge at player — draws attention |
| `CombatRaptor_Beta_001` | BETA_LEFT_FLANK | (1400, 1400) | Wide left arc — cuts off escape route |
| `CombatRaptor_Gamma_001` | GAMMA_RIGHT_FLANK | (2200, 1400) | Wide right arc — mirrors Beta |
| `CombatRaptor_Delta_001` | DELTA_REAR_AMBUSH | (1800, 400) | Waits behind player — ambush on flee |

### Pack Coordination Logic
```
PACK_HUNT_SEQUENCE:
1. DETECT: Alpha spots player (detection radius 800 UU)
2. SIGNAL: Alpha vocalizes → Beta/Gamma/Delta receive signal
3. SPREAD: Beta/Gamma break left/right (flanking arcs)
4. ALPHA_CHARGE: Alpha charges frontally (draws player attention)
5. FLANK_CLOSE: Beta/Gamma close arcs simultaneously
6. DELTA_ACTIVATE: If player flees, Delta intercepts from rear
7. ENCIRCLE: All four raptors within 300 UU of player → ATTACK phase
```

### Patrol Waypoint Routes
**Alpha charge path:** (2600,800) → (2200,600) → (1800,400) → (1400,200)
**Beta left arc:** (2400,1200) → (1800,1600) → (1000,1400) → (400,800)
**Gamma right arc:** (2400,400) → (2000,-200) → (1200,-400) → (400,-200)

---

## T-Rex Aggro Zone System

### Zone Hierarchy
| Actor Label | Radius | Function |
|---|---|---|
| `Combat_TRex_AggroCore` | 600 UU | Immediate aggro — player in range = CHARGE |
| `Combat_TRex_DetectRing` | 1200 UU | Detection — player spotted, T-Rex alerts |
| `Combat_TRex_ChargeVector` | 400 UU | Charge path start marker |
| `Combat_TRex_StompZone` | 350 UU | Stomp AoE impact area |

### T-Rex Charge Path
(3800,1800) → (3200,1400) → (2600,1000) → (2000,600)

### T-Rex Behavior States
```
TREX_AI_STATES:
- IDLE: Patrols between (4000,2000) and (3500,2500), slow pace
- ALERT: Player detected in DetectRing — head turns, roar, 3s delay
- CHARGE: Player in AggroCore — full speed charge along vector
- STOMP: Player within 300 UU — AoE stomp (3m radius, 80 damage)
- RECOVER: Post-stomp 2s cooldown — vulnerable window
- RETURN: Player escaped DetectRing — T-Rex returns to patrol
```

---

## Combat State Machine

### State Transition Markers
| Actor Label | Color | State | Trigger |
|---|---|---|---|
| `CombatState_Patrol_Zone` | Green | PATROL | Default — no player nearby |
| `CombatState_Alert_Zone` | Yellow | ALERT | Player within 1500 UU |
| `CombatState_Combat_Zone` | Orange | COMBAT | Player within 600 UU |
| `CombatState_Retreat_Zone` | Blue | RETREAT | Health < 30% OR pack member killed |
| `CombatState_Ambush_Zone` | Purple | AMBUSH | Player unaware, Delta pre-positioned |

### State Transition Rules
```
PATROL → ALERT:     player_distance < 1500 AND line_of_sight = true
ALERT → COMBAT:     player_distance < 600 OR player_attacked_pack
COMBAT → RETREAT:   health < 0.3 OR pack_size < 2
COMBAT → AMBUSH:    player_fleeing AND delta_in_position
AMBUSH → COMBAT:    player_enters_ambush_radius
RETREAT → PATROL:   distance_from_player > 2000 AND 10s elapsed
```

---

## Integration with Agent #11 NPC Zones

### Zone Connections
| NPC Zone (Agent #11) | Combat AI Usage |
|---|---|
| `NPC_DangerZone_TRex` (4000,2000) | T-Rex AggroCore anchor point |
| `NPCMem_ThreatZone_River` | Raptor ambush trigger (Delta activation) |
| `NPCMem_AllyDied_Clearing` | High-tension escalation — pack aggression +50% |
| `NPC_Savana_HuntZone` (2500,1000) | Raptor patrol overlap zone |
| `NPC_Forest_GatherZone` (-200,1500) | Safe zone — raptors avoid (NPC memory) |

### Memory Zone → Combat State Transitions
```
THREAT_SEEN memory → Raptor pack ALERT state (all members)
DANGER_ZONE memory → T-Rex patrol range expands +400 UU
ALLY_DIED memory → Pack aggression multiplier 1.5x, retreat threshold 0.15
SAFE_ZONE memory → Raptors will not enter (NPC safe haven respected)
```

---

## Voice Lines (TTS Assets)

### RaptorAlpha Combat Call
**URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781805869770_RaptorAlpha_CombatCall.mp3
**Text:** "Flanking position confirmed. Alpha moves in three... two... one. NOW!"
**Usage:** Play when Alpha raptor initiates charge (NPC observer hears this as raptor vocalization cue)

### T-Rex Combat Warning (Human NPC)
**URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781805890502_TRex_CombatWarning.mp3
**Text:** "The great beast charges! Run to the rocks! Split up — do not let it corner you!"
**Usage:** Play when T-Rex enters CHARGE state — tribal NPC shouts warning to player

---

## Actors Deployed This Cycle

### Combat Zone Actors (13 total)
- 4x Raptor pack formation markers (CombatRaptor_Alpha/Beta/Gamma/Delta)
- 4x T-Rex aggro zones (AggroCore, DetectRing, ChargeVector, StompZone)
- 5x Combat state machine markers (Patrol/Alert/Combat/Retreat/Ambush)

### Combat Waypoint Actors (16 total)
- 4x Alpha charge path waypoints
- 4x Beta left flanking arc waypoints
- 4x Gamma right flanking arc waypoints
- 4x T-Rex charge vector waypoints

**Total new actors this cycle: 29**
**MAP_SAVED: True**

---

## Next Steps for Agent #13 (Crowd & Traffic Simulation)

1. Use `CombatState_Retreat_Zone` (800,600) as crowd scatter origin
2. Raptor pack formation at (1800,800) should trigger crowd flee behavior
3. T-Rex AggroCore (4000,2000) = maximum crowd avoidance radius (2000 UU)
4. NPC crowd agents should transition to FLEE state when `CombatState_Combat_Zone` is active
5. Implement crowd stampede physics when T-Rex enters CHARGE state
6. Reference `NPC_Camp_SleepZone` (-800,-800) as crowd safe gathering point during combat
