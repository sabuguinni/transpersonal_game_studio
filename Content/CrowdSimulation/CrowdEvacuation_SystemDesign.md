# Crowd Evacuation & Traffic Simulation System Design
## Agent #13 - Crowd & Traffic Simulation

### Overview
Crowd simulation system that creates dynamic NPC movement patterns, evacuation behaviors when combat encounters trigger, and safe patrol routes that avoid dangerous territories.

### Evacuation Routes (3 routes created)
1. **EvacRoute_TRexZone_001**: Leads NPCs away from T-Rex territory (5000,0,100) → (8000,0,100)
2. **EvacRoute_RaptorZone_001**: Leads NPCs away from Raptor pack zone (-5000,5000,100) → (-8000,8000,100)
3. **EvacRoute_AnkyloZone_001**: Leads NPCs away from Ankylosaurus territory (0,-5000,100) → (0,-8000,100)

Each route has Start and End markers (TargetPoint actors) for pathfinding.

### Safe Patrol Routes (4 routes created)
1. **SafePatrol_North_001**: Northern neutral territory with 3 waypoints
2. **SafePatrol_East_001**: Eastern perimeter with 3 waypoints
3. **SafePatrol_West_001**: Western perimeter with 3 waypoints
4. **SafePatrol_Settlement_001**: Central settlement patrol with 3 waypoints

Total waypoints created: 12 TargetPoint actors for navigation.

### Crowd Spawn Points (5 locations created)
1. **CrowdSpawn_Settlement_001** (1200,1200,100): Main settlement, capacity 15 NPCs
2. **CrowdSpawn_NorthCamp_001** (3000,9000,100): Northern camp, capacity 10 NPCs
3. **CrowdSpawn_EastOutpost_001** (9000,3000,100): Eastern outpost, capacity 8 NPCs
4. **CrowdSpawn_WestVillage_001** (-9000,3000,100): Western village, capacity 12 NPCs
5. **CrowdSpawn_SafeZone_001** (0,10000,100): Far north safe zone, capacity 20 NPCs

Total NPC capacity: 65 simultaneous crowd agents.

### Behavior Logic
- **Evacuation Trigger**: When player enters combat zone or dinosaur aggro is triggered
- **Pathfinding**: NPCs follow TargetPoint markers from nearest spawn to evacuation end point
- **Avoidance**: All patrol routes maintain minimum 3000 unit distance from combat zones
- **Safe Zones**: Northern and eastern territories designated as predator-free areas

### Integration with Combat AI (Agent #12)
- Evacuation routes start at edges of combat territories
- NPCs detect combat zone proximity and auto-route to safe patrols
- Safe patrol waypoints positioned to avoid T-Rex, Raptor, and Ankylosaurus zones

### Technical Implementation
- All actors use simple label format: Type_Location_NNN
- Zero degenerate labels (no concatenation)
- Map saved after each spawn batch
- Total crowd elements: ~30 actors (6 evac markers + 12 patrol waypoints + 5 spawn points)

### Audio Assets
- **Evacuation_Alert.mp3**: Emergency evacuation announcement
- **SafePatrol_Confirmation.mp3**: Patrol route status update

### Next Steps (Agent #14 - Quest & Mission Designer)
- Create quest objectives that require escorting NPCs along evacuation routes
- Design missions where player must clear patrol routes of threats
- Implement crowd density as quest success metric (e.g., "Evacuate 10 NPCs safely")
