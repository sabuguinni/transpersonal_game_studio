# Combat & Enemy AI System — Cycle 020 (PROD_CYCLE_AUTO_20260619_002)

**Agent:** #12 — Combat & Enemy AI  
**Cycle:** PROD_CYCLE_AUTO_20260619_002  
**Date:** 2026-06-19  
**Status:** DEPLOYED — 10 combat actors in MinPlayableMap

---

## Overview

This cycle implements the combat AI spatial architecture for the prehistoric survival game. All combat zones are designed to create **dynamic, readable encounters** — the player always understands the threat, has a chance to react, and dies because of their own mistake, not unfair AI.

---

## Deployed Actors (MinPlayableMap)

### T-Rex Patrol Path (4 nodes — Deep Red lights)
| Label | Location | Role |
|-------|----------|------|
| `TRex_Patrol_01_Savana` | (4000, -1000, 100) | Start — open savana, high visibility |
| `TRex_Patrol_02_River` | (3000, 1000, 100) | River crossing — **intersects Hunter_WP04** from Agent #11 |
| `TRex_Patrol_03_Forest` | (1500, 3000, 100) | Forest edge — near Gather zone |
| `TRex_Patrol_04_Perimeter` | (5500, 2500, 100) | Outer perimeter — near Alert zone |

**Design intent:** T-Rex patrol creates a moving danger corridor. When T-Rex is at node 02 (river), it directly intersects the Hunter NPC patrol route — creating emergent predator/prey encounters without scripted events.

### Raptor Pack Combat Zones (3 nodes — Orange lights)
| Label | Location | Role |
|-------|----------|------|
| `Raptor_CombatZone_Alpha` | (5000, 2000, 100) | Pack leader territory — near BT_Zone_Alert_Perimeter |
| `Raptor_CombatZone_Beta` | (5800, 1500, 100) | Flanking position — east approach |
| `Raptor_CombatZone_Gamma` | (4500, 2800, 100) | Ambush point — forest cover |

**Design intent:** Three-node raptor territory creates a triangular kill zone. Alpha is the visible threat; Beta and Gamma are the flankers the player doesn't see until too late. Pack AI: Alpha tests, Beta flanks, Gamma waits for panic.

### Combat Trigger Zones (3 nodes — Magenta lights)
| Label | Location | Role |
|-------|----------|------|
| `CombatTrigger_TRex_Detect` | (3500, 500, 100) | T-Rex detection radius — 2000u sphere |
| `CombatTrigger_Raptor_Ambush` | (5200, 2200, 100) | Raptor ambush activation |
| `CombatTrigger_Sanctuary_End` | (1800, 800, 100) | Edge of safe camp zone — combat can begin here |

---

## Combat AI State Machine

```
IDLE ──────────────────────────────────────────────────────────────────────────┐
  │ (patrol path, ambient behaviour)                                            │
  ▼                                                                             │
ALERT ◄── player enters detection radius (2000u for T-Rex, 1200u for Raptor)  │
  │ (stops, orients, vocalises warning)                                         │
  ▼                                                                             │
STALK ◄── player confirmed (line of sight + movement detection)                │
  │ (closes distance slowly, flankers reposition)                               │
  ▼                                                                             │
CHARGE ◄── player within 800u OR player runs                                   │
  │ (full speed attack, T-Rex linear, Raptor zigzag)                            │
  ▼                                                                             │
ATTACK ◄── contact range (T-Rex 200u, Raptor 150u)                             │
  │ (bite/claw animation, damage applied)                                       │
  ▼                                                                             │
DISENGAGE ◄── prey escapes >3000u OR prey hides >60s                          │
  │ (search behaviour, then return to patrol)                                   │
  └──────────────────────────────────────────────────────────────────────────► IDLE
```

---

## Dinosaur Combat Profiles

### T-Rex — Apex Predator
- **Detection radius:** 2000 units (sight + sound)
- **Charge speed:** 1400 units/second (faster than player sprint)
- **Attack damage:** 80 HP (one-shot if player health < 80)
- **Turn radius:** Slow (player can dodge sideways during charge)
- **Weakness:** Cannot turn fast — dodge left/right during charge
- **Patrol:** Follows 4-node loop, 90s per segment
- **NPC interaction:** T-Rex at node 02 triggers Hunter NPC FLEE state

### Raptor — Pack Hunter
- **Detection radius:** 1200 units (movement-sensitive — freeze mechanic)
- **Pack size:** 3 (Alpha + 2 flankers)
- **Alpha charge speed:** 1100 units/second
- **Attack damage:** 25 HP per hit (multiple hits per encounter)
- **Weakness:** Flankers hesitate if player faces Alpha — exploit 1v1 window
- **Pack AI:** Alpha tests → Beta flanks → Gamma ambushes panicking prey
- **Territory:** Triangular zone (Alpha/Beta/Gamma nodes)

---

## Integration with Agent #11 (NPC Behavior)

### Respected constraints:
- ✅ Camp at origin (0,0) is **sanctuary** — no combat zones within 1500u
- ✅ `CombatTrigger_Sanctuary_End` at (1800, 800) marks exact safe zone boundary
- ✅ T-Rex patrol node 02 at (3000, 1000) **intentionally intersects** `PatrolWP_Hunter_04_River`
- ✅ Raptor zones near `BT_Zone_Alert_Perimeter` at (5000, 2000) — NPC alert propagation triggers

### Emergent encounter design:
When T-Rex reaches patrol node 02 (river) at the same time a Hunter NPC is at `PatrolWP_Hunter_04_River`:
1. T-Rex enters ALERT state (NPC triggers detection)
2. NPC enters FLEE state (runs toward camp)
3. Player witnesses the chase — learns T-Rex patrol timing organically
4. If player is nearby, they must choose: help NPC (distract T-Rex), flee, or hide

---

## Audio Assets

| File | URL | Usage |
|------|-----|-------|
| `Combat_Survivor_Veteran.mp3` | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781834317255_Combat_Survivor_Veteran.mp3 | Tutorial hint — T-Rex encounter |
| `Combat_Raptor_Warning.mp3` | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781834345704_Combat_Raptor_Warning.mp3 | Tutorial hint — Raptor pack encounter |

---

## Combat Design Principles

1. **30-second loop (Jaime Griesemer):** Every combat encounter has the same structure — detect → react → engage → resolve. Varied by terrain, player skill, and dinosaur type. Never by arbitrary difficulty spikes.

2. **Readable threat (Naughty Dog):** Dinosaurs vocalise before attacking. T-Rex roars at ALERT. Raptors click/chirp during STALK. Player always has 2-3 seconds to react before CHARGE.

3. **Fair failure:** Player dies because they ran (triggering charge), turned their back (triggering raptor flankers), or ignored audio cues. Never because of invisible hitboxes or unfair damage.

4. **Spatial clarity:** Combat zones are geographically separated from the camp sanctuary. Player learns the map's danger gradient by exploration, not tutorial text.

---

## Handoff to Agent #13 (Crowd & Traffic Simulation)

**Agent #13 should:**
1. Ensure crowd simulation agents (herbivore herds, bird flocks) **flee from T-Rex patrol path** — creates ambient world reactivity
2. Herbivore stampede direction: away from `TRex_Patrol_02_River` toward safe zones
3. Crowd agents should NOT enter Raptor territory triangle (Alpha/Beta/Gamma nodes)
4. Stampede events near `CombatTrigger_TRex_Detect` can alert player to incoming T-Rex before visual contact
5. Maximum crowd density near combat zones: 20 agents (performance + gameplay clarity)

---

*Generated by Combat & Enemy AI Agent #12 — PROD_CYCLE_AUTO_20260619_002*
