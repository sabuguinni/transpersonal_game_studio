# STUDIO DIRECTOR COORDINATION - CYCLE 004

## EXECUTIVE SUMMARY
Studio Director coordinating PLAYABLE PROTOTYPE development. Current status: MinPlayableMap has basic terrain and character movement, but lacks expanded world and interactive elements.

## CRITICAL PRIORITIES FOR AGENTS

### IMMEDIATE ACTION REQUIRED (CYCLE 004)

#### Agent #5 - Procedural World Generator
**PRIORITY 1: TERRAIN EXPANSION**
- Current: Basic terrain with hills
- Required: Expand to 10km² with 5 distinct biomes
- Locations: Swamp (SW), Forest (NW), Savanna (center), Desert (E), Mountains (NE)
- Deliverable: Real terrain with height variation and biome transitions

#### Agent #9 - Character Artist + Agent #10 - Animation
**PRIORITY 2: DINOSAUR ACTORS**
- Current: 5 basic shape placeholders (TRex, 3 Raptors, Brachiosaurus)
- Required: Replace with proper actors having collision and basic AI
- Deliverable: Dinosaur actors that can be seen and interacted with

#### Agent #12 - UI/UX Designer
**PRIORITY 3: SURVIVAL HUD**
- Current: TranspersonalCharacter has survival stats (health/hunger/thirst/stamina/fear)
- Required: Visual HUD displaying these stats
- Deliverable: Working UI that shows player status

#### Agent #8 - Lighting & Atmosphere
**PRIORITY 4: CRETACEOUS ATMOSPHERE**
- Current: Basic sun/sky/fog
- Required: Proper Cretaceous period lighting and atmosphere
- Deliverable: Tropical daylight, blue sky, stable lighting

## AGENT COORDINATION FRAMEWORK

### DEPENDENCY CHAIN
1. **Agent #5** (Terrain) → **Agent #6** (Environment) → **Agent #8** (Lighting)
2. **Agent #9** (Characters) → **Agent #10** (Animation) → **Agent #12** (Combat AI)
3. **Agent #12** (UI) → **Agent #14** (Quest) → **Agent #15** (Narrative)

### SUCCESS CRITERIA
Each agent MUST produce:
- Minimum 1 working .cpp file with implementation
- Minimum 1 UE5 command that creates visible content
- Clear handoff to next agent in chain

### QUALITY GATES
- **Compilation**: All .h files must have matching .cpp files
- **Visibility**: Every system must create something visible in MinPlayableMap
- **Integration**: Systems must work together, not in isolation

## TECHNICAL REQUIREMENTS

### C++ IMPLEMENTATION STANDARDS
- Use "Dir_" prefix for all new types to avoid conflicts
- Every .h file requires matching .cpp with full implementation
- No abstract systems without concrete functionality
- Build on existing TranspersonalCharacter and TranspersonalGameMode

### UE5 Integration Standards
- Use existing MinPlayableMap as base
- Spawn actors at specific coordinates for organization
- Save work using unreal.EditorLoadingAndSavingUtils.save_map()
- Test all implementations with Python validation scripts

## RESOURCE ALLOCATION

### Asset Purchase Criteria (Hugo's Requirements)
1. **CRITERION 1**: Landscape expanded to 10km² with 5 biomes → Agent #5 responsible
2. **CRITERION 2**: Stable Cretaceous atmosphere for 3+ cycles → Agent #8 responsible  
3. **CRITERION 3**: FBX import pipeline tested with free Fab asset → Agents #6/#7/#9 responsible

Once criteria met: TurboSquid dinosaurs (~$1500) + RealBiomes assets (~$300) will be purchased.

### Time Budget Management
- Agents #1-#6: Full scope allowed
- Agents #7-#12: Reduced scope if cycle >600s
- Agents #13-#20: Emergency mode if cycle >900s

## DELIVERABLES TRACKING

### CYCLE 004 EXPECTED OUTPUTS
- Agent #5: Expanded terrain with biome system
- Agent #9: Dinosaur actor classes with collision
- Agent #12: Survival HUD implementation
- Agent #8: Cretaceous lighting setup

### NEXT CYCLE PRIORITIES
- Integration testing of all new systems
- Player interaction with dinosaurs
- Biome-specific environmental effects
- Combat system foundation

## RISK MITIGATION

### UE5 Bridge Stability
- Maximum 1 ue5_execute per agent to prevent crashes
- No bulk spawning operations (>20 actors at once)
- Chunked operations with time.sleep(2) between batches

### Compilation Safety
- Verify all includes before writing .cpp files
- Use forward declarations for cross-module dependencies
- Test compilation with minimal Python scripts

---

**Studio Director Authorization**: Miguel Martins  
**Cycle**: PROD_CYCLE_AUTO_20260513_004  
**Status**: ACTIVE - Agent coordination in progress