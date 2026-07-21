# Quest & Mission System — Agent #14 Design Document
**Version:** 1.0  
**Author:** Quest & Mission Designer Agent #14  
**Cycle:** PROD_CYCLE_AUTO_20260702_012  

---

## Overview

The quest system converts narrative beats from Agent #15 (Narrative & Dialogue) into concrete, playable survival objectives. Every quest has a full emotional arc — setup, tension, resolution — grounded in realistic prehistoric survival mechanics.

**Design Philosophy:**  
A quest that the player completes without feeling anything has failed, regardless of how technically correct it is. Each mission is a piece of theatre where the player is the lead actor and the world responds to their choices.

---

## Active Quests in MinPlayableMap

### QUEST 001 — "Track the Migration"
**Type:** Exploration / Tracking  
**Quest Giver:** `NPC_Elder_Camp_001` (at camp edge, coordinates: 300, -200, 100)  
**Trigger Zone:** `QuestTrigger_Migration_001` (coordinates: 1200, 1200, 120)  

**Setup:**  
The Elder tells the player that the herd moves south before the rains. Following the migration corridor will reveal the old hunting grounds used by their ancestors — rich in resources, but dangerous.

**Objectives:**
1. Speak to the Elder at camp
2. Find the migration entry point (reach `QuestTrigger_Migration_001`)
3. Follow the 5 waypoints: `MigPath_Savana_001` → `MigPath_Savana_005`
4. Reach the river crossing at `QuestTrigger_Gather_001` (coordinates: 3200, 800, 120)
5. Discover the old hunting grounds

**Emotional Beat:**  
Discovery and wonder. The player sees the scale of the prehistoric world — thousands of animals moving as one. The landscape opens up. This is the world they must survive in.

**Rewards:**
- Unlock `Zone_RiverCrossing` area
- Receive map fragment showing T-Rex territory
- +50 Reputation with Elder faction

**Integration with Agent #13 (Crowd Sim):**  
- Migration waypoints `MigPath_Savana_001–005` are live in the scene
- Herd scatter event fires when player approaches within 1500 units of predator
- Quest dialogue triggers: "The herd is spooked — something big is nearby"

---

### QUEST 002 — "Cause a Stampede"
**Type:** Combat Strategy / Environmental Puzzle  
**Quest Giver:** `NPC_Elder_Camp_001`  
**Trigger Zone:** `QuestTrigger_Stampede_001` (coordinates: 2400, 2200, 120)  
**Prerequisite:** Quest 001 complete  

**Setup:**  
A T-Rex has been terrorising the camp's hunting routes. The Elder knows that the Triceratops herd can be used as a weapon — if the player can spook them at the right moment, the stampede will drive the predator away.

**Objectives:**
1. Locate the Triceratops herd (`Trike_Savana_001–006`)
2. Observe the T-Rex patrol route (stealth approach)
3. Wait for the T-Rex to enter the herd's scatter radius (2000 units)
4. Fire a weapon near the herd to trigger stampede
5. Confirm T-Rex has fled the zone

**Emotional Beat:**  
Cunning over brute force. The player learns that survival is about reading the ecosystem, not fighting it directly. The stampede is terrifying and beautiful — a force of nature the player has unleashed.

**Mechanical Notes:**
- Stampede trigger: player fires weapon within 1500 units of any `Trike_Savana_*` actor
- Scatter direction: away from nearest predator with `Predator` tag
- T-Rex flee condition: stampede within 2000 units → 60-second flee behaviour
- Crowd sim integration: `CrowdSim_HerdScatter` tag activates on all Trike actors

**Rewards:**
- Unlock `NPC_HunterChief_Forest_001` as quest giver
- Receive Bone Spear recipe
- T-Rex territory temporarily cleared (15 in-game minutes)

---

### QUEST 003 — "Gather After the Hunt"
**Type:** Resource Gathering / Survival  
**Quest Giver:** `NPC_HunterChief_Forest_001` (forest edge, coordinates: 800, 600, 100)  
**Trigger Zone:** `QuestTrigger_Gather_001`  
**Prerequisite:** Quest 002 complete  

**Setup:**  
The Hunter Chief explains that nothing is wasted. The T-Rex's prey left behind bone, hide, and sinew — materials needed to craft advanced tools. But the carcass won't last long before scavengers arrive.

**Objectives:**
1. Reach the bone pile at `QuestObj_BonePile_001` (coordinates: 2600, 1900, 60)
2. Collect 3x Bone from the carcass
3. Collect 2x Hide from `QuestObj_HideCache_001` (coordinates: 2700, 2100, 60)
4. Return to `CraftStation_Camp_001` before scavengers arrive (time limit: 5 in-game minutes)
5. Craft a Stone Axe using: 2x Rock + 1x Stick (resources scattered nearby)

**Emotional Beat:**  
Urgency and resourcefulness. The player is racing against the ecosystem — scavengers are real threats, not just timers. Success feels earned because the world was genuinely dangerous.

**Resource Actors in Scene:**
- Rocks: `Resource_Rock_Savana_001–006`
- Sticks: `Resource_Stick_Savana_001–005`
- Leaves: `Resource_Leaf_Savana_001–003`
- Crafting station: `CraftStation_Camp_001`

**Rewards:**
- Stone Axe (inventory item)
- Unlock advanced crafting recipes
- +75 Reputation with Hunter Chief faction

---

### QUEST 004 — "Defend the Camp" (Planned)
**Type:** Defense / Combat  
**Quest Giver:** `NPC_HunterChief_Forest_001`  
**Status:** Design complete, awaiting Agent #12 (Combat AI) implementation  

**Setup:**  
Raptors have been scouting the camp perimeter. The Hunter Chief needs the player to set traps and defend through the night.

**Objectives:**
1. Craft 3x Spike Trap using gathered resources
2. Place traps at camp entry points (3 designated locations)
3. Survive 3 raptor attack waves (night cycle)
4. Repair camp defences if damaged

**Integration Requirements:**
- Agent #12 (Combat AI): Raptor pack attack behaviour, wave spawning
- Agent #08 (Lighting): Night cycle for atmosphere
- Agent #16 (Audio): Raptor calls, camp fire crackling, attack sounds

---

## Crafting System — Recipe Definitions

### Recipe 001: Stone Axe
- **Ingredients:** 2x Rock + 1x Stick
- **Craft Time:** 3 seconds
- **Station:** `CraftStation_Camp_001` (any campfire)
- **Result:** Stone Axe (melee weapon, 25 damage, 40 durability)
- **Quest Link:** Quest 003 objective

### Recipe 002: Campfire
- **Ingredients:** 3x Stick
- **Craft Time:** 5 seconds
- **Station:** Open ground (no station required)
- **Result:** Campfire actor (warmth, cooking, light source, scares small predators)

### Recipe 003: Water Container
- **Ingredients:** 1x Rock + 1x Leaf
- **Craft Time:** 4 seconds
- **Station:** Near water source
- **Result:** Water Container (holds 3 drinks, refillable at rivers)

### Recipe 004: Bone Spear (Unlocked by Quest 002)
- **Ingredients:** 2x Bone + 1x Stick + 1x Hide Strip
- **Craft Time:** 8 seconds
- **Station:** `CraftStation_Camp_001`
- **Result:** Bone Spear (ranged/melee, 45 damage, 20 durability)

---

## NPC Quest Givers

### NPC_Elder_Camp_001
- **Location:** Camp edge (300, -200, 100)
- **Role:** Gives Quests 001 and 002
- **Voice:** Calm, experienced, speaks in observations about nature
- **Audio Asset:** `tts/1783017619664_QuestGiver_Elder.mp3`
- **Dialogue Style:** Uses animal behaviour as metaphors for survival wisdom

### NPC_HunterChief_Forest_001
- **Location:** Forest edge (800, 600, 100)
- **Role:** Gives Quests 003 and 004
- **Voice:** Direct, pragmatic, rewards competence
- **Audio Asset:** `tts/1783017636516_QuestGiver_HunterChief.mp3`
- **Dialogue Style:** Practical instructions, no sentiment — survival is the only metric

---

## Integration Contracts

### → Agent #12 (Combat & Enemy AI)
- Quest 002 requires: T-Rex flee behaviour when stampede within 2000 units
- Quest 004 requires: Raptor wave spawning system (3 waves, escalating difficulty)
- Shared actors: `TRex_*` and `Raptor_*` actors (do NOT duplicate — reference by label)

### → Agent #13 (Crowd & Traffic Simulation)
- Quest 001 uses: `MigPath_Savana_001–005` waypoints (live in scene)
- Quest 002 uses: Herd scatter trigger on `Trike_Savana_001–006`
- Stampede mechanic: fires when player weapon discharge within 1500 units of herd

### → Agent #15 (Narrative & Dialogue)
- Quest dialogue lines recorded and available:
  - Elder: `tts/1783017619664_QuestGiver_Elder.mp3`
  - Hunter Chief: `tts/1783017636516_QuestGiver_HunterChief.mp3`
- Full dialogue scripts needed for: quest accept, objective update, quest complete

### → Agent #16 (Audio)
- Quest completion fanfare needed (non-musical — tribal drum beat)
- Ambient audio for quest zones: river crossing, bone pile, herd zone
- NPC voice lines integrated via MetaSounds

### → Agent #17 (VFX)
- Stampede dust cloud effect needed at `QuestTrigger_Stampede_001`
- Resource pickup sparkle effect for all `Resource_*` actors
- Quest objective completion burst effect

---

## Scene Actor Registry (Quest System)

| Actor Label | Coordinates | Tags | Purpose |
|---|---|---|---|
| NPC_Elder_Camp_001 | 300, -200, 100 | NPC, QuestGiver | Quest 001+002 giver |
| NPC_HunterChief_Forest_001 | 800, 600, 100 | NPC, QuestGiver | Quest 003+004 giver |
| QuestTrigger_Migration_001 | 1200, 1200, 120 | QuestTrigger | Quest 001 entry |
| QuestTrigger_Stampede_001 | 2400, 2200, 120 | QuestTrigger | Quest 002 zone |
| QuestTrigger_Gather_001 | 3200, 800, 120 | QuestTrigger | Quest 003 zone |
| CraftStation_Camp_001 | 150, -100, 60 | CraftStation | Crafting anchor |
| QuestObj_BonePile_001 | 2600, 1900, 60 | QuestObjective | Quest 003 collect |
| QuestObj_HideCache_001 | 2700, 2100, 60 | QuestObjective | Quest 003 collect |
| Resource_Rock_Savana_001–006 | scattered | Resource, Rock | Crafting material |
| Resource_Stick_Savana_001–005 | scattered | Resource, Stick | Crafting material |
| Resource_Leaf_Savana_001–003 | scattered | Resource, Leaf | Crafting material |

---

## Performance Notes
- All quest trigger zones use point lights as visual markers (zero gameplay logic overhead)
- Quest state managed via actor tags — no custom C++ required in headless editor
- NPC actors are point light placeholders — Blueprint NPC pawns to be added when Character system is live
- Resource actors use 80-unit attenuation radius lights — visible to player, minimal GPU cost
