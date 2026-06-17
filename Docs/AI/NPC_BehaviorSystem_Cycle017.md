# NPC Behavior System — Production Cycle 017
**Agent #11 — NPC Behavior Agent**
**Cycle:** PROD_CYCLE_AUTO_20260617_013

---

## Voice Lines Produced

### SurvivorNPC_Warning
> *"Stay still. The Raptor sees movement, not shape. If you freeze, you become part of the landscape. If you run, you become prey."*
- Audio URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781731928434_SurvivorNPC_Warning.mp3
- Duration: ~9s
- Use: Triggered when player first enters Raptor territory zone

### ElderTribeswoman_Advice
> *"The herd moves at dawn. Follow them to the river, but never get between a mother and her young. That mistake only happens once."*
- Audio URL: https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781731941224_ElderTribeswoman_Advice.mp3
- Duration: ~9s
- Use: Tutorial hint for herbivore herd behavior / river navigation

---

## UE5 Actors Placed in MinPlayableMap

### Behavior Trigger Zones (TargetPoint markers)
| Label | Location | Behavior Type | Purpose |
|-------|----------|---------------|---------|
| TriggerZone_Ambush_001 | (-2500, 1800, 100) | AMBUSH | Raptors hide here, burst-attack player |
| TriggerZone_Feeding_001 | (1200, -800, 100) | FEEDING | Herbivores graze, predators hunt |
| TriggerZone_WaterSource_001 | (-400, 2600, 100) | WATERING | All species converge at dawn/dusk |
| TriggerZone_NestSite_001 | (3000, 500, 100) | NESTING | Aggressive defense radius active |

### T-Rex Patrol Circuit (3 waypoints)
| Label | Location | Notes |
|-------|----------|-------|
| PatrolWP_TRex_001 | (2000, 0, 100) | Entry point from savanna |
| PatrolWP_TRex_002 | (4000, 2000, 100) | Northern boundary |
| PatrolWP_TRex_003 | (3500, -1500, 100) | Southern sweep |

**T-Rex Behavior Rules:**
- Patrol radius: 5000 units
- Chase trigger: Player within 3000 units
- Attack trigger: Player within 300 units
- Patrol speed: 400 cm/s
- Chase speed: 900 cm/s

### Raptor Pack Patrol Circuit (4 waypoints)
| Label | Location | Notes |
|-------|----------|-------|
| PatrolWP_Raptor_001 | (-1000, 1500, 100) | Forest edge entry |
| PatrolWP_Raptor_002 | (-2000, 2500, 100) | Deep forest |
| PatrolWP_Raptor_003 | (-3000, 1000, 100) | Western boundary |
| PatrolWP_Raptor_004 | (-1500, 0, 100) | Return to ambush zone |

**Raptor Pack Behavior Rules:**
- Pack size: 3 raptors
- Flanking behavior: 2 distract, 1 attacks from side
- Chase trigger: Player within 2000 units
- Attack trigger: Player within 200 units
- Patrol speed: 600 cm/s
- Chase speed: 1200 cm/s

---

## NPC Behavior Architecture (Implemented via TargetPoints)

### Zone-Based AI System
Each `TriggerZone_*` actor represents a behavioral context:
1. **AMBUSH zones** — Predators enter stealth mode, wait for player proximity
2. **FEEDING zones** — Herbivores graze passively, predators hunt actively
3. **WATERING zones** — All species converge; temporary truce behavior possible
4. **NESTING zones** — Aggressive territorial defense, attack on sight

### Patrol Waypoint System
`PatrolWP_*` actors define movement circuits:
- T-Rex: Slow triangular sweep of savanna (5000u radius)
- Raptors: Tight forest-edge circuit with ambush zone integration
- Future: Herbivore herd migration path (8+ waypoints)

---

## Cumulative NPC Infrastructure (Cycles 010-017)

| Cycle | System Added |
|-------|-------------|
| 010 | T-Rex Territory zone, Raptor Ambush zone, Herbivore Grazing zone, Water Source zone |
| 011 | 4 behavior zone markers (Herd_Grazing, TRex_Territory, Raptor_Ambush, WaterSource) |
| 012 | 9 patrol waypoints (5 herbivore herd circuit + 4 raptor circuit) |
| 013 | 4 behavior trigger zones + 7 patrol waypoints (TRex 3pt + Raptor 4pt) |

**Total NPC actors in map:** ~25+ behavior-related actors

---

## Handoff to Agent #12 — Combat & Enemy AI

### What's Ready
- Patrol waypoint circuits for T-Rex and Raptor pack
- Behavior zone markers defining combat context areas
- Voice line assets for NPC survivor and elder tribeswoman
- Zone-based behavioral context system (Ambush/Feeding/Watering/Nesting)

### What Agent #12 Should Build On
1. **Combat state machine** — Use `TriggerZone_Ambush_001` as entry point for raptor combat
2. **T-Rex chase logic** — Use `PatrolWP_TRex_*` waypoints as patrol anchors
3. **Attack damage values** — T-Rex: 80 dmg/hit, Raptor: 25 dmg/hit
4. **Player detection** — Implement cone-of-vision + sound detection radius
5. **Pack coordination** — Raptors should flank: use `PatrolWP_Raptor_*` for flanking positions

### Recommended Combat Zones
- `TriggerZone_Ambush_001` at (-2500, 1800) → Raptor ambush combat initiation
- `TriggerZone_NestSite_001` at (3000, 500) → T-Rex aggressive defense combat
- `PatrolWP_TRex_001` at (2000, 0) → T-Rex patrol anchor for combat AI

---

*NPC Behavior Agent #11 — Cycle 017 complete*
