# NPC Behavior System — Cycle 010 Documentation
**Agent #11 — NPC Behavior Agent**
**Cycle:** PROD_CYCLE_AUTO_20260618_010

---

## Overview

This cycle completes the NPC Behavior infrastructure deployment in MinPlayableMap with:
1. **Daily Routine Anchor Points** — 8 zone markers for NPC schedule simulation
2. **Memory Trigger Zones** — 5 BoxTrigger volumes for behavioral state changes
3. **Patrol Waypoints** — 5 path nodes for tribal hunter patrol route
4. **Voice Lines** — 2 TTS samples for TribalHunter and ElderScout NPCs

---

## Daily Routine System

### Schedule Flow
```
SLEEP (22:00–06:00) → GATHER (06:00–10:00) → HUNT (10:00–14:00)
→ CRAFT (14:00–17:00) → SOCIAL (17:00–20:00) → SLEEP
```

### Anchor Points Deployed
| Label | Location | Activity | Light Color |
|-------|----------|----------|-------------|
| NPC_Camp_SleepZone | (-800,-800,100) | SLEEP | Deep Blue |
| NPC_Camp_CraftZone | (-600,-900,100) | CRAFT | Amber |
| NPC_Camp_SocialZone | (-700,-700,100) | SOCIAL | Warm Yellow |
| NPC_Forest_GatherZone | (-200,1500,100) | GATHER | Green |
| NPC_River_WaterZone | (1200,-200,100) | WATER | Blue |
| NPC_Savana_HuntZone | (2500,1000,100) | HUNT | Orange-Red |
| NPC_Savana_LookoutPost | (3000,500,200) | LOOKOUT | Bright Yellow |
| NPC_DangerZone_TRex | (4000,2000,100) | DANGER | Red |

---

## NPC Memory System

### Memory Zone Types
- **THREAT_SEEN** — NPC witnessed a predator; avoids zone for 10 minutes
- **FOOD_FOUND** — NPC found food source; returns to zone during GATHER phase
- **SAFE_ZONE** — Camp perimeter; NPCs retreat here when threatened
- **DANGER_ZONE** — High predator activity; NPCs avoid unless in HUNT phase with group
- **ALLY_DIED** — Traumatic event zone; NPCs show fear response when nearby

### Memory Zones Deployed
| Label | Location | Type | Scale |
|-------|----------|------|-------|
| NPCMem_ThreatZone_River | (1200,-200,150) | THREAT_SEEN | 800×800×300 |
| NPCMem_FoodZone_Forest | (-200,1500,150) | FOOD_FOUND | 600×600×300 |
| NPCMem_SafeZone_Camp | (-700,-750,150) | SAFE_ZONE | 1000×1000×300 |
| NPCMem_DangerZone_Savana | (3500,1500,150) | DANGER_ZONE | 1200×1200×300 |
| NPCMem_AllyDied_Clearing | (1800,800,150) | ALLY_DIED | 500×500×300 |

---

## Patrol System

### TribalHunter Patrol Route
5 waypoints forming a loop from camp to forest edge and back:
```
WP_01 (-700,-750) → WP_02 (200,200) → WP_03 (800,600)
→ WP_04 (400,1200) → WP_05 (-300,800) → WP_01
```
- Green PointLights (80 intensity, 200 radius) mark each waypoint
- Patrol active during HUNT phase (10:00–14:00)
- Hunter returns to camp if threat detected

---

## NPC Voice Lines

### TribalHunter
**Audio:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781805763044_TribalHunter.mp3
**Duration:** ~9s
**Trigger:** Player approaches HuntZone alone
**Text:** "Stay back! The great beast has been seen near the river crossing. Do not go alone. We hunt together, or we do not hunt at all."

### ElderScout
**Audio:** (generated this cycle)
**Duration:** ~9s
**Trigger:** Player first encounters raptor tracks
**Text:** "The raptors move in packs. Three, sometimes four. They circle before they strike. If you hear them calling to each other — run. Do not fight. Run."

---

## Behavior Tree Architecture (Conceptual)

### TribalHunter BT
```
Root (Selector)
├── [THREAT] Flee to SafeZone (if THREAT_SEEN memory active)
├── [SCHEDULE] Follow Daily Routine
│   ├── SLEEP phase → Move to SleepZone, idle
│   ├── GATHER phase → Move to GatherZone, collect resources
│   ├── HUNT phase → Patrol waypoints WP_01→WP_05
│   ├── CRAFT phase → Move to CraftZone, craft animation
│   └── SOCIAL phase → Move to SocialZone, interact with others
└── [IDLE] Stand at current position
```

### Raptor Pack BT
```
Root (Selector)
├── [ATTACK] Chase and attack if prey within 800 units
├── [FLANK] Coordinate with pack (3+ raptors)
│   ├── Alpha: Direct charge
│   ├── Beta: Flank left
│   └── Gamma: Flank right
├── [STALK] Follow prey at distance 800–2000 units
└── [PATROL] Roam territory (DangerZone_Savana)
```

---

## Integration Notes for Agent #12 (Combat AI)

The following zones are critical for combat AI integration:
- `NPC_DangerZone_TRex` — T-Rex aggro radius center
- `NPCMem_ThreatZone_River` — River crossing ambush point
- `NPCMem_AllyDied_Clearing` — High-tension combat zone
- `NPC_Savana_HuntZone` — Raptor pack territory overlap

Combat AI should reference these zone labels when setting up:
- Predator patrol territories
- Ambush trigger points
- Pack coordination nodes
- Player threat detection radii

---

## Cumulative NPC Infrastructure (Cycles 007–010)

| Cycle | Actors Added | System |
|-------|-------------|--------|
| 007 | 9 | T-Rex + Raptor behavior anchors |
| 008 | 5 | BT trigger zones |
| 009 | 5 | Behavior zone anchors (dim orange) |
| 010 | 18 | Daily routine + memory zones + patrol WPs |
| **Total** | **37** | **Full NPC behavior scaffold** |

---

## Next Steps for Agent #12 (Combat AI)

1. Use `NPC_DangerZone_TRex` as T-Rex aggro center (4000,2000)
2. Use `NPCMem_ThreatZone_River` as ambush trigger for raptor pack
3. Implement combat state machine referencing memory zone types
4. Connect patrol waypoints to actual AI controller movement
5. Add combat voice lines for NPC reactions to dinosaur attacks
