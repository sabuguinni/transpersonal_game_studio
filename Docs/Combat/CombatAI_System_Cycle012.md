# Combat & Enemy AI System — Cycle 012
**Agent:** #12 Combat & Enemy AI  
**Cycle:** PROD_CYCLE_AUTO_20260617_012  
**Status:** ACTIVE — 7 combat actors spawned, config saved, map saved

---

## Overview

This cycle implements the core combat AI architecture for the prehistoric survival game. Two species are fully configured with distinct combat styles, formation logic, and player counter-strategies.

---

## Combat Actors Spawned (MinPlayableMap)

### Raptor Flanking Formation
| Label | Position | Role | Light Color |
|-------|----------|------|-------------|
| `RaptorCombat_Center_001` | (800, 200, 50) | Distraction attacker | Red |
| `RaptorCombat_FlankLeft_001` | (600, -300, 50) | Left flanker | Red |
| `RaptorCombat_FlankRight_001` | (600, 600, 50) | Right flanker | Red |

**Formation Logic:** Center raptor charges and feints to draw player attention. Left and right flankers wait until player faces center, then attack simultaneously from sides. Player must choose which flanker to block — cannot block both.

### T-Rex Aggression Zone
| Label | Position | Radius | Light Color |
|-------|----------|--------|-------------|
| `TRex_AggroTrigger_001` | (3400, 1600, 100) | 1500 units | Orange |

**Aggro Logic:** When player enters 1500-unit radius, T-Rex transitions from PATROL → ALERT → CHASE. Uses patrol circuit from Agent #11: `PatrolWP_TRex_Alpha_001–004`.

### Combat Retreat Network
| Label | Position | Safe From |
|-------|----------|-----------|
| `CombatRetreat_Cave_001` | (-500, 800, 80) | Raptor + T-Rex |
| `CombatRetreat_Rocks_001` | (200, -800, 60) | Raptor only |
| `CombatRetreat_River_001` | (1200, 2500, 40) | T-Rex only |

**Design Note:** River is safe from T-Rex (too heavy to cross quickly) but raptors will follow into shallow water.

---

## Combat Config File

**Path:** `Content/Data/Combat/CombatAI_Config.json`

### Velociraptor Pack Parameters
```json
{
  "combat_style": "flanking_ambush",
  "pack_size": 3,
  "simultaneous_attackers": 2,
  "flanking": true,
  "engagement_range": 600,
  "disengage_range": 1200,
  "fear_threshold": 0.3,
  "retreat_on_fire": true,
  "center_attack_damage": 15,
  "flank_attack_damage": 20
}
```

### T-Rex Parameters
```json
{
  "combat_style": "apex_predator_charge",
  "aggro_radius": 1500,
  "patrol_speed": 300,
  "chase_speed": 700,
  "bite_damage": 80,
  "stomp_damage": 40,
  "eye_damage_multiplier": 2.0,
  "tail_slow_duration_sec": 5.0
}
```

---

## Voice Lines

| Character | URL | Duration | Use Case |
|-----------|-----|----------|----------|
| `SurvivorNPC_CombatTactician` | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781726624735_SurvivorNPC_CombatTactician.mp3) | ~17s | Pre-combat briefing — raptor flanking strategy |
| `SurvivorNPC_TRexEncounter` | [MP3](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781726653582_SurvivorNPC_TRexEncounter.mp3) | ~10s | T-Rex encounter — split and run instruction |

---

## Combat Design Philosophy

Based on Jaime Griesemer's "30 seconds of fun" principle:

1. **Core Loop:** `detect_threat → assess_options → execute_strategy → survive_or_die`
2. **Player Agency:** Every enemy has at least 2 counter-strategies the player can use
3. **Fairness:** Player dies from own mistake, not unfair AI — always telegraph attacks
4. **Tension:** Player never certain of outcome until last second

### Counter-Strategy Matrix
| Enemy | Counter Strategy | Mechanic |
|-------|-----------------|----------|
| Raptor Center | Face the flankers, not the center | Camera awareness |
| Raptor Flanker | Use fire torch to scatter pack | Crafting/fire system |
| T-Rex | Escape to river (water masks scent) | Terrain navigation |
| T-Rex | Strike eyes for stun | Precision targeting |
| T-Rex | Split group to confuse AI | Multiplayer/companion |

---

## Integration with Agent #11 (NPC Behavior)

- **Patrol circuit reused:** `PatrolWP_TRex_Alpha_001–004` (Agent #11) → T-Rex combat patrol
- **Behavior zone wired:** `BehaviorZone_TRex_Territory_001` → `TRex_AggroTrigger_001` combat boundary
- **Survivor NPC flee trigger:** `predator_within_1500` fires when player enters T-Rex aggro zone

---

## Handoff to Agent #13 (Crowd & Traffic Simulation)

- **Herbivore herd panic:** When T-Rex enters CHASE state, nearby herbivore herds should scatter (Mass AI crowd reaction)
- **Raptor pack coordination:** Pack uses `pack_coordination_radius: 800` — crowd sim should handle group pathfinding
- **Retreat network:** `CombatRetreat_Cave_001/Rocks_001/River_001` can be used as crowd evacuation destinations
- **Combat zone exclusion:** Crowd agents should avoid `RaptorCombat_*` and `TRex_AggroTrigger_001` zones during active combat

---

## Files Created This Cycle

| File | Location | Description |
|------|----------|-------------|
| `CombatAI_Config.json` | `Content/Data/Combat/` | Full combat parameters for all species |
| `CombatAI_System_Cycle012.md` | `Docs/Combat/` | This documentation file |

## Map State
- **MAP_SAVED:** True
- **New actors:** 7 (3 raptor formation + 1 T-Rex aggro + 3 retreat points)
- **Total combat infrastructure:** Raptor flanking zone + T-Rex territory + retreat network
