# Quest & Mission Design - Core Missions

## Quest System Architecture

### Mission Types
1. **Hunt Missions**: Track and eliminate dangerous predators
2. **Escort Missions**: Guide NPCs to safety using evacuation routes
3. **Gather Missions**: Collect resources at safe gathering points
4. **Exploration Missions**: Discover new territories and map danger zones
5. **Defense Missions**: Protect settlements from predator attacks

### Quest Markers Created (15 total)
- **Hunt Markers (5)**: Positioned above dangerous predators (T-Rex, Velociraptors)
  - QuestMarker_Hunt_001 to QuestMarker_Hunt_005
  - Located 500 units above target dinosaurs for visibility
  
- **Escort Markers (3)**: Positioned at evacuation route endpoints
  - QuestMarker_Escort_001 to QuestMarker_Escort_003
  - Integrated with Crowd Simulation evacuation routes
  - 200 units offset from route markers for clarity
  
- **Gather Markers (4)**: Positioned at safe gathering points
  - QuestMarker_Gather_001 to QuestMarker_Gather_004
  - Located at crowd gathering points created by Agent #13
  - 300 units offset to avoid overlap

### Quest Trigger Volumes (3 total)
- **QuestTrigger_HuntZone_001**: 2000x2000x500 volume at (5000,3000,100)
  - Activates hunt missions when player enters
  
- **QuestTrigger_EscortZone_001**: 1500x1500x500 volume at (-3000,4000,100)
  - Activates escort missions near evacuation routes
  
- **QuestTrigger_GatherZone_001**: 1800x1800x500 volume at (2000,-5000,100)
  - Activates resource gathering missions

## Core Mission Designs

### Mission 1: Hunt the Alpha Predator
**Type**: Hunt  
**Objective**: Eliminate the Alpha T-Rex in the northern territories  
**Location**: QuestMarker_Hunt_001  
**Trigger**: QuestTrigger_HuntZone_001  
**Rewards**: Advanced crafting materials, territory control  
**Emotional Arc**: Fear → Preparation → Confrontation → Triumph  

**Narrative Hook**: The Alpha T-Rex has killed three hunters. The tribe needs a hero.

**Gameplay Flow**:
1. Player enters hunt zone trigger
2. Quest giver voice plays: "Hunt the Alpha Predator..."
3. Quest marker appears above T-Rex
4. Player tracks and engages predator
5. Success: Quest complete fanfare, rewards granted
6. Failure: Respawn at nearest safe point, quest remains active

### Mission 2: Escort to Safety
**Type**: Escort  
**Objective**: Guide 10 NPCs from combat zone to evacuation point  
**Location**: QuestMarker_Escort_001  
**Trigger**: QuestTrigger_EscortZone_001  
**Rewards**: Tribe reputation, access to new NPCs  
**Emotional Arc**: Urgency → Responsibility → Tension → Relief  

**Narrative Hook**: Survivors are trapped. Every second counts.

**Gameplay Flow**:
1. Player enters escort zone trigger
2. Quest giver voice plays: "Urgent mission: Escort survivors..."
3. 10 NPCs spawn at crowd spawn points
4. Player must guide NPCs along evacuation routes
5. NPCs use crowd avoidance volumes to avoid combat zones
6. Success: All NPCs reach safety, quest complete
7. Failure: If >5 NPCs die, mission fails

### Mission 3: Gather Resources
**Type**: Gather  
**Objective**: Collect 20 resources at safe gathering points  
**Location**: QuestMarker_Gather_001 to QuestMarker_Gather_004  
**Trigger**: QuestTrigger_GatherZone_001  
**Rewards**: Crafting materials, survival supplies  
**Emotional Arc**: Exploration → Discovery → Satisfaction  

**Narrative Hook**: The tribe needs supplies. The gathering points are safe, but not for long.

**Gameplay Flow**:
1. Player enters gather zone trigger
2. Quest markers appear at 4 gathering points
3. Player collects resources at each location
4. Crowd NPCs at gathering points provide ambient life
5. Success: 20 resources collected, quest complete

## Quest Integration with Other Systems

### Integration with Crowd Simulation (Agent #13)
- Escort missions use evacuation routes created by Agent #13
- NPCs follow crowd avoidance volumes to avoid combat zones
- Gathering points double as quest objectives and crowd density zones
- Quest success metrics tied to crowd survival rates

### Integration with Combat AI (Agent #12)
- Hunt missions target dinosaurs with combat AI behavior
- Quest triggers detect combat state and adjust difficulty
- Predator patrols affect escort mission routes

### Integration with NPC Behavior (Agent #11)
- Quest givers use NPC behavior trees for dialogue
- Escort NPCs use NPC memory system to remember safe routes
- Quest completion affects NPC reputation and relationships

### Integration with Narrative (Agent #15)
- Quest dialogue written by Narrative Agent
- Story progression unlocks new quest chains
- Quest outcomes affect world state and lore

## Audio Assets Generated
- **QuestGiver_Hunt.mp3**: Hunt mission announcement
- **QuestGiver_Escort.mp3**: Escort mission announcement

## Next Steps for Agent #15 (Narrative & Dialogue)
- Write detailed quest dialogue for all 3 core missions
- Create quest giver character profiles (tribal elder, scout, shaman)
- Design quest chain progression (how missions unlock)
- Write failure/success dialogue variants
- Create lore entries for quest locations and objectives
