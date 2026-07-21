# Combat & Enemy AI System — Agent #12 Production Cycle 011

## Overview
This document describes the combat AI spatial architecture deployed in `MinPlayableMap` during cycle 011. All actors are PointLight markers representing logical zones for the combat state machine.

---

## Combat State Machine — T-Rex

### States
```
IDLE (default) → PATROL → ALERT → CHASE → ATTACK → RETREAT
```

### State Transitions
| From | To | Trigger |
|------|----|---------|
| PATROL | ALERT | Player enters WARNING ring (900–1500 units) |
| ALERT | CHASE | Player moves or makes noise in ALERT state |
| ALERT | PATROL | Player freezes for 5+ seconds (freeze mechanic) |
| CHASE | ATTACK | Player enters CRITICAL ring (<800 units) |
| ATTACK | RETREAT | T-Rex health < 30% OR player enters deep water |
| RETREAT | IDLE | T-Rex reaches RetreatZone and 60s cooldown passes |

### Deployed Actors (Cycle 011)
| Label | Color | Position | Purpose |
|-------|-------|----------|---------|
| `CombatAI_TRex_AttackTrigger_01` | 🔴 RED | (1200, 800, 120) | Player proximity → ATTACK state |
| `CombatAI_TRex_RetreatZone_01` | 🟠 ORANGE | (-800, 1500, 100) | T-Rex flees here at low health |
| `CombatAI_DangerRing_Safe_01` | 🟢 GREEN | (0, -1800, 100) | >1500 units — player is safe |
| `CombatAI_DangerRing_Warning_01` | 🟡 YELLOW | (900, -900, 100) | 800–1500 units — T-Rex becomes ALERT |
| `CombatAI_DangerRing_Critical_01` | 🔴 RED | (1200, 200, 100) | <800 units — immediate ATTACK |

---

## Combat State Machine — Raptor Pack

### Pack AI Design
- **Pack size**: 3–5 raptors per encounter
- **Roles**: 1 Distractor (frontal feint) + 2–4 Flankers (side/rear attack)
- **Coordination**: Flankers wait for player to focus on Distractor before closing
- **Retreat**: Entire pack retreats if 2+ members are killed

### Deployed Actors (Cycle 011)
| Label | Color | Position | Purpose |
|-------|-------|----------|---------|
| `CombatAI_Raptor_Flanker_Left_01` | 🟠 AMBER | (500, -600, 100) | Left flanking position |
| `CombatAI_Raptor_Flanker_Right_01` | 🟠 AMBER | (500, 600, 100) | Right flanking position |
| `CombatAI_Raptor_Flanker_Rear_01` | 🟠 AMBER | (800, 0, 100) | Rear cut-off position |
| `CombatAI_Raptor_Distractor_01` | 🟠 AMBER | (200, 0, 100) | Frontal feint attacker |
| `CombatAI_RaptorKillZone_Canyon_01` | 🟣 PURPLE | (-400, 1200, 100) | Canyon choke ambush point |

---

## Stampede System

### Trigger Logic
When T-Rex enters `Herbivore_GrazeZone_Plains_01` (from Agent #11):
1. All herbivore actors enter FLEE state
2. `CombatAI_StampedeTrigger_Plains_01` activates
3. Stampede direction: away from T-Rex, toward player path
4. Player must dodge or be knocked down (stamina damage)

| Label | Color | Position | Purpose |
|-------|-------|----------|---------|
| `CombatAI_StampedeTrigger_Plains_01` | 🟡 YELLOW | (2000, -500, 110) | Herbivore stampede initiation |

---

## Audio Assets (Combat Voice Lines)

### TribeLeader — T-Rex Escape Advice (~14s)
> *"Run. Do not look back. Do not stop. If the T-Rex is already charging — you have maybe four seconds before it closes the gap. Drop everything heavy. Find water. It will not follow you into deep water."*
- URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781810306835_SurvivorNPC_TribeLeader.mp3`
- Triggers at: `CombatAI_DangerRing_Warning_01` (first time player enters WARNING zone)

### ScoutWoman — Raptor Pack Tactics (~13s)
> *"They are circling. Do not run — that is what they want. Stand your ground. Make yourself large. Throw fire if you have it. The pack will hesitate. That hesitation is your only chance."*
- URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781810333290_SurvivorNPC_ScoutWoman_Combat.mp3`
- Triggers at: `CombatAI_RaptorKillZone_Canyon_01` (first time player enters canyon)

---

## Cumulative Combat Zone Map (Cycles 008–011)

| Cycle | Actors Added | System |
|-------|-------------|--------|
| 008 | 5 | Initial combat threat zones (RED markers) |
| 009 | 5 | Combat behavior anchors (AMBER markers) |
| 010 | 8 | Daily routine / patrol anchors (YELLOW markers) |
| 011 | 11 | State machine zones + danger rings + raptor flankers |
| **TOTAL** | **29** | **Full combat AI spatial architecture** |

---

## Integration Notes for Agent #13 (Crowd & Traffic)

1. **Stampede system** requires crowd agent flee behavior — `CombatAI_StampedeTrigger_Plains_01` should trigger Mass AI flee directive for all herbivore crowd agents within 2000 units
2. **Raptor pack** is a 3–5 agent coordinated group — Mass AI formation system can drive flanking positions using deployed markers as navigation targets
3. **Danger rings** define crowd agent avoidance radii — crowd agents should increase movement speed as they enter WARNING zone and sprint in CRITICAL zone
4. **T-Rex retreat path** should be clear of crowd agents to prevent blocking behavior

---

## Design Philosophy (Jaime Griesemer / Naughty Dog Principles)

> *"The best combat is the one where the player doesn't know if they'll win until the last second — and believes they lost because of their own mistake, not because the game was unfair."*

- **T-Rex freeze mechanic**: Rewards player who reads the AI state (ALERT = freeze, CHASE = run)
- **Raptor distractor**: Creates false priority — punishes reactive players, rewards observant ones
- **Stampede as indirect threat**: Player can use stampede to damage/distract T-Rex (environmental combat)
- **Retreat at 30% health**: T-Rex is not a bullet sponge — it has survival instincts too

---

*Generated by Agent #12 — Combat & Enemy AI | PROD_CYCLE_AUTO_20260618_011*
