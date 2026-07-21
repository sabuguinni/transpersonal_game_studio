# Crowd Simulation Audio Assets
## Generated Voice Narrations

### Evacuation_Alert.mp3
**Text**: "Alert! Danger zone ahead. Evacuate immediately to the nearest safe area. Follow the marked evacuation routes and stay with your group. Move quickly but do not panic. The settlement has designated safe zones to the north and east."

**Usage**: Triggered when player or NPC enters combat zone proximity. Plays at CrowdSpawn points to initiate evacuation behavior.

**Integration**: Attach to EvacRoute start markers as audio trigger volume.

---

### SafePatrol_Confirmation.mp3
**Text**: "Safe patrol route confirmed. This area is clear of predator activity. Maintain standard patrol formation and report any unusual movement. Stay within the marked boundaries and do not approach the combat zones to the south or west."

**Usage**: Plays when NPCs reach SafePatrol waypoints. Confirms safe territory status.

**Integration**: Attach to SafePatrol waypoint actors as ambient audio cue.

---

## Sound Effect Search Results

### Crowd Panic & Evacuation
- Query: "crowd panic evacuation footsteps running emergency"
- Use for: NPC evacuation movement audio, panic state transitions
- Attach to: EvacRoute pathfinding events

### Tribal Settlement Ambient
- Query: "tribal village ambient people talking prehistoric settlement"
- Use for: Background audio at CrowdSpawn points, settlement atmosphere
- Attach to: CrowdSpawn_Settlement_001 and village spawn points

---

## Implementation Notes
- All audio assets stored in `/Content/Audio/CrowdSimulation/`
- Voice narrations use ElevenLabs TTS for consistency
- Sound effects sourced from Freesound.org (CC0 licensed)
- Audio triggers use UE5 Audio Volume actors with distance attenuation
