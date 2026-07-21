# Quest System Design Document
## Transpersonal Game Studio - Prehistoric Survival Game

### Overview
The quest system provides structured objectives that guide player progression through survival challenges, exploration, and mastery of the prehistoric environment.

---

## Core Quest Categories

### 1. HUNT QUESTS
**Purpose:** Eliminate threats to settlement survival

**Mechanics:**
- Track specific dangerous dinosaurs
- Use environmental knowledge to set ambushes
- Coordinate with NPC hunters for pack takedowns
- Harvest resources from successful hunts

**Example Quests:**
- "Alpha Predator" - Hunt the T-Rex terrorizing the valley
- "Pack Elimination" - Clear raptor pack from water source
- "Night Defense" - Survive and eliminate nocturnal predators

**Rewards:**
- Rare crafting materials (bones, hide, teeth)
- Reputation with hunter NPCs
- Unlock advanced weapon crafting recipes

---

### 2. GATHER QUESTS
**Purpose:** Secure resources for crafting and survival

**Mechanics:**
- Navigate to resource-rich but dangerous areas
- Manage inventory weight vs. risk
- Avoid or confront territorial dinosaurs
- Return materials to settlement

**Example Quests:**
- "Flint Expedition" - Gather flint from raptor-controlled cave
- "Medicinal Herbs" - Collect rare plants from swamp biome
- "Hardwood Collection" - Harvest strong branches for construction

**Rewards:**
- Crafting materials
- Unlock new crafting recipes
- Settlement upgrade materials

---

### 3. EXPLORATION QUESTS
**Purpose:** Discover strategic locations and expand territorial knowledge

**Mechanics:**
- Reach waypoints in unexplored territories
- Survive environmental hazards
- Document dinosaur behaviors and migration patterns
- Establish fast travel points

**Example Quests:**
- "Cave Mapping" - Explore and map cave system
- "Waterfall Discovery" - Find fresh water source
- "Nesting Grounds" - Locate and study dinosaur breeding areas

**Rewards:**
- Map reveals (fog of war removal)
- Fast travel point unlocks
- Environmental knowledge (weather patterns, migration routes)

---

### 4. DEFENSE QUESTS
**Purpose:** Protect settlement and NPCs from threats

**Mechanics:**
- Fortify settlement structures
- Position defensive traps
- Coordinate with NPC defenders
- Survive waves of predator attacks

**Example Quests:**
- "Fortify the Perimeter" - Build defensive structures
- "Night Watch" - Defend camp through the night
- "Rescue Mission" - Save trapped NPCs from predators

**Rewards:**
- Settlement upgrades
- NPC loyalty and assistance
- Defensive equipment blueprints

---

## Quest Structure

### Quest Phases
1. **Introduction** - Quest giver presents the challenge
2. **Preparation** - Player gathers equipment, scouts location
3. **Execution** - Player completes objective
4. **Resolution** - Return to quest giver, receive rewards

### Quest Giver Archetypes
- **Hunter Elder** - Combat and hunting quests
- **Crafting Leader** - Resource gathering and crafting quests
- **Scout Captain** - Exploration and mapping quests
- **Settlement Chief** - Defense and community quests

---

## Quest Markers in MinPlayableMap

### Hunt Objectives (3 markers)
- `QuestMarker_HuntTRex_001` @ (5000, 3000, 150)
- `QuestMarker_HuntRaptorPack_001` @ (-3000, 4000, 120)
- `QuestMarker_DefendCamp_001` @ (0, 0, 100)

### Gather Points (5 markers)
- `GatherPoint_FlintRocks_001` @ (2000, -1500, 80)
- `GatherPoint_Branches_001` @ (-1500, 2000, 90)
- `GatherPoint_Vines_001` @ (3000, 1000, 75)
- `GatherPoint_Berries_001` @ (-2000, -1000, 85)
- `GatherPoint_Herbs_001` @ (1000, 3000, 95)

### Exploration Waypoints (5 markers)
- `ExplorePoint_Cave_001` @ (-4000, -3000, 200)
- `ExplorePoint_Waterfall_001` @ (4500, -2000, 150)
- `ExplorePoint_CliffOverlook_001` @ (-3500, 3500, 300)
- `ExplorePoint_DinoNest_001` @ (3000, -3500, 120)
- `ExplorePoint_AncientRuins_001` @ (-2500, 4000, 110)

---

## Quest Progression Design

### Early Game (Hours 0-5)
**Focus:** Basic survival and settlement establishment
- Gather basic materials (wood, stone, vines)
- Hunt small herbivores for food
- Explore immediate surroundings

### Mid Game (Hours 5-20)
**Focus:** Territory expansion and advanced crafting
- Hunt medium-sized predators
- Gather rare materials from dangerous areas
- Establish outposts in distant biomes

### Late Game (Hours 20+)
**Focus:** Mastery and apex challenges
- Hunt apex predators (T-Rex, Spinosaurus)
- Explore extreme environments (volcanic, deep caves)
- Defend against massive predator migrations

---

## Quest Failure Conditions

### Time Limits
- Some quests have day/night cycle constraints
- Migration-based quests expire when herds move

### NPC Death
- Escort quests fail if NPC dies
- Defense quests fail if settlement is overrun

### Resource Depletion
- Gather quests fail if resources are consumed by wildlife
- Hunt quests fail if target dinosaur migrates away

---

## Reward Scaling

### Experience Tiers
- **Basic Quests:** Common materials, small reputation gains
- **Advanced Quests:** Rare materials, significant reputation, recipe unlocks
- **Epic Quests:** Unique items, major settlement upgrades, story progression

### Dynamic Rewards
- Rewards scale based on player level and difficulty
- Bonus rewards for completing quests without deaths
- Efficiency bonuses for speed completions

---

## Integration Requirements

### Systems Needed
1. **Quest Manager** - Tracks active/completed quests
2. **Dialogue System** - Quest giver interactions
3. **Objective Tracker** - HUD display of current objectives
4. **Reward System** - Item/reputation/unlock distribution

### Dependencies
- NPC Behavior System (quest givers, companions)
- Inventory System (material tracking)
- Combat System (hunt quest validation)
- Exploration System (waypoint discovery)

---

## Anti-Patterns (FORBIDDEN)

### Spiritual/Mystical Content
- NO meditation quests
- NO consciousness expansion objectives
- NO spiritual awakening narratives
- NO mystical communication with dinosaurs

### Unrealistic Mechanics
- NO telepathy or supernatural powers
- NO magic-based solutions
- NO divine intervention or prophecies

---

**Document Status:** APPROVED
**Implementation Phase:** Markers placed, awaiting dialogue and logic systems
**Next Steps:** Narrative Agent to create dialogue trees, future cycles to implement quest logic
