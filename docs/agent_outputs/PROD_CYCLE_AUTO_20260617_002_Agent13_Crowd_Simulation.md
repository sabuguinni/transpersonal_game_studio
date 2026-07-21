# Agent #13 — Crowd & Traffic Simulation
## Production Cycle: PROD_CYCLE_AUTO_20260617_002
## Date: 2025-06-17

---

## DELIVERABLES THIS CYCLE

### 1. UE5 Crowd Infrastructure Created
**Tribal Settlement Spawn Points (4 actors):**
- CrowdSpawn_Settlement_001 @ (2000, 3000, 150)
- CrowdSpawn_Settlement_002 @ (2200, 3200, 150)
- CrowdSpawn_Settlement_003 @ (1800, 2800, 150)
- CrowdSpawn_Settlement_004 @ (2400, 3400, 150)

**Gathering Area Markers (4 actors):**
- GatherArea_Water_001 @ (2100, 3100, 150) — water source for crowd gathering
- GatherArea_Food_001 @ (2300, 3000, 150) — food storage area
- GatherArea_Crafting_001 @ (1900, 3200, 150) — crafting station
- GatherArea_Fire_001 @ (2200, 2900, 150) — communal fire pit

**Patrol Route Waypoints (7 actors):**
- PatrolWaypoint_Perimeter_001-004 — settlement perimeter patrol route
- PatrolWaypoint_Resource_001-003 — resource gathering patrol route

**Total Crowd Infrastructure:** 15 actors placed in MinPlayableMap

---

## CROWD SIMULATION SYSTEM DESIGN

### Core Concept
The crowd simulation creates a living, breathing tribal settlement where NPCs:
1. **Spawn** at designated CrowdSpawn points
2. **Gather** at functional areas (water, food, crafting, fire)
3. **Patrol** along predefined routes for security and resource collection
4. **React** to player presence and environmental events (dinosaur attacks, weather)

### Behavior Patterns
**Daily Routine Cycle:**
- Morning (6-10h): Gather at fire, then disperse to resource areas
- Midday (10-16h): Active work at crafting/food areas, patrol perimeter
- Evening (16-20h): Return to fire, social gathering
- Night (20-6h): Reduced activity, guards on perimeter patrol

**Crowd Density:**
- Settlement center: 8-12 NPCs
- Gathering areas: 3-5 NPCs per area
- Patrol routes: 2-3 NPCs per route
- Total active NPCs: 20-30 (scalable to 50+ with Mass AI)

### Integration with Combat AI (Agent #12)
The crowd system integrates with combat zones created by Agent #12:
- **Threat Detection:** When dinosaurs enter combat zones, crowd NPCs flee to settlement center
- **Alert State:** Patrol NPCs move to defensive positions
- **Evacuation Routes:** Gathering areas have escape paths to safe zones

---

## TECHNICAL IMPLEMENTATION

### UE5 Systems Used
1. **TargetPoint Actors** — lightweight markers for spawn/gather/patrol locations
2. **Navigation System** — NavMesh-based pathfinding for crowd movement
3. **Mass AI Framework** — scalable crowd simulation (ready for future expansion)
4. **Behavior Trees** — individual NPC decision-making (to be implemented by Agent #11)

### Performance Optimization
- **LOD System:** Crowd NPCs use 3-tier LOD (high detail <20m, medium 20-50m, low >50m)
- **Culling:** NPCs beyond 100m from player are simplified or despawned
- **Batching:** Crowd rendering uses instanced meshes for identical NPCs
- **Target:** 30+ active NPCs at 60fps on PC, 30fps on console

---

## LABEL COMPLIANCE
All actors follow the **simple label format** mandated by global memory:
- Format: `Type_Location_NNN`
- Examples: `CrowdSpawn_Settlement_001`, `GatherArea_Water_001`, `PatrolWaypoint_Perimeter_001`
- **Zero degenerate labels** — no concatenated system names
- **Zero duplicates** — checked existing labels before spawning

---

## NEXT AGENT FOCUS (Agent #14 — Quest & Mission Designer)

### Inputs from Crowd Simulation
1. **Settlement Hub:** Use CrowdSpawn and GatherArea locations as quest hubs
2. **NPC Quest Givers:** Place quest NPCs at GatherArea_Fire_001 (elder), GatherArea_Crafting_001 (craftsman)
3. **Escort Missions:** Use PatrolWaypoint routes for escort quest paths
4. **Crowd Events:** Create quests triggered by crowd behavior (e.g., "Defend the settlement from raptor pack")

### Recommended Quest Types
1. **Gather Resources:** Send player to collect materials for crafting area
2. **Perimeter Defense:** Patrol route becomes quest objective (clear threats)
3. **Social Quests:** Interact with NPCs at gathering areas (trade, learn skills)
4. **Emergency Response:** Dinosaur attack on settlement triggers defense quest

### Integration Points
- **Quest Markers:** Place at GatherArea locations for easy player navigation
- **Objective Tracking:** Use PatrolWaypoint actors as quest checkpoints
- **Narrative Triggers:** Crowd density changes trigger story events (e.g., low crowd = plague quest)

---

## VALIDATION RESULTS

### CAP Enforcement
- **Actor Count:** Within 8000 limit (checked before spawning)
- **Dinosaur Count:** Within 150 limit (validated by CAP script)
- **Map Saved:** True (confirmed after each spawn batch)

### Crowd Infrastructure
- **Spawn Points:** 4 created
- **Gathering Areas:** 4 created
- **Patrol Waypoints:** 7 created
- **Total:** 15 actors (minimal footprint, maximum gameplay value)

---

## PRODUCTION NOTES

### Workflow Compliance
✅ Bridge validation: OK  
✅ CAP enforcement: OK (actor count validated before work)  
✅ Simple labels: 100% compliance (no degenerate labels)  
✅ Map saved: True (after each spawn batch)  
✅ Zero C++ files: Full compliance with Python-only workflow  

### Future Expansion
The current crowd infrastructure is a **foundation** for:
1. **Mass AI Integration:** Scale to 500+ NPCs using UE5 Mass Entity system
2. **Dynamic Events:** Crowd reacts to weather, time of day, player reputation
3. **Traffic Simulation:** Add resource caravans between settlements (future feature)
4. **Social Dynamics:** NPCs form relationships, factions, hierarchies

---

**Status:** COMPLETE — Crowd simulation infrastructure ready for NPC behavior implementation (Agent #11) and quest integration (Agent #14).
