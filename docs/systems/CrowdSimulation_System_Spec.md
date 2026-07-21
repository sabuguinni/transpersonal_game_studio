# Crowd & Traffic Simulation System Specification
## Transpersonal Game Studio — Prehistoric Survival Game

---

## SYSTEM OVERVIEW

The Crowd Simulation System creates living, breathing settlements populated by NPCs who follow realistic daily routines, react to environmental threats, and provide gameplay opportunities through social interaction, trading, and quest-giving.

---

## CORE COMPONENTS

### 1. Spawn System
**CrowdSpawn Points** — designated locations where NPCs appear in the world.

**Placement Rules:**
- Spawn points placed in safe zones (away from dinosaur territories)
- Minimum 200 units apart to prevent overcrowding
- Near essential resources (water, food, shelter)

**Current Implementation:**
- 4 spawn points in tribal settlement
- Located at settlement perimeter for natural NPC entry
- Simple labels: `CrowdSpawn_Settlement_NNN`

---

### 2. Gathering Areas
**Functional Zones** where NPCs congregate for specific activities.

**Area Types:**
1. **Water Source** — drinking, washing, socializing
2. **Food Storage** — meal preparation, resource distribution
3. **Crafting Station** — tool-making, equipment repair
4. **Fire Pit** — warmth, cooking, storytelling, social hub

**Behavior Rules:**
- NPCs spend 5-15 minutes at each area
- Area capacity: 3-5 NPCs (prevents overcrowding)
- Priority system: water > food > fire > crafting

**Current Implementation:**
- 4 gathering areas in settlement
- Labels: `GatherArea_Type_NNN` (e.g., `GatherArea_Water_001`)

---

### 3. Patrol Routes
**Waypoint Networks** defining NPC movement paths.

**Route Types:**
1. **Perimeter Patrol** — settlement security (4 waypoints, circular route)
2. **Resource Patrol** — gathering expeditions (3 waypoints, linear route)

**Patrol Behavior:**
- Guards walk perimeter route continuously (2-3 NPCs)
- Gatherers follow resource route during work hours
- Routes connect to gathering areas for natural flow

**Current Implementation:**
- 7 patrol waypoints total
- Labels: `PatrolWaypoint_Type_NNN`

---

## NPC BEHAVIOR SYSTEM

### Daily Routine Cycle
**Morning (6:00-10:00):**
- Spawn at CrowdSpawn points
- Gather at fire pit for morning ritual
- Disperse to work areas (crafting, food prep)

**Midday (10:00-16:00):**
- Active work at gathering areas
- Perimeter patrol active
- Resource gathering expeditions

**Evening (16:00-20:00):**
- Return to fire pit for communal meal
- Social interactions peak
- Crafting continues at reduced pace

**Night (20:00-6:00):**
- Most NPCs despawn or enter idle state
- 2-3 guards remain on perimeter patrol
- Fire pit remains active (ambient light/warmth)

---

### Threat Response
**Dinosaur Detected (Combat Zone Entry):**
1. **Alert Phase:** Guards shout warning, crowd stops current activity
2. **Evacuation Phase:** Non-combatants flee to settlement center
3. **Defense Phase:** Guards move to defensive positions
4. **Recovery Phase:** After threat cleared, NPCs resume routines

**Player Interaction:**
- NPCs greet player within 5m radius
- Quest givers marked with visual indicator
- Traders available at crafting/food areas

---

## MASS AI INTEGRATION (Future)

### Scalability Plan
**Current:** 20-30 active NPCs (traditional AI)  
**Target:** 500+ NPCs using UE5 Mass Entity system

**Mass AI Features:**
1. **Entity Component System** — lightweight NPC representation
2. **LOD Chain** — 3 tiers (high/medium/low detail)
3. **Processor Pipeline** — movement, animation, perception, decision-making
4. **Visualization** — debug tools for crowd flow analysis

**Performance Target:**
- 500 NPCs @ 60fps (PC)
- 200 NPCs @ 30fps (console)
- Seamless transition between traditional AI and Mass AI

---

## INTEGRATION WITH OTHER SYSTEMS

### Combat AI (Agent #12)
- Crowd NPCs flee from combat zones
- Guards engage threats if player is nearby
- Evacuation routes connect to safe zones

### NPC Behavior (Agent #11)
- Behavior Trees control individual NPC decisions
- Memory system tracks player reputation
- Social interactions (greet, trade, quest)

### Quest System (Agent #14)
- Gathering areas serve as quest hubs
- Patrol routes become escort mission paths
- Crowd events trigger dynamic quests

### Audio System (Agent #16)
- Ambient crowd chatter at gathering areas
- Alert shouts during threat response
- Footstep sounds scaled by crowd density

---

## PERFORMANCE OPTIMIZATION

### LOD System
**Tier 1 (0-20m):** Full skeletal mesh, facial animations, individual AI  
**Tier 2 (20-50m):** Simplified mesh, basic animations, group AI  
**Tier 3 (50-100m):** Instanced mesh, idle animation, no AI  
**Beyond 100m:** Despawn or placeholder representation

### Culling Strategy
- Frustum culling: NPCs outside camera view simplified
- Distance culling: NPCs beyond 100m despawned
- Occlusion culling: NPCs behind terrain/buildings despawned

### Batching
- Identical NPCs rendered as instanced meshes
- Shared animation states for crowd groups
- Material instancing for clothing variations

---

## LABEL CONVENTIONS

### Naming Rules
All crowd-related actors follow strict label format:

**Format:** `Type_Location_NNN`

**Examples:**
- `CrowdSpawn_Settlement_001`
- `GatherArea_Water_001`
- `PatrolWaypoint_Perimeter_001`

**Prohibited:**
- Degenerate labels (concatenated system names)
- Duplicate labels (checked before spawning)
- Spaces in labels (use underscores)

---

## FUTURE FEATURES

### Traffic Simulation
**Resource Caravans:**
- NPCs transport goods between settlements
- Follow predefined routes with waypoints
- Vulnerable to dinosaur attacks (escort quest opportunity)

**Migration Events:**
- Seasonal NPC movement (follow herds, avoid weather)
- Dynamic settlement population
- Player can influence migration (reputation system)

### Social Dynamics
**Faction System:**
- NPCs belong to tribes/clans
- Reputation affects crowd behavior
- Faction conflicts create dynamic events

**Relationship Network:**
- NPCs form friendships, rivalries
- Social hierarchy (elders, craftsmen, warriors)
- Player actions affect NPC relationships

### Dynamic Events
**Crowd Reactions:**
- Weather: NPCs seek shelter during storms
- Time of Day: Routines adapt to day/night cycle
- Player Reputation: Crowd cheers/jeers based on actions

---

## TECHNICAL SPECIFICATIONS

### Actor Types
- **CrowdSpawn:** TargetPoint (lightweight marker)
- **GatherArea:** TargetPoint with radius trigger
- **PatrolWaypoint:** TargetPoint with route index

### Navigation
- NavMesh-based pathfinding
- Dynamic obstacle avoidance
- Group movement coordination

### Animation
- Motion Matching for natural movement
- IK for foot placement on terrain
- Blend spaces for idle/walk/run transitions

---

## VALIDATION CHECKLIST

✅ **CAP Compliance:** Actor count within 8000 limit  
✅ **Label Compliance:** Simple format, zero duplicates  
✅ **Map Saved:** After each spawn batch  
✅ **Performance:** 60fps target maintained  
✅ **Integration:** Compatible with combat, quest, NPC systems  

---

**Status:** Foundation complete — ready for NPC behavior implementation and Mass AI integration.

**Next Steps:**
1. Agent #11: Implement Behavior Trees for individual NPCs
2. Agent #14: Create quests using crowd infrastructure
3. Agent #16: Add ambient crowd audio
4. Future: Integrate UE5 Mass AI for 500+ NPC scalability
