# NPC Behavior System — Cycle 020 (PROD_CYCLE_AUTO_20260619_002)

## Agent #11 — NPC Behavior Agent

---

## Overview

This document defines the complete NPC Behavior Tree architecture for the prehistoric survival game. All NPCs are autonomous agents with daily routines, memory systems, and reactive behaviors — they exist for themselves, not to serve the player.

---

## NPC Archetypes (Active in MinPlayableMap)

### NPC_Hunter_001
- **Role**: Skilled tracker, hunts medium prey (deer-analogues, small dinosaurs)
- **Daily Routine**: Dawn departure → forest edge → stalking zone → river → dusk return
- **Behavior States**: Idle → Patrol → Track → Stalk → Attack → Retreat → Rest
- **Fear Threshold**: Flees from T-Rex, Carnotaurus; fights Raptors if cornered
- **Memory**: Remembers last 3 predator sighting locations (48h retention)
- **Voice**: `NPC_Scout_Survivor.mp3` — tactical, clipped, survival-focused

### NPC_Gatherer_001
- **Role**: Resource specialist, collects plants, roots, water
- **Daily Routine**: Mid-morning departure → berry patches → root fields → water source → afternoon return
- **Behavior States**: Idle → Patrol → Gather → Carry → Flee → Rest
- **Fear Threshold**: Flees from ANY large predator; does not fight
- **Memory**: Remembers resource patch locations, depleted patches avoided for 24h
- **Social**: Calls out to nearby NPCs when predator spotted (alert propagation)

### NPC_Scout_001
- **Role**: Perimeter watcher, early warning system for camp
- **Daily Routine**: Wide perimeter sweep (N → NE → E → SE → S → return)
- **Behavior States**: Idle → Patrol → Observe → Alert → Signal → Flee
- **Fear Threshold**: Observes from distance, never engages predators
- **Memory**: Tracks predator movement patterns over 72h; predicts patrol routes
- **Voice**: `NPC_Elder_Survivor.mp3` — measured, experienced, strategic

---

## Behavior Tree State Machine

```
                    ┌─────────────┐
                    │    IDLE     │ ← Rest zone, night hours
                    └──────┬──────┘
                           │ Dawn trigger
                    ┌──────▼──────┐
                    │   PATROL    │ ← Follow waypoint chain
                    └──────┬──────┘
                           │ Resource/prey detected
              ┌────────────┼────────────┐
              │            │            │
       ┌──────▼─────┐ ┌───▼────┐ ┌────▼──────┐
       │   GATHER   │ │ TRACK  │ │  OBSERVE  │
       └──────┬─────┘ └───┬────┘ └────┬──────┘
              │            │            │
       ┌──────▼─────┐ ┌───▼────┐ ┌────▼──────┐
       │   CARRY    │ │ STALK  │ │   ALERT   │
       └──────┬─────┘ └───┬────┘ └────┬──────┘
              │            │            │
              └────────────┼────────────┘
                           │ Predator detected (any state)
                    ┌──────▼──────┐
                    │    FLEE     │ ← Priority override
                    └──────┬──────┘
                           │ Safe distance reached
                    ┌──────▼──────┐
                    │   RECOVER   │ ← Elevated heartrate, scan
                    └──────┬──────┘
                           │ Calm restored
                    └──── PATROL ──┘
```

---

## Memory System Design

### Short-Term Memory (Session)
- Last known predator position (updates every 2s when in sight)
- Current resource patch status (depleted/available)
- Player interaction history (neutral/hostile/helpful)

### Long-Term Memory (Persistent across day cycles)
- Predator territory boundaries (learned from sightings)
- Safe route preferences (paths with no predator encounters)
- Resource patch productivity (high-yield locations remembered)
- Social bonds (NPCs who helped during flee events gain trust)

### Memory Decay
- Predator sighting: 48h before location marked "uncertain"
- Resource depletion: 24h before patch re-checked
- Player hostility: 72h before NPC reverts to neutral stance

---

## Zone Architecture (Deployed in MinPlayableMap)

### BT State Zones (PointLight markers)
| Label | Color | Position | Purpose |
|-------|-------|----------|---------|
| BT_Zone_Idle_Camp | Green | (0, 0, 200) | Rest/sleep zone |
| BT_Zone_Patrol_River | Blue | (3000, 1000, 200) | River patrol corridor |
| BT_Zone_Alert_Perimeter | Yellow | (5000, 2000, 200) | High-alert boundary |
| BT_Zone_Flee_Escape | Orange | (-2000, 1500, 200) | Safe retreat destination |
| BT_Zone_Gather_Forest | Light Green | (1500, 3500, 200) | Resource gathering area |

### Patrol Waypoints (SpotLight markers, color-coded by NPC)
- **Hunter** (Orange-Red): 5 waypoints — camp to river loop
- **Gatherer** (Green): 5 waypoints — camp to forest resources
- **Scout** (Blue): 5 waypoints — wide perimeter sweep

---

## NPC Social Dynamics

### Alert Propagation
When any NPC spots a predator:
1. Emit `ALERT` signal (radius 2000 units)
2. All NPCs within radius enter `ALERT` state
3. NPCs share last known predator position via blackboard
4. Camp NPCs move to `BT_Zone_Flee_Escape` as priority

### Player Interaction Rules
- **Neutral**: NPC ignores player, continues routine
- **Player approaches within 300 units**: NPC pauses, watches (1-3s), resumes
- **Player gives food**: NPC trust +1, may share information (voice line trigger)
- **Player attacks NPC**: All nearby NPCs enter `FLEE`, player marked hostile for 72h
- **Player helps NPC flee predator**: NPC trust +2, may follow player briefly

### Inter-NPC Relations
- Hunter + Scout: High trust (complementary roles), share patrol intel
- Hunter + Gatherer: Medium trust (resource competition)
- Scout + Gatherer: High trust (Scout warns Gatherer of predators)

---

## Voice Lines Deployed

### NPC_Scout_Survivor (~15s)
> *"Stay low. Move slow. The raptors hunt by movement — if you freeze when they look your way, they lose you. Three seconds. That is all you have before they charge. Use the rocks. Use the trees. Never run in a straight line."*
- URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781834177398_NPC_Scout_Survivor.mp3`
- Trigger: Player approaches Scout within 500 units for first time

### NPC_Elder_Survivor (~14s)
> *"The big one has a territory. A circle, maybe half a day's walk across. It patrols the same path every dawn. Learn the path. Memorize it. That is the difference between finding food and becoming food."*
- URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781834179758_NPC_Elder_Survivor.mp3`
- Trigger: Player approaches Elder/Hunter within 500 units after first T-Rex encounter

---

## Integration with Animation System (Agent #10 handoff)

The following animation zones from Agent #10 map directly to BT states:
- `AnimZone_Safe_001` → `BT_Zone_Idle_Camp` (Idle animations)
- `AnimZone_Combat_001` → `BT_Zone_Alert_Perimeter` (Alert/Flee animations)
- `AnimMarker_Walk_Path` → Patrol waypoint transitions (Walk blend)
- `AnimMarker_Run_Zone` → Flee state (Run blend, max speed)
- `AnimMarker_Crouch_Zone` → Stalk/Gather states (Crouch blend)

---

## Handoff to Agent #12 — Combat & Enemy AI

### What Agent #12 receives from this cycle:
1. **BT Zone positions** — Alert perimeter at (5000, 2000) defines combat engagement boundary
2. **NPC flee destination** — `BT_Zone_Flee_Escape` at (-2000, 1500) is the safe zone combat AI must respect
3. **NPC fear thresholds** — Hunter flees T-Rex/Carno, fights Raptors; Gatherer flees all; Scout observes all
4. **Alert propagation radius** — 2000 units; combat AI dinosaurs should trigger NPC alerts when entering this radius
5. **Player hostility flag** — If player attacks NPC, combat AI can use this flag to coordinate dinosaur aggression

### Combat AI should implement:
- Dinosaur `HUNT` state triggers NPC `FLEE` state (via shared blackboard or proximity)
- T-Rex patrol path intersects `PatrolWP_Hunter_04_River` — creates dynamic encounter risk
- Raptor pack spawns near `BT_Zone_Alert_Perimeter` for maximum tension
- Combat zones must NOT overlap `BT_Zone_Idle_Camp` (camp is safe sanctuary)

---

## Files in this cycle
- `Docs/NPC/NPCBehaviorSystem_Cycle020.md` — this document
- UE5 actors deployed: 5 BT zones + 15 patrol waypoints = 20 actors total
- Voice lines: 2 NPC voice samples (Scout + Elder)
