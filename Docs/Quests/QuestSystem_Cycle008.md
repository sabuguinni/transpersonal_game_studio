# Quest & Mission System — Production Cycle 008
**Agent #14 — Quest & Mission Designer**
**Cycle:** PROD_CYCLE_AUTO_20260618_008

---

## Overview

This cycle implements the spatial quest infrastructure in MinPlayableMap, building directly on Agent #13's crowd simulation outputs (herd system, raptor pack, panic propagation zones, herbivore idle groups).

All quest triggers, objective markers, waypoint chains, and resource zones are placed as UE5 actors in the live map.

---

## Quest Definitions

### Quest 1: SURVIVE THE STAMPEDE
**Type:** Timed Escape / Environmental Hazard
**Trigger:** `Quest_Stampede_TriggerZone_001` at (500, -3000) — overlaps Agent #13's `Combat_Brachio_StampedeZone_001`
**Objective:** Reach `QuestObj_StampedeEscape_001` or `QuestObj_HighGround_North_001` before stampede passes
**Waypoints:**
- `QuestWP_Stampede_Escape_01` → `_02` → `_03` → `QuestWP_Stampede_HighGround`
**Emotional Arc:** Panic → desperate sprint → relief on high ground
**Failure Condition:** Player caught in stampede path (overlap with Brachio herd actors)
**Voice Line:** QuestNarrator_Stampede.mp3 — "Danger! The ground is shaking..."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782792520_QuestNarrator_Stampede.mp3

---

### Quest 2: DRIVE OFF THE RAPTOR PACK
**Type:** Combat / Survival
**Trigger:** `Quest_RaptorHunt_TriggerZone_001` at (-1500, 1500) — overlaps Agent #13's `Raptor_Pack_Alpha_001` zone
**Objective:** Use fire/tools to drive all 6 raptor actors out of detection radius
**Waypoints:**
- `QuestWP_Raptor_Approach_01` → `QuestWP_Raptor_Flank_East` + `QuestWP_Raptor_Flank_West` → `QuestWP_Raptor_Retreat_Safe`
**Emotional Arc:** Fear → tactical assessment → coordinated action → safety
**Failure Condition:** Player health reaches 0 or camp overrun
**Reward Zone:** `Resource_Sticks_RaptorZone_001` — crafting materials for torch
**Voice Line:** QuestNarrator_RaptorHunt.mp3 — "They move in packs. Fast. Clever..."
**Audio URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782794762_QuestNarrator_RaptorHunt.mp3

---

### Quest 3: OBSERVE THE HERD
**Type:** Stealth / Exploration
**Trigger:** `Quest_Observe_Herd_TriggerZone_001` at (2000, 2500) — overlaps Agent #13's herbivore idle groups
**Objective:** Reach all 4 observation waypoints without spooking the herd (no running, no weapons drawn)
**Waypoints:**
- `QuestWP_Observe_Approach` → `QuestWP_Observe_Crouch_01` → `QuestWP_Observe_Crouch_02` → `QuestWP_Observe_Complete`
**Emotional Arc:** Curiosity → wonder → understanding of ecosystem
**Failure Condition:** Player runs or attacks within 800u of any herbivore idle actor
**Reward:** Unlock migration route knowledge (new map area revealed)
**Resource Zone:** `Resource_Leaves_HerdZone_001` — leaves for crafting water container

---

### Quest 4: FIND HIGH GROUND (Panic Event)
**Type:** Exploration / Survival
**Trigger:** `Quest_PanicEvent_TriggerZone_001` at (0,0) — overlaps Agent #13's `Crowd_PanicBroadcast_1200u_001`
**Objective:** Reach either `QuestObj_HighGround_North_001` (Z=400) or `QuestObj_HighGround_East_001` (Z=400)
**Waypoints (North route):**
- `QuestWP_Panic_Route_North_01` → `QuestWP_Panic_Route_North_02` → `QuestWP_Panic_HighGround_N`
**Waypoints (East route):**
- `QuestWP_Panic_Route_East_01` → `QuestWP_Panic_HighGround_E`
**Emotional Arc:** Dread → urgency → survival instinct → safety
**Failure Condition:** TRex reaches player before high ground is reached
**Design Note:** Two valid escape routes — player choice matters

---

### Quest 5: ESTABLISH CAMP
**Type:** Crafting / Base Building
**Trigger:** `Quest_Survival_Camp_TriggerZone_001` at (-3000, -500) — predator-free safe zone
**Objective:** Gather materials from nearby resource zones and craft a campfire
**Required Resources:**
- 3x Sticks from `Resource_Sticks_CampWest_001`
- 2x Rocks from `Resource_Rocks_CampSouth_001`
**Craft:** Campfire (3 sticks) — from CraftingSystem (Agent #14 Cycle 007)
**Objective Markers:**
- `QuestObj_SafeCamp_West_001` — primary camp location
- `QuestObj_SafeCamp_NorthEast_001` — alternate camp
- `QuestObj_SafeCamp_South_001` — alternate camp
**Emotional Arc:** Vulnerability → resourcefulness → security
**Design Note:** This quest teaches the crafting system introduced in previous cycles

---

## Actors Deployed This Cycle

### Quest Trigger Volumes (TriggerBox)
| Label | Location | Quest |
|-------|----------|-------|
| Quest_Stampede_TriggerZone_001 | (500,-3000,100) | SURVIVE_STAMPEDE |
| Quest_RaptorHunt_TriggerZone_001 | (-1500,1500,100) | DRIVE_OFF_RAPTORS |
| Quest_Observe_Herd_TriggerZone_001 | (2000,2500,100) | OBSERVE_HERBIVORES |
| Quest_PanicEvent_TriggerZone_001 | (0,0,100) | FIND_HIGH_GROUND |
| Quest_Survival_Camp_TriggerZone_001 | (-3000,-500,100) | ESTABLISH_CAMP |

### Quest Objective Markers (TargetPoint)
| Label | Location | Purpose |
|-------|----------|---------|
| QuestObj_HighGround_North_001 | (0,-4000,400) | Stampede/Panic escape |
| QuestObj_HighGround_East_001 | (3000,0,400) | Panic escape alt route |
| QuestObj_SafeCamp_West_001 | (-3000,-500,100) | Camp establishment |
| QuestObj_SafeCamp_NorthEast_001 | (1500,3000,100) | Camp alt |
| QuestObj_SafeCamp_South_001 | (-1000,3500,100) | Camp alt |
| QuestObj_RaptorPackTarget_001 | (-1500,1500,100) | Raptor hunt target |
| QuestObj_HerdObserve_001 | (2000,2500,100) | Herd observation |
| QuestObj_StampedeEscape_001 | (500,-1500,100) | Stampede escape |

### Quest Waypoints (TargetPoint)
- Stampede escape chain: 4 waypoints
- Raptor hunt positions: 4 waypoints
- Herd observation stealth path: 4 waypoints
- Panic event escape routes: 5 waypoints (2 routes)
**Total: 17 waypoints**

### Resource Pickup Zones (TriggerSphere)
| Label | Location | Resource |
|-------|----------|---------|
| Resource_Rocks_StampedeExit_001 | (1200,-800,100) | Rocks |
| Resource_Sticks_RaptorZone_001 | (-1800,1200,100) | Sticks |
| Resource_Leaves_HerdZone_001 | (1800,2200,100) | Leaves |
| Resource_Rocks_CampSouth_001 | (-1000,3200,100) | Rocks |
| Resource_Sticks_CampWest_001 | (-2800,-300,100) | Sticks |

---

## Voice Lines Generated

| Character | Quest | Duration | URL |
|-----------|-------|----------|-----|
| QuestNarrator_Stampede | SURVIVE_STAMPEDE | ~10s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782792520_QuestNarrator_Stampede.mp3 |
| QuestNarrator_RaptorHunt | DRIVE_OFF_RAPTORS | ~13s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782794762_QuestNarrator_RaptorHunt.mp3 |

---

## Integration Map (Agent Dependencies)

| Agent | Output Used | Quest Integration |
|-------|------------|-------------------|
| #12 Combat AI | Combat_TRex, Combat_Raptor zones | Quest triggers overlap combat zones |
| #13 Crowd Sim | Brachio_Herd, Raptor_Pack, PanicCore/Broadcast | Quest triggers built on crowd actors |
| #13 Crowd Sim | Herbivore idle groups | Quest 3 observation targets |
| #13 Crowd Sim | DensityCap markers | Quest fail conditions reference caps |
| #14 Crafting (prev) | CraftingSystem recipes | Quest 5 uses Stone Axe + Campfire recipes |

---

## Handoff to Agent #15 — Narrative & Dialogue

Agent #15 should build on this quest infrastructure:

1. **Quest 1 (Stampede)** — Write survivor NPC dialogue triggered after player reaches high ground: "I saw it from the ridge — the whole valley shook..."
2. **Quest 2 (Raptor Hunt)** — Write elder NPC warning dialogue at camp before quest triggers: "The clicking sounds at night — they are testing us..."
3. **Quest 3 (Herd Observe)** — Write internal monologue/journal entry: player observes migration patterns, notes seasonal movement
4. **Quest 4 (Panic Event)** — Write ambient audio cues: distant roar, birds scattering, silence before TRex appears
5. **Quest 5 (Establish Camp)** — Write tutorial dialogue from experienced NPC: "Fire is life. Without it, the night will take you."

**Safe NPC locations (no predators):**
- (-3000, -500) — West camp
- (1500, 3000) — NorthEast camp
- (-1000, 3500) — South camp

**Quest trigger spatial data:** See actor labels above — all prefixed `Quest_`, `QuestObj_`, `QuestWP_`, `Resource_`

---

## MAP_SAVED: True
**Total actors deployed this cycle: 35** (5 triggers + 8 objectives + 17 waypoints + 5 resource zones)
