# NPC Behavior System — Cycle 008 Documentation
**Agent #11 — NPC Behavior Agent**
**Cycle:** PROD_CYCLE_AUTO_20260617_008

---

## Behavior Zones Created This Cycle

### Raptor Pack AI — Pack Hunting Behavior
Raptors operate as a coordinated pack. The behavior system uses 5 actors placed in MinPlayableMap:

| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| Zone_RaptorHunt_001 | TriggerBox (15x15x3) | (-500, 800, 100) | Central hunt activation zone |
| Waypoint_Raptor_FlankA_001 | TargetPoint | (-800, 600, 100) | Left flank attack position |
| Waypoint_Raptor_FlankB_001 | TargetPoint | (-200, 600, 100) | Right flank attack position |
| Waypoint_Raptor_Ambush_001 | TargetPoint | (-500, 1400, 100) | Ambush position ahead of prey |
| Zone_Raptor_Retreat_001 | TriggerBox (8x8x3) | (-1200, 400, 100) | Retreat zone when injured |

**Pack Hunting Logic:**
1. When player enters Zone_RaptorHunt_001 → pack activates
2. Raptor_A moves to FlankA, Raptor_B moves to FlankB
3. Raptor_C (alpha) moves to Ambush position
4. If any raptor health < 30% → retreat to Zone_Raptor_Retreat_001
5. Pack disengages if all raptors in retreat zone

---

### Brachiosaurus Herd — Peaceful Grazing Behavior
Brachio herd grazes in open savanna, detects predators at long range, flees as group:

| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| Zone_Brachio_Graze_001 | TriggerBox (25x25x5) | (3000, -500, 100) | Primary grazing area |
| Waypoint_Brachio_001 | TargetPoint | (2500, -800, 100) | Grazing path north |
| Waypoint_Brachio_002 | TargetPoint | (3500, -300, 100) | Grazing path east |
| Waypoint_Brachio_003 | TargetPoint | (3200, -700, 100) | Grazing path south |
| Zone_Brachio_Alarm_001 | TriggerBox (40x40x8) | (3000, -500, 100) | Predator detection radius |
| Waypoint_Brachio_Flee_001 | TargetPoint | (5000, -1000, 100) | Escape direction when alarmed |

**Herd Grazing Logic:**
1. Herd cycles through Waypoints 001→002→003→001 at slow walk speed
2. Any predator entering Zone_Brachio_Alarm_001 → all herd members alarm state
3. In alarm state → all members move toward Waypoint_Brachio_Flee_001
4. Herd returns to grazing after 120 seconds if no predator detected

---

### T-Rex Daily Routine — Territorial Behavior
T-Rex follows a daily cycle tied to time-of-day:

| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| Zone_TRex_Rest_001 | TriggerBox (12x12x4) | (1500, 2500, 100) | Midday rest shelter |
| Zone_TRex_Hunt_001 | TriggerBox (30x30x6) | (2800, 1200, 100) | Morning hunting ground |
| Waypoint_TRex_Evening_001 | TargetPoint | (2000, 1800, 100) | Evening patrol point A |
| Waypoint_TRex_Evening_002 | TargetPoint | (3200, 2000, 100) | Evening patrol point B |
| Zone_TRex_Territory_001 | TriggerBox (60x60x10) | (2000, 1800, 100) | Full territory boundary |

**Daily Routine Logic:**
- **Dawn (06:00-09:00):** T-Rex moves from rest zone to hunt zone, actively hunts
- **Midday (09:00-15:00):** T-Rex rests in Zone_TRex_Rest_001, reduced aggression
- **Evening (15:00-20:00):** T-Rex patrols Evening_001 → Evening_002 → back
- **Night (20:00-06:00):** T-Rex returns to rest zone, high aggression if disturbed
- **Territory Violation:** Any large predator in Zone_TRex_Territory_001 → immediate chase

---

## Voice Lines Generated

### Raptor Warning
- **File:** SurvivorNarrator_Raptor.mp3
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781707579341_SurvivorNarrator_Raptor.mp3
- **Text:** "Stay low. Do not move. The raptors hunt by movement — they cannot see you if you are still. Wait for them to pass. Wait."
- **Use:** Tutorial hint when player first encounters raptor pack

### Brachio Herd Warning
- **File:** SurvivorNarrator_Brachio.mp3
- **URL:** https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781707587222_SurvivorNarrator_Brachio.mp3
- **Text:** "The herd is moving. When the big ones run, the ground shakes. That is your warning. That is the only warning you will get."
- **Use:** Environmental audio hint when brachio herd enters alarm state

---

## Cumulative NPC Behavior Infrastructure (Cycles 005-008)

### All Behavior Zones in MinPlayableMap
**T-Rex Zones (Cycles 006-008):**
- Zone_TRex_Patrol_001 (patrol trigger)
- Zone_TRex_Chase_001 (chase activation)
- Zone_TRex_Attack_001 (attack range)
- Zone_TRex_Rest_001 (daily rest)
- Zone_TRex_Hunt_001 (morning hunt)
- Zone_TRex_Territory_001 (full territory)
- Waypoint_TRex_001 through 004 (patrol path)
- Waypoint_TRex_Evening_001, 002 (evening patrol)

**Raptor Zones (Cycles 007-008):**
- Zone_RaptorHunt_001 (pack hunt activation)
- Zone_Raptor_Retreat_001 (injured retreat)
- Waypoint_Raptor_FlankA_001, FlankB_001 (flanking)
- Waypoint_Raptor_Ambush_001 (ambush position)
- Waypoint_Floresta_011 through 014 (forest patrol)

**Brachiosaurus Zones (Cycle 008):**
- Zone_Brachio_Graze_001 (grazing area)
- Zone_Brachio_Alarm_001 (predator detection)
- Waypoint_Brachio_001, 002, 003 (grazing path)
- Waypoint_Brachio_Flee_001 (escape route)

---

## Handoff to Agent #12 — Combat & Enemy AI

The following behavior infrastructure is ready for Combat AI integration:

1. **T-Rex Territory System** — Zone_TRex_Territory_001 defines the combat engagement boundary. Combat AI should trigger when player enters this zone.

2. **Raptor Pack Coordination** — FlankA, FlankB, and Ambush waypoints define the tactical positions. Combat AI needs to assign individual raptors to these positions during combat.

3. **Brachio Flee System** — When brachio herd flees, it creates a stampede hazard. Combat AI should use the flee direction (toward Waypoint_Brachio_Flee_001) to create environmental danger.

4. **Daily Routine Timing** — T-Rex aggression varies by time of day (see daily routine table above). Combat AI should read the current game time and adjust T-Rex combat difficulty accordingly.

5. **Retreat Behavior** — Zone_Raptor_Retreat_001 is the disengage point. Combat AI should allow raptors to flee to this zone when health is critical, preventing unrealistic fight-to-death behavior.

---

## Next Cycle Recommendations

**Agent #12 (Combat AI) should:**
- Use Zone_TRex_Territory_001 as the combat engagement trigger
- Implement raptor flanking logic using the FlankA/FlankB waypoints
- Create damage zones around Waypoint_Brachio_Flee_001 (stampede damage)
- Implement T-Rex daily routine aggression multipliers
- Add combat state transitions: Idle → Alert → Chase → Attack → Retreat

**Technical Notes:**
- All zones use TriggerBox with scale multipliers (not absolute size)
- TargetPoints use Rotator to indicate facing direction at each waypoint
- All actors saved to /Game/Maps/MinPlayableMap
- MAP_SAVED confirmed in UE5 log
