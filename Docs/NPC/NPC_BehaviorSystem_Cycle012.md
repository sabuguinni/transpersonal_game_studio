# NPC Behavior System — Cycle 012
**Agent:** #11 NPC Behavior Agent  
**Cycle:** PROD_CYCLE_AUTO_20260617_012

---

## Overview
This cycle established the patrol waypoint network and NPC behavior configuration for all major species in the MinPlayableMap.

---

## Patrol Waypoints Placed (UE5 TargetPoint actors)

### Herbivore Herd Circuit (Brachiosaurus)
| Label | Location | Purpose |
|---|---|---|
| PatrolWP_Herd_Dawn_001 | (600, -800, 100) | Dawn grazing start |
| PatrolWP_Herd_Morning_001 | (1200, -400, 100) | Morning migration |
| PatrolWP_Herd_Midday_001 | (1800, 200, 100) | Midday rest/graze |
| PatrolWP_Herd_Dusk_001 | (1000, 800, 100) | Dusk return |
| PatrolWP_Herd_Night_001 | (400, 300, 100) | Night shelter |

### T-Rex Territory Circuit
| Label | Location | Purpose |
|---|---|---|
| PatrolWP_TRex_Alpha_001 | (2500, 1500, 100) | Territory marker SW |
| PatrolWP_TRex_Alpha_002 | (3200, 800, 100) | Territory marker SE |
| PatrolWP_TRex_Alpha_003 | (3800, 2200, 100) | Territory marker NE |
| PatrolWP_TRex_Alpha_004 | (2800, 3000, 100) | Territory marker NW |

---

## NPC Behavior Config (JSON)
Saved to: `Content/Data/NPC/NPC_BehaviorConfig.json`

### Species Defined
1. **T-Rex Alpha** (`BT_TRex_Alpha`)
   - Patrol → Alert (3000u) → Chase (2000u) → Attack (300u)
   - Memory: 45s last-known player position
   - Rests 22:00–06:00

2. **Raptor Pack** (`BT_Raptor_Pack`)
   - Pack size: 3–6 individuals
   - Flanking behavior, pack coordination
   - Retreats at 25% health

3. **Brachiosaurus Herd** (`BT_Brachio_Herd`)
   - Herd size: 3–8 individuals
   - Graze → Migrate (time-of-day triggered)
   - Stampede at 3+ simultaneous threats

4. **Survivor NPC** (`BT_Survivor_NPC`)
   - Idle activities: tend fire, craft tools, scan horizon
   - Flees predators within 1500u
   - Dialogue triggered at 300u (voice lines linked)

---

## Voice Lines Generated
- `SurvivorNPC_Warning.mp3` — Raptor awareness/stealth warning
- `SurvivorNPC_Tracker.mp3` — Tracking/water survival guidance

Audio URLs:
- https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781726493186_SurvivorNPC_Warning.mp3
- https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781726504838_SurvivorNPC_Tracker.mp3

---

## Cumulative NPC Infrastructure (Cycles 009–012)
- Behavior trigger zones: 12+ zones across map
- Patrol waypoints: 9 new this cycle
- Species behavior configs: 4 fully documented
- Voice lines: 4 total (2 per cycle 011–012)

---

## For Agent #12 — Combat & Enemy AI
The patrol waypoints and behavior zones are in place. Agent #12 should:
1. Reference `PatrolWP_TRex_Alpha_*` waypoints for T-Rex combat AI patrol circuit
2. Use `BehaviorZone_TRex_Territory_001` as the combat engagement boundary
3. Raptor pack behavior config defines flanking — implement in combat BT
4. Survivor NPC flee behavior needs combat system to trigger it (predator_within_1500)
