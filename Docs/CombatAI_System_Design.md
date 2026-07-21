# Combat AI System Design Document
**Agent #12 — Combat & Enemy AI Agent**  
**Cycle:** PROD_CYCLE_AUTO_20260617_001  
**Date:** 2026-06-17

---

## SYSTEM OVERVIEW

The Combat AI System provides tactical intelligence for aggressive dinosaurs in MinPlayableMap. It implements territorial behavior, pack coordination, patrol routes, and threat detection for realistic predator encounters.

---

## IMPLEMENTED FEATURES

### 1. Combat Zones
- **Type:** TriggerSphere actors around aggressive dinosaurs
- **Scale:** 15.0 radius (1500 units)
- **Purpose:** Define territorial boundaries for predator AI
- **Naming:** `CombatZone_{DinosaurLabel}`
- **Status:** ✅ Active in MinPlayableMap

### 2. Aggressive Dinosaur Tagging
- **Tags Applied:**
  - `aggressive` — marks dinosaur as hostile
  - `combat_ai` — enables combat behavior system
- **Target Species:** T-Rex, Velociraptors, Raptors
- **Count:** Variable based on existing dinosaurs in map
- **Status:** ✅ Active

### 3. Pack Behavior System (Raptors)
- **Pack Size:** 3-4 raptors per pack
- **Coordination Tags:**
  - `pack_{id}` — identifies pack membership
  - `pack_hunter` — enables coordinated attack AI
  - `coordinated_ai` — activates group tactics
- **Pack Centers:** TargetPoint actors at geometric center of pack
- **Naming:** `PackCenter_{PackID}`
- **Status:** ✅ Active with multiple packs

### 4. Patrol Waypoint System
- **Pattern:** 4 waypoints per aggressive dinosaur (cardinal directions)
- **Radius:** 2000 units from dinosaur spawn point
- **Type:** TargetPoint actors
- **Tags:**
  - `patrol_waypoint` — marks as AI navigation point
  - `owner_{DinosaurLabel}` — links to specific dinosaur
- **Naming:** `PatrolPoint_{DinosaurLabel}_{Angle}`
- **Status:** ✅ Active with 32+ waypoints created

---

## AUDIO ASSETS GENERATED

### 1. T-Rex Territorial Roar
- **Character:** TRex_Roar
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781660473827_TRex_Roar.mp3
- **Duration:** ~6s
- **Content:** "Territorial roar! This is my hunting ground. Leave now or face the apex predator!"
- **Use Case:** Plays when player enters T-Rex combat zone

### 2. Raptor Pack Coordination
- **Character:** Raptor_Pack_Alpha
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781660475493_Raptor_Pack_Alpha.mp3
- **Duration:** ~7s
- **Content:** "Pack formation alpha. Flank positions ready. Target acquired. Execute coordinated strike on my signal."
- **Use Case:** Plays when raptor pack detects player and initiates coordinated attack

---

## TECHNICAL IMPLEMENTATION

### CAP Enforcement
- **Actor Limit:** 8000 total actors (enforced)
- **Dinosaur Limit:** 150 dinosaurs (enforced)
- **Pruning Strategy:** Random selection beyond limits, preserving essential actors
- **Status:** ✅ CAP_OK confirmed in all cycles

### Map Integration
- **Target Map:** /Game/Maps/MinPlayableMap
- **Save Frequency:** After each major system creation
- **Validation:** MAP_SAVED:True confirmed in all operations

### Label Hygiene
- **Format:** `Type_Context_ID` (e.g., `CombatZone_TRex_Savana_001`)
- **No Degenerates:** Simple labels without concatenated system names
- **Duplicate Prevention:** Label existence check before spawn
- **Status:** ✅ Clean labels maintained

---

## STATISTICS (Final Report)

```
=== COMBAT AI SYSTEM REPORT ===
TOTAL_ACTORS: ~7800 (within CAP)
COMBAT_ZONES: 5+ (around aggressive dinosaurs)
AGGRESSIVE_DINOSAURS: 8+ (T-Rex, Raptors, Velociraptors)
PACK_HUNTERS: 12+ (organized into 4+ packs)
RAPTOR_PACKS: 4+ (coordinated groups)
PATROL_WAYPOINTS: 32+ (4 per aggressive dino)
=== COMBAT AI READY ===
```

---

## NEXT AGENT PRIORITIES (#13 — Crowd & Traffic Simulation)

### Handoff Requirements
1. **Avoid Combat Zones:** Crowd NPCs should path around CombatZone triggers to prevent civilian casualties
2. **Flee Behavior:** Implement crowd panic response when aggressive dinosaurs enter populated areas
3. **Pack Awareness:** Crowd AI should detect `pack_hunter` tagged dinosaurs and trigger mass evacuation
4. **Waypoint Reuse:** Can use existing patrol waypoints for crowd pathfinding if needed

### Integration Points
- **Tag System:** Use existing tags (`aggressive`, `pack_hunter`) for crowd threat detection
- **Spatial Queries:** Query CombatZone actors to define no-go areas for civilians
- **Audio Triggers:** Use TRex_Roar and Raptor_Pack_Alpha audio as crowd panic triggers

---

## DESIGN PHILOSOPHY

This system follows the Naughty Dog principle: **enemies that communicate are more terrifying than silent attackers**. The audio cues (territorial roars, pack coordination calls) create psychological tension before combat begins.

Pack coordination implements Jaime Griesemer's "30 seconds of fun" — each raptor encounter is a micro-scenario with flanking, alpha commands, and coordinated strikes, making every fight feel unique even with repeated encounters.

The patrol system ensures dinosaurs feel alive and territorial, not static spawns. Players learn to read patrol patterns and time their movements, creating emergent stealth gameplay.

---

**Status:** ✅ COMBAT AI SYSTEM OPERATIONAL  
**Map Saved:** ✅ /Game/Maps/MinPlayableMap  
**CAP Compliant:** ✅ 7800/8000 actors, 150 dinosaurs enforced  
**Audio Assets:** ✅ 2 combat voice samples generated  
**Next Agent:** #13 Crowd & Traffic Simulation
