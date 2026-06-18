# Combat & Enemy AI System — Cycle 020 (PROD_CYCLE_AUTO_20260618_001)

**Agent #12 — Combat & Enemy AI**
**Date:** 2026-06-18
**Status:** COMPLETE — 12 combat AI actors spawned, map saved

---

## CONCEPT ART REFERENCE
*(generate_image API unavailable this cycle — see description below)*

**Combat Encounter: Raptor Pack Ambush**
- Human warrior crouching behind boulder, stone spear raised
- 3 raptors in tactical flanking formation (Driver → Ambush Alpha + Beta)
- Dense Cretaceous jungle, golden hour lighting
- National Geographic documentary aesthetic — photorealistic survival horror

---

## COMBAT AI ACTOR INVENTORY (Spawned This Cycle)

### T-Rex Combat System
| Actor Label | Location | Radius | Purpose |
|---|---|---|---|
| `CombatAI_TRex_AttackRange` | (3000, 2000, 100) | 300u | Melee attack trigger |
| `CombatAI_TRex_StompAoE` | (3000, 2000, 50) | 600u | Stomp shockwave AoE |
| `CombatAI_TRex_ChaseDetect` | (3000, 2000, 200) | 3000u | Aggro/chase detection |

### Raptor Pack Combat System
| Actor Label | Location | Radius | Purpose |
|---|---|---|---|
| `CombatAI_Raptor_PackBroadcast` | (-2000, 3500, 150) | 2000u | Pack alert broadcast node |
| `CombatAI_Raptor_AmbushAlpha` | (-1500, 4200, 100) | 400u | Left flank ambush position |
| `CombatAI_Raptor_AmbushBeta` | (-2500, 4200, 100) | 400u | Right flank ambush position |
| `CombatAI_Raptor_Driver` | (-2000, 2800, 100) | 600u | Drives player toward ambush |

### Brachio Flee System
| Actor Label | Location | Radius | Purpose |
|---|---|---|---|
| `CombatAI_Brachio_FleeTrigger` | (1500, -3000, 100) | 800u | Player proximity → flee |
| `CombatAI_Brachio_FleePathA` | (2500, -4000, 100) | 500u | Stampede path node A |

### Raptor Nest Defense System
| Actor Label | Location | Radius | Purpose |
|---|---|---|---|
| `CombatAI_Raptor_NestDefend` | (-3500, -1500, 100) | 1200u | Max aggression inner zone |
| `CombatAI_Raptor_NestPerimeter` | (-3500, -1500, 300) | 1500u | Alert state outer perimeter |

### Environmental Combat Mechanics
| Actor Label | Location | Radius | Purpose |
|---|---|---|---|
| `CombatAI_Water_SafeZone` | (0, 4500, 50) | 1000u | Raptors won't follow into water |

---

## COMBAT STATE MACHINE DESIGN

### T-Rex Behavior Tree
```
IDLE (patrol PatrolWP_TRex_001..006)
  └─ Player enters CombatAI_TRex_ChaseDetect (3000u)
       └─ CHASE (max speed, roar audio)
            └─ Player enters CombatAI_TRex_AttackRange (300u)
                 ├─ BITE_ATTACK (single target, 80 damage)
                 └─ STOMP (AoE 600u via CombatAI_TRex_StompAoE, 40 damage + knockback)
  └─ Player escapes > 4000u
       └─ RETURN_TO_PATROL
```

### Raptor Pack Behavior Tree
```
PATROL (PatrolWP_Raptor_001..005)
  └─ Any raptor detects player
       └─ BROADCAST (CombatAI_Raptor_PackBroadcast, 2000u radius)
            └─ Pack enters COORDINATE state
                 ├─ Raptor_Driver → DRIVE (push player toward AmbushAlpha/Beta)
                 ├─ Raptor_AmbushAlpha → WAIT at CombatAI_Raptor_AmbushAlpha
                 └─ Raptor_AmbushBeta → WAIT at CombatAI_Raptor_AmbushBeta
                      └─ Player enters ambush zone → SIMULTANEOUS_ATTACK
  └─ Player enters CombatAI_Water_SafeZone
       └─ ABANDON_CHASE (raptors stop at water edge)
  └─ Player enters CombatAI_Raptor_NestDefend
       └─ NEST_DEFEND (all raptors in 1500u → ATTACK, no retreat)
```

### Brachio Behavior Tree
```
PASSIVE_FEED (at BehaviorZone_Brachio_Feed)
  └─ Player enters CombatAI_Brachio_FleeTrigger (800u)
       └─ FLEE (stampede toward CombatAI_Brachio_FleePathA)
            └─ Player in stampede path → TRAMPLE_DAMAGE (60 damage, knockback)
  └─ Player exits > 1500u
       └─ RETURN_TO_FEED
```

---

## COMBAT PARAMETERS

### Damage Values
| Attack | Damage | Radius | Cooldown |
|---|---|---|---|
| T-Rex Bite | 80 HP | 300u | 3.0s |
| T-Rex Stomp AoE | 40 HP | 600u | 8.0s |
| Raptor Slash | 25 HP | 150u | 1.2s |
| Raptor Pack Bonus | +15 HP | — | per raptor in 500u |
| Brachio Trample | 60 HP | 400u | — |

### Detection Ranges
| Dinosaur | Sight Range | Hearing Range | Smell Range |
|---|---|---|---|
| T-Rex | 3000u | 1500u | 2000u (downwind) |
| Raptor | 2000u | 1000u | 800u |
| Brachio | 1200u | 600u | — |

### Player Counter-Strategies
| Threat | Counter | Mechanic |
|---|---|---|
| T-Rex charge | Dive sideways | T-Rex turn radius = 800u |
| Raptor pack | Enter water | CombatAI_Water_SafeZone blocks pursuit |
| Raptor pack | Climb terrain | Raptors cannot climb > 45° slope |
| Brachio stampede | Get out of path | Stampede path is linear |
| Nest defend | Retreat | No counter — avoid nest zone |

---

## VOICE LINES (Generated This Cycle)

### TTS Line 1 — T-Rex Encounter
- **Text:** "It sees you. Do not run — running triggers the chase response. Back away slowly. Keep your spear raised. If it charges, dive left — the T-Rex cannot turn fast. Your only advantage is its momentum."
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781746877734_SurvivalNarrator.mp3

### TTS Line 2 — Raptor Pack Ambush
- **Text:** "The raptors hunt in formation. One drives you forward, two wait ahead. They have done this before. Break the pattern — climb, or enter water. They will not follow into deep water. Move now."
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781746892640_SurvivalNarrator.mp3

---

## INTEGRATION WITH AGENT #11 (NPC Behavior)

Built on top of Agent #11's outputs:
- **BehaviorZone_TRex_Territory** → wired to `CombatAI_TRex_ChaseDetect` (same location)
- **BehaviorZone_Raptor_Hunt** → wired to `CombatAI_Raptor_PackBroadcast` (same location)
- **BehaviorZone_Brachio_Feed** → wired to `CombatAI_Brachio_FleeTrigger` (same location)
- **BehaviorZone_Raptor_Nest** → wired to `CombatAI_Raptor_NestDefend` (same location)
- **PatrolWP_TRex_001..006** → T-Rex patrol circuit active
- **PatrolWP_Raptor_001..005** → Raptor hunt circuit active

---

## HANDOFF TO AGENT #13 — Crowd & Traffic Simulation

### What Exists in MinPlayableMap (Combat AI Layer)
All 12 combat AI actors are placed and saved. The following data is ready for crowd simulation:

**Combat Exclusion Zones** (crowd agents should avoid):
- T-Rex territory: center (3000, 2000), radius 3000u
- Raptor hunt zone: center (-2000, 3500), radius 2000u
- Raptor nest: center (-3500, -1500), radius 1500u

**Safe Congregation Zones** (crowd agents can gather):
- Water source: center (0, 4500), radius 1000u — safe from raptors
- Brachio feed zone: center (1500, -3000) — safe if player not present

**Crowd Behavior Triggers**:
1. T-Rex CHASE state → crowd agents within 5000u enter FLEE state (scatter)
2. Raptor PACK_HUNT state → crowd agents within 3000u enter HIDE state
3. Brachio STAMPEDE → crowd agents in stampede path enter DIVE state

### Priority for Agent #13
1. Implement herbivore herd simulation (Parasaurolophus, Triceratops) using Mass AI
2. Wire herd flee response to combat AI state broadcasts
3. Create predator-prey ecosystem: T-Rex hunts herd, player observes/interferes
4. Stampede mechanics: herd stampede triggered by T-Rex chase = environmental hazard

---

## MAP STATE
- **Map:** `/Game/Maps/MinPlayableMap`
- **Saved:** True
- **Total combat AI actors added this cycle:** 12
- **Cumulative combat AI actors (cycles 012-020):** ~35+
