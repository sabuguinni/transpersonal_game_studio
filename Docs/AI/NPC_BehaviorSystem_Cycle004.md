# NPC Behavior System — Agent #11 Production Cycle 004

## Overview
This document defines the NPC behavior architecture for the Transpersonal Game Studio prehistoric survival game.
All NPCs are primitive human tribespeople living in the prehistoric world alongside dinosaurs.

---

## NPC Types Implemented (MinPlayableMap)

### 1. Gatherer (NPC_Tribesman_Camp_001)
- **Location**: Camp zone (-1200, 800)
- **Behavior**: Patrol camp perimeter → collect resources → return to camp
- **Threat Response**: Flee to camp center when dinosaur detected within 2000 units
- **Light Marker**: Green (placeholder until mesh available)

### 2. Guard (NPC_Tribesman_Camp_002)
- **Location**: Camp zone (-1400, 600)
- **Behavior**: Patrol camp boundary → alert others on threat detection → hold position
- **Threat Response**: Stand ground, alert other NPCs, throw spear if threat < 500 units
- **Light Marker**: Blue (placeholder)

### 3. Hunter x2 (NPC_Tribesman_Hunt_001/002)
- **Location**: Hunting zone (2500-2700, -1800 to -2000)
- **Behavior**: Track prey → stalk → ambush → return to camp with kill
- **Threat Response**: Retreat when T-Rex detected, continue hunt for smaller prey
- **Light Marker**: Orange-red (placeholder)

### 4. Elder (NPC_Elder_Camp_001)
- **Location**: Camp center (-1000, 1000)
- **Behavior**: Stationary at camp fire → respond to player approach → give survival tips
- **Threat Response**: Move to shelter, direct others to flee
- **Light Marker**: Gold (placeholder)

---

## Behavior Zones

| Zone Label | Center | Radius | Purpose |
|---|---|---|---|
| BehaviorZone_NPC_Camp | (-1200, 800) | 800 units | Camp safe zone, NPC home base |
| BehaviorZone_NPC_Hunt | (2600, -1900) | 1200 units | Active hunting grounds |

---

## Patrol Waypoints

### Camp Patrol Route (4 waypoints)
```
NPC_Waypoint_Camp_A → (-1000, 700)
NPC_Waypoint_Camp_B → (-1300, 500)
NPC_Waypoint_Camp_C → (-1500, 900)
NPC_Waypoint_Camp_D → (-1100, 1100)
```

### Hunt Patrol Route (4 waypoints)
```
NPC_Waypoint_Hunt_A → (2300, -1600)
NPC_Waypoint_Hunt_B → (2800, -1700)
NPC_Waypoint_Hunt_C → (3000, -2100)
NPC_Waypoint_Hunt_D → (2500, -2300)
```

---

## Behavior Tree Architecture (Design — Blueprint Implementation)

### NPC_BT_Tribesman (Base Behavior Tree)
```
Root (Selector)
├── Flee_Sequence (Sequence) [Priority: HIGHEST]
│   ├── BTTask_DetectThreat (Dinosaur within FleeRadius=2000)
│   └── BTTask_FleeToSafeZone (Run to camp center)
│
├── Alert_Sequence (Sequence) [Priority: HIGH]
│   ├── BTTask_DetectThreat (Dinosaur within AlertRadius=3000)
│   └── BTTask_AlertNearbyNPCs (Broadcast alert to 1500 unit radius)
│
├── Role_Behavior (Selector) [Priority: MEDIUM]
│   ├── [Gatherer] BTTask_GatherResources → BTTask_ReturnToCache
│   ├── [Guard]    BTTask_PatrolBoundary → BTTask_ScanForThreats
│   ├── [Hunter]   BTTask_TrackPrey → BTTask_StalkPrey → BTTask_Attack
│   └── [Elder]    BTTask_IdleAtFirepit → BTTask_RespondToPlayer
│
└── Idle_Sequence (Sequence) [Priority: LOW]
    ├── BTTask_PatrolWaypoints (Follow assigned waypoint route)
    └── BTTask_IdleAnimation (Sit, look around, tend fire)
```

---

## Blackboard Variables (NPC_BB_Tribesman)

| Variable | Type | Description |
|---|---|---|
| TargetLocation | Vector | Current movement destination |
| ThreatActor | Actor | Detected threat (dinosaur/predator) |
| ThreatDistance | Float | Distance to nearest threat |
| bIsAlarmed | Bool | Alert state active |
| bIsInSafeZone | Bool | Currently inside camp zone |
| HomeLocation | Vector | NPC's home/spawn location |
| PatrolIndex | Int | Current waypoint index |
| CurrentRole | Enum | Gatherer/Guard/Hunter/Elder |
| PlayerDetected | Bool | Player is within interaction range |
| ResourceTarget | Actor | Resource node being gathered |

---

## NPC Interaction System

### Player Proximity Responses
- **< 500 units**: NPC acknowledges player (look-at, gesture)
- **< 200 units**: NPC enters dialogue mode (Elder gives tips, Hunters warn of dangers)
- **< 100 units**: NPC stops patrol, faces player fully

### Dialogue Triggers (Elder NPC)
1. First encounter: "You are not from this tribe. The land is dangerous — stay near the fire at night."
2. After player survives T-Rex: "You have the spirit of a survivor. The great lizard fears nothing — but it can be outrun."
3. When player has no food: "The river valley has fish. The forest has roots. Hunger makes you slow — and slow means dead."

---

## TTS Audio Assets Generated

| File | URL | Duration | Usage |
|---|---|---|---|
| DinoNarrator_Brachio.mp3 | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781761955374_DinoNarrator_Brachio.mp3 | ~14s | Brachiosaurus herd warning |
| DinoNarrator_Survival.mp3 | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781761970000_DinoNarrator_Survival.mp3 | ~14s | Fire/survival tip narration |

---

## Dependencies for Next Agent (#12 — Combat & Enemy AI)

1. **Behavior zones are placed** — Combat AI can use BehaviorZone_NPC_Camp as a "protected zone" that dinosaurs should not enter (or enter only when aggressive)
2. **NPC flee behavior** — When dinosaur enters camp zone, NPCs flee. Combat AI should trigger this by broadcasting a "ThreatEntered" event
3. **Hunter NPCs** — Hunters in BehaviorZone_NPC_Hunt should react to dinosaur AI: small prey = hunt, large predator = retreat
4. **Patrol waypoints** — NPC_Waypoint_Hunt_* waypoints can double as dinosaur patrol interference points

## Next Cycle Priorities
- Implement Blueprint Behavior Tree assets (BT_Tribesman, BB_Tribesman) via UE5 Python
- Add NavMesh-aware movement to NPC actors
- Connect Elder NPC to dialogue system when player approaches
