# CYCLE 004 - AGENT TASK COORDINATION
## Studio Director Production Plan

### CRITICAL PATH TO PLAYABLE PROTOTYPE

**MILESTONE TARGET:** Walk Around Prototype - Player can move in a world with dinosaurs

### PRIORITY AGENT ASSIGNMENTS

#### **AGENT #2 - Engine Architect** 🔥 CRITICAL
- **Task:** UE5.5 compatibility fixes and compilation error resolution
- **Deliverable:** Core engine headers that compile without errors
- **Dependencies:** None - blocks all other agents
- **Status:** URGENT - must complete before others can proceed

#### **AGENT #3 - Core Systems Programmer** 🔥 CRITICAL  
- **Task:** Physics system implementation with collision detection
- **Deliverable:** Working physics components for character and terrain interaction
- **Dependencies:** Agent #2 completion
- **Status:** HIGH PRIORITY - needed for character movement

#### **AGENT #5 - Procedural World Generator** 🔥 CRITICAL
- **Task:** Terrain generation with height variation and proper collision
- **Deliverable:** Landscape with hills, valleys, and walkable surfaces
- **Dependencies:** Agent #3 physics system
- **Status:** HIGH PRIORITY - foundation for all gameplay

#### **AGENT #9 - Character Artist** 🔥 CRITICAL
- **Task:** Playable character controller with WASD movement and camera
- **Deliverable:** TranspersonalCharacter that responds to keyboard input
- **Dependencies:** Agent #3 physics, Agent #5 terrain
- **Status:** HIGH PRIORITY - core gameplay requirement

#### **AGENT #10 - Animation Agent** ⚡ HIGH
- **Task:** Basic character animations - idle, walk, run, jump
- **Deliverable:** Animation Blueprint with movement state machine
- **Dependencies:** Agent #9 character controller
- **Status:** MEDIUM PRIORITY - enhances prototype feel

#### **AGENT #11 - NPC Behavior Agent** ⚡ HIGH
- **Task:** Dinosaur NPC placement and basic behavior patterns
- **Deliverable:** 5+ dinosaur actors with simple AI behaviors in the world
- **Dependencies:** Agent #5 terrain for placement
- **Status:** HIGH PRIORITY - core game content

#### **AGENT #12 - Combat & Enemy AI** ⚡ HIGH
- **Task:** Survival stats system and basic combat foundation
- **Deliverable:** Health/hunger/stamina UI and basic interaction system
- **Dependencies:** Agent #9 character controller
- **Status:** MEDIUM PRIORITY - survival game core

### SUPPORT AGENTS

#### **AGENT #15 - Narrative Designer** 📝 MEDIUM
- **Task:** Game Design Document and world lore foundation
- **Deliverable:** Complete game bible and narrative framework
- **Dependencies:** None - can work in parallel
- **Status:** MEDIUM PRIORITY - guides other agents

#### **AGENT #18 - QA & Testing** 🧪 MEDIUM
- **Task:** Automated testing framework for prototype validation
- **Deliverable:** Test suite that validates core gameplay functions
- **Dependencies:** Agent #9 character controller
- **Status:** MEDIUM PRIORITY - ensures quality

#### **AGENT #19 - Integration & Build** 🔧 MEDIUM
- **Task:** Build pipeline management and compilation tracking
- **Deliverable:** Automated build system with error reporting
- **Dependencies:** Agent #2 compilation fixes
- **Status:** MEDIUM PRIORITY - supports all agents

### CURRENT PROTOTYPE STATUS

**Map State:** MinPlayableMap exists with basic actors
- **Characters:** 1 (TranspersonalCharacter - needs movement implementation)
- **Terrain:** 1 (basic landscape - needs height variation)  
- **Lighting:** 3+ (directional light, sky, fog)
- **Dinosaurs:** 5 (placeholder shapes - need proper models and AI)
- **Total Actors:** ~50+ (including props and environment)

**Compilation Status:** ✅ Project compiles successfully
**Playable Status:** ❌ Character does not respond to input yet

### SUCCESS CRITERIA FOR CYCLE 004-006

1. **Character Movement:** WASD controls move the character
2. **Camera Control:** Mouse look controls camera
3. **Terrain Interaction:** Character walks on varied terrain with proper collision
4. **Dinosaur Presence:** At least 3 dinosaurs visible and moving in the world
5. **Basic UI:** Health/stamina bars visible on screen

### AGENT EXECUTION ORDER

```
Phase 1 (Cycle 004): Agent #2 → Agent #3 → Agent #5
Phase 2 (Cycle 005): Agent #9 → Agent #11 → Agent #15  
Phase 3 (Cycle 006): Agent #10 → Agent #12 → Agent #18 → Agent #19
```

### BLOCKERS TO RESOLVE

1. **Compilation Errors:** Some .h files missing .cpp implementations
2. **Character Input:** TranspersonalCharacter needs input binding
3. **Terrain Collision:** Landscape needs proper collision mesh
4. **Dinosaur AI:** Static meshes need basic movement behaviors

---

**Studio Director Notes:**
- Focus on MINIMAL VIABLE PROTOTYPE first
- Every agent must produce WORKING code, not just headers
- Test in UE5 editor after every implementation
- Prioritize visible results over perfect architecture