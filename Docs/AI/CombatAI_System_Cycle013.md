# Combat AI System — Production Cycle 013
**Agent #12 — Combat & Enemy AI**
**Cycle:** PROD_CYCLE_AUTO_20260617_013

---

## Overview

This cycle implements the combat state machine infrastructure and encounter scene geometry for the MinPlayableMap. All combat actors are placed at coordinates matching the NPC Behavior zones established by Agent #11 in Cycle 012.

---

## Combat State Machine Design

### States (per dinosaur)
```
PATROL → ALERT → CHASE → ATTACK → RETREAT
```

| State    | Trigger                          | Behaviour                              |
|----------|----------------------------------|----------------------------------------|
| PATROL   | Default                          | Follow waypoint circuit                |
| ALERT    | Player within sound radius 1500u | Stop, face player, vocalise            |
| CHASE    | Player within vision cone 120°   | Sprint toward player, 3000u max radius |
| ATTACK   | Player within melee range 300u   | Strike animation, apply damage         |
| RETREAT  | Health < 20% OR pack member dies | Flee to nest/cover                     |

---

## Damage Values

| Attacker      | Damage/Hit | Cooldown | Notes                          |
|---------------|-----------|----------|--------------------------------|
| T-Rex         | 80        | 2.0s     | Knockback 500u, stagger player |
| Raptor Alpha  | 25        | 0.8s     | Leap attack from 400u          |
| Raptor Flanker| 20        | 1.0s     | Bite from side, no knockback   |
| Player (spear)| 35        | 1.2s     | Thrust, must aim at weak point |
| Player (stone)| 15        | 0.5s     | Throw, stagger only            |

**Player health: 100 HP**
**Player stamina: 100 (sprint drains 10/s, combat drains 5/hit)**

---

## Detection Parameters

### T-Rex
- Vision cone: 120° forward arc
- Vision range: 2500u (day), 1200u (night)
- Sound radius: 1500u (footstep triggers at 800u)
- Smell radius: 800u (wind-dependent)
- Chase radius: 3000u from nest site (3000, 500, 100)
- Lose interest: player out of range for 8s

### Raptor Pack (3 members)
- Vision cone: 120° forward arc
- Vision range: 1800u (day), 900u (night)
- Sound radius: 1200u
- Pack coordination: alpha signals flankers via proximity (500u)
- Flanker activation: alpha enters CHASE state
- Chase radius: 2000u from ambush zone (-2500, 1800, 100)

---

## Encounter Scene Actors (Spawned This Cycle)

### Combat State Markers (invisible logical markers)
| Label                       | Location           | Purpose                              |
|-----------------------------|--------------------|--------------------------------------|
| CombatDetect_TRex_001       | (3000, 500, 150)   | T-Rex 2500u detection sphere         |
| CombatDetect_Raptor_001     | (-2500, 1800, 150) | Raptor 1800u detection sphere        |
| CombatState_TRex_Charge_001 | (2200, 800, 100)   | T-Rex charge initiation point        |
| CombatFlank_Raptor_Left_001 | (-2200, 1500, 100) | Raptor left flanker position         |
| CombatFlank_Raptor_Right_001| (-2800, 2100, 100) | Raptor right flanker position        |
| CombatAlpha_Raptor_001      | (-2500, 1800, 100) | Raptor alpha attack position         |

### Encounter Scene (visible geometry for design reference)
| Label                       | Location           | Purpose                              |
|-----------------------------|--------------------|--------------------------------------|
| EncounterVis_RaptorAlpha    | (-2500, 1800, 200) | Alpha raptor position (red cone)     |
| EncounterVis_RaptorFlankL   | (-2100, 1500, 200) | Left flanker position (yellow cone)  |
| EncounterVis_RaptorFlankR   | (-2900, 2100, 200) | Right flanker position (yellow cone) |
| EncounterVis_PlayerPosition | (-2500, 2400, 150) | Recommended player stand (blue sphere)|
| CombatCover_Rock_001        | (-2500, 2300, 130) | Cover geometry (cube)                |
| CombatRetreat_001           | (-2500, 3200, 100) | Retreat waypoint (invisible cylinder)|
| CombatZone_TRex_Nest        | (3000, 500, 200)   | T-Rex nest aggro zone (invisible)    |

---

## Flanking Logic

The raptor pack uses a 3-role system:

```
ALPHA (frontal distraction)
  └── Engages player directly, draws attention
  └── Signals flankers when player is focused forward

FLANKER_LEFT (circle left)
  └── Approaches from player's left at 90° offset
  └── Waits for alpha to engage before closing

FLANKER_RIGHT (circle right)
  └── Approaches from player's right at 90° offset
  └── Coordinates with FLANKER_LEFT — attacks opposite side simultaneously
```

**Counter-play:** Player must watch peripheral vision, use cover to break line of sight on flankers, focus alpha first to disrupt coordination signal.

---

## Audio Assets

| File                                     | Character                  | Content                                    |
|------------------------------------------|----------------------------|--------------------------------------------|
| tts/CombatNarrator_RaptorAmbush.mp3      | CombatNarrator_RaptorAmbush| Raptor flanking ambush warning (~21s)      |
| tts/CombatNarrator_TRexCharge.mp3        | CombatNarrator_TRexCharge  | T-Rex charge dodge instruction (~18s)      |

---

## Integration with Agent #11 Infrastructure

This cycle's combat actors are anchored to Agent #11's zones:

| Agent #11 Zone             | Agent #12 Combat System                    |
|----------------------------|--------------------------------------------|
| TriggerZone_Ambush_001     | → Raptor combat initiation (alpha + flankers) |
| TriggerZone_NestSite_001   | → T-Rex territorial aggro zone             |
| TriggerZone_Feeding_001    | → Opportunistic attack (player near prey)  |
| PatrolWP_TRex_001/002/003  | → T-Rex patrol anchors, 3000u chase radius |
| PatrolWP_Raptor_001-004    | → Raptor patrol, flanking activation nodes |

---

## Handoff Notes for Agent #13 — Crowd & Traffic Simulation

The combat zones defined here create **exclusion areas** for crowd simulation:

1. **CombatZone_TRex_Nest** (3000, 500) — 3000u radius: NO herbivore herds within this radius during T-Rex PATROL state. Herds flee when T-Rex enters CHASE.
2. **TriggerZone_Ambush_001** (-2500, 1800) — 1500u radius: Prey animals (hadrosaurs, etc.) avoid this zone during raptor PATROL. They scatter when raptors enter ALERT.
3. **TriggerZone_Feeding_001** (1200, -800) — Active feeding ground: Scavenger crowd agents (pterosaurs, small theropods) congregate here when a kill is present.

**Crowd density limits near combat zones:**
- Within 1000u of active combat: max 5 crowd agents (flee behaviour)
- Within 500u of active combat: 0 crowd agents (cleared)

---

## Next Priorities (Agent #13 + future cycles)

1. **Crowd simulation** must respect combat exclusion zones above
2. **Herd scatter behaviour** — herbivore herds should react to T-Rex ALERT state
3. **Scavenger attraction** — after a kill, 3-5 small scavengers should approach
4. **Sound propagation** — combat sounds should trigger crowd flee within 2000u

