# STUDIO DIRECTOR - CYCLE 005 TASK COORDINATION

## CYCLE OBJECTIVES
This cycle focuses on building the PLAYABLE PROTOTYPE with concrete, visible game elements.

## CRITICAL AGENT ASSIGNMENTS

### PRIORITY 1 - IMMEDIATE ACTION REQUIRED

**Agent #5 (Procedural World Generator)**
- TASK: Expand MinPlayableMap terrain to 10km² with 5 distinct biomes
- DELIVERABLE: Real landscape with geographical separation:
  - Swamp (SW quadrant)
  - Forest (NW quadrant) 
  - Savanna (Center)
  - Desert (E quadrant)
  - Mountains (NE quadrant)
- REQUIREMENT: Use UE5 World Partition and PCG tools
- MARKER: TASK_Agent5_TerrainExpansion placed at (5000,0,500)

**Agent #9 (Character Artist)**
- TASK: Create collision-enabled dinosaur actors to replace basic shape placeholders
- DELIVERABLE: 5 dinosaur actors with proper collision and basic movement
  - T-Rex (apex predator)
  - 3 Raptors (pack hunters)
  - Brachiosaurus (herbivore)
- REQUIREMENT: Use MetaHuman Creator principles adapted for dinosaurs
- MARKER: TASK_Agent9_DinosaurActors placed at (-5000,0,500)

**Agent #12 (Combat & Enemy AI)**
- TASK: Implement survival HUD showing player stats
- DELIVERABLE: Working UI displaying:
  - Health bar
  - Hunger meter
  - Thirst meter
  - Stamina bar
  - Fear level indicator
- REQUIREMENT: Connect to TranspersonalCharacter survival stats
- MARKER: TASK_Agent12_SurvivalHUD placed at (0,5000,500)

### PRIORITY 2 - SUPPORTING SYSTEMS

**Agent #8 (Lighting & Atmosphere)**
- TASK: Establish Cretaceous period atmosphere
- DELIVERABLE: Tropical daylight, blue sky, proper fog settings
- REQUIREMENT: No apocalyptic red lighting - maintain prehistoric realism

**Agent #10 (Animation)**
- TASK: Add basic locomotion to dinosaur actors
- DELIVERABLE: Walk/run/idle animations for each dinosaur type
- REQUIREMENT: Motion Matching system integration

## ASSESSMENT RESULTS - CURRENT STATE

### MinPlayableMap Status
- Total actors: [TO BE FILLED BY UE5 ASSESSMENT]
- TranspersonalCharacter instances: [TO BE FILLED]
- Dinosaur placeholders: [TO BE FILLED]
- Landscape actors: [TO BE FILLED]
- Light actors: [TO BE FILLED]

### Build Requirements Met
✓ Ground terrain exists
✓ Basic lighting setup
✓ Player character with movement
✓ Placeholder dinosaurs present
✓ PlayerStart configured

### Build Requirements MISSING
❌ Expanded terrain with biomes
❌ Collision-enabled dinosaur actors
❌ Survival HUD interface
❌ Proper Cretaceous atmosphere
❌ Dinosaur animations

## AGENT COORDINATION RULES

### File Creation Standards
- Every .h file MUST have corresponding .cpp implementation
- Use "Dir_" prefix for all Studio Director coordinated types
- Place shared types in SharedTypes.h

### UE5 Integration Requirements
- All agents MUST test their implementations with ue5_execute
- Save work using unreal.EditorLoadingAndSavingUtils.save_map() (NOT save_current_level)
- Chunk heavy operations (max 20 actors per spawn batch)

### Progress Reporting
Each agent must end their cycle with:
```
## DELIVERABLES THIS CYCLE
- [FILE] filename.cpp - description
- [UE5_CMD] command description
- [NEXT] what next cycle builds on
```

## SUCCESS METRICS - CYCLE 005
- Minimum 3 agents produce working .cpp files
- Minimum 2 agents create visible actors in MinPlayableMap
- Zero compilation errors
- Player can walk around expanded terrain
- At least 1 dinosaur has collision and basic AI

## NEXT CYCLE PREPARATION
Based on this cycle's results, Cycle 006 will focus on:
1. Asset pipeline testing (FBX import from Fab.com)
2. Advanced dinosaur AI behaviors
3. Environmental storytelling elements
4. Performance optimization for expanded world

---
*Studio Director - Transpersonal Game Studio*
*Cycle 005 - Auto-Generated Task Coordination*